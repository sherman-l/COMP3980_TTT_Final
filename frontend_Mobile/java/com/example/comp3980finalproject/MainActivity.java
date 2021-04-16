package com.example.comp3980finalproject;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class MainActivity extends AppCompatActivity {
    public static Socket socket;
    public static Socket getSocket() {return socket;}

    private String hostIP = null;
    private int port = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void connect(View view) {
        EditText hostIPET = findViewById(R.id.home_ip);
        EditText portET = findViewById(R.id.home_port);
        if (!hostIPET.getText().toString().equals("")) hostIP = hostIPET.getText().toString();
        if (!portET.getText().toString().equals("")) port = Integer.parseInt(portET.getText().toString());
        Log.d("state", "hostIP = " + hostIP + ", port = " + port);
        Connection conn = new Connection(port, hostIP);
        conn.connect();

        Intent i = new Intent(this, RPSActivity.class);
        i.putExtra(getResources().getString(R.string.connKey), conn);
        i.putExtra(getResources().getString(R.string.ipKey), hostIP);
        i.putExtra(getResources().getString(R.string.portKey), port);
        startActivity(i);
    }
}