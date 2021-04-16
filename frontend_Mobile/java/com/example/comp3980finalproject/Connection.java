package com.example.comp3980finalproject;
import android.util.Log;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.Serializable;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Objects;
import java.util.Scanner;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.InputStream;
import java.io.InputStreamReader;

public class Connection implements Serializable {
    public enum State {
        INIT,       // 0
        MOVE,       // 1
        WAIT,       // 2
        RESULT,     // 4
        END         // 3
    }
    private State gameState;
    private String hostIP;
    private int port;
    private InetAddress addr;
    private BufferedReader reader;
    private PrintWriter pwriter;
    private DataOutputStream os;
    private DataInputStream is;
    private final byte PROTOCOL_VERSION = 1;

    public static synchronized void setSocket(Socket socket) {MainActivity.socket = socket;}

    /**
     * Constructor for the connection class, sets the connection settings
     * @param port, port used to connect to server
     * @param hostIP, ip used to connect to server
     */
    public Connection(int port, String hostIP) {
        this.port = port;
        this.hostIP = hostIP;
        gameState = State.INIT;
    }

    public byte getVersion() {return PROTOCOL_VERSION;}

    public State getState() { return gameState; }
    public void setState(Connection.State s) {this.gameState = s;}
    public void connect() {
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    InetAddress addr = InetAddress.getByName(hostIP);
                    Socket socket = new Socket(addr.getHostName(), port);
                    setSocket(socket);

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
}
