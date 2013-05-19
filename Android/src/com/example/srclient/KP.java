package com.example.srclient;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.Toast;
import android.util.Log;
import android.view.View;
import android.content.Intent;
import java.util.ArrayList;
import android.widget.EditText;
import android.widget.CheckBox;
import android.view.KeyEvent;

// TODO: Auto-generated Javadoc
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
	
	/** The ch box anonim. */
	CheckBox chBoxAnonim;
	
	/** The connection state. */
	static int connectionState;
	
	static boolean isChairman;
	
	/** The timeslot list. */
	ArrayList<String> timeslotList;
	
	/** The uuid. */
	String uuid;
	
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
	public static native int loadPresentation(String presentationUuid, Projector projector);
	
	/**
	 * Inits the subscription.
	 *
	 * @return the int
	 */
	public native int initSubscription();
	public static native int startConference();
	public static native int endConference();
	
	
	static {
		System.loadLibrary("sslog");
	}
	
	
	/**
	 * Instantiates a new kp.
	 */
	public KP() {}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.kp_interface);
        
        connectBtn = (Button) findViewById (R.id.connectBtn);
        connectBtn.setOnClickListener(this);
        
        editName = (EditText) findViewById (R.id.editName);
        editPassword = (EditText) findViewById (R.id.editPassword);
        
        chBoxAnonim = (CheckBox) findViewById (R.id.checkBoxAnonim);
        chBoxAnonim.setChecked(false);
        
        editName.setText("vdovenko");
        editPassword.setText("vdovenko");
        
        isChairman = false;
	}
	
	
	/* (non-Javadoc)
	 * @see android.view.View.OnClickListener#onClick(android.view.View)
	 */
	public void onClick(View view) {
		
		String name = editName.getText().toString();
		String password = editPassword.getText().toString();
		
		Intent intent = new Intent();
		intent.setClass(this, Agenda.class);
		
		if(name.equals("") || password.equals("")) {
			Toast.makeText(this, "Please fill all fields", Toast.LENGTH_SHORT).show();
			return;
		}
		
		switch(view.getId()) {
			case R.id.connectBtn:
				
				/* If already connected */
				if(connectionState != 1) {
					
					// 10.0.2.2 - local; 
					// 172.21.0.181 - it-guest
					// 192.168.112.109 - smart room SIB
					if(connectSmartSpace("X", "192.168.0.20", 10010) != 0) {
						Toast.makeText(this, "Connection failed", Toast.LENGTH_SHORT).show();
						return;
					}
					
					if(initSubscription() != 0)
						Log.e("Java KP", "Init subscription failed");
					
					if(!chBoxAnonim.isChecked()) {
						if(userRegistration(name, password) == 0) {
							Log.i("Java KP", "Registration successful");
							connectionState = 1;
							if(name.equals("vdovenko") && password.equals("vdovenko"))
								isChairman = true;
						} else {
							Toast.makeText(this, "Registration failed", Toast.LENGTH_SHORT).show();
							return;
						}
					}
				} else
					Toast.makeText(this, "You are already connected", Toast.LENGTH_SHORT).show();
				
				startActivity(intent);
				break;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
	    if (keyCode == KeyEvent.KEYCODE_BACK) {
	    	KP.disconnectSmartSpace();
	    	connectionState = -1;
	    }
	    return super.onKeyDown(keyCode, event);
	}
}
