package com.example.srclient;

import android.app.Activity;
import android.os.Bundle;
import android.provider.Settings;
import android.widget.Button;
import android.widget.Toast;
import android.util.Log;
import android.view.View;
import android.content.Intent;
import java.util.ArrayList;
import android.widget.EditText;
import android.widget.CheckBox;
import android.view.KeyEvent;
import android.content.SharedPreferences;
import 	android.content.Context;

import 	java.lang.ClassCastException;

/**
 * The Class KP.
 */
public class KP extends Activity 
	implements View.OnClickListener {
	
	/** The connect btn. */
	Button connectBtn;
	
	/** The edit name. */
	EditText editName;
	
	/** The edit password. */
	EditText editPassword;
	
	EditText editIP;
	
	/** The ch box anonim. */
	CheckBox chBoxAnonim;
	
	/** The connection state. */
	static int connectionState;
	
	static boolean isChairman;
	static boolean isRegistered;
	
	/** The timeslot list. */
	ArrayList<String> timeslotList;
	
	/** The uuid. */
	String uuid;
	public static String ip;
	
	/*
	 * 	Native functions prototypes
	 */
	/**
	 * Connect smart space.
	 *
	 * @param hostname the hostname
	 * @param ip the ip
	 * @param port the port
	 * @return the int
	 */
	public static native int connectSmartSpace(String hostname, String ip,  int port);
	
	/**
	 * Load timeslot list.
	 *
	 * @param obj the obj
	 * @return the int
	 */
	public static native int loadTimeslotList(Agenda obj);
	
	/**
	 * Disconnect smart space.
	 */
	public static native void disconnectSmartSpace();
	
	/**
	 * Gets the services info.
	 *
	 * @param menu the menu
	 * @return the services info
	 */
	public static native int getServicesInfo(ServicesMenu menu);
	
	/**
	 * User registration.
	 *
	 * @param userName the user name
	 * @param password the password
	 * @return the int
	 */
	public static native int  userRegistration(String userName, String password);
	
	/**
	 * Load presentation.
	 *
	 * @param presentationUuid the presentation uuid
	 * @param projector the projector
	 * @return the int
	 */
	public static native int loadPresentation(Projector projector);
	
	/**
	 * Inits the subscription.
	 *
	 * @return the int
	 */
	public static native int initSubscription();
	public static native int startConference();
	public static native int endConference();
	public static native void getProjectorClassObject();
	public static native int showSlide(int slideNumber);
	public static native int endPresentation();
	public static native int getCurrentTimeslotIndex();
	public static native boolean checkSpeakerState();
	public static native String getMicServiceIP();
	public static native String getMicServicePort();
	public static native String getSpeakerName();
	
	static {
		System.loadLibrary("sslog");
	}
	
	
	@Override
	protected void onResume() {
		super.onResume();
		SharedPreferences prefs = getSharedPreferences("srclient_conf", Context.MODE_PRIVATE);
		
		try {
			String ip = prefs.getString("ip", "192.168.112.109");
			String username = prefs.getString("username", "vdovenko");
			String password = prefs.getString("password", "vdovenko");
			
			int timeout = prefs.getInt(SettingsMenu.TIMEOUT_SCREEN_PREF, 
					SettingsMenu.defaultTimeout);
			
			android.provider.Settings.System.putInt(getContentResolver(), 
					Settings.System.SCREEN_OFF_TIMEOUT, timeout);
			
			editIP.setText(ip);
			editName.setText(username);
	        editPassword.setText(password);
		} catch(ClassCastException e) {
			e.printStackTrace();
			editIP.setText("192.168.0.20");
			editName.setText("");
	        editPassword.setText("");
		}
		
		if(connectionState == 1 && isRegistered)
			loadAgenda();
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		SharedPreferences prefs = getSharedPreferences("srclient_conf", Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		editor.putString("ip", editIP.getText().toString());
		editor.putString("username", editName.getText().toString());
		editor.putString("password", editPassword.getText().toString());
		editor.commit();
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.kp_interface);
        
        SharedPreferences prefs = getSharedPreferences("srclient_conf", Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		// Save system timeout screen value
		int screenTimeout = android.provider.Settings.System.getInt(getContentResolver(), 
				Settings.System.SCREEN_OFF_TIMEOUT , SettingsMenu.defaultTimeout);
		
		editor.putInt(SettingsMenu.SYSTEM_TIMEOUT_SCREEN_PREF, screenTimeout);
		editor.commit();
        
        connectBtn = (Button) findViewById (R.id.connectBtn);
        connectBtn.setOnClickListener(this);
        
        editName = (EditText) findViewById (R.id.editName);
        editPassword = (EditText) findViewById (R.id.editPassword);
        editIP = (EditText) findViewById (R.id.editIP);
        
        //chBoxAnonim = (CheckBox) findViewById (R.id.checkBoxAnonim);
        //chBoxAnonim.setChecked(false);
        
        isChairman = false;
        isRegistered = false;
	}
	
	
	/* (non-Javadoc)
	 * @see android.view.View.OnClickListener#onClick(android.view.View)
	 */
	public void onClick(View view) {
		
		String name = editName.getText().toString();
		String password = editPassword.getText().toString();
		ip = editIP.getText().toString();
		
		if(ip.equals("")) {
			Toast.makeText(this, "Please enter IP address", Toast.LENGTH_SHORT).show();
			return;
		}
		
		if((name.equals("") || password.equals("")) 
				&& !chBoxAnonim.isChecked()) {
			Toast.makeText(this, "Please fill all fields", Toast.LENGTH_SHORT).show();
			return;
		}
		
		switch(view.getId()) {
			case R.id.connectBtn:
				
				if(connectionState != 1) {

					if(connectSmartSpace("X", ip, 10010) != 0) {
						Toast.makeText(this, "Connection failed", Toast.LENGTH_SHORT).show();
						return;
					}
					
					connectionState = 1;
					Log.i("Connection", "DONE");
					
					if(initSubscription() != 0)
						Log.e("Java KP", "Init subscription failed");
					Log.i("InitSbcr", "DONE");
				}

				if(!isRegistered) {
					//if(!chBoxAnonim.isChecked()) {
						int ret_value = userRegistration(name, password);
						
						if(ret_value == 0) {
							Log.i("Java KP", "Registration successful");

							if(password.equals("chairman"))
								isChairman = true;
							isRegistered = true;
						
						} else if(ret_value == 1) {
							Toast.makeText(this, "Such login is already exists", Toast.LENGTH_SHORT).show();
							return;
							
						} else {
							Toast.makeText(this, "Registration failed due to connection problems", 
									Toast.LENGTH_SHORT).show();
							return;
						}
						
					//} else
						//isSpectator = true;
					
					Log.i("Registration", "DONE");
					
				}

				try {
				    Thread.sleep(400);
				} catch(InterruptedException ex) {
				    Thread.currentThread().interrupt();
				}
				
				loadAgenda();
				break;
		}
	}
	
	public void loadAgenda() {
		Intent intent = new Intent();
		intent.setClass(this, Agenda.class);
		startActivity(intent);
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
		
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			SharedPreferences prefs = getSharedPreferences("srclient_conf", Context.MODE_PRIVATE);
			
			int timeout = prefs.getInt(SettingsMenu.SYSTEM_TIMEOUT_SCREEN_PREF, 
					SettingsMenu.defaultTimeout);
			
			android.provider.Settings.System.putInt(getContentResolver(), 
					Settings.System.SCREEN_OFF_TIMEOUT, timeout);
		}
		
		return super.onKeyDown(keyCode, event);
	}
}
