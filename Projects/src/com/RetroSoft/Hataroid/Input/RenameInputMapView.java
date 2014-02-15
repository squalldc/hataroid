package com.RetroSoft.Hataroid.Input;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.R;

public class RenameInputMapView extends Activity
{
	public static final String CONFIG_CURNAME = "Config_CurName";
	public static final String RESULT_NAME= "Result_Name";

	String _curName = "";
	Intent _retIntent = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.renameinputmap_view);
		
		_parseOptions(savedInstanceState);
		_retIntent = new Intent();
		
		View v = findViewById(R.id.rim_name);
		if (v != null && v instanceof EditText)
		{
			EditText et = (EditText)v;
			et.setText(_curName);
			et.selectAll();
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
		_curName = null;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			_curName = b.getString(CONFIG_CURNAME);
		}
		
		if (_curName == null)
		{
			_curName = "";
		}
	}

	void setupButtonListeners()
	{
		View vw = findViewById(R.id.rim_cancelBtn);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				sendFinish(RESULT_CANCELED);
			}
		});
		
		vw = findViewById(R.id.rim_okBtn);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_updateCurName();
				_retIntent.putExtra(RESULT_NAME, _curName);
				sendFinish(RESULT_OK);
			}
		});
		
		vw = findViewById(R.id.rim_keyboardBtn);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				// show input method selector
				Input input = HataroidActivity.instance.getInput();
				input.showInputMethodSelector();
			}
		});
	}

	private void sendFinish(int resultCode)
	{
		setResult(resultCode, _retIntent);
		finish();
	}
	
	private void _updateCurName()
	{
		View v = findViewById(R.id.rim_name);
		if (v != null && v instanceof EditText)
		{
			EditText et = (EditText)v;
			_curName = et.getText().toString();
		}
	}
}
