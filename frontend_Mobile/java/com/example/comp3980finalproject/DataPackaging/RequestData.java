package com.example.comp3980finalproject.DataPackaging;

import android.util.Log;

import com.example.comp3980finalproject.Connection;
import com.example.comp3980finalproject.MainActivity;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;

public class RequestData {
    private byte[] uid;
    private byte reqType;
    private byte reqContext;
    private byte payloadLen;
    private byte[] payload;
    private PrintWriter pwriter;
    private DataOutputStream os;

    public RequestData(
            byte[] payload)
    {
        Socket tmpSocket = MainActivity.getSocket();
        try {
            this.os = new DataOutputStream(tmpSocket.getOutputStream());
        } catch (IOException e) {
            Log.d("state", "unable to write socket");
        }
        this.uid = new byte[4];
        this.reqType = (byte) 1;
        this.reqContext = (byte) 1;
        this.payloadLen = (byte) 2;
        this.payload = payload;
    }

    public void set_uid(byte[] uidArr) {

        for (int i = 0; i < uidArr.length; i++) {
            uid[i] = uidArr[i];
        }
    }
    public void set_reqType(byte reqType) {this.reqType = reqType;}
    public void set_reqContext(byte reqContext) {this.reqContext = reqContext;}
    public void set_payloadLen(byte payloadLen) {this.payloadLen = payloadLen;}
    public void set_payload(byte[] payload) {
        this.payload = new byte[this.payloadLen];
        for (int i = 0; i < payloadLen; i++) {
            this.payload[i] = payload[i];
        }
    }
    public void set_payload(byte val) {
        this.payload = new byte[1];
        this.payload[0] = val;
    }

    public void writeRequest() {
        write4B(os, uid);
        try {
            os.write((byte) reqType);
            os.flush();
            os.write((byte) reqContext);
            os.flush();
            os.write((byte) payloadLen);
            os.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
        writePayload(os, payload);
    }

    public void writePayload(DataOutputStream os, byte[] val) {
        for (int i = 0; i < payloadLen; i++) {
            try {
                os.write(val[i]);
                os.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static void write4B(DataOutputStream os, byte[] val) {
        for (int i = 0; i < val.length; i++) {
            try {
                os.write(val[i]);
                os.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public String arrToString(byte[] arr) {
        String ret = "" + arr[0];
        for (int i = 1; i < arr.length; i++) {
            ret = ret + " " + arr[i];
        }
        return ret;
    }

    public void printObj(Connection.State s) {
        Log.d("state", "\nState = " + s);
        Log.d("state", "Sent:\n===================");
        Log.d("state", "uid = " + arrToString(uid));
        Log.d("state", "reqType = " + reqType);
        Log.d("state", "reqContext = " + reqContext);
        Log.d("state", "payloadLen = " + payloadLen);
        Log.d("state", "payload = " + arrToString(payload));
    }
}
