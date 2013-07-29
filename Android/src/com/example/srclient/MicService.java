package com.example.srclient;

import android.app.Service;
import android.os.IBinder;
import android.content.Intent;

public class MicService extends Service {

	public IBinder onBind(Intent intent) {
		return null;
	}
}
