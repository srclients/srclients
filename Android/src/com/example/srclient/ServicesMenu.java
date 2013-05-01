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

public class ServicesMenu extends ListActivity {

	public int agendaServiceState;
	public int projectorServiceState;
	
	private ArrayList<ServicesMenuItem> list;
	
	public ServicesMenu() {
		if(KP.getServicesInfo(this) == -1) {
			Log.e("servicesClass", "Faild to init services");
			finish();
		}
		
		list = new ArrayList<ServicesMenuItem>();
	}
	
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
