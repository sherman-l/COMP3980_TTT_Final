import java.util.Scanner;

public class Client {
	public static void main(String args[]) {
		Scanner scan = new Scanner(System.in);
		//String hostIP = "127.0.0.1";
		String hostIP = "50.92.70.107";
		int port = 13541;
		
		Connection conn = new Connection(port, hostIP, scan);
		conn.connect();
		
		while (conn.getState() != Connection.State.END) {
			conn.execute();
		}
	}
}
