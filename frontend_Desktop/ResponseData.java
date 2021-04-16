import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.IOException;

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
		payload = new byte[payloadLen];
		this.payloadLen = payloadLen;
	}
	private void set_payload(byte val, int index) {payload[index] = val;}
	
	public byte get_status() {return status;}
	public byte get_resType() {return resType;}
	public byte get_payloadLen() {return payloadLen;}
	public byte[] get_payload() {return payload;}
	public int get_payloadVal() {
		int val = 0;
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
	
	public void readResponse(BufferedReader reader, DataInputStream is) {
		boolean exit = false;
		while (!exit) {
			try {
				byte val = (byte) is.read();
				readData(val);
				count++;
				if (count == 3 + get_payloadLen()) {
					exit = true;
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				exit = true;
			}
		}
	}
	
	public void printObj(Connection.State s) {
		System.out.println("\nState = " + s);
		System.out.println("Received:\n===================");
		System.out.println("status = " + status);
		System.out.println("resType = " + resType);
		System.out.println("payloadLen = " + payloadLen);
		System.out.println("payload bytes = " + arrToString(payload));
		System.out.println("payload int = " + get_payloadVal());
	}
	
	public String arrToString(byte[] arr) {
		String ret = "";
		if (arr.length > 0) ret += arr[0];
		for (int i = 1; i < arr.length; i++) {
			ret = ret + " " + arr[i];
		}
		return ret;
	}
}
