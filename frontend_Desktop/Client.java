import java.util.Scanner;

public class Client {
	public static void main(String args[]) {
		Scanner scan = new Scanner(System.in);
		System.out.println("Enter the host IP Address");
        String hostIP = scan.nextLine();
        System.out.println("Enter the port");
        int port = scan.nextInt();
		
		Connection conn = new Connection(port, hostIP, scan);
		conn.connect();
		
		while (conn.getState() != Connection.State.END) {
			conn.execute();
		}
	}
}
