package com.RetroSoft.Hataroid;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
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
import android.view.View;
import android.view.WindowManager;

import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;
import com.RetroSoft.Hataroid.Help.HelpActivity;
import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.InputMapConfigureView;
import com.RetroSoft.Hataroid.Input.Shortcut.ShortcutMap;
import com.RetroSoft.Hataroid.Input.Shortcut.ShortcutMapConfigureView;
import com.RetroSoft.Hataroid.Preferences.Settings;
import com.RetroSoft.Hataroid.SaveState.SaveStateBrowser;


public class HataroidActivity extends Activity
{
	public static final String LOG_TAG = "hataroid";

	private static final int ACTIVITYRESULT_FLOPPYA					= 1;
	private static final int ACTIVITYRESULT_FLOPPYB					= 2;
	private static final int ACTIVITYRESULT_SETTINGS				= 3;
	private static final int ACTIVITYRESULT_SS_SAVE					= 4;
	private static final int ACTIVITYRESULT_SS_LOAD					= 5;
	private static final int ACTIVITYRESULT_SS_DELETE				= 6;
	private static final int ACTIVITYRESULT_SS_QUICKSAVESLOT		= 7;

	public static HataroidActivity	instance = null;
	private boolean				_lostFocus = false;

	private HataroidViewGL2		_viewGL2;
	private Thread				_emuThread;

	private AudioTrack			_audioTrack;
	private Boolean				_audioPaused = true;
	
	private Input				_input = null;

	boolean						_waitSaveAndQuit = false;

	boolean						_allowDeveloperOptions = false;
	
	boolean						_tryUseImmersiveMode = false;
	boolean						_wantImmersiveMode = false;

	@Override protected void onCreate(Bundle icicle)
	{
		super.onCreate(icicle);
		
		_tryUseImmersiveMode = android.os.Build.VERSION.SDK_INT >= 19;
		
		try
		{
			PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
			_setupDefaultCheckboxPreferences();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

        instance = this;

        _input = new Input();
		_input.init(getApplicationContext());
		try
		{
			View rootView = findViewById(android.R.id.content);
			_input.initMouse(rootView);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

        System.loadLibrary("hataroid");
        
        _viewGL2 = new HataroidViewGL2(getApplication(), false, 0, 0);
		setContentView(_viewGL2);
		
		try
		{
			_setupDeviceOptions();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		//startEmulationThread();
	}
	
	void _readPrefDefaults(Map<String,String[]> prefDefs) throws XmlPullParserException, IOException
	{
		final String CheckBoxPrefTag = "CheckBoxPreference";

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
					String attrType = (p.getName().compareTo(CheckBoxPrefTag)==0) ? "boolean" : "string";
					prefDefs.put(attrKey, new String[] {attrDefaultVal, attrType});
				}
			}

			et = p.next();
		}
	}

	// Hack to fix Android issue where it doesn't store default values of "false" and doesn't add new values if preferences have been added
	private void _setupDefaultCheckboxPreferences()
	{
		Map<String,String[]> prefDefs = new HashMap<String,String[]>();

		boolean error = false;
		try { _readPrefDefaults(prefDefs); }
		catch (Exception e) { error = true; }
		if (error)
		{
			showErrorDialog("Error Reading Preferences", "Please let the author know.\n. Some things might not work correctly");
		}

		// get a list of missing prefs
		List<String> missingPrefs = new LinkedList<String>();
		Map<String, Object> options = getEmulatorOptions(false);
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
		_setPreferenceList(missingPrefs, prefDefs);
	}
	
