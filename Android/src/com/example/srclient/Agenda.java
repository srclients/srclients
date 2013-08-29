package com.example.srclient;

import android.app.AlertDialog;
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
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.content.DialogInterface;
import android.content.Intent;

import java.io.InputStream;
import java.net.URL;

import android.graphics.drawable.Drawable;
import java.lang.Thread;


public class Agenda extends ListActivity {
	
	public static int agendaCreated = 0;
	public static boolean conferenceStarted = false;
	public static boolean conferenceEnded = false;
	public static int currentTimeslotIndex = -1;
	
	private static ArrayList<Timeslot> list;
	private static boolean updated = false;
	
	
	private ListAdapter adapter;
	
	Drawable imgDefault;
	String absentImg = "absentImage";
	int connectionState = 0;
	
	
	public Agenda() { }
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	
	@Override
	protected void onPause() {
		super.onPause();
		
		agendaCreated = 0;
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		
		int value = KP.getCurrentTimeslotIndex(); 
		
		if(value != currentTimeslotIndex) {
			currentTimeslotIndex = value;
			updateAgenda();
		}
		
		agendaCreated = 1;
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		setSelection(currentTimeslotIndex - 1);
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		imgDefault = this.getResources().getDrawable(R.drawable.user);
		
		if(savedInstanceState == null || updated) {
			prepareAgendaData();
			
			// Transition to Projector if speaker
			if(KP.checkSpeakerState()) {
				Intent intent = new Intent();
				intent.setClass(this, Projector.class);
				startActivity(intent);
			}
			
			updated = false;
		}
		
		setCurrentTimeslot(KP.getCurrentTimeslotIndex());
        
		adapter = new AgendaAdapter(
        		this, list, R.layout.agenda_interface, 
        		new String[] {Timeslot.NAME, Timeslot.TITLE, Timeslot.IMG},
        		new int[] {R.id.speakerName, R.id.presentationTitle, R.id.avatar});

		((SimpleAdapter) adapter).setViewBinder(new AgendaViewBinder());
		setListAdapter(adapter);
		
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
	public void addTimeslotItemToList(final String name, final String title, 
			final String img) throws InterruptedException {
		
		if(!img.equals(absentImg)) {
			
			Thread t = new Thread() {
					@Override
					public void run() {
							Drawable imgAvatar = loadImage(img);
							list.add(new Timeslot(name, title, imgAvatar));
					};
			};
			t.start();
			t.join();
			
		} else
				list.add(new Timeslot(name, title, imgDefault));
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
		
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.agenda_menu, menu);
		
		if(!KP.isChairman) {
			menu.findItem(R.id.conferenceStart).setVisible(false);
			menu.findItem(R.id.conferenceEnd).setVisible(false);

		}
			
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		
		if(conferenceStarted) {
			menu.findItem(R.id.conferenceStart).setEnabled(false);
			menu.findItem(R.id.conferenceEnd).setEnabled(true);
		
		} else if(conferenceEnded)
			menu.findItem(R.id.conferenceStart).setEnabled(true);
		
		return true;
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
	 */
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		
		switch(item.getItemId()) {
		
			case R.id.services:
				Intent intentServices = new Intent();
				intentServices.setClass(this, ServicesMenu.class);
				startActivity(intentServices);
				break;
			
			case R.id.settings:
				Intent intentSettings = new Intent();
				intentSettings.setClass(this, SettingsMenu.class);
				startActivity(intentSettings);
				break;
				
			case R.id.conferenceStart:
				item.setEnabled(false);
				if(startConference() != 0)
					Toast.makeText(this, "Start conference failed", Toast.LENGTH_SHORT).show();
				conferenceStarted = true;
				break;
				
			case R.id.conferenceEnd:
				if(endConference() != 0)
					Toast.makeText(this, "End conference failed", Toast.LENGTH_SHORT).show();
				conferenceEnded = true;
				conferenceStarted = false;
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
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			AlertDialog.Builder dialog = new AlertDialog.Builder(this);
			dialog.setTitle(R.string.exitClientTitle);
			dialog.setMessage(R.string.exitClientQuestion);
			dialog.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			
				@Override
				public void onClick(DialogInterface dialog, int which) {
					agendaCreated = 0;
					KP.disconnectSmartSpace();
					KP.connectionState = -1;
					KP.isRegistered = false;
					finish();
				}
			});
			
			dialog.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
				
				@Override
				public void onClick(DialogInterface dialog, int which) {
					return;
				}
			});
			dialog.show();
		}
		
		return super.onKeyDown(keyCode, event);
	}
	
	
	public void setCurrentTimeslot(int index) {
		currentTimeslotIndex = index;
	}
	
	public View getView(int position, View convertView, ViewGroup parent) {
		
		return convertView;
	}
}
