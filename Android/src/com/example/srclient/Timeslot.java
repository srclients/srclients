/*
 * 
 */
package com.example.srclient;

import java.util.HashMap;

import android.graphics.drawable.Drawable;

// TODO: Auto-generated Javadoc
/**
 * The Class TimeslotItem.
 */
public class Timeslot extends HashMap<String, Object> {
	
	/** The Constant serialVersionUID. */
	private static final long serialVersionUID = 1L;
	
	/** The Constant NAME. */
	public static final String NAME = "name";
	
	/** The Constant DURATION. */
	public static final String DURATION = "duration";
	
	/** The Constant TITLE. */
	public static final String TITLE = "title";
	
	/** The Constant IMG. */
	public static final String IMG = "img";
	
	/** The person uuid. */
	public String personUuid;
	
	
	/**
	 * Instantiates a new timeslot item.
	 */
	public Timeslot() {}
	
	/**
	 * Instantiates a new timeslot item.
	 *
	 * @param name the name
	 * @param duration the duration
	 * @param title the title
	 * @param img the img
	 */
	public Timeslot(String name, String duration, String title, String img) {
		super();
		super.put(NAME, name);
		super.put(DURATION, duration);
		super.put(TITLE, title);
		super.put(IMG, img);
	}
	
	/**
	 * Instantiates a new timeslot item.
	 *
	 * @param name the name
	 * @param duration the duration
	 * @param title the title
	 * @param img the img
	 */
	public Timeslot(String name, String duration, String title, Drawable img) {
		super();
		super.put(NAME, name);
		super.put(DURATION, duration);
		super.put(TITLE, title);
		super.put(IMG, img);
	}
	
	/**
	 * Instantiates a new timeslot item.
	 *
	 * @param item the item
	 */
	public Timeslot(Timeslot item) {
		super();
		super.put(NAME, item.get(item.NAME));
		super.put(DURATION, item.get(item.DURATION));
		super.put(TITLE, item.get(item.TITLE));
		super.put(IMG, item.get(item.IMG));
	}
	
	/**
	 * Sets the person uuid.
	 *
	 * @param uuid the new person uuid
	 */
	public void setPersonUuid(String uuid) {
		personUuid = uuid;
	}
}
