package com.example.srclient;

import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.ImageView;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.RelativeLayout;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.content.Context;
import java.net.URLConnection;
import android.content.BroadcastReceiver;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.lang.Thread;
import java.net.URL;

/**
 * The Class Projector.
 */
public class Projector extends Activity 
	implements View.OnClickListener {
	
	private static final int IDM_SERVICES = 100;
	private static final int IDM_SETTINGS = 101;
	private static final int IDM_END_PRESENTATION = 102;
	
	public static final String BROADCAST_STATUS_SERVICE = "com.example.srclient.stopMicService";
	public static final String SERVICE_STATUS = "status";
	
	public static int projectorCreated = 0;

	/** The linear layout. */
	RelativeLayout linearLayout;
	
	/** The left arrow btn. */
	ImageView leftArrowBtn;
	
	/** The right arrow btn. */
	ImageView rightArrowBtn;
	
	/** The pause btn. */
	ImageView stopBtn;
	
	/** The presentation image. */
	ImageView presentationImage;
	
	ImageView microphoneBtn;
	
	ImageView resfreshBtn;
	
	/** The slide image drawable. */
	Drawable slideImageDrawable;
	
	/** The layout is active. */
	boolean layoutIsActive;
	
	/** The slide image link. */
	String slideImageLink = "";
	
	/** The slide number. */
	int slideNumber;
	
	/** The slide count. */
	int slideCount;
	
	Handler uiHandler;
	
	Message msg;
	
	boolean micServiceStarted = false;
	
	Context context;
	
	public boolean isSpeaker = false;
	
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
		
		slideImageLink = slideImageLink.replace("127.0.0.1", KP.ip);
		
		Thread thread = new Thread() {
			@Override
			public void run() {
				slideImageDrawable = loadImage(slideImageLink);				
			};
		};
		
		thread.start();
		try {
			thread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		linearLayout = (RelativeLayout) findViewById (R.id.presMenu);
		
		leftArrowBtn = (ImageView) findViewById (R.id.btnBack);
		leftArrowBtn.setOnClickListener(this);
		rightArrowBtn = (ImageView) findViewById (R.id.btnForward);
		rightArrowBtn.setOnClickListener(this);
		stopBtn = (ImageView) findViewById (R.id.btnStop);
		stopBtn.setOnClickListener(this);
		
		microphoneBtn = (ImageView) findViewById (R.id.mic);
		microphoneBtn.setOnClickListener(this);
		
		resfreshBtn = (ImageView) findViewById (R.id.refresh);
		resfreshBtn.setOnClickListener(this);
		
		presentationImage = (ImageView) findViewById (R.id.presImage);
		presentationImage.setImageDrawable(slideImageDrawable);
		presentationImage.setOnClickListener(this);
		
		if(!KP.isSpectator)
			isSpeaker = KP.checkSpeakerState();
		
		if(!isSpeaker) {
			linearLayout.setVisibility(RelativeLayout.INVISIBLE);
			microphoneBtn.setVisibility(RelativeLayout.INVISIBLE);
			presentationImage.setClickable(false);
		} else if(isSpeaker || KP.isChairman) {
			linearLayout.setVisibility(RelativeLayout.VISIBLE);
			microphoneBtn.setVisibility(RelativeLayout.VISIBLE);
			presentationImage.setClickable(true);
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
				
			case R.id.btnForward:
				if(nextSlide() != 0)
					Log.e("Projector", "NEXT SLIDE error");
				break;
				
			case R.id.btnBack:
				previousSlide();
				break;
				
			case R.id.btnStop:
				endPresentation();
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
				
			case R.id.refresh:
				try {
					KP.loadPresentation(this);
					updateProjector();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				break;
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
			URL url = new URL(link);
			URLConnection connection = url.openConnection();
			InputStream in = new BufferedInputStream(connection.getInputStream());
			imgDrawable = Drawable.createFromStream(in, null);
			
			if(imgDrawable == null) {
				Log.e("Java projector", "imgDrawable = null");
				return null;
			}
			
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
		menu.add(Menu.NONE, IDM_SETTINGS, Menu.NONE, R.string.menu_settings);
		menu.add(Menu.NONE, IDM_END_PRESENTATION, Menu.NONE, R.string.menu_endPresentation);
		
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
				
			case IDM_END_PRESENTATION:
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
		
		if(!KP.isSpectator)
			isSpeaker = KP.checkSpeakerState();
		
		if(!isSpeaker && micIsActive) {
			micIsActive = false;
			stopService(new Intent(this, MicService.class));
		}
		
		uiHandler = new Handler(Looper.getMainLooper()) {
			@Override
			public void handleMessage(Message inMsg) {
				presentationImage.setImageDrawable(slideImageDrawable);
				presentationImage.invalidate();
				
				if(!isSpeaker) {
					linearLayout.setVisibility(RelativeLayout.INVISIBLE);
					microphoneBtn.setVisibility(RelativeLayout.INVISIBLE);
					presentationImage.setClickable(false);
				} else if(isSpeaker || KP.isChairman) {
					linearLayout.setVisibility(RelativeLayout.VISIBLE);
					microphoneBtn.setVisibility(RelativeLayout.VISIBLE);
					presentationImage.setClickable(true);
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
