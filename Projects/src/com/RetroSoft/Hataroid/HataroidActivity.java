package com.RetroSoft.Hataroid;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.xmlpull.v1.XmlPullParserException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.XmlResourceParser;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;
import com.RetroSoft.Hataroid.Preferences.Settings;


public class HataroidActivity extends Activity
{
	public static final String LOG_TAG = "hataroid";

	private static final int ACTIVITYRESULT_FLOPPYA = 1;
	private static final int ACTIVITYRESULT_FLOPPYB = 2;
	private static final int ACTIVITYRESULT_SETTINGS =3;

	public static HataroidActivity	instance = null;

	private HataroidViewGL2		_viewGL2;
	private Thread				_emuThread;

	private AudioTrack			_audioTrack;
	private Boolean				_audioPaused = true;
	
	@Override protected void onCreate(Bundle icicle)
	{
		super.onCreate(icicle);

		PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
		
		try
		{
			_setupDefaultCheckboxPreferences();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

        instance = this;

        System.loadLibrary("hataroid");
        
        _viewGL2 = new HataroidViewGL2(getApplication(), false, 0, 0);
		setContentView(_viewGL2);
		
		//startEmulationThread();
	}
	
	// Hack to fix Android issue where it doesn't store default values of "false" and doesn't add new values if preferences have been added
	private void _setupDefaultCheckboxPreferences() throws XmlPullParserException, IOException
	{
		final String CheckBoxPrefTag = "CheckBoxPreference";

		Map<String,String> prefDefs = new HashMap<String,String>();
		Map<String,String> prefTypes = new HashMap<String,String>();

		XmlResourceParser p = getResources().getXml(R.xml.preferences);
		int et = p.getEventType();
		while (et != XmlResourceParser.END_DOCUMENT)
		{
			if (et == XmlResourceParser.START_TAG)
			{
				String attrKey = null;
				String attrDefaultVal = null;

				for (int i = 0; i < p.getAttributeCount(); ++i)
				{
					String attrName = p.getAttributeName(i);
					if (attrName.compareTo("defaultValue")==0)
					{
						attrDefaultVal = p.getAttributeValue(i);
					}
					else if (attrName.compareTo("key")==0)
					{
						attrKey = p.getAttributeValue(i);
					}
				}
				
				if (attrKey != null && attrDefaultVal != null)
				{
					prefDefs.put(attrKey,  attrDefaultVal);
					if (p.getName().compareTo(CheckBoxPrefTag)==0)
					{
						prefTypes.put(attrKey, "boolean");
					}
					else
					{
						prefTypes.put(attrKey,  "string");
					}
				}
			}

			et = p.next();
		}
		
		// get a list of missing prefs
		List<String> missingPrefs = new LinkedList<String>();
		Map<String, Object> options = getEmulatorOptions();
		final String [] optionKeys = (String [])options.get("keys");
		for (String curKey : prefDefs.keySet())
		{
			boolean found = false;
			for (int k = 0; k < optionKeys.length; ++k)
			{
				if (optionKeys[k].compareTo(curKey)==0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{				
				missingPrefs.add(curKey);
			}
		}

		// now add the missing prefs
		if (missingPrefs.size() > 0)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			Editor ed = prefs.edit();
			for (int i = 0; i < missingPrefs.size(); ++i)
			{
				String curKey = missingPrefs.get(i);
				String defVal = (String)prefDefs.get(curKey);
				String attrType = (String)prefTypes.get(curKey);
	
				if (attrType.compareTo("boolean")==0)
				{
					ed.putBoolean(curKey, Boolean.parseBoolean(defVal));
				}
				else
				{
					ed.putString(curKey, defVal);
				}
				Log.i("hataroid", "Adding missing pref:" + curKey + " : " + defVal + " : " + attrType);
			}
			ed.commit();
		}
	}
	
	@Override protected void onDestroy()
	{
		instance = null;

		Log.i(LOG_TAG, "stoppping emulation thread");
		stopEmulationThread();

		Log.i(LOG_TAG, "stoppping audio");
		deinitAudio();

		Log.i(LOG_TAG, "super destroy");
		super.onDestroy();

		Log.i(LOG_TAG, "lib Exit");
		HataroidNativeLib.libExit();
	}
	
	public void quitHataroid()
	{
    	this.runOnUiThread(new Runnable()
    	{
			public void run()
			{
				finish();
			}
		});
	}

	public void startEmulationThread()
	{
		Map<String, Object> options = getEmulatorOptions();
		final String [] optionKeys = (String [])options.get("keys");
		final String [] valKeys = (String [])options.get("vals");

		if (_emuThread == null)
		{
			_emuThread = new Thread()
			{
				public void run()
				{
					HataroidNativeLib.emulationInit(HataroidActivity.instance, optionKeys, valKeys);
					emuThreadMain();
				}
			};
			_emuThread.start();
		}
	}
	private void stopEmulationThread()
	{
		if (_emuThread != null)
		{
			_emuThread.interrupt();
			_emuThread = null;
		}

		HataroidNativeLib.emulationDestroy(this);
	}
	
	public void emuThreadMain()
	{
		HataroidNativeLib.emulationMain();
	}

	@Override protected void onPause()
	{
		super.onPause();

		_pause();
	}
	
	@Override protected void onResume()
	{
		super.onResume();
		
		_resume();
	}
	
	private void _pause()
	{		
		_viewGL2.onPause();
		
		pauseEmulation();
		pauseAudio();
	}
	
	private void _resume()
	{
		_viewGL2.onResume();
		
		resumeEmulation();
		playAudio();
	}

	private void pauseEmulation()
	{
		HataroidNativeLib.emulationPause();
	}
	
	private void resumeEmulation()
	{
		HataroidNativeLib.emulationResume();
	}
	
	public void initAudio(int freq, int bits, int channels, int bufSizeBytes)
    {
    	if (_audioTrack == null)
    	{
    		Log.i(LOG_TAG, "Starting Audio. freq: " + freq + ", bits: " + bits + ", channels: " + channels + ", bufSizeBytes: " + bufSizeBytes);

    		_audioTrack = new AudioTrack(
    				AudioManager.STREAM_MUSIC,
    				freq,
    				(channels == 2) ? AudioFormat.CHANNEL_CONFIGURATION_STEREO : AudioFormat.CHANNEL_CONFIGURATION_MONO,
    				(bits == 8) ? AudioFormat.ENCODING_PCM_8BIT : AudioFormat.ENCODING_PCM_16BIT,
    				bufSizeBytes,
    				AudioTrack.MODE_STREAM);

    		_audioTrack.play();
    		_audioPaused = false;
    	}
    }
    
    public void deinitAudio()
    {
		if (_audioTrack != null)
		{
			_audioTrack.pause();
			_audioTrack.flush();
			_audioTrack.stop();
			_audioTrack.release();

			_audioTrack = null;
		}
		_audioPaused = true;
    }

    public void pauseAudio()
    {
    	if (_audioTrack != null)
    	{
    		Log.i(LOG_TAG, "Pausing Audio");
    		_audioTrack.pause();
    	}
    	_audioPaused = true;
    }
    
    public void playAudio()
    {
    	if (_audioTrack != null)
    	{
    		Log.i(LOG_TAG, "Playing Audio");
    		_audioTrack.play();
    	}
    	_audioPaused = false;
    }
    
    public void sendAudio(short data [])
    {
    	if (_audioTrack != null && !_audioPaused)
    	{
    		_audioTrack.write(data, 0, data.length);
    		if (_audioTrack.getPlayState() != AudioTrack.PLAYSTATE_PLAYING)
    		{
    			_audioTrack.play();
    		}
    	}
    }

    private Map<String, Object> getEmulatorOptions()
    {
    	Map<String, Object> options = new HashMap<String, Object>();

    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		Map<String,?> keys = prefs.getAll();
		int numKeys = keys.size();

		String [] keyarray = new String [numKeys];
		String [] valarray = new String [numKeys];

		int i = 0;
		for (Map.Entry<String,?> entry : keys.entrySet())
		{
			keyarray[i] = entry.getKey();
			valarray[i] = entry.getValue().toString();
			++i;
		}
		
		options.put("keys", keyarray);
		options.put("vals", valarray);

		return options;
    }
    
    @Override public boolean onPrepareOptionsMenu(Menu menu)
    {
		// update floppy option strings
		int [] itemID = { R.id.ejecta, R.id.ejectb };
		String [] title = { "Eject Floppy A", "Eject Floppy B" };
		for (int i = 0; i < 2; ++i)
		{
			String filename = HataroidNativeLib.emulatorGetCurFloppy(i);
			String zipname = HataroidNativeLib.emulatorGetCurFloppyZip(i);
			MenuItem item = menu.findItem(itemID[i]);
			if (filename.length() == 0 && zipname.length() == 0)
			{
				item.setEnabled(false);
				item.setTitle(title[i]);
			}
			else
			{
				item.setEnabled(true);
				String [] fileSplit = filename.split("/");
				item.setTitle(title[i] + " (" + fileSplit[fileSplit.length-1] + (zipname.length()>0 ? ("/" + zipname) : "") + ")");
			}
		}
		return super.onPrepareOptionsMenu(menu);
    }
    
    @Override public boolean onCreateOptionsMenu(Menu menu)
	{
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.mainmenu, menu);
		return true;
	}

	@Override public boolean onOptionsItemSelected(MenuItem item)
	{
		// Handle item selection
		int id = item.getItemId();
		switch (id)
		{
			case R.id.controls:
			{
				return true;
			}
			case R.id.floppya:
			case R.id.floppyb:
			{
		        Intent fileBrowser = new Intent(this, FileBrowser.class);
		        startActivityForResult(fileBrowser, (id==R.id.floppya)?ACTIVITYRESULT_FLOPPYA:ACTIVITYRESULT_FLOPPYB);
		        return true;
			}
			case R.id.ejecta:
			case R.id.ejectb:
			{
				int floppy = (id==R.id.ejecta)?0:1;
				HataroidNativeLib.emulatorEjectFloppy(floppy);
		        return true;
			}
			case R.id.coldreset:
			{
				HataroidNativeLib.emulatorResetCold();
				return true;
			}
			case R.id.warmreset:
			{
				HataroidNativeLib.emulatorResetWarm();
				return true;
			}
			case R.id.quit:
			{
				showQuitConfirm();
				return true;
			}
			case R.id.settings:
			{
		        Intent settings = new Intent(this, Settings.class);
		        startActivityForResult(settings, ACTIVITYRESULT_SETTINGS);
		        return true;
			}
			case R.id.togglemousejoystick:
			{
				HataroidNativeLib.toggleMouseJoystick();
				return true;
			}
			case R.id.help:
			{
				return true;
			}
			default:
			{
				return super.onOptionsItemSelected(item);
			}
		}
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			case ACTIVITYRESULT_FLOPPYA:
			case ACTIVITYRESULT_FLOPPYB:
			{
				if (resultCode == RESULT_OK)
				{
					String filePath = data.getStringExtra(FileBrowser.RESULT_PATH);
					String zipPath = data.getStringExtra(FileBrowser.RESULT_ZIPPATH);
					HataroidNativeLib.emulatorInsertFloppy((requestCode==ACTIVITYRESULT_FLOPPYA) ? 0 : 1, filePath, zipPath);
					
					if (data.hasExtra(FileBrowser.RESULT_RESETCOLD))
					{
						Boolean coldReset = data.getBooleanExtra(FileBrowser.RESULT_RESETCOLD, false);
						if (coldReset)
						{
							HataroidNativeLib.emulatorResetCold();
						}
					}
					else if (data.hasExtra(FileBrowser.RESULT_RESETWARM))
					{
						Boolean coldReset = data.getBooleanExtra(FileBrowser.RESULT_RESETWARM, false);
						if (coldReset)
						{
							HataroidNativeLib.emulatorResetWarm();
						}
					}
				}
				break;
			}
			
			case ACTIVITYRESULT_SETTINGS:
			{
				Map<String, Object> options = getEmulatorOptions();
				String [] optionKeys = (String [])options.get("keys");
				String [] valKeys = (String [])options.get("vals");
				HataroidNativeLib.emulatorSetOptions(optionKeys, valKeys);
				break;
			}
		}
	}

	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				showQuitConfirm();
				return false;
			}
		}

		return super.onKeyDown(keyCode, event);
	}
	
	public void showQuitConfirm()
	{
		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
		alertDialog.setTitle("Quit Hataroid?");
		alertDialog.setMessage("Are you sure you want to quit?");
		alertDialog.setButton("No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { } });
		alertDialog.setButton2("Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { finish(); } });
		alertDialog.show();
	}
	
    public void showOptionsDialog()
    {
    	this.runOnUiThread(new Runnable()
    	{
			public void run()
			{
		        Intent settings = new Intent(HataroidActivity.instance, Settings.class);
		        startActivityForResult(settings, ACTIVITYRESULT_SETTINGS);
			}
		});
    }

    public void showGenericDialog(final int ok, final int noyes, final String message)
    {
    	this.runOnUiThread(new Runnable()
    	{
			public void run()
			{
    			AlertDialog alertDialog = new AlertDialog.Builder(HataroidActivity.this).create();
    			alertDialog.setTitle("Hataroid Alert");
    			alertDialog.setMessage(message);
				alertDialog.setCancelable(false);
				alertDialog.setCanceledOnTouchOutside(false);
    			if (ok==1)
   				{
    				alertDialog.setButton("Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { HataroidNativeLib.hataroidDialogResult(0); } });
   				}
    			else if (noyes==1)
    			{
    				//alertDialog.setOnDismissListener( new DialogInterface.OnDismissListener() { public void onDismiss(DialogInterface dialog) { HataroidNativeLib.hataroidDialogResult(0); } });
    				//alertDialog.setOnCancelListener( new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { HataroidNativeLib.hataroidDialogResult(0); } });
    				alertDialog.setButton("No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { HataroidNativeLib.hataroidDialogResult(0); } });
    				alertDialog.setButton2("Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { HataroidNativeLib.hataroidDialogResult(1); } });
    			}
    			alertDialog.show();
			}
    	});
	}
}
