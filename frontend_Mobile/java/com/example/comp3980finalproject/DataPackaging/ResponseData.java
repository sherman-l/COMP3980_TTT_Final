package com.example.comp3980finalproject.DataPackaging;

import android.util.Log;

import com.example.comp3980finalproject.Connection;
import com.example.comp3980finalproject.MainActivity;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;

public class ResponseData {
    private byte status;
    private byte resType;
    private byte payloadLen;
    private byte[] payload;
    private int count;

    public ResponseData() {
        this.count = 0;
    }

    private void set_status(byte status) {this.status = status;}
    private void set_resType(byte resType) {this.resType = resType;}
    private void set_payloadLen(byte payloadLen) {
        Log.d("state", "setting payload of size " + (byte) payloadLen);
        if (payloadLen != 0) {

            payload = new byte[payloadLen];
            this.payloadLen = payloadLen;
        } else {
            payload = null;
            this.payloadLen = 0;
        }
    }
    private void set_payload(byte val, int index) {payload[index] = val;}

    public byte get_status() {return status;}
    public byte get_resType() {return resType;}
    public byte get_payloadLen() {return payloadLen;}
    public byte[] get_payload() {return payload;}
    public int get_payloadVal() {
        int val = 0;
        if (payload == null) return val;
        for (int i = 0; i < payload.length; i++) {
            val = (val << 8);
            val += (byte) payload[i];
        }
        return val;
    }

    private void readData(byte val) {
        switch(count) {
            case 0:
                set_status(val);
                break;
            case 1:
                set_resType(val);
                break;
            case 2:
                set_payloadLen(val);
                break;
            default:
                set_payload(val, count - 3);
                break;
        }
    }

    public void readResponse() {
        Socket tmpSocket = MainActivity.getSocket();
        boolean exit = false;
        try {
            DataInputStream is = new DataInputStream(tmpSocket.getInputStream());
            while (!exit) {
                try {
                    byte val = (byte) is.read();

                    readData(val);
                    count++;
                    if (count == 3 + get_payloadLen()) {
                        exit = true;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    exit = true;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void printObj(Connection.State s) {
        Log.d("state","\nState = " + s);
        Log.d("state","Received:\n===================");
        Log.d("state","status = " + status);
        Log.d("state","resType = " + resType);
        Log.d("state","payloadLen = " + payloadLen);
        Log.d("state","payload bytes = " + arrToString(payload));
        Log.d("state","payload int = " + get_payloadVal());
    }

    public String arrToString(byte[] arr) {
        String ret = "";
        if (arr == null) return ret;
        if (arr.length > 0) ret += arr[0];
        for (int i = 1; i < arr.length; i++) {
            ret = ret + " " + arr[i];
        }
        return ret;
    }
}
