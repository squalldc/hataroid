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
	private static final int FILEACTIVITYRESULT_STTOSIMAGE = 1;
	private static final int FILEACTIVITYRESULT_STETOSIMAGE = 2;

	private static final String kPrefName_ST_TosImage = "pref_system_tos";
	private static final String kPrefName_STE_TosImage = "pref_system_tos_ste";

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
		linkFileSelector(kPrefName_ST_TosImage, FILEACTIVITYRESULT_STTOSIMAGE);
		linkFileSelector(kPrefName_STE_TosImage, FILEACTIVITYRESULT_STETOSIMAGE);
	}
	
	void linkFileSelector(String prefKey, int fileResultID)
	{
		final Settings ctx = this;
		final int resultID = fileResultID;

		Preference fileSelector = (Preference)findPreference(prefKey);
		fileSelector.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
		        Intent fileBrowser = new Intent(ctx, FileBrowser.class);
		        fileBrowser.putExtra(FileBrowser.CONFIG_OPENZIPS, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_RESETST, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_EXT, new String [] {".img", ".rom"});
		        ctx.startActivityForResult(fileBrowser, resultID);
				return true;
			}
		});
	}

	@Override protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			case FILEACTIVITYRESULT_STTOSIMAGE:
			case FILEACTIVITYRESULT_STETOSIMAGE:
			{
				if (resultCode == RESULT_OK)
				{
					String tosPath = data.getStringExtra(FileBrowser.RESULT_PATH);
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
					Editor ed = prefs.edit();
					String key = (requestCode==FILEACTIVITYRESULT_STTOSIMAGE)?kPrefName_ST_TosImage:kPrefName_STE_TosImage;
					ed.putString(key, tosPath);
					ed.commit();

					onSharedPreferenceChanged(prefs, key);
				}
				break;
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
