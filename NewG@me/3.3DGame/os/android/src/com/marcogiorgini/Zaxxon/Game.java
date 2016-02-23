package com.marcogiorgini.Zaxxon;

import android.app.Activity;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;

import java.io.File;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.util.Log;
import android.view.Display;
import android.view.Window;
import android.view.WindowManager;
import android.view.Surface;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.Sensor;

//@SuppressWarnings("deprecation")
public class Game extends Activity implements SensorEventListener 
{
	private SensorManager sm = null;
	private Display mDisplay = null;
	private WindowManager mWindowManager = null;
	private GameView mGLSurfaceView;
	public static final Handler mHandler = new Handler();
	
	public static final String GAME_PACKAGE_NAME = "com.marcogiorgini.Zaxxon";
	public static String GAME_DATA_FOLDER = "";
	public static String GAME_SAVE_FOLDER = "";
	
	private static int mDumpTimes = 0;
	
	private String getApkFilePath()
	{
		String apkFilePath = null;
		ApplicationInfo appInfo = null;
		PackageManager packMgmr = this.getPackageManager();
		try
		{
			appInfo = packMgmr.getApplicationInfo(GAME_PACKAGE_NAME, 0 );
			apkFilePath = appInfo.sourceDir;
		}
		catch( NameNotFoundException e)
		{
		}
		return apkFilePath;	
	}
	
	public void exitApp(final int code)
	{
		mHandler.post(new Runnable() 
		{
			public void run() 
			{
				showExitApp();
			}
		});
	}
	
	public void openUrl(final String url)
	{
		mHandler.post(new Runnable() {
			public void run() 
			{
				//System.exit(code);
				Intent urlIntent = new Intent(
                         Intent.ACTION_VIEW,
                         Uri.parse(url));
                   startActivity(urlIntent);
			}
		});
	}
	
	public void showExitApp()
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
	    builder.setMessage("Are you sure you want to exit?")
	           .setCancelable(false)
	           .setPositiveButton("Yes", new DialogInterface.OnClickListener() 
	           {
	               public void onClick(DialogInterface dialog, int id) 
	               {
	            	   System.exit(0);
	               }
	           })
	           .setNegativeButton("No", new DialogInterface.OnClickListener() 
	           {
	               public void onClick(DialogInterface dialog, int id) 
	               {
	                    dialog.cancel();
	               }
	           });
	    AlertDialog alert = builder.create();
	    alert.show();
	}
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) 
	{
		/* We like to be fullscreen */
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);    	
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		super.onCreate(savedInstanceState);
		
		setVolumeControlStream(AudioManager.STREAM_MUSIC);

        // Get an instance of the SensorManager
        sm = (SensorManager) getSystemService(SENSOR_SERVICE);
        if (sm.getSensorList(Sensor.TYPE_ACCELEROMETER).size()!=0)
        {
        	Sensor s = sm.getSensorList(Sensor.TYPE_ACCELEROMETER).get(0);
        	sm.registerListener(this, s, SensorManager.SENSOR_DELAY_GAME);
        }
		
        // Get an instance of the WindowManager
        mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        mDisplay = mWindowManager.getDefaultDisplay();
        
		GAME_DATA_FOLDER = getApkFilePath();
		GAME_SAVE_FOLDER = this.getFilesDir().toString();
		GAME_SAVE_FOLDER += File.separator;
		
		mGLSurfaceView = new GameView(this);
		setContentView(mGLSurfaceView);
		mGLSurfaceView.requestFocus();
		mGLSurfaceView.setId(1);
		GameJNI.app = Game.this;
	}
	
	@Override
	protected void onStop() 
	{
		super.onStop();
	}
	
	@Override
	protected void onDestroy() 
	{
		GameJNI.destroyGame();
		super.onDestroy();
	}

	@Override
	protected void onPause() 
	{
		super.onPause();
		GameJNI.pauseGame();
		mGLSurfaceView.onPause();
		sm.unregisterListener(this);
	}

	@Override
	protected void onResume() 
	{
		super.onResume();
		if (sm.getSensorList(Sensor.TYPE_ACCELEROMETER).size()!=0)
		{
			Sensor s = sm.getSensorList(Sensor.TYPE_ACCELEROMETER).get(0);
			sm.registerListener(this, s, SensorManager.SENSOR_DELAY_GAME);
		}
		if (mGLSurfaceView != null)
		{
			mGLSurfaceView.onResume();
		}
		GameJNI.resumeGame();
	}
	
	/*
	public void onSensorChanged(int sensor, float[] values) 
	{
        synchronized (this) 
        {
        	if (sensor == SensorManager.SENSOR_ACCELEROMETER) 
        	{
                float acc_x = values[0];
                float acc_y = values[1];
                float acc_z = values[2];
                
                if (acc_x>5.0f)
                	acc_x = 5.0f;
                else if (acc_x<-5.0f)
                	acc_x = -5.0f;
                if (acc_y>5.0f)
                	acc_y = 5.0f;
                else if (acc_y<-5.0f)
                	acc_y = -5.0f;
                if (acc_z>5.0f)
                	acc_z = 5.0f;
                else if (acc_z<-5.0f)
                	acc_z = -5.0f;
                
                float os_roll = 100.0f - (acc_x + 5.0f)/10.0f * 100.0f;
                float os_pitch = 100.0f - (acc_y + 5.0f)/10.0f * 100.0f;
                float os_z = 100.0f - (acc_z + 5.0f)/10.0f * 100.0f;
                
                GameJNI.accelEvent(os_pitch, os_roll, os_z);
        	}    
        }
    }
    
    public void onAccuracyChanged(int sensor, int accuracy) 
    {
    }
    */
	@Override
    public void onSensorChanged(SensorEvent event) 
	{
        float sensorX = 0.0f;
        float sensorY = 0.0f;
        float sensorZ = 0.0f;
		
        if (event.sensor.getType() != Sensor.TYPE_ACCELEROMETER)
            return;
 
        switch (mDisplay.getRotation()) 
        {
            case Surface.ROTATION_0:
                sensorX = event.values[0];
                sensorY = event.values[1];
                sensorZ = event.values[2];
                //textview.setText(String.valueOf( mSensorX));
                break;
            case Surface.ROTATION_90:
                sensorX = -event.values[1];
                sensorY = event.values[0];
                sensorZ = event.values[2];
                //textview.setText(String.valueOf( mSensorX));
                break;
            case Surface.ROTATION_180:
                sensorX = -event.values[0];
                sensorY = -event.values[1];
                sensorZ = event.values[2];
                //textview.setText(String.valueOf( mSensorX));
                break;
            case Surface.ROTATION_270:
                sensorX = event.values[1];
                sensorY = -event.values[0];
                sensorZ = event.values[2];
                //textview.setText(String.valueOf( mSensorX));
                break;
        }
        
        if (sensorX > 5.0f)
        	sensorX = 5.0f;
        else if (sensorX < -5.0f)
        	sensorX = -5.0f;
        
        /*
        if (++mDumpTimes>=30)
        {
        	String dump = String.format("sensorX = %.3f, sensorY = %.3f, sensorZ = %.3f", sensorX, sensorY, sensorZ);
        	Log.d("accelerometer", dump);
        	mDumpTimes=0;
        }
        */
        float os_z = 100.0f - (sensorX + 5.0f)/10.0f * 100.0f;
        GameJNI.accelEvent(0.0f, 0.0f, os_z);
 
    }
 
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) 
	{
		// TODO Auto-generated method stub
	}

}