	void _setPreferenceList(List<String> updatedPrefs, Map<String,String[]> prefDefs)
	{
		if (updatedPrefs.size() > 0)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			Editor ed = prefs.edit();
			for (int i = 0; i < updatedPrefs.size(); ++i)
			{
				String curKey = updatedPrefs.get(i);
				String[] valPair = prefDefs.get(curKey);
				String defVal = valPair[0];
				String attrType = valPair[1];
	
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

	void _setupDeviceOptions()
	{
		try
		{
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
	    	Map<String,?> allPrefs = prefs.getAll();
	    	Object val;
	    	
	    	_allowDeveloperOptions = false;
	    	val = allPrefs.get("pref_device_developer_options");
	    	if (val != null)
	    	{
	    		String sval = val.toString();
	    		_allowDeveloperOptions = !(sval.compareTo("false")==0 || sval.compareTo("0")==0);
	    	}
	    	
	    	val = allPrefs.get("pref_display_keepscreenawake");
	    	if (val != null)
	    	{
	    		String sval = val.toString();
	    		boolean bval = !(sval.compareTo("false")==0 || sval.compareTo("0")==0);
	    		_keepScreenAwake(bval);
	    	}
	    	
	    	val = allPrefs.get("pref_input_mouse_default_android");
	    	if (val != null)
	    	{
	    		String sval = val.toString();
	    		boolean bval = !(sval.compareTo("false")==0 || sval.compareTo("0")==0);
	    		try
	    		{
	    			boolean newMouse = !bval;
	    			if (_input != null)
	    			{
	    				_input.enableNewMouse(newMouse);
	    			}
	    			if (_viewGL2 != null)
	    			{
	    				_viewGL2.enableNewMouse(newMouse);
	    			}
	    		}
	    		catch(Exception e)
	    		{
	    			e.printStackTrace();
	    		}
	    	}
	    	
	    	if (_allowDeveloperOptions)
	    	{
	    		if (_tryUseImmersiveMode)
	    		{
			    	val = allPrefs.get("pref_device_kitkat_immersive");
			    	if (val != null)
			    	{
			    		String sval = val.toString();
			    		boolean bval = !(sval.compareTo("false")==0 || sval.compareTo("0")==0);
			    		_wantImmersiveMode = bval;
			    		_setupImmersiveMode();
			    	}
	    		}
	    	}

	    	_input.setupOptionsFromPrefs(prefs);
		}
		catch (Exception e)
		{
			e.printStackTrace();
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
				if (HataroidActivity.instance != null)
				{
					HataroidActivity.instance.finish();
				}
			}
		});
	}

	public void startEmulationThread()
	{
		Map<String, Object> options = getFullEmulatorOptions(true);
		final String [] optionKeys = (String [])options.get("keys");
		final String [] valKeys = (String [])options.get("vals");

		if (_emuThread == null)
		{
			_emuThread = new Thread()
			{
				public void run()
				{
					Thread.currentThread().setPriority(Thread.NORM_PRIORITY+1);
					
					HataroidNativeLib.emulationInit(HataroidActivity.instance, optionKeys, valKeys);
					emuThreadMain();
				}
			};
			_emuThread.start();

			_checkAutoSaveOnStart();
		}
	}

	private void stopEmulationThread()
	{
		try
		{
			if (_emuThread != null)
			{
				_emuThread.interrupt();
				_emuThread = null;
			}
		}
		catch (Exception e) {}
		catch (Error e) {}

		try
		{
			HataroidNativeLib.emulationDestroy(this);
		}
		catch (Exception e) {}
		catch (Error e) {}
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
		
		if (!_lostFocus)
		{
			_resume();
		}
	}

	@Override public void onWindowFocusChanged(boolean hasFocus)
	{
		if (!hasFocus)
		{
			if (!_lostFocus)
			{
				_lostFocus = true;
				//_pause();
			}
		}
		else
		{
			_setupImmersiveMode();

			if (_lostFocus)
			{
				_resume();
			}
		}
	}

	void _setupImmersiveMode()
	{
		if (!_allowDeveloperOptions || !_tryUseImmersiveMode)
		{
			return;
		}

		// UNTESTED, need some people to test before I enable this for everyone
		try
		{
			View decorView = getWindow().getDecorView();
			
			int visibility = 0;

            if (_wantImmersiveMode)
			{
				visibility = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
			}
            else
            {
            	visibility = View.SYSTEM_UI_FLAG_VISIBLE;
            }

            decorView.setSystemUiVisibility(visibility);
		}
		catch (Error e)
		{
			_tryUseImmersiveMode = false;
		}
		catch (Exception e)
		{
			_tryUseImmersiveMode = false;
		}
	}
	
