package com.RetroSoft.Hataroid.Preferences;

import java.util.Map;

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

import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;

public class Settings extends PreferenceActivity implements OnSharedPreferenceChangeListener
{
	private static final int FILEACTIVITYRESULT_STTOSIMAGE		= 1;
	private static final int FILEACTIVITYRESULT_STETOSIMAGE		= 2;
	private static final int FILEACTIVITYRESULT_ACSI_IMAGE		= 3;
	private static final int FILEACTIVITYRESULT_IDEMASTER_IMAGE	= 4;
	private static final int FILEACTIVITYRESULT_IDESLAVE_IMAGE	= 5;
	private static final int FILEACTIVITYRESULT_GEMDOS_FOLDER	= 6;

	public static final String kPrefName_ST_TosImage		= "pref_system_tos";
	public static final String kPrefName_STE_TosImage		= "pref_system_tos_ste";

	public static final String kPrefName_ACSI_Image			= "pref_storage_harddisks_acsiimage";
	public static final String kPrefName_IDEMaster_Image	= "pref_storage_harddisks_idemasterimage";
	public static final String kPrefName_IDESlave_Image		= "pref_storage_harddisks_ideslaveimage";

	public static final String kPrefName_GEMDOS_Folder		= "pref_storage_harddisks_gemdosdrive";

	@Override protected void onCreate(Bundle savedInstanceState)
	{		
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
		linkFileSelector(kPrefName_ST_TosImage, FILEACTIVITYRESULT_STTOSIMAGE, false, false, true);
		linkFileSelector(kPrefName_STE_TosImage, FILEACTIVITYRESULT_STETOSIMAGE, false, false, true);
		
		// add hd image file chooser hooks
		linkFileSelector(kPrefName_ACSI_Image, FILEACTIVITYRESULT_ACSI_IMAGE, true, false, false);
		linkFileSelector(kPrefName_IDEMaster_Image, FILEACTIVITYRESULT_IDEMASTER_IMAGE, true, false, false);
		linkFileSelector(kPrefName_IDESlave_Image, FILEACTIVITYRESULT_IDESLAVE_IMAGE, true, false, false);
		
		// gemdos folder
		linkFileSelector(kPrefName_GEMDOS_Folder, FILEACTIVITYRESULT_GEMDOS_FOLDER, true, true, false);
	}
	
	void linkFileSelector(String prefKey, int fileResultID, boolean allFiles, boolean selectFolder, boolean tosImage)
	{
		final Settings ctx = this;
		final int resultID = fileResultID;
		final boolean allowAllFiles = allFiles;
		final boolean chooseFolder = selectFolder;
		final boolean isTosImage = tosImage;

		Preference fileSelector = (Preference)findPreference(prefKey);
		fileSelector.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
		        Intent fileBrowser = new Intent(ctx, FileBrowser.class);
		        fileBrowser.putExtra(FileBrowser.CONFIG_OPENZIPS, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_RESETST, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_SELECTFOLDER, chooseFolder);
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
		}
		
		if (key != null)
		{
			if (resultCode == RESULT_OK)
			{
				String tosPath = data.getStringExtra(FileBrowser.RESULT_PATH);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
				Editor ed = prefs.edit();
				ed.putString(key, tosPath);
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
}
