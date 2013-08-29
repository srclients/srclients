package com.example.srclient;

import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.RelativeLayout;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.content.Context;
import java.net.HttpURLConnection;
import android.content.BroadcastReceiver;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.lang.Thread;
import java.net.URL;

/**
 * The Class Projector.
 */
public class Projector extends Activity 
	implements View.OnClickListener, View.OnTouchListener {
	
	public static final String BROADCAST_STATUS_SERVICE = "com.example.srclient.stopMicService";
	public static final String SERVICE_STATUS = "status";
	
	public static int projectorCreated = 0;

	/** The linear layout. */
	RelativeLayout linearLayout;
	
	RelativeLayout speakerNameLayout;
	
	/** The left arrow btn. */
	ImageView leftArrowBtn;
	
	/** The right arrow btn. */
	ImageView rightArrowBtn;
	
	/** The presentation image. */
	ImageView presentationImage;
	
	ImageView microphoneBtn;
	
	ImageView refreshBtn;
	
	TextView textNumAndCount;
	
	TextView speakerNameView;
	
	/** The slide image drawable. */
	Drawable slideImageDrawable;
	
	/** The layout is active. */
	boolean layoutIsActive;
	
	/** The slide image link. */
	String slideImageLink = "";
	
	String speakerName;
	
	/** The slide number. */
	int slideNumber;
	
	/** The slide count. */
	int slideCount;
	
	Handler uiHandler;
	
	Message msg;
	
	boolean micServiceStarted = false;
	
	Context context;
	
	public static boolean isSpeaker = false;
	
	public static boolean micIsActive = false;
	
	public BroadcastReceiver bcReceiver;
	
	/**
	 * Instantiates a new projector.
	 *
	 * @throws InterruptedException the interrupted exception
	 */
	public Projector() throws InterruptedException {
		if(KP.loadPresentation(this) != 0) {
			Log.i("Projector GUI", "Projector init fail");
			return;
		}
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		SharedPreferences prefs = getSharedPreferences("srclient_conf", Context.MODE_PRIVATE);
		
		micIsActive = prefs.getBoolean("micIsActive", false);
		if(micIsActive)
			microphoneBtn.setImageResource(R.drawable.start_mic);
		
		boolean showSpeakerName = prefs.getBoolean(SettingsMenu.SHOW_SPEAKER_NAME, true);
		
		speakerNameLayout = (RelativeLayout) findViewById (R.id.presSpeaker);
		
		if(showSpeakerName)
			speakerNameLayout.setVisibility(RelativeLayout.VISIBLE);
		else
			speakerNameLayout.setVisibility(RelativeLayout.INVISIBLE);
		
		projectorCreated = 1;
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		SharedPreferences prefs = getSharedPreferences("srclient_conf", Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = prefs.edit();
		
		editor.putBoolean("micIsActive", micIsActive);
		editor.commit();
		
		projectorCreated = 0;
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.projector_interface);
		
		linearLayout = (RelativeLayout) findViewById (R.id.presMenu);
		
		leftArrowBtn = (ImageView) findViewById (R.id.btnBack);
		leftArrowBtn.setOnTouchListener(this);
		rightArrowBtn = (ImageView) findViewById (R.id.btnForward);
		rightArrowBtn.setOnTouchListener(this);
		
		microphoneBtn = (ImageView) findViewById (R.id.mic);
		microphoneBtn.setOnClickListener(this);
		
		refreshBtn = (ImageView) findViewById (R.id.refresh);
		refreshBtn.setOnTouchListener(this);
		
		presentationImage = (ImageView) findViewById (R.id.presImage);
		presentationImage.setOnClickListener(this);
		
		textNumAndCount = (TextView) findViewById (R.id.slideNumber);
		
		speakerName = KP.getSpeakerName();
		speakerNameView = (TextView) findViewById (R.id.speakerName);
		speakerNameView.setText(speakerName + " speaking");

		try {
			updateProjector();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		if(isSpeaker || KP.isChairman) {
			linearLayout.setVisibility(RelativeLayout.VISIBLE);
			microphoneBtn.setVisibility(RelativeLayout.VISIBLE);
			presentationImage.setClickable(true);
			
		} else {
			linearLayout.setVisibility(RelativeLayout.INVISIBLE);
			microphoneBtn.setVisibility(RelativeLayout.INVISIBLE);
			presentationImage.setClickable(false);
		}
		
		// Registering receiver for changing mic picture
		// if service is not available or stopped
		bcReceiver = new BroadcastReceiver() {
			public void onReceive(Context context, Intent intent) {
				boolean status = intent.getBooleanExtra(SERVICE_STATUS, false);
				
				if(!status) {
					microphoneBtn.setImageResource(R.drawable.inactive_mic);
					micIsActive = false;
				}
			}
		};
		
		IntentFilter filter = new IntentFilter(BROADCAST_STATUS_SERVICE);
		registerReceiver(bcReceiver, filter);
		
		layoutIsActive = false;
		projectorCreated = 1;
	}
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		
		unregisterReceiver(bcReceiver);
	}
	
	
	/* (non-Javadoc)
	 * @see android.view.View.OnClickListener#onClick(android.view.View)
	 */
	public void onClick(View v) {
		
		switch(v.getId()) {
			case R.id.presImage:
				if(layoutIsActive) {
					linearLayout.setVisibility(RelativeLayout.INVISIBLE);
					layoutIsActive = false;
				} else {
					linearLayout.setVisibility(RelativeLayout.VISIBLE);
					layoutIsActive = true;
				}
				break;
				
			case R.id.mic:
				if(!micIsActive) {
					microphoneBtn.setImageResource(R.drawable.start_mic);
					startService(new Intent(this, MicService.class));
					micIsActive = true;
				} else {
					microphoneBtn.setImageResource(R.drawable.inactive_mic);
					stopService(new Intent(this, MicService.class));
					micIsActive = false;
				}
				break;
		}
	}
	
	@Override
	public boolean onTouch(View view, MotionEvent event) {
		
		switch(view.getId()) {
		
			case R.id.btnForward:
				if(event.getAction() == MotionEvent.ACTION_DOWN) {
					rightArrowBtn.setImageResource(R.drawable.right_arrow_pressed);
				} else if(event.getAction() == MotionEvent.ACTION_UP) {
					rightArrowBtn.setImageResource(R.drawable.right_arrow);
					nextSlide();
				}
				break;
				
			case R.id.btnBack:
				if(event.getAction() == MotionEvent.ACTION_DOWN) {
					leftArrowBtn.setImageResource(R.drawable.left_arrow_pressed);
				} else if(event.getAction() == MotionEvent.ACTION_UP) {
					leftArrowBtn.setImageResource(R.drawable.left_arrow);
					previousSlide();
				}
				break;
				
			case R.id.refresh:
				if(event.getAction() == MotionEvent.ACTION_DOWN) {
					refreshBtn.setImageResource(R.drawable.refresh_pressed);
				
				} else if(event.getAction() == MotionEvent.ACTION_UP) {
					refreshBtn.setImageResource(R.drawable.refresh);
					try {
						KP.loadPresentation(this);
						updateProjector();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				break;
		
		}
		
		return true;
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
		HttpURLConnection connection = null;

		try {
			URL url = new URL(link);
			connection = (HttpURLConnection) url.openConnection();
			InputStream in = new BufferedInputStream(connection.getInputStream());
			imgDrawable = Drawable.createFromStream(in, null);
			
			connection.disconnect();
			
		} catch (Exception e) {
			e.printStackTrace();
			
			if(connection != null)
				connection.disconnect();
		} 
		
		return imgDrawable;
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.projector_menu, menu);
			
		if(isSpeaker || KP.isChairman)
			menu.findItem(R.id.endPresentation).setVisible(true);
		else
			menu.findItem(R.id.endPresentation).setVisible(false);
		
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		
		if(isSpeaker || KP.isChairman)
			menu.findItem(R.id.endPresentation).setVisible(true);
		else
			menu.findItem(R.id.endPresentation).setVisible(false);
		
		return super.onPrepareOptionsMenu(menu);
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
	 */
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		
		switch(item.getItemId()) {
		
			case R.id.services:
				Intent intent = new Intent();
				intent.setClass(this, ServicesMenu.class);
				startActivity(intent);
				break;
			
			case R.id.settings:
				Intent intentSettings = new Intent();
				intentSettings.setClass(this, SettingsMenu.class);
				startActivity(intentSettings);
				break;
				
			case R.id.endPresentation:
				isSpeaker = false;
				endPresentation();
				break;
		}
		
		return super.onOptionsItemSelected(item);
	}
	
	public int updateProjector() throws InterruptedException {
		
		slideImageLink = slideImageLink.replace("127.0.0.1", KP.ip);
		
		Thread thread = new Thread() {
			@Override
			public void run() {
				slideImageDrawable = loadImage(slideImageLink);
			};
		};
		
		thread.start();
		thread.join();
		
		speakerName = KP.getSpeakerName();
		isSpeaker = KP.checkSpeakerState();
		
		if(!isSpeaker && micIsActive) {
			micIsActive = false;
			stopService(new Intent(this, MicService.class));
		}
		
		uiHandler = new Handler(Looper.getMainLooper()) {
			@Override
			public void handleMessage(Message inMsg) {
				speakerNameView.setText(speakerName + " speaking");
				textNumAndCount.setText(String.valueOf(slideNumber) + "/" + 
						String.valueOf(slideCount));
				presentationImage.setImageDrawable(slideImageDrawable);
				presentationImage.invalidate();
				
				if(isSpeaker || KP.isChairman) {
					linearLayout.setVisibility(RelativeLayout.VISIBLE);
					microphoneBtn.setVisibility(RelativeLayout.VISIBLE);
					presentationImage.setClickable(true);
				} else {
					linearLayout.setVisibility(RelativeLayout.INVISIBLE);
					microphoneBtn.setVisibility(RelativeLayout.INVISIBLE);
					presentationImage.setClickable(false);
				}
			}
		};
		
		msg = new Message();
		msg.setTarget(uiHandler);
		msg.sendToTarget();
		
		return 0;
	}
	
	public int nextSlide() {
		if(slideNumber < slideCount)
			return KP.showSlide(slideNumber + 1);

		return 0;
	}
	
	public int previousSlide() {
		if(slideNumber > 1)
			return KP.showSlide(slideNumber - 1);
		
		return 0;
	}
	
	public int endPresentation() {
		return KP.endPresentation();
	}
	
	public void setSlideNumber(String number) {
		slideNumber = Integer.parseInt(number);
	}
	
	public void setSlideCount(String count) {
		slideCount = Integer.parseInt(count);
	}
	
	public void setSlideImage(String link) throws InterruptedException {
		slideImageLink = link;
	}
}