	private void _pause()
	{		
		_viewGL2.onPause();
		
		pauseEmulation();
		pauseAudio();
	}
	
	private void _resume()
	{
		_lostFocus = false;

		_viewGL2.onResume();
		
		resumeEmulation();
		_checkPlayAudio();
	}
	
	private void _checkPlayAudio()
	{
		boolean paused = HataroidNativeLib.emulatorGetUserPaused();
		if (paused)
		{
			pauseAudio();
		}
		else
		{
			playAudio();
		}
	}
	
	private void pauseEmulation()
	{
		HataroidNativeLib.emulationPause();
	}
	
	private void resumeEmulation()
	{
		HataroidNativeLib.emulationResume();
	}
	
	public int getMinBufSize(int freq, int bits, int channels)
	{
		int minBufSize = AudioTrack.getMinBufferSize(
				freq,
				(channels == 2) ? AudioFormat.CHANNEL_OUT_STEREO : AudioFormat.CHANNEL_OUT_MONO,
				(bits == 8) ? AudioFormat.ENCODING_PCM_8BIT : AudioFormat.ENCODING_PCM_16BIT);
		return minBufSize;
	}
	
	public void initAudio(int freq, int bits, int channels, int bufSizeBytes)
    {
    	if (_audioTrack == null)
    	{
    		Log.i(LOG_TAG, "Starting Audio. freq: " + freq + ", bits: " + bits + ", channels: " + channels + ", bufSizeBytes: " + bufSizeBytes);

    		boolean error = false;
    		try
    		{
	    		_audioTrack = new AudioTrack(
	    				AudioManager.STREAM_MUSIC,
	    				freq,
	    				(channels == 2) ? AudioFormat.CHANNEL_OUT_STEREO : AudioFormat.CHANNEL_OUT_MONO,
	    				(bits == 8) ? AudioFormat.ENCODING_PCM_8BIT : AudioFormat.ENCODING_PCM_16BIT,
	    				bufSizeBytes,
	    				AudioTrack.MODE_STREAM);
	
	    		_audioPaused = false;
	    		_checkPlayAudio();
    		}
    		catch (Exception e)
    		{
    			error = true;
    			deinitAudio();
    		}
    		
    		if (error)
    		{
    			_showAudioErrorDialog();
    		}
    	}
    }
	
