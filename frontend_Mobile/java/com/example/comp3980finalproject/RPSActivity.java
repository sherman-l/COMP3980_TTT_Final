package com.example.comp3980finalproject;

import androidx.appcompat.app.AppCompatActivity;

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

import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

public class RPSActivity extends AppCompatActivity {

    private Connection conn;
    private RequestData req;

    private final byte STATUS_SUCCESS = 10;
    private final byte STATUS_UPDATE = 20;
    private final byte STATUS_ERR_TYP = 32;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_r_p_s);

        conn = (Connection) getIntent().getExtras().get(getResources().getString(R.string.connKey));
        String ip = (String) getIntent().getExtras().get(getResources().getString(R.string.ipKey));

        TextView msg = findViewById(R.id.title);
        msg.setText("Successfully connected to " + ip);
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
}