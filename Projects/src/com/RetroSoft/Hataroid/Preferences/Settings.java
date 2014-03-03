package com.RetroSoft.Hataroid.Preferences;

import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Map;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;
import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.InputMapConfigureView;
import com.RetroSoft.Hataroid.SaveState.SaveStateBrowser;

public class Settings extends PreferenceActivity implements OnSharedPreferenceChangeListener
{
	private static final int FILEACTIVITYRESULT_STTOSIMAGE				= 1;
	private static final int FILEACTIVITYRESULT_STETOSIMAGE				= 2;
	private static final int FILEACTIVITYRESULT_ACSI_IMAGE				= 3;
	private static final int FILEACTIVITYRESULT_IDEMASTER_IMAGE			= 4;
	private static final int FILEACTIVITYRESULT_IDESLAVE_IMAGE			= 5;
	private static final int FILEACTIVITYRESULT_GEMDOS_FOLDER			= 6;
	private static final int FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER	= 7;
	private static final int FILEACTIVITYRESULT_SAVESTATE_FOLDER		= 8;
	private static final int INPUTMAPACTIVITYRESULT_OK					= 9;

	
		
	public static final String kPrefName_ST_TosImage		= "pref_system_tos";
	public static final String kPrefName_STE_TosImage		= "pref_system_tos_ste";

	public static final String kPrefName_ACSI_Image			= "pref_storage_harddisks_acsiimage";
	public static final String kPrefName_IDEMaster_Image	= "pref_storage_harddisks_idemasterimage";
	public static final String kPrefName_IDESlave_Image		= "pref_storage_harddisks_ideslaveimage";

	public static final String kPrefName_GEMDOS_Folder		= "pref_storage_harddisks_gemdosdrive";

	public static final String kPrefName_DumpInputMap_Folder			= "pref_input_device_dumpmaps";

	public static final String kPrefName_SaveState_Folder				= "pref_storage_savestate_folder";

	public static final String kPrefName_InputDevice_InputMethod		= "pref_input_device_inputmethod";
	public static final String kPrefName_InputDevice_ConfigureMap		= "pref_input_device_configuremap";
	
	public Map<String,Boolean> _noSetSummary = new HashMap<String,Boolean>();

	@Override protected void onCreate(Bundle savedInstanceState)
	{
		_noSetSummary.clear();
		_noSetSummary.put(kPrefName_InputDevice_InputMethod, true);
		_noSetSummary.put(kPrefName_InputDevice_ConfigureMap, true);
		_noSetSummary.put(kPrefName_DumpInputMap_Folder, true);
		
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		PreferenceScreen prefscr = getPreferenceScreen();
		Map<String,?> keys = prefs.getAll();
		for (Map.Entry<String,?> entry : keys.entrySet())
		{
	    	Preference pref = prefscr.findPreference(entry.getKey());
	    	_setPreferenceSummary(prefs, pref, entry.getKey());
		}

		// add rom file chooser hooks
		linkFileSelector(kPrefName_ST_TosImage, FILEACTIVITYRESULT_STTOSIMAGE, false, false, true, false);
		linkFileSelector(kPrefName_STE_TosImage, FILEACTIVITYRESULT_STETOSIMAGE, false, false, true, false);
		
		// add hd image file chooser hooks
		linkFileSelector(kPrefName_ACSI_Image, FILEACTIVITYRESULT_ACSI_IMAGE, true, false, false, false);
		linkFileSelector(kPrefName_IDEMaster_Image, FILEACTIVITYRESULT_IDEMASTER_IMAGE, true, false, false, false);
		linkFileSelector(kPrefName_IDESlave_Image, FILEACTIVITYRESULT_IDESLAVE_IMAGE, true, false, false, false);
		
		// gemdos folder
		linkFileSelector(kPrefName_GEMDOS_Folder, FILEACTIVITYRESULT_GEMDOS_FOLDER, true, true, false, false);
		
		// add input device click hooks
		linkInputDeviceSelector(kPrefName_InputDevice_InputMethod);
		linkInputMethodConfigureView(kPrefName_InputDevice_ConfigureMap, INPUTMAPACTIVITYRESULT_OK);
		
		// dump map folder
		linkFileSelector(kPrefName_DumpInputMap_Folder, FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER, true, true, false, false);
		
		// save state folder
		linkFileSelector(kPrefName_SaveState_Folder, FILEACTIVITYRESULT_SAVESTATE_FOLDER, true, true, false, true);
	}
	
