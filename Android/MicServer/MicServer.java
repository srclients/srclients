import java.net.ServerSocket;
import java.net.Socket;
import java.io.BufferedInputStream;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import javax.sound.sampled.*;
import java.net.InetSocketAddress;
import java.lang.Thread;

import java.io.IOException;

class MicServer extends ServerSocket{
	private ServerSocket socket;
	private AudioFormat format;
	private static SourceDataLine srcLine;
	private DataLine.Info info;

	private float sampleRate = 44100;
	private int sampleSizeInBits = 16;
	private int channels = 2;
	private boolean signed = true;
	private boolean bigEndian = false;
	private static int bufferLen = 1024;
	private static boolean playStarted = false;
	private int frameSize = 0;

	MicServer() throws IOException, LineUnavailableException {
	    socket = new ServerSocket();
	    format = new AudioFormat(sampleRate, sampleSizeInBits, 
				channels, signed, bigEndian);
	    info = new DataLine.Info(SourceDataLine.class, format);

	    // Open the line through which we will play stream data
	    srcLine = (SourceDataLine) AudioSystem.getLine(info);
	    srcLine.open(format, bufferLen);
	}

	public static void main(String[] args) {
		MicServer server;
		Socket client = new Socket();
		BufferedInputStream bufInStream;
		//PrintWriter prWriter;
		final BufferedReader reader = new BufferedReader(
		    new InputStreamReader(System.in));
		
		// Thread for coorect stop of a server
		Thread t = new Thread() {
		    @Override
		    public void run() {
			System.out.println("Press 'q' to exit program.\n");
			while(true) {
			    try {
				if(reader.readLine().equals("q") ||
				    reader.readLine().equals("Q"))
				    System.exit(0);
			    } catch (IOException e) {
				e.printStackTrace();
			    }
			}
		    }
		};
		t.start();

		try {
		
		    // Loop which listens requests from clients
		    while(true) {
			server = new MicServer();
		    
			System.out.println("Waiting for a client...");
		    
			server.setSoTimeout(0);
			server.bind(new InetSocketAddress("192.168.0.20", 4444));
			client = server.accept();
		    
			System.out.println("Client connected.");
		    
			bufInStream = new BufferedInputStream(client.getInputStream());
			//prWriter = new PrintWriter(client.getOutputStream());
		    
			readLoop(bufInStream);
			playStarted = false;
			bufInStream.close();
			server.close();
		    }
		} catch(IOException e) {
		    e.printStackTrace();
		    System.exit(-1);
		} catch (IllegalArgumentException e) {
		    e.printStackTrace();
		    System.out.println("Couldn't write data from byteArray.");
		} catch (ArrayIndexOutOfBoundsException e) {
		    e.printStackTrace();
		    System.out.println("Couldn't write data from byteArray.");
		} catch	(LineUnavailableException e) {
		    e.printStackTrace();
		    System.out.println("The line is not available.");
		    System.exit(-1);
		}
	}

	// Reading and playing data from mobile device microphone
	public static void readLoop(BufferedInputStream bufInStream) throws 
			IOException, IllegalArgumentException, 
			ArrayIndexOutOfBoundsException {

		while(true) {
		    byte byteArray[] = new byte[bufferLen];
		    int readBytes = 0;

		    if(!playStarted) {
			srcLine.start();
			playStarted = true;
		    }
			
		    readBytes = bufInStream.read(byteArray, 0, bufferLen);
		    if(readBytes != -1)
			srcLine.write(byteArray, 0, bufferLen);
		    else
			return;
		}
	}
}
