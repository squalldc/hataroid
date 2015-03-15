package com.RetroSoft.Hataroid.Preferences;

import java.io.File;
import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Map;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
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
import com.RetroSoft.Hataroid.Input.Shortcut.ShortcutMapConfigureView;
import com.RetroSoft.Hataroid.Midi.InstrPatchMap;
import com.RetroSoft.Hataroid.SaveState.SaveStateBrowser;
import com.RetroSoft.Hataroid.Util.MapSetConfigureView;

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
	private static final int FILEACTIVITYRESULT_FLUIDSYNTH_SOUNDFONT	= 9;
	private static final int INPUTMAPACTIVITYRESULT_OK					= 10;
	private static final int SHORTCUTMAPACTIVITYRESULT_OK				= 11;
	private static final int MIDIINSTRPATCHMAPACTIVITYRESULT_OK			= 12;
	private static final int MIDIPERCPATCHMAPACTIVITYRESULT_OK			= 13;

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
	
	public static final String kPrefName_OnScreen_ConfigureShortcutsMap	= "pref_input_onscreen_configureshortcutmap";
	
	public static final String kPrefName_Storage_SaveStateKitKatExtSD	= "pref_storage_savestate_kitkat_extsd";
	public static final String kPrefName_Storage_SaveStateFolder		= "pref_storage_savestate_folder";

	public static final String kPrefName_Midi_Out						= "pref_midi_out";
	public static final String kPrefName_FluidSynth_UseCustomSF			= "pref_midi_fluidsynth_use_custom_sf";
	public static final String kPrefName_FluidSynth_SoundFont			= "pref_midi_fluidsynth_soundfont";
	public static final String kPrefName_Midi_FluidSynth_Category		= "pref_midi_fluidsynth_category";
	public static final String kPrefName_Midi_Patches_Category			= "pref_midi_patches_category";
	public static final String kPrefName_Midi_Configure_InstrPatch		= "pref_midi_instrument_patches_configure";
	public static final String kPrefName_Midi_Configure_PercPatch		= "pref_midi_drum_patches_configure";

	public static final String kPrefName_Midi_MuteSTSound				= "pref_sound_mute";
	public static final String kPrefName_Midi_Tweak_Ignore_Pgm_Changes	= "pref_midi_tweak_ignore_pgm_changes";
	public static final String kPrefName_Midi_Chan_Transpose			= "pref_midi_chan_transpose";
	public static final String kPrefName_Midi_Setup_KK2Tweaks			= "pref_midi_setup_kk2tweaks";

	public static final String kPrefName_HelpCategory					= "pref_help_online_category";

	public static final String LastTOSDirItemPrefixKey					= "pref_system_tosimage_lastdir";
	public static final String LastHDDImageDirItemPrefixKey				= "pref_system_hddimage_lastdir";
	public static final String LastGEMDOSDirItemPrefixKey				= "pref_system_gemdos_lastdir";
	public static final String LastSaveStateDirItemPrefixKey			= "pref_system_sstate_lastdir";
	public static final String LastFluidSynthSFDirItemPrefixKey			= "pref_system_fssfont_lastdir";

	static final String kHelpSiteLink									= "http://sites.google.com/site/hataroid/help";

	public Map<String,Boolean> _noSetSummary = new HashMap<String,Boolean>();

	@SuppressWarnings("deprecation")
	@Override protected void onCreate(Bundle savedInstanceState)
	{
		_noSetSummary.clear();
		_noSetSummary.put(kPrefName_InputDevice_InputMethod, true);
		_noSetSummary.put(kPrefName_InputDevice_ConfigureMap, true);
		_noSetSummary.put(kPrefName_DumpInputMap_Folder, true);
		_noSetSummary.put(kPrefName_OnScreen_ConfigureShortcutsMap, true);
		_noSetSummary.put(kPrefName_Midi_Configure_InstrPatch, true);
		_noSetSummary.put(kPrefName_Midi_Configure_PercPatch, true);
		
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
		linkFileSelector(kPrefName_ST_TosImage, LastTOSDirItemPrefixKey, FILEACTIVITYRESULT_STTOSIMAGE, false, false, false, null);
		linkFileSelector(kPrefName_STE_TosImage, LastTOSDirItemPrefixKey, FILEACTIVITYRESULT_STETOSIMAGE, false, false, false, null);
		
		// add hd image file chooser hooks
		linkFileSelector(kPrefName_ACSI_Image, LastHDDImageDirItemPrefixKey, FILEACTIVITYRESULT_ACSI_IMAGE, true, false, false, null);
		linkFileSelector(kPrefName_IDEMaster_Image, LastHDDImageDirItemPrefixKey, FILEACTIVITYRESULT_IDEMASTER_IMAGE, true, false, false, null);
		linkFileSelector(kPrefName_IDESlave_Image, LastHDDImageDirItemPrefixKey, FILEACTIVITYRESULT_IDESLAVE_IMAGE, true, false, false, null);
		
		// gemdos folder
		linkFileSelector(kPrefName_GEMDOS_Folder, LastGEMDOSDirItemPrefixKey, FILEACTIVITYRESULT_GEMDOS_FOLDER, true, true, false, null);
		
		// add input device click hooks
		linkInputDeviceSelector(kPrefName_InputDevice_InputMethod);
		linkConfigureView(kPrefName_InputDevice_ConfigureMap, INPUTMAPACTIVITYRESULT_OK, InputMapConfigureView.class, null, null);
		
		// dump map folder
		//linkFileSelector(kPrefName_DumpInputMap_Folder, null, FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER, true, true, false, null);
		
		// save state folder
		linkFileSelector(kPrefName_SaveState_Folder, LastSaveStateDirItemPrefixKey, FILEACTIVITYRESULT_SAVESTATE_FOLDER, true, true, true, null);

		// add shorcut keys mapping click hooks
		linkConfigureView(kPrefName_OnScreen_ConfigureShortcutsMap, SHORTCUTMAPACTIVITYRESULT_OK, ShortcutMapConfigureView.class, null, null);
		
		// kit kat external sd option
		_setupKitKatExtSDOption();
		
		// midi options
		linkConfigureView(kPrefName_Midi_Configure_InstrPatch, MIDIINSTRPATCHMAPACTIVITYRESULT_OK, MapSetConfigureView.class,
				new String[] {MapSetConfigureView.CONFIG_MAPSETCLASSNAME}, new String[] {InstrPatchMap.class.getName()});
		//linkConfigureView(kPrefName_Midi_Configure_PercPatch, MIDIPERCPATCHMAPACTIVITYRESULT_OK, MapSetConfigureView.class,
		//		new String[] {MapSetConfigureView.CONFIG_MAPSETCLASSNAME}, new String[] {PercPatchMap.class.getName()});
		_setupMidiOptions();

		// add sound font file chooser hooks
		linkFileSelector(kPrefName_FluidSynth_SoundFont, LastFluidSynthSFDirItemPrefixKey, FILEACTIVITYRESULT_FLUIDSYNTH_SOUNDFONT, true, false, false, new String[] {".sf2"});

		_setupHelpOptions();
	}
	
	@SuppressWarnings("deprecation")
	void linkFileSelector(String prefKey, String lastDirKeyPrefix, int fileResultID, boolean allFiles, boolean selectFolder, boolean showNewFolder, String[] allowedExts)
	{
		final Settings ctx = this;
		final int resultID = fileResultID;
		final boolean allowAllFiles = allFiles;
		final boolean chooseFolder = selectFolder;
		final boolean newFolder = showNewFolder;
		final String lastDirPrefix = lastDirKeyPrefix;
		final String[] exts = (allowedExts!=null) ? allowedExts : (allowAllFiles ? new String[] {"*"} : new String[] {".img", ".rom"});

		Preference fileSelector = (Preference)findPreference(prefKey);
		fileSelector.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
		        Intent fileBrowser = new Intent(ctx, FileBrowser.class);
		        fileBrowser.putExtra(FileBrowser.CONFIG_OPENZIPS, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_RESETST, false);
		        fileBrowser.putExtra(FileBrowser.CONFIG_SELECTFOLDER, chooseFolder);
		        fileBrowser.putExtra(FileBrowser.CONFIG_NEWFOLDER, newFolder);
		        fileBrowser.putExtra(FileBrowser.CONFIG_EXT, exts);
		        if (lastDirPrefix != null)
		        {
			        fileBrowser.putExtra(FileBrowser.CONFIG_PREFLASTITEMPATH, lastDirPrefix+"_itempath");
			        fileBrowser.putExtra(FileBrowser.CONFIG_PREFLASTITEMNAME, lastDirPrefix+"_itemname");
		        }

		        ctx.startActivityForResult(fileBrowser, resultID);
				return true;
			}
		});
	}
	
	@SuppressWarnings("deprecation")
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

	@SuppressWarnings("deprecation")
	void linkConfigureView(String prefKey, int viewResultID, Class<?> c, String[] extraOptionNames, String[] extraOptionVals)
	{
		final Settings ctx = this;
		final int resultID = viewResultID;
		final Class<?> sc = c;
		final String[] extOpts = extraOptionNames;
		final String[] extOptVals = extraOptionVals;
		Preference item = (Preference)findPreference(prefKey);

		item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference preference) {
		        Intent view = new Intent(ctx, sc);
		        if (extOpts != null && extOptVals != null)
		        {
		        	for (int i = 0; i < extOpts.length; ++i)
		        	{
		        		view.putExtra(extOpts[i], extOptVals[i]);
		        	}
		        }
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
			case FILEACTIVITYRESULT_FLUIDSYNTH_SOUNDFONT:	key = kPrefName_FluidSynth_SoundFont; break;
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

    @SuppressWarnings("deprecation")
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
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
			alertDialog.show();
    	}
    	else
    	{
    		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
    		alertDialog.setTitle("Error occured");
    		alertDialog.setMessage("There was a problem saving your input maps. Please try again or log a bug report");
    		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
    		alertDialog.show();
    	}
	}

	@SuppressWarnings("deprecation")
	void _setupKitKatExtSDOption()
	{
		try
		{
			CheckBoxPreference item = (CheckBoxPreference)findPreference(kPrefName_Storage_SaveStateKitKatExtSD);
			if (item != null)
			{
				if (Build.VERSION.SDK_INT >= 19)
				{
					Preference saveFolderItem = (Preference)findPreference(kPrefName_Storage_SaveStateFolder);
					saveFolderItem.setEnabled(!item.isChecked());
				}
				else
				{
					item.setEnabled(false);
				}
			}
			if (Build.VERSION.SDK_INT >= 19)
			{
				item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
					public boolean onPreferenceClick(Preference preference) { _onSaveStateKitKatExtSDClicked(preference); return true; }
				});
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
    @SuppressWarnings("deprecation")
	void _onSaveStateKitKatExtSDClicked(Preference pref)
	{
    	try
    	{
    		boolean isChecked = false;
			{
				CheckBoxPreference item = (CheckBoxPreference)pref;
				isChecked = item.isChecked();
			}
			
			{
				Preference saveFolderItem = (Preference)findPreference("pref_storage_savestate_folder");
				saveFolderItem.setEnabled(!isChecked);
			}
			
			File[] paths = getExternalFilesDirs(null);
    		if (paths != null && paths.length > 1)
    		{
    			boolean updatePrefs = false;
    			String savePath = paths[1].getAbsolutePath() + "/saves"; // just use first ext for now
    			File newFolder = new File(savePath);
				if (!newFolder.exists())
				{
					if (newFolder.mkdir())
					{
						updatePrefs = true;
					}
				}
				else
				{
					updatePrefs = true;
				}
				
				if (updatePrefs)
				{
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
					Editor ed = prefs.edit();
					ed.putString("pref_storage_savestate_folder", savePath);
					ed.commit();
				}
    		}

    		if (isChecked)
			{
	    		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
	    		alertDialog.setTitle("KIT KAT/Lollipop Warning");
	    		alertDialog.setMessage("If you choose to save to External SD Card on KIT KAT/Lollipop,\nWhen uninstalling Hataroid, Android will delete your saves as well.\nMake sure you manually back up your saves when uninstalling.");
	    		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
	    		alertDialog.show();
			}
    	}
    	catch (Error e)
    	{
    		e.printStackTrace();
    	}
    	catch (Exception e)
    	{
    		e.printStackTrace();
    	}
	}
    
	@SuppressWarnings("deprecation")
	void _setupMidiOptions()
	{
		try
		{
			{
				CheckBoxPreference item = (CheckBoxPreference)findPreference(kPrefName_Midi_Out);
				if (item != null)
				{
					item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
						public boolean onPreferenceClick(Preference preference) { _onMidiOutClicked(preference); return true; }
					});
				}

				item = (CheckBoxPreference)findPreference(kPrefName_FluidSynth_UseCustomSF);
				if (item != null)
				{
					item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
						public boolean onPreferenceClick(Preference preference) { _onMidiFluidSynthUseCustomSFClicked(preference); return true; }
					});
				}
			}

			{
				Preference item = findPreference(kPrefName_Midi_Setup_KK2Tweaks);
				if (item != null)
				{
					item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
						public boolean onPreferenceClick(Preference preference) { _onMidiKK2TweaksClicked(preference); return true; }
					});
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		_updateMidiVisibility();
	}

	void _onMidiOutClicked(Preference pref)
	{
		_updateMidiVisibility();

		try
    	{
			CheckBoxPreference item = (CheckBoxPreference)pref;
    		boolean isChecked = item.isChecked();

    		if (isChecked)
			{
	    		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
	    		alertDialog.setTitle("MIDI information");
	    		alertDialog.setMessage("MIDI output is cpu intensive. A very fast device is required.\n"
	    				+ "If you have stuttering audio, try lowering the emulation sample rate (eg to 22050hz or below) in the Sound section or reducing the Max Polyphony\n\n"
	    				+ "You can also specify your own SoundFont if you want different synth sounds or higher quality instruments.\n\n"
                        + "NOTE: Notator works, but currently requires the ST's cpu to be set to 16Mhz"
	    				);
	    		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
	    		alertDialog.show();
			}
    	}
    	catch (Error e)
    	{
    		e.printStackTrace();
    	}
    	catch (Exception e)
    	{
    		e.printStackTrace();
    	}
	}

	void _onMidiFluidSynthUseCustomSFClicked(Preference pref)
	{
    	_updateMidiVisibility();
	}

    @SuppressWarnings("deprecation")
	void _updateMidiVisibility()
	{
		try
		{
			boolean midiOutEnabled = false;

			CheckBoxPreference item = (CheckBoxPreference)findPreference(kPrefName_Midi_Out);
			if (item != null)
			{
				midiOutEnabled = item.isChecked();
			}

			Preference cat = (Preference)findPreference(kPrefName_Midi_FluidSynth_Category);
			if (cat != null) { cat.setEnabled(midiOutEnabled); }

			cat = (Preference)findPreference(kPrefName_Midi_Patches_Category);
			if (cat != null) { cat.setEnabled(midiOutEnabled); }

			cat = (Preference)findPreference(kPrefName_Midi_Tweak_Ignore_Pgm_Changes);
			if (cat != null) { cat.setEnabled(midiOutEnabled); }
			cat = (Preference)findPreference(kPrefName_Midi_Chan_Transpose);
			if (cat != null) { cat.setEnabled(midiOutEnabled); }
			cat = (Preference)findPreference(kPrefName_Midi_Setup_KK2Tweaks);
			if (cat != null) { cat.setEnabled(midiOutEnabled); }

			{
				boolean showCustomSFFile = false;
				CheckBoxPreference customSF = (CheckBoxPreference)findPreference(kPrefName_FluidSynth_UseCustomSF);
				if (customSF != null) { showCustomSFFile = customSF.isChecked(); }

				Preference sf = (Preference)findPreference(kPrefName_FluidSynth_SoundFont);
				if (sf != null) { sf.setEnabled(showCustomSFFile); }

				Preference percPatchPref = findPreference(kPrefName_Midi_Configure_PercPatch);
				if (percPatchPref != null) { percPatchPref.setEnabled(false); } // NOT IMPLEMENTED YET
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void _onMidiKK2TweaksClicked(Preference pref)
	{
		try
		{
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Karate Kid 2 Tweaks");
			alertDialog.setMessage("This will automatically set the following recommended options for Karate Kid 2 Midi output:\n\n"
							+ "    - Mute ST sound output\n"
							+ "    - Setup recommended instruments\n"
							+ "    - Ignore program changes\n"
							+ "    - Transponse keys by a tone (the original output is off-key)\n"
			);
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Cancel", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { } });
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _applyKK2Options(); } });
			alertDialog.setCancelable(true);
			alertDialog.setCanceledOnTouchOutside(true);
			alertDialog.show();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	@SuppressWarnings("deprecation")
	void _applyKK2Options()
	{
		try
		{
			{
				CheckBoxPreference item = (CheckBoxPreference) findPreference(kPrefName_Midi_MuteSTSound);
				if (item != null) { item.setChecked(true); }
				item = (CheckBoxPreference) findPreference(kPrefName_Midi_Tweak_Ignore_Pgm_Changes);
				if (item != null) { item.setChecked(true); }
			}

			{
				SeekBarPreference item = (SeekBarPreference) findPreference(kPrefName_Midi_Chan_Transpose);
				if (item != null) { item.setCurrentValue(2, true); }
			}

			{
				InstrPatchMap patch = new InstrPatchMap();
				patch.initFromPreset(InstrPatchMap.getKK2PresetPatchID());
				patch.sendPatchDirect();
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	@SuppressWarnings("deprecation")
	void _setupHelpOptions()
	{
		try
		{
			Preference item = findPreference(kPrefName_HelpCategory);
			if (item != null)
			{
				item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
					public boolean onPreferenceClick(Preference preference) { _onHelpClicked(preference); return true; }
				});
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		_updateMidiVisibility();
	}

	void _onHelpClicked(Preference pref)
	{
		try
		{
			Intent myIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(kHelpSiteLink));
			startActivity(myIntent);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
