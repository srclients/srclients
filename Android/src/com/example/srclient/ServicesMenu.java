/*
 * 
 */
package com.example.srclient;

import java.util.ArrayList;
import android.app.ListActivity;
import android.content.Intent;
import android.util.Log;
import android.os.Bundle;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;
import android.widget.ListView;
import android.view.View;

// TODO: Auto-generated Javadoc
/**
 * The Class ServicesMenu.
 */
public class ServicesMenu extends ListActivity {

	/** The agenda service state. */
	public int agendaServiceState;
	
	/** The projector service state. */
	public int projectorServiceState;
	
	/** The list. */
	private ArrayList<ServicesMenuItem> list;
	
	/**
	 * Instantiates a new services menu.
	 */
	public ServicesMenu() {
		if(KP.getServicesInfo(this) == -1) {
			Log.e("servicesClass", "Faild to init services");
			finish();
		}
		
		list = new ArrayList<ServicesMenuItem>();
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		if(agendaServiceState == 1)
			list.add(new ServicesMenuItem(getResources().getString(R.string.agenda), 
					getResources().getString(R.string.agenda_descr)));
		
		if(projectorServiceState == 1)
			list.add(new ServicesMenuItem(getResources().getString(R.string.projector), 
					getResources().getString(R.string.projector_descr)));
		
		ListAdapter adapter = new SimpleAdapter(
        		this, list, R.layout.services_interface, 
        		new String[] {ServicesMenuItem.SERVICE_NAME, 
        				ServicesMenuItem.SERVICE_DESCR}, 
        		new int[] {R.id.service_name, R.id.service_descr});
        
        setListAdapter(adapter);
	}
	
	/* (non-Javadoc)
	 * @see android.app.ListActivity#onListItemClick(android.widget.ListView, android.view.View, int, long)
	 */
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		Intent intent = new Intent();
		String serviceName = l.getItemAtPosition(position).toString();
		
		if(serviceName.contains("Agenda")) {
			intent.setClass(this, Agenda.class);
			startActivity(intent);
		} else if(serviceName.contains("Projector")) {
			intent.setClass(this, Projector.class);
			startActivity(intent);
		}
		
	}
}
