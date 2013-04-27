package com.example.srclient;

import java.util.HashMap;

import android.graphics.drawable.Drawable;

public class TimeslotItem extends HashMap<String, Object> {
	
	private static final long serialVersionUID = 1L;
	
	public static final String NAME = "name";
	public static final String DURATION = "duration";
	public static final String TITLE = "title";
	public static final String IMG = "img";
	
	public String personUuid;
	
	
	public TimeslotItem() {}
	
	public TimeslotItem(String name, String duration, String title, String img) {
		super();
		super.put(NAME, name);
		super.put(DURATION, duration);
		super.put(TITLE, title);
		super.put(IMG, img);
	}
	
	public TimeslotItem(String name, String duration, String title, Drawable img) {
		super();
		super.put(NAME, name);
		super.put(DURATION, duration);
		super.put(TITLE, title);
		super.put(IMG, img);
	}
	
	public TimeslotItem(TimeslotItem item) {
		super();
		super.put(NAME, item.get(item.NAME));
		super.put(DURATION, item.get(item.DURATION));
		super.put(TITLE, item.get(item.TITLE));
		super.put(IMG, item.get(item.IMG));
	}
	
	public void setPersonUuid(String uuid) {
		personUuid = uuid;
	}
}
