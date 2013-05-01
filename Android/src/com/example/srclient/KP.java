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

public class KP extends Activity 
	implements View.OnClickListener {
	
	Button connectBtn;
	EditText editName;
	EditText editPassword;
	CheckBox chBoxAnonim;
	static int connectionState;
	ArrayList<String> timeslotList;
	String uuid;
	
	/*
	 * 	Native functions prototypes
	 */
	public static native int connectSmartSpace(String hostname, String ip,  int port);
	public static native int loadTimeslotList(Agenda obj);
	public static native void disconnectSmartSpace();
	public static native int getServicesInfo(ServicesMenu menu);
	public static native int  userRegistration(String userName, String password);
	public static native int loadPresentation(String presentationUuid, Projector projector);
	
	
	static {
		System.loadLibrary("sslog");
	}
	
	
	public KP() {}
	
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
	}
	
	/* Push button handler */
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
					// 192.168.0.1 - hostapd
					// 192.168.112.109 - smart room SIB
					if(connectSmartSpace("X", "10.0.2.2", 10010) != 0) {
						Toast.makeText(this, "Connection failed", Toast.LENGTH_SHORT).show();
						return;
					} else
						connectionState = 1;
					
					if(!chBoxAnonim.isChecked()) {
						if(userRegistration(name, password) == 0)
							Log.i("Java KP", "Registration successful");
						
						else {
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
}
