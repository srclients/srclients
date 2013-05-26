package com.example.srclient;

import android.app.ListActivity;
import android.os.Bundle;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;
import java.util.ArrayList;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.content.Intent;
import java.io.InputStream;
import java.net.URL;
import android.graphics.drawable.Drawable;
import java.lang.Thread;


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
	
	/** The list. */
	private ArrayList<Timeslot> list;
	
	/** The adapter. */
	private ListAdapter adapter;
	
	/** The img default. */
	Drawable imgDefault;
	
	/** The absent img. */
	String absentImg = "http://upload.wikimedia.org/wikipedia/commons/3/36/Bonhomme_crystal_marron.png";
	
	/** The connection state. */
	int connectionState = 0;
	
	/**
	 * Instantiates a new agenda.
	 *
	 * @throws InterruptedException the interrupted exception
	 */
	public Agenda() throws InterruptedException {		
		ArrayList<Timeslot> list_tmp = new ArrayList<Timeslot>();
		
		list = new ArrayList<Timeslot>();
		
		Thread t = new Thread() {
			public void run() {
				imgDefault = loadImage(absentImg);				
			};
		};
		
		t.start();
		t.join();
		
		
		if(connectionState == 0) {
			
			if(KP.loadTimeslotList(this) == -1) {
				Log.i("Agenda GUI", "Fill agenda fail");
				finish();
			}
			
			list_tmp = (ArrayList<Timeslot>)list.clone();
			list.clear();
			
			/* Reverse timeslot list */
			for(int i = list_tmp.size() - 1; i >= 0; i--) {
				list.add(new Timeslot(list_tmp.get(i)));
			}
		}
		
		connectionState = 1;
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        
		adapter = new SimpleAdapter(
        		this, list, R.layout.agenda_interface, 
        		new String[] {Timeslot.NAME, Timeslot.TITLE, 
        				Timeslot.DURATION, Timeslot.IMG}, 
        		new int[] {R.id.speakerName, R.id.presentationTitle, 
        				R.id.duration, R.id.avatar});

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
	public void addTimeslotItemToList(final String name, final String duration, 
			 final String title, final String img) throws InterruptedException {
		
		if(!img.equals(absentImg)) {
			
			Thread t = new Thread() {
					@Override
					public void run() {
						synchronized(list) {
							Drawable imgAvatar = loadImage(img);
							list.add(new Timeslot(name, duration, title, imgAvatar));
						}
					};
			};
			t.start();
			t.join();
			
		} else
			synchronized(list) {
				list.add(new Timeslot(name, duration, title, imgDefault));
			}	
	}
	
	
	/* Loads image by url */
	/**
	 * Load image.
	 *
	 * @param link the link
	 * @return the drawable
	 */
	synchronized public Drawable loadImage(String link) {
		Drawable imgDrawable = null;

		
		try {			
			InputStream is = (InputStream) new URL(link).getContent();
			imgDrawable = Drawable.createFromStream(is, "avatar");
			
			if(imgDrawable == null)
				Log.e("Java agenda", "imgDrawable = null");
			
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
		
		if(KP.isChairman) {
			menu.add(Menu.NONE, IDM_STARTCONFERENCE, Menu.NONE, R.string.startConference);
			menu.add(Menu.NONE, IDM_ENDCONFERENCE, Menu.NONE, R.string.endConference);
		}
		
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
				startConference();
				break;
				
			case IDM_ENDCONFERENCE:
				KP.endConference();
				break;
		}
		
		return super.onOptionsItemSelected(item);
	}
	
	public void updateAgenda() {
		Intent restartIntent = getIntent();
		finish();
		startActivity(restartIntent);
	}
	
	public void startConference() {
		KP.startConference();
	}
	
	public void startPresentation() {
		Intent intent = new Intent();
		intent.setClass(this, Projector.class);
		startActivity(intent);
	}
}