	void linkFileSelector(String prefKey, int fileResultID, boolean allFiles, boolean selectFolder, boolean tosImage, boolean showNewFolder)
	{
		final Settings ctx = this;
		final int resultID = fileResultID;
		final boolean allowAllFiles = allFiles;
		final boolean chooseFolder = selectFolder;
		final boolean isTosImage = tosImage;
		final boolean newFolder = showNewFolder;

		Preference fileSelector = (Preference)findPreference(prefKey);
		fileSelector.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
		        Intent fileBrowser = new Intent(ctx, FileBrowser.class);
		        fileBrowser.putExtra(FileBrowser.CONFIG_OPENZIPS, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_RESETST, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_SELECTFOLDER, chooseFolder);
		        fileBrowser.putExtra(FileBrowser.CONFIG_NEWFOLDER, newFolder);
		        fileBrowser.putExtra(FileBrowser.CONFIG_EXT, allowAllFiles ? new String[] {"*"} : new String[] {".img", ".rom"});
		        if (isTosImage)
		        {
			        fileBrowser.putExtra(FileBrowser.CONFIG_PREFLASTITEMPATH, FileBrowser.LastTOSDirItemPathKey);
			        fileBrowser.putExtra(FileBrowser.CONFIG_PREFLASTITEMNAME, FileBrowser.LastTOSDirItemNameKey);
		        }

		        ctx.startActivityForResult(fileBrowser, resultID);
				return true;
			}
		});
	}
	
	void linkInputDeviceSelector(String prefKey)
	{
		Preference item = (Preference)findPreference(prefKey);

		item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
				// show input method selector
				Input input = HataroidActivity.instance.getInput();
				input.showInputMethodSelector();
				return true;
			}
		});
	}

	void linkInputMethodConfigureView(String prefKey, int viewResultID)
	{
		final Settings ctx = this;
		final int resultID = viewResultID;
		Preference item = (Preference)findPreference(prefKey);

		item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
		        Intent view = new Intent(ctx, InputMapConfigureView.class);
		        ctx.startActivityForResult(view, resultID);
				return true;
			}
		});
	}

	@Override protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		String key = null;
		switch (requestCode)
		{
			case FILEACTIVITYRESULT_STTOSIMAGE:			key = kPrefName_ST_TosImage; break;
			case FILEACTIVITYRESULT_STETOSIMAGE:		key = kPrefName_STE_TosImage; break;
			case FILEACTIVITYRESULT_ACSI_IMAGE:			key = kPrefName_ACSI_Image; break;
			case FILEACTIVITYRESULT_IDEMASTER_IMAGE:	key = kPrefName_IDEMaster_Image; break;
			case FILEACTIVITYRESULT_IDESLAVE_IMAGE:		key = kPrefName_IDESlave_Image; break;
			case FILEACTIVITYRESULT_GEMDOS_FOLDER:		key = kPrefName_GEMDOS_Folder; break;
			case FILEACTIVITYRESULT_SAVESTATE_FOLDER:	key = kPrefName_SaveState_Folder; break;
			case FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER:
			{
				if (resultCode == RESULT_OK)
				{
					String path = data.getStringExtra(FileBrowser.RESULT_PATH);
					_dumpInputMap(path);
				}
				return;
			}
		}
		
		if (key != null)
		{
			if (resultCode == RESULT_OK)
			{
				String tosPath = data.getStringExtra(FileBrowser.RESULT_PATH);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
				Editor ed = prefs.edit();
				
				ed.putString(key, tosPath);
				if (requestCode == FILEACTIVITYRESULT_SAVESTATE_FOLDER)
				{
					ed.putInt(SaveStateBrowser.kPrefQuickSaveSlot, -1);
				}
				
				ed.commit();

				onSharedPreferenceChanged(prefs, key);
			}
		}
	}
	
	@Override protected void onResume()
	{
		super.onResume();

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.registerOnSharedPreferenceChangeListener(this);
	}

	@Override protected void onPause()
	{
		super.onPause();

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.unregisterOnSharedPreferenceChangeListener(this);
	}

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key)
	{
    	_setPreferenceSummary(sharedPreferences, findPreference(key), key);
	}
    
    private void _setPreferenceSummary(SharedPreferences sharedPrefs, Preference pref, String key)
    {
    	if (key == null || _noSetSummary.containsKey(key))
    	{
    		return;
    	}

    	if (pref instanceof ListPreference)
		{
			ListPreference listPref = (ListPreference) pref;
			pref.setSummary(listPref.getEntry());
		}
		else if (pref instanceof EditTextPreference)
		{
			EditTextPreference editPref = (EditTextPreference)pref;
			pref.setSummary(editPref.getText());
		}
		else if (pref != null)
		{
			// only support string for now
			String prefVal = "";
			if (sharedPrefs != null)
			{
				try
				{
					prefVal = sharedPrefs.getString(key, "");
				}
				catch (Exception e)
				{
					prefVal = "";
				}
			}
			
			if (prefVal != null)
			{
				pref.setSummary(prefVal);
			}
		}
    }

    void _dumpInputMap(String path)
	{
    	boolean done = false;

    	String fname = path + "/inputmaps.txt";
    	FileOutputStream out = null;
    	try
    	{
    		String userMaps = Input.getUserInputMaps(PreferenceManager.getDefaultSharedPreferences(getApplicationContext()));
    		out = new FileOutputStream(fname);
    		if (out != null)
    		{
    			out.write(userMaps.getBytes());
    			out.flush();
    			out.close();
    		}
    		done = true;
    	}
    	catch (Exception e)
    	{
    		e.printStackTrace();
    	}
    	finally
    	{
    		try
    		{
    			if (out != null)
    			{
    				out.close();
    			}
    		}
    		catch (Exception e)
    		{
    			e.printStackTrace();
    		}
    	}
    	
		if (done)
    	{
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Saved Input Maps");
			alertDialog.setMessage("Your input maps have been saved to:\n" + fname + "\nPlease email these to the developer if you want these in the default presets");
			alertDialog.setButton("Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
			alertDialog.show();
    	}
    	else
    	{
    		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
    		alertDialog.setTitle("Error occured");
    		alertDialog.setMessage("There was a problem saving your input maps. Please try again or log a bug report");
    		alertDialog.setButton("Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
    		alertDialog.show();
    	}
	}
}
