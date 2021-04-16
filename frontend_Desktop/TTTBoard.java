
public class TTTBoard {
	private char playerChar;
	private byte[] boardArr;
	private int playerId;
	private int otherPlayer;
	private byte lastMove;
	
	public TTTBoard(int playerId) {
		this.playerId = playerId;
		
		if (playerId == 1) otherPlayer = 2;
		else otherPlayer = 1;
		playerChar = playerId == 1 ? 'X' : 'O'; 
		boardArr = new byte[9];
		clearBoard();
	}
	
	public int getId() { return playerId; }
	public int otherId() { return otherPlayer; }
	
	public void clearBoard() {
		for (int i = 0; i < boardArr.length; i++) {
			boardArr[i] = 0;
		}
		lastMove = 9;
	}
	
	public boolean checkSquare(byte cell) {
		return boardArr[cell] == 0;
	}
	
	public void clearSquare() {
		if (lastMove != 9) {
			boardArr[lastMove] = 0;
			lastMove = 9;
		}
	}
	
	public void setSquare(byte cell, byte thisPlayer) { 
		lastMove = cell;
		boardArr[cell] = thisPlayer == playerId ? (byte) playerId : (byte) otherPlayer; 
	}
	
	public void displayBoard() {
		System.out.println("\nPlayer: " + playerChar);
		
		for (int i = 0; i < boardArr.length; i++) {
			int row = i % 3;
			
			if (row == 0) System.out.println();
			char user;
			switch (boardArr[i]) {
				case 1: user = 'X'; break;
				case 2: user = 'O'; break;
				default: user = '-'; break;
			}
			System.out.print(user + " ");
		}
		System.out.println();
	}
}
