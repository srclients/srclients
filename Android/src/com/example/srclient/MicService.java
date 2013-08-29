package com.example.srclient;

import android.app.Service;
import android.os.IBinder;
import android.content.Intent;
import android.media.MediaRecorder.AudioSource;
import android.media.AudioRecord;
import android.media.AudioFormat;
import android.util.Log;
import android.widget.Toast;

import java.io.BufferedOutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.lang.Thread;
import java.net.InetSocketAddress;

import java.lang.ArrayIndexOutOfBoundsException;
import java.net.UnknownHostException;
import java.io.IOException;
import java.lang.IllegalArgumentException;

public class MicService extends Service {
	
	private static DatagramSocket socket;
	private static String ip;
	private static String port;
	DatagramPacket packet;
	private AudioRecord mInputData;
	private final int mSampleRate = 44100;
	private int mMinBufferSize;
	private ByteBuffer mInBuffer;
	private static boolean mActiveMic = false;

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		
		ip = KP.getMicServiceIP();
		port = KP.getMicServicePort();
		
		if((ip == null) || (port == null)) {
			Toast.makeText(this, "Mic service is not available", Toast.LENGTH_SHORT).show();
			
			Intent recvIntent = new Intent(Projector.BROADCAST_STATUS_SERVICE);
			recvIntent.putExtra(Projector.SERVICE_STATUS, false);
			sendBroadcast(recvIntent);
			
			stopSelf();
		}
		
		mMinBufferSize = AudioRecord.getMinBufferSize(mSampleRate, AudioFormat.CHANNEL_IN_STEREO, 
				AudioFormat.ENCODING_PCM_16BIT);
			
		mInputData = new AudioRecord(AudioSource.MIC, mSampleRate, 
				AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT, mMinBufferSize);
		
		try {
			socket = new DatagramSocket();
		} catch (SocketException e) {
			e.printStackTrace();
		}
		
		if(mInputData.getState() == AudioRecord.STATE_INITIALIZED) {
			mActiveMic = true;
			
			Thread t = new Thread() {
				@Override
				public void run() {
					int bytesRead;
					
					// Send to server minimal buffer size value
					try {
						String str = String.valueOf(mMinBufferSize);
						byte bufSize[] = str.getBytes();
						
						DatagramSocket socketBuffer = new DatagramSocket();
						packet = new DatagramPacket(bufSize, bufSize.length,
								new InetSocketAddress(ip, 4445));
						
						socketBuffer.send(packet);
						socketBuffer.close();
						
						sleep(400);
					} catch (SocketException e) {
						e.printStackTrace();
					} catch (IOException e1) {
						e1.printStackTrace();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					
					// Start recording
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
							
							byte audioData[] = new byte[bytesRead];
							
							mInBuffer.get(audioData);
							mInBuffer.rewind();
							
							try {
								packet = new DatagramPacket(audioData, audioData.length,
										new InetSocketAddress(ip, Integer.parseInt(port)));
								socket.send(packet);
								
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
	}
	
	public void stopRecording() {
		try {
			mInputData.stop();
			mInputData.release();
			socket.close();
		} catch(IllegalStateException e) {
			e.printStackTrace();
		}
	}
}
