package com.RetroSoft.Hataroid.Input;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

import java.io.Serializable;

public class InputCaptureView extends Activity
{
	public static final String CONFIG_EMUKEY		= "Config_EmuKey";
	//public static final String CONFIG_SYSTEMKEY	= "Config_SystemKey";
	public static final String CONFIG_MAPID			= "Config_MapID";
	public static final String CONFIG_LOCALEID		= "Config_LocaleID";
	public static final String CONFIG_CANCANCEL     = "Config_CanCancel";
	public static final String CONFIG_CALLERDATA	= "Config_CallerData";

	public static final String RESULT_KEYCODE		= "ResultKeyCode";
	public static final String RESULT_UNMAP			= "ResultUnmap";
	public static final String RESULT_PREVEMUKEY	= "ResultPrevEmuKey";
	//public static final String RESULT_PREVSYSTEMKEY = "ResultPrevSystemKey";
	public static final String RESULT_PREVMAPID		= "ResultPrevMapID";
	public static final String RESULT_CALLERDATA	= "ResultCallerData";

	//int []	_groupJoystickKeys = {VirtKeyDef.VKB_KEY_JOYUP, VirtKeyDef.VKB_KEY_JOYDOWN, VirtKeyDef.VKB_KEY_JOYLEFT, VirtKeyDef.VKB_KEY_JOYRIGHT, VirtKeyDef.VKB_KEY_JOYFIRE, VirtKeyDef.VKB_KEY_SPACE};
	//boolean	_groupCapture = false;
	
	//int		_prevSystemKey = -1;
	String			_prevMapID = null;
	
	int				_emuKey = -1;
	int				_scannedKeyCode = -1;

	int             _localeID = Input.kLocale_EN;

	Serializable	_callerData = null;

	boolean         _canCancel = true;
	Intent			_retIntent = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.inputcapture_view);

		_parseOptions(savedInstanceState);

		if (!_canCancel) {
			try {
				this.setFinishOnTouchOutside(false);
			} catch (Exception e) {
				e.printStackTrace();
			} catch (Error e) {
				e.printStackTrace();
			}
		}

		_retIntent = new Intent();
		
		View v = findViewById(R.id.capture_button_name);
		if (v != null && v instanceof TextView)
		{
			TextView tv = (TextView)v;
			String buttonText = "unknown";
			if (_emuKey >= 0 && _emuKey < VirtKeyDef.VKB_KEY_NumOf)
			{
				buttonText = VirtKeyDef.kDefs[_emuKey].getName(_localeID);
			}
			tv.setText(buttonText);
			tv.setTextColor(Color.CYAN);
		}
		
		setupButtonListeners();
	}
	
	@Override protected void onDestroy()
	{
		super.onDestroy();
	}

	@Override protected void onPause()
	{
		super.onPause();
	}
	
	@Override protected void onResume()
	{
		super.onResume();
	}

	@Override protected void onSaveInstanceState(Bundle outState)
	{
		try
		{
			outState.putInt(CONFIG_EMUKEY, _emuKey);
			//outState.putInt(CONFIG_SYSTEMKEY, _prevSystemKey);
			outState.putString(CONFIG_MAPID, _prevMapID);
			outState.putInt(CONFIG_LOCALEID, _localeID);
			outState.putBoolean(CONFIG_CANCANCEL, _canCancel);
			outState.putSerializable(CONFIG_CALLERDATA, _callerData);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void _parseOptions(Bundle savedInstanceState)
	{
		_emuKey = -1;

		//_prevSystemKey = -1;
		_prevMapID = null;
		_localeID = Input.kLocale_EN;

		_callerData = null;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			_emuKey = b.getInt(CONFIG_EMUKEY, -1);

			//_prevSystemKey = b.getInt(CONFIG_SYSTEMKEY, -1);
			_prevMapID = b.getString(CONFIG_MAPID);

			_localeID = b.getInt(CONFIG_LOCALEID);

			_canCancel = b.getBoolean(CONFIG_CANCANCEL, true);

			_callerData = b.getSerializable(CONFIG_CALLERDATA);
		}
	}

	void setupButtonListeners()
	{
//		View vw = findViewById(R.id.ic_cancelBtn);
//		vw.setFocusable(false);
//		vw.setOnClickListener(new OnClickListener() {
//			public void onClick(View v) {
//				sendFinish(RESULT_CANCELED);
//			}
//		});
		
//		vw = findViewById(R.id.ic_unmapBtn);
//		vw.setFocusable(false);
//		vw.setOnClickListener(new OnClickListener() {
//			public void onClick(View v) {
//				_retIntent.putExtra(RESULT_UNMAP, true);
//				sendFinish(RESULT_OK);
//			}
//		});
	}
	@Override public boolean dispatchKeyEvent(KeyEvent event)
	{
		if (event.getAction() == KeyEvent.ACTION_UP)
		{
			int c = event.getKeyCode();
			//Log.i("hataroid", "CONFIGURE keyup: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

			_scannedKeyCode = c;
			_retIntent.putExtra(RESULT_KEYCODE, _scannedKeyCode);
			sendFinish(RESULT_OK);
		}
		return true; // always consume
	}
	
/*
	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		int c = event.getKeyCode();
		Log.i("hataroid", "CONFIGURE keydown: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

		_scannedKeyCode = c;

		_retIntent.putExtra(RESULT_KEYCODE, _scannedKeyCode);
		sendFinish(RESULT_OK);
		return true;

		//return super.onKeyDown(keyCode, event);
	}
	
	@Override public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		int c = event.getKeyCode();
		Log.i("hataroid", "CONFIGURE keyup: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

		return true;
		//return super.onKeyUp(keyCode, event);
	}
*/
	private void sendFinish(int resultCode)
	{
		_retIntent.putExtra(RESULT_PREVEMUKEY, _emuKey);
		//_retIntent.putExtra(RESULT_PREVSYSTEMKEY, _prevSystemKey);
		_retIntent.putExtra(RESULT_PREVMAPID, _prevMapID);
		_retIntent.putExtra(RESULT_CALLERDATA, _callerData);
		
		setResult(resultCode, _retIntent);
		finish();
	}
}