	void _showAudioErrorDialog()
	{
    	this.runOnUiThread(new Runnable()
    	{
			public void run()
			{
    			AlertDialog alertDialog = new AlertDialog.Builder(HataroidActivity.this).create();
    			alertDialog.setTitle("Unsupported audio setting");
    			alertDialog.setMessage("The chosen audio setting is not supported on this device.\nResetting to default audio settings.");
				alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _resetAudioSettings(); } });
    			alertDialog.show();
			}
    	});
	}

	void _resetAudioSettings()
	{
		Map<String,String[]> prefDefs = new HashMap<String,String[]>();

		boolean error = false;
		try { _readPrefDefaults(prefDefs); }
		catch (Exception e) { error = true; }
		if (error)
		{
			showErrorDialog("Error Reading Preferences", "Please let the author know.\n. Some things might not work correctly");
		}
		
		List<String> audioPrefs = Arrays.asList("pref_sound_quality");
		for (int i = 0; i < audioPrefs.size(); ++i)
		{
			if (!prefDefs.containsKey(audioPrefs.get(i)))
			{
				audioPrefs.remove(i);
				--i;
			}
		}

		// now add the missing prefs
		_setPreferenceList(audioPrefs, prefDefs);

		_updateOptions();
	}
    
    public void deinitAudio()
    {
		if (_audioTrack != null)
		{
			try
			{
				_audioTrack.pause();
				_audioTrack.flush();
				_audioTrack.stop();
				_audioTrack.release();
			}
			catch (Exception e)
			{
			}

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
        	_addSilence();
    	}
    	_audioPaused = true;
    }
    
    public void playAudio()
    {
    	if (_audioTrack != null)
    	{
    		Log.i(LOG_TAG, "Playing Audio");
    		_audioTrack.play();
        	_addSilence();
    	}
    	_audioPaused = false;
    }

	void _addSilence()
	{
    	if (_audioTrack != null)
    	{
//    		_audioTrack.flush();
			
/*
			short[] silence = new short [2048];
    		for (int i = 0; i < 5; ++i)
    		{
    			_audioTrack.write(silence, 0, silence.length);
    		}
 */
    	}
	}
    
    public void sendAudio(short data [], int len, int flush)
    {
    	if (_audioTrack != null && !_audioPaused)
    	{
    		if (flush != 0)// || flushNow)
    		{
    			_audioTrack.flush();
    		}
    		if (_audioTrack.getPlayState() != AudioTrack.PLAYSTATE_PLAYING)
    		{
    			//_audioTrack.flush();
    			_audioTrack.play();
    		}
    		_audioTrack.write(data, 0, len);//data.length);
    	}
    }
    
    private Map<String, Object> getFullEmulatorOptions(boolean stripJavaOptions)
    {
		Map<String, Object> baseOptions = getEmulatorOptions(stripJavaOptions);

		String [] optionKeysNormal = (String [])baseOptions.get("keys");
		String [] valKeysNormal = (String [])baseOptions.get("vals");
		
		String [] optionKeys = optionKeysNormal;
		String [] valKeys = valKeysNormal;

		Map<String, String> dynOptions = getDynamicEmulatorOptions();
		int numDynOptions = dynOptions.size();
		if (dynOptions != null && numDynOptions > 0)
		{
			optionKeys = new String [optionKeysNormal.length + numDynOptions];
			valKeys = new String [optionKeysNormal.length + numDynOptions];

			int i = 0;
			for (; i < optionKeysNormal.length; ++i)
			{
				optionKeys[i] = optionKeysNormal[i];
				valKeys[i] = valKeysNormal[i];
			}
			
			for (Map.Entry<String, String> entry : dynOptions.entrySet()) {
				optionKeys[i] = entry.getKey();
				valKeys[i] = entry.getValue();
				++i;
			}
		}

		Map<String, Object> options = new HashMap<String, Object>();
		options.put("keys", optionKeys);
		options.put("vals", valKeys);
		return options;
    }

    private Map<String, Object> getEmulatorOptions(boolean stripJavaOptions)
    {
    	Map<String, Object> options = new HashMap<String, Object>();

    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		Map<String,?> keys = prefs.getAll();

		int numKeys = keys.size();
		if (stripJavaOptions)
		{
			numKeys = 0;
			for (String key : keys.keySet())
			{
				if (!key.startsWith(InputMapConfigureView.kPrefPrefix)
				 && !key.startsWith(ShortcutMapConfigureView.kPrefPrefix))
				{
					++numKeys;
				}
			}
		}

		String [] keyarray = new String [numKeys];
		String [] valarray = new String [numKeys];

		int i = 0;
		for (Map.Entry<String,?> entry : keys.entrySet())
		{
			String key = entry.getKey();
			if (stripJavaOptions)
			{
				if (key.startsWith(InputMapConfigureView.kPrefPrefix)
				 || key.startsWith(ShortcutMapConfigureView.kPrefPrefix))
				{
					continue;
				}
			}

			keyarray[i] = key;
			valarray[i] = entry.getValue().toString();
			++i;
		}
		
		options.put("keys", keyarray);
		options.put("vals", valarray);

		return options;
    }
    
    @Override public boolean onPrepareOptionsMenu(Menu menu)
    {
    	try
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
			
			// update pause/unpause option strings
			{
				MenuItem item = menu.findItem(R.id.pause);
				if (item != null)
				{
					boolean paused = HataroidNativeLib.emulatorGetUserPaused();
					
					String s = getApplicationContext().getResources().getString(paused ? R.string.Unpause : R.string.Pause);
					item.setTitle(s);
					item.setTitleCondensed(s);
				}
			}
    	}
    	catch (Exception e)
    	{
    		e.printStackTrace();
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
		if (_waitSaveAndQuit)
		{
			return true;
		}

		// Handle item selection
		int id = item.getItemId();
		switch (id)
		{
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
				_resetEmu(true);
				return true;
			}
			case R.id.warmreset:
			{
				_resetEmu(false);
				return true;
			}
			case R.id.pause:
			{
				HataroidNativeLib.emulatorToggleUserPaused();
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
			case R.id.help:
			{
		        Intent help = new Intent(this, HelpActivity.class);
		        startActivity(help);
				return true;
			}
			case R.id.ss_save:
			case R.id.ss_load:
			case R.id.ss_delete:
			case R.id.ss_quicksaveslot:
			{
				int mode = -1;
				int resultId = -1;
				switch (id)
				{
					case R.id.ss_save:	{ mode = SaveStateBrowser.SSMODE_SAVE; resultId = ACTIVITYRESULT_SS_SAVE; break; }
					case R.id.ss_load:	{ mode = SaveStateBrowser.SSMODE_LOAD; resultId = ACTIVITYRESULT_SS_LOAD; break; }
					case R.id.ss_delete:{ mode = SaveStateBrowser.SSMODE_DELETE; resultId = ACTIVITYRESULT_SS_DELETE; break; }
					case R.id.ss_quicksaveslot: { mode = SaveStateBrowser.SSMODE_QUICKSAVESLOT; resultId = ACTIVITYRESULT_SS_QUICKSAVESLOT; break; }
				}
				if (mode >= 0)
				{
			        Intent browser = new Intent(this, SaveStateBrowser.class);
			        browser.putExtra(SaveStateBrowser.CONFIG_MODE, mode);
			        startActivityForResult(browser, resultId);
			        return true;
				}
				break;
			}
		}

		return super.onOptionsItemSelected(item);
	}
	
	void _resetEmu(boolean cold)
	{
		if (cold)
		{
			HataroidNativeLib.emulatorResetCold();
		}
		else
		{
			HataroidNativeLib.emulatorResetWarm();
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
							_resetEmu(true);
						}
					}
					else if (data.hasExtra(FileBrowser.RESULT_RESETWARM))
					{
						Boolean coldReset = data.getBooleanExtra(FileBrowser.RESULT_RESETWARM, false);
						if (coldReset)
						{
							_resetEmu(false);
						}
					}
				}
				break;
			}
			
			case ACTIVITYRESULT_SETTINGS:
			{
				_updateOptions();
				_setupDeviceOptions();
				HataroidNativeLib.hataroidSettingsResult(1);
				break;
			}
			
			case ACTIVITYRESULT_SS_SAVE:
			case ACTIVITYRESULT_SS_LOAD:
			{
				if (resultCode == RESULT_OK)
				{
					int saveSlot = data.getIntExtra(SaveStateBrowser.RESULT_SAVESTATE_SLOT, -1);
					String filePath = data.getStringExtra(SaveStateBrowser.RESULT_SAVESTATE_FILENAME);

					String path = filePath;
					int lastIndex = filePath.lastIndexOf("/");
					if (lastIndex > 0)
					{
						path = filePath.substring(0, lastIndex);
					}
					
					Log.i(LOG_TAG, "save/load: path: " + path + ", file: " + filePath + ", slot: " + saveSlot);
					
					if (requestCode == ACTIVITYRESULT_SS_SAVE)
					{
						HataroidNativeLib.emulatorSaveStateSave(path, filePath, saveSlot);
					}
					else if (requestCode == ACTIVITYRESULT_SS_LOAD)
					{
						HataroidNativeLib.emulatorSaveStateLoad(path, filePath, saveSlot);
					}
				}

				_updateOptions();
				break;
			}
			
			case ACTIVITYRESULT_SS_DELETE:
			case ACTIVITYRESULT_SS_QUICKSAVESLOT:
			{
				_updateOptions();
				break;
			}
		}
	}
	
	void _updateOptions()
	{
		Map<String, Object> options = getFullEmulatorOptions(true);
		String [] optionKeys = (String [])options.get("keys");
		String [] valKeys = (String [])options.get("vals");

		HataroidNativeLib.emulatorSetOptions(optionKeys, valKeys);
	}

	Map<String, String> getDynamicEmulatorOptions()
	{
		Map<String, String> dynOptions = new HashMap<String, String>();
		
    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
    	Map<String,?> allPrefs = prefs.getAll();
    	
    	ShortcutMap.getSelectedOptionFromPrefs(prefs, allPrefs, dynOptions);
    	
    	return dynOptions;
	}
	
	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (_input.onKeyDown(keyCode, event))
		{
			return false;
		}
		
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				showQuitConfirm();
				return false;
			}
			default:
			{
				break;
			}
		}

		return super.onKeyDown(keyCode, event);
	}

	@Override public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		if (_input.onKeyUp(keyCode, event))
		{
			return false;
		}

		return super.onKeyDown(keyCode, event);
	}

	static boolean _showingQuitConfirm = false;
	public void showQuitConfirm()
	{
		if (_waitSaveAndQuit)
		{
			return;
		}

		if (!_showingQuitConfirm)
		{
			_showingQuitConfirm = true;
	
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Quit Hataroid?");
			alertDialog.setMessage("Are you sure you want to quit?");
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingQuitConfirm = false; } });
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingQuitConfirm = false; _onQuit(); } });
			alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { _showingQuitConfirm = false; }});
			alertDialog.show();
		}
	}
	
	void _onQuit()
	{
		if (_waitSaveAndQuit)
		{
			return;
		}

		// auto save
		if (!_storeAutoSaveOnExit())
		{
			finish();
		}
	}
	
	public void showErrorDialog(String title, String message)
	{
		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
		alertDialog.setTitle(title);
		alertDialog.setMessage(message);
		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { } });
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
	
	static boolean _showingGenericDialog = false;
    public void showGenericDialog(final int ok, final int noyes, final String message)
    {
    	if (_showingGenericDialog || _waitSaveAndQuit)
    	{
    		return;
    	}
    	
    	//_showingGenericDialog = true;

    	this.runOnUiThread(new Runnable()
    	{
			public void run()
			{
    			AlertDialog alertDialog = new AlertDialog.Builder(HataroidActivity.this).create();
    			alertDialog.setTitle("Hataroid Info");
    			alertDialog.setMessage(message);
				alertDialog.setCancelable(false);
				alertDialog.setCanceledOnTouchOutside(false);
    			if (ok==1)
   				{
    				alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingGenericDialog = false; HataroidNativeLib.hataroidDialogResult(0); } });
   				}
    			else if (noyes==1)
    			{
    				//alertDialog.setOnDismissListener( new DialogInterface.OnDismissListener() { public void onDismiss(DialogInterface dialog) { HataroidNativeLib.hataroidDialogResult(0); } });
    				alertDialog.setOnCancelListener( new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { _showingGenericDialog = false; HataroidNativeLib.hataroidDialogResult(0); } });
    				alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingGenericDialog = false; HataroidNativeLib.hataroidDialogResult(0); } });
    				alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingGenericDialog = false; HataroidNativeLib.hataroidDialogResult(1); } });
    			}
    			alertDialog.show();
			}
    	});
	}

    void _keepScreenAwake(boolean set)
	{
    	try
    	{
			if (set)
			{
				getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
			else
			{
				getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
    	}
    	catch (Exception e)
    	{
    	}
	}

	public Input getInput() { return _input; }

	String _getAutoSaveFolder()
	{
		try
		{
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
	    	String saveFolder = prefs.getString(Settings.kPrefName_SaveState_Folder, null);
	    	if (saveFolder != null)
	    	{
	    		saveFolder = saveFolder.trim();
	    		if (saveFolder.length() > 0)
	    		{
		    		return saveFolder;
	    		}
	    	}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
		return null;
	}

	boolean _autoSaveOnExitEnabled()
	{
		try
		{
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
	    	Map<String,?> allPrefs = prefs.getAll();
	    	
	    	Object val = allPrefs.get("pref_storage_savestate_exitautosave");
	    	if (val != null)
	    	{
	    		String sval = val.toString();
	    		boolean bval = !(sval.compareTo("false")==0 || sval.compareTo("0")==0);

	    		return bval;
	    	}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
		return false;
	}
	
	boolean _storeAutoSaveOnExit()
	{
		String saveFolder = _getAutoSaveFolder();
		boolean autoSaveOnExit = _autoSaveOnExitEnabled();
		if (saveFolder != null && autoSaveOnExit)
		{
			//Log.i("hataroid", "Save FOlder: '" + saveFolder + "'");
			if (HataroidNativeLib.emulatorAutoSaveStoreOnExit(saveFolder))
			{
				_waitSaveAndQuit = true;
				return true;
			}
		}
		return false;
	}
	
	void _checkAutoSaveOnStart()
	{
		// check load auto save
		if (_autoSaveOnExitEnabled())
		{
			String saveFolder = _getAutoSaveFolder();
			if (saveFolder != null)
			{
				String autoSaveMetaName = saveFolder + "/as.qs";
				String autoSaveName = saveFolder + "/as.sav";
				File metaFile = new File(autoSaveMetaName);
				File saveFile = new File(autoSaveName);
				if (metaFile.exists() && saveFile.exists())
				{
					_showLoadAutoSaveDialog(saveFolder);
				}
			}
		}
	}

	void _showLoadAutoSaveDialog(String saveFolder_)
	{
		final String saveFolder = saveFolder_;
    	this.runOnUiThread(new Runnable()
    	{
			public void run()
			{
    			AlertDialog alertDialog = new AlertDialog.Builder(HataroidActivity.this).create();
    			alertDialog.setTitle("Restore previous session?");
    			alertDialog.setMessage("Do you want to load the last auto saved session?");
    			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { } });
    			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { HataroidNativeLib.emulatorAutoSaveLoadOnStart(saveFolder); } });
    			alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { }});
    			alertDialog.setCanceledOnTouchOutside(false);
    			alertDialog.show();
			}
    	});
		;
	}

	public void setConfigOnSaveStateLoad(String options [])
	{
		if (_waitSaveAndQuit)
		{
			return;
		}

		try
		{
			if (options.length > 1)
			{
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		    	Map<String,?> allPrefs = prefs.getAll();

		    	Editor ed = prefs.edit();
				for (int i = 0; i < options.length-1; i+=2)
				{
					String key = options[i];
					String valStr = options[i+1];
					if (valStr.compareTo("_dk_") == 0)
					{
						continue;
					}
					
					Object curVal = allPrefs.get(key);
					if (curVal != null)
					{
						if (curVal instanceof Boolean)
						{
							ed.putBoolean(key, ((valStr.compareTo("false")==0) || (valStr.compareTo("0")==0)) ? false : true);
						}
						else if (curVal instanceof Integer)
						{
							int val = 0;
							if (valStr.compareTo("false") == 0)		{ val = 0; }
							else if (valStr.compareTo("true") == 0) { val = 1; }
							else									{ val = Integer.parseInt(valStr); }
							ed.putInt(key, val);
						}
						else if (curVal instanceof Long)
						{
							long val = 0;
							if (valStr.compareTo("false") == 0)		{ val = 0; }
							else if (valStr.compareTo("true") == 0) { val = 1; }
							else									{ val = Long.parseLong(valStr); }
							ed.putLong(key, val);
						}
						else if (curVal instanceof Float)
						{
							float val = Float.parseFloat(valStr);
							ed.putFloat(key, val);
						}
						else if (curVal instanceof String)
						{
							ed.putString(key, valStr);
						}
					}
				}
	
				ed.commit();

				_updateOptions();
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

}
