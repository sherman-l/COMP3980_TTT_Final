import java.io.DataOutputStream;
import java.io.IOException;
import java.io.PrintWriter;

public class RequestData {
	private byte[] uid;
	private byte reqType;
	private byte reqContext;
	private byte payloadLen;
	private byte[] payload;
	private PrintWriter pwriter;
	private DataOutputStream os;
	
	public RequestData(
			PrintWriter pwriter,
			DataOutputStream os,
			byte[] payload) 
	{
		this.os = os;
		this.pwriter = pwriter;
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
	public void set_payload(byte[] payload) {this.payload = payload;}
	public void set_payload(byte val) {
		payload = new byte[1];
		payload[0] = val;
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
		System.out.println("\nState = " + s);
		System.out.println("Sent:\n===================");
		System.out.println("uid = " + arrToString(uid));
		System.out.println("reqType = " + reqType);
		System.out.println("reqContext = " + reqContext);
		System.out.println("payloadLen = " + payloadLen);
		System.out.println("payload = " + arrToString(payload));
	}
}