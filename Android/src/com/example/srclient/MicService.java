package com.example.srclient;

import android.app.Service;
import android.os.IBinder;
import android.content.Intent;
import android.media.MediaRecorder.AudioSource;
import android.media.AudioRecord;
import android.media.AudioFormat;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.lang.Thread;
import java.net.InetSocketAddress;

import java.lang.ArrayIndexOutOfBoundsException;
import java.net.UnknownHostException;
import java.io.IOException;
import java.lang.IllegalArgumentException;

public class MicService extends Service {
	
	private static Socket socket = null;
	private static BufferedOutputStream bufOutStream = null;
	private AudioRecord mInputData;
	private final static int mSampleRate = 44100;
	private int mMinBufferSize;
	private ByteBuffer mInBuffer;
	private boolean mActiveMic = false;

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	@Override
	public void onCreate() {
		mMinBufferSize = AudioRecord.getMinBufferSize(mSampleRate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, 
				AudioFormat.ENCODING_PCM_16BIT);
		mInputData = new AudioRecord(AudioSource.MIC, mSampleRate, 
				AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT, mMinBufferSize);
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		initConnection();
		
		if(mInputData.getState() == AudioRecord.STATE_INITIALIZED) {
			mActiveMic = true;
			
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
					stopRecording();
				}
			};
			
			t.start();
		}
	}
	
	// Recording stops and closing connection
	@Override
	public void onDestroy() {
		mActiveMic = false;
		
		try {
			Thread.sleep(300);
			bufOutStream.close();
			if(socket.isConnected())
				socket.close();
		} catch(IllegalStateException e) {
			e.printStackTrace();
		} catch(IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	public void stopRecording() {
		try {
			mInputData.stop();
			mInputData.release();
		} catch(IllegalStateException e) {
			e.printStackTrace();
		}
	}
	
	public int initConnection() {

		socket = new Socket();
		Thread t = new Thread() {
			@Override
			public void run() {
				try {
					socket.connect(new InetSocketAddress(KP.ip, 4444));
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
}
