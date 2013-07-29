package com.example.microphonesr;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.MenuItem;
import android.media.MediaRecorder.AudioSource;
import android.media.AudioRecord;
import android.media.AudioFormat;
import java.nio.ByteBuffer;
import java.lang.Thread;
import java.net.Socket;
import java.io.BufferedOutputStream;
import java.net.InetSocketAddress;
import android.util.Log;

import java.lang.ArrayIndexOutOfBoundsException;
import java.net.UnknownHostException;
import java.io.IOException;
import java.lang.IllegalArgumentException;

public class MainActivity extends Activity {
	
	private static final int MENU_START = 101;
	private static final int MENU_STOP = 102;
	
	private Socket socket = null;
	private BufferedOutputStream bufOutStream = null;
	private AudioRecord mInputData;
	private final static int mSampleRate = 44100;
	private int mMinBufferSize;
	private ByteBuffer mInBuffer;
	private boolean mActiveMic = false;
	
	// TODO: невозможен старт после выключения, а так норм
	

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}
	
	// recording and transferring data to remote machine
	public void recording() {
		
		// initialize recorder (input data)
		mMinBufferSize = AudioRecord.getMinBufferSize(mSampleRate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, 
				AudioFormat.ENCODING_PCM_16BIT);
		mInputData = new AudioRecord(AudioSource.MIC, mSampleRate, 
				AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT, mMinBufferSize);
		
		if(mInputData.getState() == AudioRecord.STATE_INITIALIZED) {
			Thread t = new Thread() {
				@Override
				public void run() {
					int bytesRead;
					
					try {
						mInputData.startRecording();
					} catch(IllegalStateException e) {
						e.printStackTrace();
					}
					
					while(mActiveMic) {
						try {
							mInBuffer = ByteBuffer.allocateDirect(mMinBufferSize);
						} catch (IllegalArgumentException e) {
							e.printStackTrace();
						}
						
						bytesRead = mInputData.read(mInBuffer, mMinBufferSize);
						if(bytesRead != AudioRecord.ERROR_INVALID_OPERATION &&
								bytesRead != AudioRecord.ERROR_BAD_VALUE) {
							byte audioData[] = new byte[mMinBufferSize];
							mInBuffer.get(audioData);
							mInBuffer.rewind();
							try {
								bufOutStream.write(audioData, 0, bytesRead);
							} catch(IOException e) {
								e.printStackTrace();
							} catch(NullPointerException e) {
								e.printStackTrace();
							} catch(ArrayIndexOutOfBoundsException e) {
								e.printStackTrace();
							} catch(IndexOutOfBoundsException e) {
								e.printStackTrace();
							}
						}
					}
				}
			};
			
			t.start();
		}
	}
	
	public void stopRecording() {
		try {
			mInputData.stop();
			mInputData.release();
			mInputData = null;
		} catch(IllegalStateException e) {
			e.printStackTrace();
		}
	}
	
	// Connection to remote machine
	public int initConnection() {

		socket = new Socket();
		Thread t = new Thread() {
			@Override
			public void run() {
				try {
					socket.connect(new InetSocketAddress("192.168.0.20", 4444));
				} catch(UnknownHostException e) {
					e.printStackTrace();
				} catch(IOException e) {
					e.printStackTrace();
					Log.e("MicSR", "socket is already connected!");
				} catch (IllegalArgumentException e) {
					e.printStackTrace();
				}
			}
		};
		t.start();
		try {
			t.join();
			bufOutStream = new BufferedOutputStream(socket.getOutputStream());
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch(IOException e) {
			e.printStackTrace();
		}
		
		return 0;
	}
	
	// Close connection with remote machine
	public void closeConnection() {
		try {
			bufOutStream.close();
			if(socket.isConnected())
				socket.close();
			bufOutStream = null;
			socket = null;
		} catch(IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		//getMenuInflater().inflate(R.menu.main, menu);
		menu.add(0, MENU_START, 0, "Mic ON");
		menu.add(0, MENU_STOP, 0, "Mic OFF");
		return true;
	}
	
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
			case MENU_START:
				mActiveMic = true;
				initConnection();
				recording();
				break;
				
			case MENU_STOP:
				mActiveMic = false;
				stopRecording();
				Log.i("MicSR", "Recording stoped");
				closeConnection();
				Log.i("MicSR", "Connection closed");
				break;
		}
		
		return true;
	}

}
