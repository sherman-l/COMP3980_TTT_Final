package com.example.comp3980finalproject;
import android.util.Log;

import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Scanner;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.InputStream;
import java.io.InputStreamReader;

public class Connection {
    enum State {
        INIT,       // 0
        END         // 1
    }
    private State gameState;
    private String hostIP;
    private int port;
    private InetAddress addr;
    private BufferedReader reader;
    private PrintWriter pwriter;

    /**
     * Constructor for the connection class, sets the connection settings
     * @param port, port used to connect to server
     * @param hostIP, ip used to connect to server
     */
    public Connection(int port, String hostIP) {
        this.port = port;
        this.hostIP = hostIP;
    }

    /**
     * converts the value into a 4 bytes, packed into an array
     * @param value
     * @return
     */
    private byte[] getArray(int value) {
        return ByteBuffer.allocate(4).putInt(value).array();
    }

    /**
     * Packs 4 8-bit int values into one 32-bit int
     * @param v1, first 8-bit value
     * @param v2, second 8-bit value
     * @param v3, third 8-bit value
     * @param v4, fourth 8-bit value
     * @return 32-bit int containing the packed arguments
     */
    private int convInt(byte v1, byte v2, byte v3, byte v4) {
        byte[] tmpArr = new byte[4];
        tmpArr[0] = v1;
        tmpArr[1] = v2;
        tmpArr[2] = v3;
        tmpArr[3] = v4;
        ByteBuffer tmp = ByteBuffer.wrap(tmpArr);
        return tmp.getInt();
    }

    public State getState() { return gameState; }

    private byte combineMsg(byte v1, byte v2) {
        return (byte) (v1 << 4 | v2);
    }

    public void connect() {
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    InetAddress addr = InetAddress.getByName(hostIP);
                    Socket socket = new Socket(addr.getHostName(), port);
                    OutputStream os = socket.getOutputStream();
                    InputStream is = socket.getInputStream();


                    reader = new BufferedReader(new InputStreamReader(is));
                    pwriter = new PrintWriter(os, true);

                    Log.d("state", "successful connection to hostIP = " + hostIP + ", port = " + port);
                } catch (UnknownHostException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        thread.start();
    }

    public void execute() {
        switch (gameState) {
            case INIT:
                // set gameID to 2 - Rock Paper Scissors
                byte gameID = 2;
                sendInit(gameID);
            default:
                break;
        }
    }

    private void sendInit(byte gameID) {
        // send uid [0] in the beginning
        pwriter.println(convInt((byte) 0, (byte)0, (byte)0, (byte)0));

        // send payload [1 | 1 | 1 | 1-2] in the beginning
        pwriter.println(convInt((byte) 1, (byte) 1, (byte) 1, combineMsg((byte) 1, (byte) gameID)));

        gameState = State.END;
    }
}
