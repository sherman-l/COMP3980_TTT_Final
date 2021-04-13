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
	
	private Scanner scan;
	private String hostIP;
	private int port;
	private InetAddress addr;
	private BufferedReader reader;
	private PrintWriter pwriter;
	private State gameState;
	
	/**
	 * Constructor for the connection class, sets the connection settings
	 * @param port, port used to connect to server
	 * @param hostIP, ip used to connect to server
	 * @param scan, scanner class used to get user input
	 */
	public Connection(int port, String hostIP, Scanner scan) {
		this.scan = scan;
		this.port = port;
		this.hostIP = hostIP;
		gameState = State.INIT;
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
	
	public State getState() {
		return gameState;
	}
	
	private byte combineMsg(byte v1, byte v2) {
		return (byte) (v1 << 4 | v2);		
	}
	
	public void connect() {
		try {
			addr = InetAddress.getByName(hostIP);
			Socket socket = new Socket(addr.getHostName(), port);
			
			OutputStream os = socket.getOutputStream();
			InputStream is = socket.getInputStream();
			
			reader = new BufferedReader(new InputStreamReader(is));
			pwriter = new PrintWriter(os, true);
			
			System.out.println("successfully connected to " + hostIP + ", port " + port);
		} catch (UnknownHostException e) {
			System.out.println("Error with IP: " + hostIP + ", port: " + port);
			System.out.println("Cannot find server: " + e.getMessage());
		} catch (IOException e) {
			System.out.println("Error with IP: " + hostIP + ", port: " + port);
			System.out.println("IO error: " + e.getMessage());
		}
	}
	
	public void execute() {
		switch (gameState) {
			case INIT:
				// set gameID to 1 - tic tac toe
				byte gameID = 1;
				sendInit(gameID);
				// TO-DO: Read the server's response
				//readResponse();
				gameState = State.END;
				
				break;
			default:
				break;
		}
	}
	
	private void sendInit(byte gameID) {
		pwriter.println(convInt((byte) 0, (byte)0, (byte)0, (byte)0));
		pwriter.println(convInt((byte) 1, (byte) 1, (byte) 1, combineMsg((byte) 1, (byte) gameID)));		
	}
	
	private void readResponse() {
		try {
			int response = reader.read();
			parseResponse(response);
		} catch (IOException e) {
			System.out.println("IOError: " + e.getMessage());
			if (e.getMessage() == null) {
				System.exit(0);
			}
		}
	}
	
	private void parseResponse(int response) {
		switch (gameState) {
			case INIT:
				
			default:
				break;
		}
	}
}
