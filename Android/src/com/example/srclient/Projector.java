package com.example.srclient;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.ImageView;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.RelativeLayout;
import android.graphics.drawable.Drawable;

import java.io.InputStream;
import java.lang.Thread;
import java.net.URL;

// TODO: Auto-generated Javadoc
/**
 * The Class Projector.
 */
public class Projector extends Activity 
	implements View.OnClickListener {
	
	/** The Constant IDM_SERVICES. */
	private static final int IDM_SERVICES = 100;
	
	/** The Constant IDM_SETTINGS. */
	private static final int IDM_SETTINGS = 101;
	
	public static int projectorCreated = 0;
	
	/** The this object. */
	Projector thisObject = this;

	/** The linear layout. */
	RelativeLayout linearLayout;
	
	/** The left arrow btn. */
	ImageView leftArrowBtn;
	
	/** The right arrow btn. */
	ImageView rightArrowBtn;
	
	/** The pause btn. */
	ImageView pauseBtn;
	
	/** The presentation image. */
	ImageView presentationImage;
	
	/** The slide image drawable. */
	Drawable slideImageDrawable;
	
	/** The layout is active. */
	boolean layoutIsActive;
	
	/** The slide image link. */
	String slideImageLink;
	
	/** The slide number. */
	int slideNumber;
	
	/** The slide count. */
	int slideCount;
	
	
	/**
	 * Instantiates a new projector.
	 *
	 * @throws InterruptedException the interrupted exception
	 */
	public Projector() throws InterruptedException {
		if(KP.loadPresentation("", this) != 0) {
			Log.i("Projector GUI", "Projector init fail");
			return;
		}
		
		slideImageLink = slideImageLink.replace("127.0.0.1", KP.ip);
		Log.i("imageLink:", slideImageLink);
		
		Thread thread = new Thread() {
			@Override
			public void run() {
				slideImageDrawable = loadImage(slideImageLink);				
			};
		};
		
		thread.start();
		thread.join();
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.projector_interface);
		
		linearLayout = (RelativeLayout) findViewById (R.id.presMenu);
		linearLayout.setVisibility(RelativeLayout.INVISIBLE);
		
		leftArrowBtn = (ImageView) findViewById (R.id.btnBack);
		rightArrowBtn = (ImageView) findViewById (R.id.btnForward);
		pauseBtn = (ImageView) findViewById (R.id.btnPause);
		
		presentationImage = (ImageView) findViewById (R.id.presImage);
		presentationImage.setImageDrawable(slideImageDrawable);
		presentationImage.setOnClickListener(this);
		
		layoutIsActive = false;
		projectorCreated = 1;
	}
	
	
	/* (non-Javadoc)
	 * @see android.view.View.OnClickListener#onClick(android.view.View)
	 */
	public void onClick(View v) {
		// TODO: handle button click (back, pause, forward) events
		
		if(v.getId() == R.id.presImage) {
			
			if(layoutIsActive) {
				linearLayout.setVisibility(RelativeLayout.INVISIBLE);
				layoutIsActive = false;
			}
			else {
				linearLayout.setVisibility(RelativeLayout.VISIBLE);
				layoutIsActive = true;
			}
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
				Log.e("Java projector", "imgDrawable = null");
			
		} catch (Exception e) {
			e.printStackTrace();
		}

		
		return imgDrawable;
	}
	
	/* Sets image url */
	/**
	 * Sets the image link.
	 *
	 * @param link the new image link
	 */
	public void setImageLink(String link) {
		slideImageLink = link;
	}
	
	/**
	 * @brief Initialize projector when `startConference` received
	 * 
	 * @param imgLink
	 * @param slideNumber
	 * @param slideCount
	 */
	public void loadProjector(String imgLink, int slideNumber, int slideCount) {
		slideImageLink = imgLink;
		this.slideNumber = slideNumber;
		this.slideCount =slideCount;  
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(Menu.NONE, IDM_SERVICES, Menu.NONE, R.string.menu_services);
		menu.add(Menu.NONE, IDM_SETTINGS, Menu.NONE, R.string.menu_settings);
		
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
		}
		
		return super.onOptionsItemSelected(item);
	}
	
	public void updateProjector() {
		Log.i("Projector", "update start");
		Intent restartIntent = getIntent();
		finish();
		startActivity(restartIntent);
		Log.i("Projector", "update DONE");
	}
}
