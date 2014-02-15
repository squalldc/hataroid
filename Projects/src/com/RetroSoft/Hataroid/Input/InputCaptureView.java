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

public class InputCaptureView extends Activity
{
	public static final String CONFIG_EMUKEY = "Config_EmuKey";
	
	public static final String RESULT_KEYCODE = "ResultKeyCode";
	public static final String RESULT_UNMAP = "ResultUnmap";

	int [] _groupJoystickKeys = {VirtKeyDef.VKB_KEY_JOYUP, VirtKeyDef.VKB_KEY_JOYDOWN, VirtKeyDef.VKB_KEY_JOYLEFT, VirtKeyDef.VKB_KEY_JOYRIGHT, VirtKeyDef.VKB_KEY_JOYFIRE, VirtKeyDef.VKB_KEY_SPACE};
	boolean _groupCapture = false;
	
	int _emuKey = -1;
	int _scannedKeyCode = -1;
	
	Intent _retIntent = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.inputcapture_view);
		
		_parseOptions(savedInstanceState);
		_retIntent = new Intent();
		
		View v = findViewById(R.id.capture_button_name);
		if (v != null && v instanceof TextView)
		{
			TextView tv = (TextView)v;
			String buttonText = "unknown";
			if (_emuKey >= 0 && _emuKey < VirtKeyDef.VKB_KEY_NumOf)
			{
				buttonText = VirtKeyDef.kDefs[_emuKey].name;
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

	void _parseOptions(Bundle savedInstanceState)
	{
		_emuKey = -1;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			_emuKey = b.getInt(CONFIG_EMUKEY, -1);
		}
	}

	void setupButtonListeners()
	{
		View vw = findViewById(R.id.ic_cancelBtn);
		vw.setFocusable(false);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				sendFinish(RESULT_CANCELED);
			}
		});
		
		vw = findViewById(R.id.ic_unmapBtn);
		vw.setFocusable(false);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_retIntent.putExtra(RESULT_UNMAP, true);
				sendFinish(RESULT_OK);
			}
		});
	}
	@Override public boolean dispatchKeyEvent(KeyEvent event)
	{
		int c = event.getKeyCode();
		//Log.i("hataroid", "CONFIGURE keydown: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

		_scannedKeyCode = c;
		_retIntent.putExtra(RESULT_KEYCODE, _scannedKeyCode);
		sendFinish(RESULT_OK);

		return true; // yum yum
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
		setResult(resultCode, _retIntent);
		finish();
	}
}
