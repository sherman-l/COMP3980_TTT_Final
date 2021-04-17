package com.example.comp3980finalproject;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.example.comp3980finalproject.DataPackaging.RequestData;
import com.example.comp3980finalproject.DataPackaging.ResponseData;

import java.io.IOException;
import java.lang.reflect.Array;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

public class RPSActivity extends AppCompatActivity {

    private Connection conn;
    private RequestData req;
    private AudioCall audio;

    private final byte STATUS_SUCCESS = 10;
    private final byte STATUS_UPDATE = 20;
    private final byte STATUS_ERR_TYP = 32;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_r_p_s);

        conn = (Connection) getIntent().getExtras().get(getResources().getString(R.string.connKey));
        String ip = (String) getIntent().getExtras().get(getResources().getString(R.string.ipKey));
        int port = (int) getIntent().getExtras().getInt(getResources().getString(R.string.portKey));

        try {
            audio = new AudioCall(InetAddress.getByName(ip), port);
        } catch (UnknownHostException e) {
            Log.d("state", "unable to connect to ip");
        }
        TextView msg = findViewById(R.id.title);
        msg.setText("Successfully connected to " + ip);

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
                != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO},
                    123);
        }

    }

    public void disconnect(View view) {
        finish();
    }

    public void submit(View view) {
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                switch (conn.getState()) {
                    case INIT:
                        Spinner gameSelect = (Spinner) findViewById(R.id.spinnerGames);
                        String game = gameSelect.getSelectedItem().toString();

                        byte gameId = game.equals("RPS") ? (byte) 2 : (byte) 1;
                        byte protocolVers = conn.getVersion();
                        byte[] payload = {protocolVers, gameId};

                        req = new RequestData(payload);
                        //req.printObj(conn.getState());
                        req.writeRequest();

                        ResponseData resInit = new ResponseData();
                        resInit.readResponse();
                        //resInit.printObj(conn.getState());

                        parseResponse(resInit, conn.getState());
                        break;
                    case MOVE:
                        Spinner moveSelect = (Spinner) findViewById(R.id.move_select);
                        String move = moveSelect.getSelectedItem().toString();

                        byte moveId = 0;
                        if (move.equals("Rock")) {
                            moveId = 1;
                        } else if (move.equals("Paper")) {
                            moveId = 2;
                        } else if (move.equals("Scissors")) {
                            moveId = 3;
                        }

                        req.set_reqType((byte) 4);
                        req.set_reqContext((byte) 1);
                        req.set_payloadLen((byte) 1);
                        req.set_payload(moveId);
                        //req.printObj(conn.getState());

                        req.writeRequest();

                        ResponseData resMove = new ResponseData();
                        resMove.readResponse();
                        //resMove.printObj(conn.getState());

                        parseResponse(resMove, conn.getState());

                        break;
                    default:
                        break;
                }
            }
        });
        thread.start();
    }

    private void parseResponse(final ResponseData res, final Connection.State s) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                TextView tv = findViewById(R.id.game_msg);
                switch (s) {
                    case INIT:
                        if (res.get_status() == STATUS_SUCCESS) {
                            req.set_uid(res.get_payload());
                            tv.setText("Waiting for opponent");
                            conn.setState(Connection.State.WAIT);
                            Thread thread = new Thread(new Runnable() {
                                @Override
                                public void run() {
                                    ResponseData resWait = new ResponseData();
                                    resWait.readResponse();
                                    //resWait.printObj(conn.getState());

                                    parseResponse(resWait, conn.getState());
                                }
                            });
                            thread.start();

                        } else if (res.get_status() == STATUS_ERR_TYP) {
                            tv.setText("Invalid game id, please try again");
                            conn.setState(Connection.State.INIT);
                        }
                        break;
                    case WAIT:
                        if (res.get_status() == STATUS_UPDATE) {
                            //req.printObj(conn.getState());
                            tv.setText("Both players connected, make your selection by selecting your choice from the drop down then clicking Submit");
                            addRPS();
                            conn.setState(Connection.State.MOVE);
                        }
                        break;
                    case MOVE:
                        if (res.get_status() == STATUS_SUCCESS) {
                            tv.setText("Waiting for opponent's move");
                            conn.setState(Connection.State.RESULT);
                            Thread thread = new Thread(new Runnable() {
                                @Override
                                public void run() {
                                    ResponseData resMove = new ResponseData();
                                    resMove.readResponse();
                                    //resMove.printObj(conn.getState());

                                    parseResponse(resMove, conn.getState());
                                }
                            });
                            thread.start();
                        }
                        break;
                    case RESULT:
                        if (res.get_status() == STATUS_UPDATE) {
                            String result = "";
                            switch (res.get_payload()[0]) {
                                case 1: result = "WIN"; break;
                                case 2: result = "LOSS"; break;
                                case 3: result = "TIE"; break;
                                default: break;
                            }
                            String move = "";
                            switch (res.get_payload()[1]) {
                                case 1: move = "ROCK"; break;
                                case 2: move = "PAPER"; break;
                                case 3: move = "SCISSORS"; break;
                            }
                            tv.setText("Result: " + result + "\nOpponent's move = " + move + "\nClick Disconnect to restart");
                            conn.setState(Connection.State.END);
                        }
                    default:
                        break;
                }
            }
        });
    }

    private void addRPS() {
        Spinner s = new Spinner(this);
        s.setId(R.id.move_select);
        List<String> list = Arrays.asList(getResources().getStringArray(R.array.rps_options));
        ArrayAdapter<String> spinAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, list);
        spinAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s.setAdapter(spinAdapter);

        LinearLayout layout = findViewById(R.id.game_layout);
        layout.addView(s);
    }

    public void startAudio(View view) {
        if (audio != null) {
            audio.startCall();
        }
    }

    public void endAudio(View view) {
        if (audio != null) {
            audio.endCall();
        }
    }

    public class AudioCall {
        private static final String LOG_TAG = "AudioCall";
        private static final int SAMPLE_RATE = 10000;
        private static final int SAMPLE_INTERVAL = 2;
        private static final int SAMPLE_SIZE = 5000;
        private static final int BUF_SIZE = SAMPLE_INTERVAL * SAMPLE_INTERVAL * SAMPLE_SIZE * 2;
        private InetAddress address;
        private int port;
        private boolean mic = false;
        private boolean speakers = false;
        private DatagramSocket socket;

        public AudioCall(InetAddress address, int port) {
            this.address = address;
            this.port = port;
            try {
                this.socket = new DatagramSocket();
            } catch (Exception e) {
                Log.d(LOG_TAG, "exception occured");
            }
        }

        public void startCall() {
            startMic();
            startSpeakers();
        }

        public void endCall() {
            Log.i(LOG_TAG, "Ending call");
            muteMic();
            muteSpeakers();
        }

        public void muteMic() {mic = false;}
        public void muteSpeakers() {speakers = false;}
        public void startMic() {
            mic = true;
            Thread thread = new Thread(new Runnable() {
                @Override
                public void run() {

                    Log.i(LOG_TAG, "Send thread started. Thread id: " + Thread.currentThread().getId());
                    AudioRecord audioRecorder = new AudioRecord (MediaRecorder.AudioSource.VOICE_COMMUNICATION, SAMPLE_RATE,
                            AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT,
                            AudioRecord.getMinBufferSize(SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT)*10);
                    int bytes_read = 0;
                    int bytes_sent = 0;
                    byte[] buf = new byte[BUF_SIZE];
                    try {
                        // Create a socket and start recording
                        Log.i(LOG_TAG, "Packet destination: " + address.toString());
                        //DatagramSocket socket = new DatagramSocket();
                        audioRecorder.startRecording();
                        int count = 0;
                        while(mic) {
                            // Capture audio from the mic and transmit it
                            bytes_read = audioRecorder.read(buf, 0, BUF_SIZE);
                            DatagramPacket packet = new DatagramPacket(buf, bytes_read, address, port);
                            byte[] uid = req.get_uid();
                            byte[] order = ByteBuffer.allocate(4).order(ByteOrder.BIG_ENDIAN).putInt(count).array();
                            byte[] totalSend = new byte[5008];
                            for (int i = 0; i < 4; i++) {
                                totalSend[i] = order[i];
                            }
                            for (int i = 0; i < 4; i++) {
                                totalSend[i + 4] = uid[i];
                            }
                            for (int i = 0; i < 5000; i++) {
                                totalSend[i + 8] = buf[i];
                            }

                            /**
                            String toSend = "Sending Packet: ";
                            for (int i = 0; i < 10; i++) {
                                toSend += i + " = " + (byte) buf[i];
                            }

                            Log.i(LOG_TAG, toSend);
                             */
                            DatagramPacket totalPacket = new DatagramPacket(totalSend, 5008, address, port);
                            socket.send(totalPacket);
                            //socket.send(headerPacket);
                            //socket.send(packet);
                            count++;
                            bytes_sent += bytes_read;
                            Log.i(LOG_TAG, "Total bytes sent: " + bytes_sent);
                            Thread.sleep(SAMPLE_INTERVAL, 0);
                        }
                        // Stop recording and release resources
                        audioRecorder.stop();
                        audioRecorder.release();
                        socket.disconnect();
                        socket.close();
                        mic = false;
                        return;
                    }
                    catch(InterruptedException e) {

                        Log.e(LOG_TAG, "InterruptedException: " + e.toString());
                        mic = false;
                    }
                    catch(SocketException e) {

                        Log.e(LOG_TAG, "SocketException: " + e.toString());
                        mic = false;
                    }
                    catch(UnknownHostException e) {

                        Log.e(LOG_TAG, "UnknownHostException: " + e.toString());
                        mic = false;
                    }
                    catch(IOException e) {

                        Log.e(LOG_TAG, "IOException: " + e.toString());
                        mic = false;
                    }
                }
            });
            thread.start();
        }
        public void startSpeakers() {
            // Creates the thread for receiving and playing back audio
            if(!speakers) {

                speakers = true;
                Thread receiveThread = new Thread(new Runnable() {

                    @Override
                    public void run() {
                        // Create an instance of AudioTrack, used for playing back audio
                        Log.i(LOG_TAG, "Receive thread started. Thread id: " + Thread.currentThread().getId());
                        AudioTrack track = new AudioTrack(AudioManager.STREAM_MUSIC, SAMPLE_RATE, AudioFormat.CHANNEL_OUT_MONO,
                                AudioFormat.ENCODING_PCM_16BIT, BUF_SIZE, AudioTrack.MODE_STREAM);
                        track.play();
                        try {
                            // Define a socket to receive the audio
                            byte[] buf = new byte[BUF_SIZE];
                            while(speakers) {
                                // Play back the audio received from packets
                                DatagramPacket packet = new DatagramPacket(buf, BUF_SIZE);
                                socket.receive(packet);
                                Log.i(LOG_TAG, "Packet received: " + packet.getLength());
                                /**
                                String toSend = "Receive Packet: ";
                                for (int i = 0; i < 10; i++) {
                                    toSend += i + " = " + (byte) packet.getData()[i];
                                }
                                Log.i(LOG_TAG, toSend);
                                */
                                track.write(packet.getData(), 0, BUF_SIZE);

                            }
                            // Stop playing back and release resources
                            socket.disconnect();
                            socket.close();
                            track.stop();
                            track.flush();
                            track.release();
                            speakers = false;
                            return;
                        }
                        catch(SocketException e) {

                            Log.e(LOG_TAG, "SocketException: " + e.toString());
                            speakers = false;
                        }
                        catch(IOException e) {

                            Log.e(LOG_TAG, "IOException: " + e.toString());
                            speakers = false;
                        }
                    }
                });
                receiveThread.start();
            }
        }
    }
}