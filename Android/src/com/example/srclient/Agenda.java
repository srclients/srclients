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


public class Agenda extends ListActivity {

	private static final int IDM_SERVICES = 100;
	private static final int IDM_SETTINGS = 101;
	private ArrayList<TimeslotItem> list;
	private ListAdapter adapter;
	Drawable imgDefault;
	String absentImg = "http://upload.wikimedia.org/wikipedia/commons/3/36/Bonhomme_crystal_marron.png";
	int connectionState = 0;
	
	public Agenda() {		
		ArrayList<TimeslotItem> list_tmp = new ArrayList<TimeslotItem>();
		
		list = new ArrayList<TimeslotItem>();
		
		new Thread() {
			public void run() {
				imgDefault = loadImage(absentImg);				
			};
		}.start();
		
		
		if(connectionState == 0) {
			
			if(KP.loadTimeslotList(this) == -1) {
				Log.i("Agenda GUI", "Fill agenda fail");
				finish();
			}
			
			list_tmp = (ArrayList<TimeslotItem>)list.clone();
			list.clear();
			
			/* Reverse timeslot list */
			for(int i = list_tmp.size() - 1; i >= 0; i--) {
				list.add(new TimeslotItem(list_tmp.get(i)));
			}
		}
		
		
		connectionState = 1;
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        
		adapter = new SimpleAdapter(
        		this, list, R.layout.agenda_interface, 
        		new String[] {TimeslotItem.NAME, TimeslotItem.TITLE, 
        				TimeslotItem.DURATION, TimeslotItem.IMG}, 
        		new int[] {R.id.speakerName, R.id.presentationTitle, 
        				R.id.duration, R.id.avatar});

		((SimpleAdapter) adapter).setViewBinder(new AgendaViewBinder());
        setListAdapter(adapter);
	}
	
	/* Addition timeslot item to timeslot Java list */
	 synchronized public void addTimeslotItemToList(final String name, final String duration, 
			 final String title, final String img) {
		
		if(!img.equals(absentImg)) {
			
				new Thread() {
					@Override
					public void run() {
						synchronized(list) {
							Drawable imgAvatar = loadImage(img);
							list.add(new TimeslotItem(name, duration, title, imgAvatar));
						}
					}
				}.start();
			
		} else
			synchronized(list) {
				list.add(new TimeslotItem(name, duration, title, imgDefault));
			}	
	}
	
	
	/* Loads image by url */
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
	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(Menu.NONE, IDM_SERVICES, Menu.NONE, R.string.menu_services);
		menu.add(Menu.NONE, IDM_SETTINGS, Menu.NONE, R.string.menu_settings);
		
		return super.onCreateOptionsMenu(menu);
	}
	
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
		}
		
		return super.onOptionsItemSelected(item);
	}
	
}
