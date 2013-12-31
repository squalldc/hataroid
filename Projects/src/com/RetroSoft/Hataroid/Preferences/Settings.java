package com.RetroSoft.Hataroid.Preferences;

import java.util.Map;

import com.RetroSoft.Hataroid.R;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;

public class Settings extends PreferenceActivity implements OnSharedPreferenceChangeListener
{
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
	    	_setPreferenceSummary(pref, entry.getKey());
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
    	_setPreferenceSummary(findPreference(key), key);
	}
    
    private void _setPreferenceSummary(Preference pref, String key)
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
    }
}
