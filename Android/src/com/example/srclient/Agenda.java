package com.example.srclient;

import android.app.ListActivity;
import android.os.Bundle;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;
import android.widget.Toast;
import android.view.View;
import java.util.ArrayList;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.content.Intent;
import android.content.SharedPreferences;

import java.io.InputStream;
import java.net.URL;

import android.graphics.Color;
import android.graphics.drawable.Drawable;
import java.lang.Thread;
import android.content.Context;


// TODO: Auto-generated Javadoc
/**
 * The Class Agenda.
 */
public class Agenda extends ListActivity {

	private static final int IDM_SERVICES = 100;	
	private static final int IDM_SETTINGS = 101;
	private static final int IDM_STARTCONFERENCE = 102;
	private static final int IDM_ENDCONFERENCE = 103;
	
	public static int agendaCreated = 0;
	
	private static ArrayList<Timeslot> list;
	private static boolean updated = false;
	
	private ListAdapter adapter;
	
	Drawable imgDefault;
	String absentImg = "absentImage";
	int connectionState = 0;
	
	/**
	 * Instantiates a new agenda.
	 *
	 * @throws InterruptedException the interrupted exception
	 */
	public Agenda() { }
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		imgDefault = this.getResources().getDrawable(R.drawable.user);
		
		if(savedInstanceState == null || updated) {
			updated = false;
			prepareAgendaData();
		}
        
		adapter = new SimpleAdapter(
        		this, list, R.layout.agenda_interface, 
        		new String[] {Timeslot.NAME, Timeslot.TITLE, 
        				Timeslot.DURATION, Timeslot.IMG},
        		new int[] {R.id.speakerName, R.id.presentationTitle, 
        				R.id.duration, R.id.avatar});

		((SimpleAdapter) adapter).setViewBinder(new AgendaViewBinder());
		setListAdapter(adapter);
		
		//setCurrentTimeslot(KP.getCurrentTimeslotIndex());
		
		agendaCreated = 1;
	}
	
	
	 /**
	 * Adds the timeslot item to list.
	 *
	 * @param name the name
	 * @param duration the duration
	 * @param title the title
	 * @param img the img
	 * @throws InterruptedException 
	 */
	public void addTimeslotItemToList(final String name, final String duration, 
			 final String title, final String img) throws InterruptedException {
		
		if(!img.equals(absentImg)) {
			
			Thread t = new Thread() {
					@Override
					public void run() {
						//synchronized(list) {
							Drawable imgAvatar = loadImage(img);
							list.add(new Timeslot(name, duration, title, imgAvatar));
						//}
					};
			};
			t.start();
			t.join();
			
		} else
			//synchronized(list) {
				list.add(new Timeslot(name, duration, title, imgDefault));
			//}
	}
	
	
	/**
	 * Load image.
	 *
	 * @param link the link
	 * @return the drawable
	 */
	synchronized public Drawable loadImage(String link) {
		Drawable imgDrawable = null;
		Log.i("loadImage", link);

		try {			
			InputStream is = (InputStream) new URL(link).getContent();
			imgDrawable = Drawable.createFromStream(is, "avatar");
			
			if(imgDrawable == null) {
				Log.e("Java agenda", "imgDrawable = null");
				return null;
			}
			
		} catch (Exception e) {
			e.printStackTrace();
		}

		
		return imgDrawable;
	}
	
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(Menu.NONE, IDM_SERVICES, Menu.NONE, R.string.menu_services);
		//menu.add(Menu.NONE, IDM_SETTINGS, Menu.NONE, R.string.menu_settings);
		
		//if(KP.isChairman) {
			menu.add(Menu.NONE, IDM_STARTCONFERENCE, Menu.NONE, R.string.startConference);
			menu.add(Menu.NONE, IDM_ENDCONFERENCE, Menu.NONE, R.string.endConference);
		//}
		
		return super.onCreateOptionsMenu(menu);
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
	 */
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		
		switch(item.getItemId()) {
		
			case IDM_SERVICES:
				Intent intent = new Intent();
				intent.setClass(this, ServicesMenu.class);
				startActivity(intent);
				break;
			
			case IDM_SETTINGS:
				break;
				
			case IDM_STARTCONFERENCE:
				if(startConference() != 0)
					Toast.makeText(this, "Start conference failed", Toast.LENGTH_SHORT).show();
				item.setEnabled(false);
				break;
				
			case IDM_ENDCONFERENCE:
				if(endConference() != 0)
					Toast.makeText(this, "End conference failed", Toast.LENGTH_SHORT).show();
				item.setEnabled(false);
				break;
		}
		
		return super.onOptionsItemSelected(item);
	}
	
	public void updateAgenda() {
		agendaCreated = 0;
		updated = true;
		Intent restartIntent = getIntent();
		finish();
		startActivity(restartIntent);
	}
	
	public int startConference() {
		return KP.startConference();
	}
	
	public int endConference() {
		return KP.endConference();
	}
	
	public void startPresentation() {
		Intent intent = new Intent();
		intent.setClass(this, Projector.class);
		startActivity(intent);
	}
	
	public int prepareAgendaData() {
		list = new ArrayList<Timeslot>();
		
		if(connectionState == 0) {
			if(KP.loadTimeslotList(this) == -1) {
				Log.i("Agenda GUI", "Fill agenda fail");
				finish();
				return -1;
			}
		}
		
		connectionState = 1;
		
		return 0;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)  {
	    //if (keyCode == KeyEvent.KEYCODE_BACK) {
	    	//KP.disconnectSmartSpace();
	    	//connectionState = -1;
	    	//finish();
	    //}
	    return super.onKeyDown(keyCode, event);
	}
	
	/*
	public void setCurrentTimeslot(int index) {
		long id = adapter.getItemId(index);
		View item = (View) findViewById ((int)id);
		Log.i("currentTimeslot", String.valueOf(index));
		
		if(item != null)
			item.setBackgroundColor(Color.BLUE);
		else
			Log.e("setCurTimeslot", "item not found");
	}*/
}
