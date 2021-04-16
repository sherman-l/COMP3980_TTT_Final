import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Scanner;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.InputStream;
import java.io.InputStreamReader;

public class Connection {
	enum State {
        INIT,       // 0
        READ,		// 1
        WAIT,		// 2
        MAKE_MOVE,	// 3
        WAIT_MOVE,	// 4
        WAIT_CONF,	// 5
        END         // 6
    }
	
	private final byte STATUS_SUCCESS = 10;
	private final byte STATUS_UPDATE = 20;
	private final byte STATUS_ERR_REQ = 30;
	private final byte STATUS_ERR_UID = 31;
	private final byte STATUS_ERR_TYP = 32;
	private final byte STATUS_ERR_CON = 33;
	private final byte STATUS_ERR_PAY = 34;
	private final byte STATUS_SER_ERR = 40;
	private final byte STATUS_GAM_INV = 50;
	private final byte STATUS_GAM_ACT = 51;
	
	private final byte REQ_TYPE_CONF = 1;
	private final byte REQ_TYPE_INF = 2;
	private final byte REQ_TYPE_META = 3;
	private final byte REQ_TYPE_GAME = 4;
	
	private final byte PROTOCOL_VERSION = 1;
	private Scanner scan;
	private String hostIP;
	private int port;
	private InetAddress addr;
	private BufferedReader reader;
	private PrintWriter pwriter;
	private State gameState;
	private RequestData req;
	private DataOutputStream os;
	private DataInputStream is;
	private TTTBoard board;
	
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
	
	public State getState() {
		return gameState;
	}
	
	public void connect() {
		try {
			addr = InetAddress.getByName(hostIP);
			Socket socket = new Socket(addr.getHostName(), port);
			
			os = new DataOutputStream(socket.getOutputStream());
			is = new DataInputStream(socket.getInputStream());
			
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
				System.out.println("Which game would you like to play? TTT = 1, RPS = 2");
				byte gameID = scan.nextByte();
				byte protocolVers = PROTOCOL_VERSION;
				byte[] payload = {protocolVers, gameID};
				
				// write the initial request
				req = new RequestData(pwriter, os, payload);
				//req.printObj(gameState);
				req.writeRequest();
				
				gameState = State.READ;
				break;
			case READ:
				// read the server's response
				ResponseData resRead = new ResponseData();
				resRead.readResponse(reader, is);
				//resRead.printObj(gameState);
				
				parseResponse(resRead);
				break;
			case WAIT:
				ResponseData resWait = new ResponseData();
				resWait.readResponse(reader, is);
				//resWait.printObj(gameState);
				
				parseResponse(resWait);
				break;
			case MAKE_MOVE:
				System.out.println("Please enter a cell, 0-8");
				byte move = scan.nextByte();
				if (!board.checkSquare(move)) {
					System.out.println("Square already occupied, or out of bounds. Enter another number:");
					move = scan.nextByte();
				}
				
				board.setSquare(move, (byte) board.getId());
				req.set_reqType((byte) 4);
				req.set_reqContext((byte) 1);
				req.set_payloadLen((byte) 1);
				req.set_payload(move);
				//req.printObj(gameState);
				
				req.writeRequest();
				
				gameState = State.WAIT_CONF;
				break;
			case WAIT_CONF:
				System.out.println("Listening for acknowledgment...");
				ResponseData resWaitConf = new ResponseData();
				resWaitConf.readResponse(reader, is);
				//resWaitConf.printObj(gameState);
				
				parseResponse(resWaitConf);
				break;
			case WAIT_MOVE:
				System.out.println("Listening for other player's move...");
				ResponseData resWaitMove = new ResponseData();
				resWaitMove.readResponse(reader, is);
				//resWaitMove.printObj(gameState);
				
				parseResponse(resWaitMove);
				break;
			default:
				break;
		}
	}
	
	private void parseResponse(ResponseData res) {
		switch (gameState) {
			case READ:
				if (res.get_status() == STATUS_SUCCESS) {
					req.set_uid(res.get_payload());
					gameState = State.WAIT;
				} else {
					String invalCode = "";
					switch(res.get_status()) {
						case STATUS_ERR_REQ: invalCode = "request"; break;
						case STATUS_ERR_UID: invalCode = "uid"; break;
						case STATUS_ERR_TYP: invalCode = "type"; break;
						case STATUS_ERR_CON: invalCode = "context"; break;
						case STATUS_ERR_PAY: invalCode = "payload"; break;
						default: invalCode = "unexpected"; break;
					}
					System.out.println("Error, invalid " + invalCode);
					gameState = State.INIT;
				} 
				break;
			case WAIT:
				if (res.get_status() == STATUS_UPDATE) {
					board = new TTTBoard(res.get_payloadVal());
					board.displayBoard();
					gameState = res.get_payloadVal() == 1 ? State.MAKE_MOVE : State.WAIT_MOVE;
				}
				break;
			case WAIT_CONF:
				if (res.get_status() == STATUS_SUCCESS && res.get_resType() == REQ_TYPE_CONF) {
					System.out.println("Move accepted, new board:");
					board.displayBoard();
					gameState = State.WAIT_MOVE;
				} else {
					System.out.println("Unexpected response, please enter the move again");
					board.clearSquare();
					gameState = State.MAKE_MOVE;
				}
				break;
			case WAIT_MOVE:
				if (res.get_status() == STATUS_UPDATE && res.get_resType() == REQ_TYPE_INF) {
					board.setSquare((byte) res.get_payloadVal(), (byte) board.otherId());
					board.displayBoard();
					gameState = State.MAKE_MOVE;
				} else if (res.get_status() == STATUS_UPDATE && res.get_resType() == REQ_TYPE_META){
					String result = "";
					switch (res.get_payload()[0]) {
						case 1: result = "Won"; break;
						case 2: result = "Lost"; break;
						case 3: result = "Tied"; break;
						default: break;
					}
					System.out.println("Game ended: " + result + ", last square placed = " + res.get_payload()[1]);
					
					board.setSquare((byte) res.get_payloadVal(), (byte) board.otherId());
					board.displayBoard();
					gameState = State.END;
				} else {
					System.out.println("Unexpected response, please enter the move again");
					gameState = State.WAIT_MOVE;
				}
				break;
			default:
				break;
		}
	}
}
