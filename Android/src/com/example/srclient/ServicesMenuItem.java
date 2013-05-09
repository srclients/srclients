/*
 * 
 */
package com.example.srclient;

import java.util.HashMap;

// TODO: Auto-generated Javadoc
/**
 * The Class ServicesMenuItem.
 */
public class ServicesMenuItem extends HashMap<String, String> {
	
	/** The Constant serialVersionUID. */
	private static final long serialVersionUID = 1L;
	
	/** The Constant SERVICE_NAME. */
	public static final String SERVICE_NAME = "name";
	
	/** The Constant SERVICE_DESCR. */
	public static final String SERVICE_DESCR = "description";
	
	/**
	 * Instantiates a new services menu item.
	 */
	public ServicesMenuItem() {}
	
	/**
	 * Instantiates a new services menu item.
	 *
	 * @param name the name
	 * @param description the description
	 */
	public ServicesMenuItem(String name, String description) {
		super();
		super.put(SERVICE_NAME, name);
		super.put(SERVICE_DESCR, description);
	}
}

