package com.example.srclient;

import java.util.List;
import java.util.Map;

import android.content.Context;
import android.graphics.Color;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SimpleAdapter;

public class AgendaAdapter extends SimpleAdapter {
	
	public AgendaAdapter(Context context, List<? extends Map<String, ?>> data,
			int resource, String[] from, int[] to) {
		super(context, data, resource, from, to);
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View rowView = super.getView(position, convertView, parent);
		
		if(position == Agenda.currentTimeslotIndex - 1)
			rowView.setBackgroundColor(Color.parseColor("#add8e6"));
		else
			rowView.setBackgroundColor(Color.WHITE);
		
		return rowView;
	}

}
