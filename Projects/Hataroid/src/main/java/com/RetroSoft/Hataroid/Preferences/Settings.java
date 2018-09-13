package com.RetroSoft.Hataroid.Preferences;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
//import android.support.v7.app.ActionBar;
import android.util.Log;
import android.view.Choreographer;
import android.view.Display;
import android.view.KeyEvent;
import android.view.LayoutInflater;
//import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.HataroidTVActivity;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;
import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.InputMapConfigureView;
import com.RetroSoft.Hataroid.Input.Shortcut.ShortcutMapConfigureView;
import com.RetroSoft.Hataroid.Midi.InstrPatchMap;
import com.RetroSoft.Hataroid.SaveState.SaveStateBrowser;
import com.RetroSoft.Hataroid.Util.MapSetConfigureView;

//public class Settings extends AppCompatPreferenceActivity implements OnSharedPreferenceChangeListener
public class Settings extends PreferenceActivity implements OnSharedPreferenceChangeListener
{
	public final static int[] kXMLPrefIDs = new int[]
	{
		R.xml.preferences,
		R.xml.prefs_device,
		R.xml.prefs_display,
		R.xml.prefs_input,
		R.xml.prefs_midi,
		R.xml.prefs_sound,
		R.xml.prefs_storage,
		R.xml.prefs_system,
	};

	private static final int FILEACTIVITYRESULT_STTOSIMAGE				= 1;
	private static final int FILEACTIVITYRESULT_STETOSIMAGE				= 2;
	private static final int FILEACTIVITYRESULT_ACSI_IMAGE				= 3;
	private static final int FILEACTIVITYRESULT_IDEMASTER_IMAGE			= 4;
	private static final int FILEACTIVITYRESULT_IDESLAVE_IMAGE			= 5;
	private static final int FILEACTIVITYRESULT_GEMDOS_FOLDER			= 6;
	//private static final int FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER	= 7;
	private static final int FILEACTIVITYRESULT_SAVESTATE_FOLDER		= 8;
	private static final int FILEACTIVITYRESULT_FLUIDSYNTH_SOUNDFONT	= 9;
	private static final int INPUTMAPACTIVITYRESULT_OK					= 10;
	private static final int SHORTCUTMAPACTIVITYRESULT_OK				= 11;
	private static final int MIDIINSTRPATCHMAPACTIVITYRESULT_OK			= 12;
	//private static final int MIDIPERCPATCHMAPACTIVITYRESULT_OK			= 13;

	public static final String kPrefName_ST_TosImage					= "pref_system_tos";
	public static final String kPrefName_STE_TosImage					= "pref_system_tos_ste";

	public static final String kPrefName_ACSI_Image						= "pref_storage_harddisks_acsiimage";
	public static final String kPrefName_IDEMaster_Image				= "pref_storage_harddisks_idemasterimage";
	public static final String kPrefName_IDESlave_Image					= "pref_storage_harddisks_ideslaveimage";

	public static final String kPrefName_GEMDOS_Folder					= "pref_storage_harddisks_gemdosdrive";

	//public static final String kPrefName_DumpInputMap_Folder			= "pref_input_device_dumpmaps";

	public static final String kPrefName_SaveState_Folder				= "pref_storage_savestate_folder";

	public static final String kPrefName_InputDevice_InputMethod		= "pref_input_device_inputmethod";
	public static final String kPrefName_InputDevice_ConfigureMap		= "pref_input_device_configuremap";
	public final static String kPrefName_Input_KeyboardRegion           = "pref_input_keyboard_region";

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

	public static final String kPrefName_Midi_Hardware_Out				= "pref_midi_hardware_out";
	public static final String kPrefName_Midi_Hardware_In				= "pref_midi_hardware_in";

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

	public static final String kPrefName_Device_VSync       			= "pref_device_dbg_vsync";
	public static final String kPrefName_Device_VSync_Autorate			= "pref_device_dbg_vsync_autorate";

	static final String        kHelpSiteLink					        = "http://sites.google.com/site/hataroid/help";

	public Map<String,Boolean>              _noSetSummary = new HashMap<String,Boolean>();

	private final int                       kMaxFreqMapCnt = 20;
	private final int                       kRefreshMeasureCount = 60*3;
	private Choreographer.FrameCallback     _refreshRateMeasureCallback = null;
	private AlertDialog                     _refreshRateMeasureDialog = null;
	private long                            _lastRefreshRateNanoTime = 0;
	private Map<Long, Integer>              _refreshFreqMap = new HashMap<Long, Integer>();
	private int                             _sampleCount = 0;


	@SuppressWarnings("deprecation")
	@Override protected void onCreate(Bundle savedInstanceState)
	{
		_noSetSummary.clear();
		_noSetSummary.put(kPrefName_InputDevice_InputMethod, true);
		_noSetSummary.put(kPrefName_InputDevice_ConfigureMap, true);
		//_noSetSummary.put(kPrefName_DumpInputMap_Folder, true);
		_noSetSummary.put(kPrefName_OnScreen_ConfigureShortcutsMap, true);
		_noSetSummary.put(kPrefName_Midi_Configure_InstrPatch, true);
		_noSetSummary.put(kPrefName_Midi_Configure_PercPatch, true);

		super.onCreate(savedInstanceState);

		String action = null;
		try
		{
//			ActionBar actionBar = getSupportActionBar();
//			actionBar.setHomeButtonEnabled(true);
//			actionBar.setDisplayHomeAsUpEnabled(true);

			action = getIntent().getAction();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		if (action == null)
		{
			action = "";
		}

		Context appContext = getApplicationContext();

		if (action.equals("com.RetroSoft.Hataroid.action.prefinput"))
		{
			addPreferencesFromResource(R.xml.prefs_input);

			// add input device click hooks
			linkInputDeviceSelector(kPrefName_InputDevice_InputMethod);
			linkConfigureView(kPrefName_InputDevice_ConfigureMap, INPUTMAPACTIVITYRESULT_OK, InputMapConfigureView.class, null, null);

			// dump map folder
			//linkFileSelector(kPrefName_DumpInputMap_Folder, null, FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER, true, true, false, null);

			// add shorcut keys mapping click hooks
			linkConfigureView(kPrefName_OnScreen_ConfigureShortcutsMap, SHORTCUTMAPACTIVITYRESULT_OK, ShortcutMapConfigureView.class, null, null);

			// add on input region changed so the input configuration views work straight away with new region
			linkKeyboardRegionListener();
		}
		else if (action.equals("com.RetroSoft.Hataroid.action.prefsystem"))
		{
			addPreferencesFromResource(R.xml.prefs_system);

			// add rom file chooser hooks
			linkFileSelector(appContext.getString(R.string.select_sttosimage), kPrefName_ST_TosImage, LastTOSDirItemPrefixKey, FILEACTIVITYRESULT_STTOSIMAGE, false, false, false, null);
			linkFileSelector(appContext.getString(R.string.select_stetosimage), kPrefName_STE_TosImage, LastTOSDirItemPrefixKey, FILEACTIVITYRESULT_STETOSIMAGE, false, false, false, null);
		}
		else if (action.equals("com.RetroSoft.Hataroid.action.prefdisplay"))
		{
			addPreferencesFromResource(R.xml.prefs_display);
		}
		else if (action.equals("com.RetroSoft.Hataroid.action.prefsound"))
		{
			addPreferencesFromResource(R.xml.prefs_sound);
		}
		else if (action.equals("com.RetroSoft.Hataroid.action.prefstorage"))
		{
			addPreferencesFromResource(R.xml.prefs_storage);

			// add hd image file chooser hooks
			linkFileSelector(appContext.getString(R.string.select_acsiimage), kPrefName_ACSI_Image, LastHDDImageDirItemPrefixKey, FILEACTIVITYRESULT_ACSI_IMAGE, true, false, false, null);
			linkFileSelector(appContext.getString(R.string.select_idemasterimage), kPrefName_IDEMaster_Image, LastHDDImageDirItemPrefixKey, FILEACTIVITYRESULT_IDEMASTER_IMAGE, true, false, false, null);
			linkFileSelector(appContext.getString(R.string.select_ideslaveimage), kPrefName_IDESlave_Image, LastHDDImageDirItemPrefixKey, FILEACTIVITYRESULT_IDESLAVE_IMAGE, true, false, false, null);

			// gemdos folder
			linkFileSelector(appContext.getString(R.string.select_gemdosdir), kPrefName_GEMDOS_Folder, LastGEMDOSDirItemPrefixKey, FILEACTIVITYRESULT_GEMDOS_FOLDER, true, true, false, null);

			// save state folder
			linkFileSelector(appContext.getString(R.string.select_savestatefolder), kPrefName_SaveState_Folder, LastSaveStateDirItemPrefixKey, FILEACTIVITYRESULT_SAVESTATE_FOLDER, true, true, true, null);

			// kit kat external sd option
			_setupKitKatExtSDOption();
		}
		else if (action.equals("com.RetroSoft.Hataroid.action.prefmidi"))
		{
			addPreferencesFromResource(R.xml.prefs_midi);

			// midi options
			linkConfigureView(kPrefName_Midi_Configure_InstrPatch, MIDIINSTRPATCHMAPACTIVITYRESULT_OK, MapSetConfigureView.class,
					new String[] {MapSetConfigureView.CONFIG_TITLE, MapSetConfigureView.CONFIG_MAPSETCLASSNAME},
					new String[] {appContext.getString(R.string.title_midi_patches), InstrPatchMap.class.getName()});
			//linkConfigureView(kPrefName_Midi_Configure_PercPatch, MIDIPERCPATCHMAPACTIVITYRESULT_OK, MapSetConfigureView.class,
			//		new String[] {MapSetConfigureView.CONFIG_MAPSETCLASSNAME}, new String[] {PercPatchMap.class.getName()});
			_setupMidiOptions();

			// add sound font file chooser hooks
			linkFileSelector(appContext.getString(R.string.select_soundfont), kPrefName_FluidSynth_SoundFont, LastFluidSynthSFDirItemPrefixKey, FILEACTIVITYRESULT_FLUIDSYNTH_SOUNDFONT, true, false, false, new String[] {".sf2"});
		}
		else if (action.equals("com.RetroSoft.Hataroid.action.prefdevice"))
		{
			addPreferencesFromResource(R.xml.prefs_device);

			_createRefreshRateMeasurer();
		}
		else
		{
			addPreferencesFromResource(R.xml.preferences);

			_setupHelpOptions();
		}

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		PreferenceScreen prefscr = getPreferenceScreen();
		Map<String,?> keys = prefs.getAll();
		for (Map.Entry<String,?> entry : keys.entrySet())
		{
	    	Preference pref = prefscr.findPreference(entry.getKey());
	    	if (pref != null)
	    	{
				_setPreferenceSummary(prefs, pref, entry.getKey());
			}
		}

		try
		{
			LayoutInflater inflater = (LayoutInflater)getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			View customPrefsView = inflater.inflate(R.layout.custom_prefs, null);

			setContentView(customPrefsView);

			TextView title = (TextView)customPrefsView.findViewById(R.id.title);
			title.setText(this.getTitle());

			ImageButton navBackBtn = (ImageButton)customPrefsView.findViewById(R.id.nav_back);
			navBackBtn.setOnClickListener(new View.OnClickListener() {
				public void onClick(View view) {
					finish();
				}
			});
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	@Override protected void onDestroy()
	{
		super.onDestroy();

		try {
			if (_refreshRateMeasureDialog != null) {
				_refreshRateMeasureDialog.dismiss();
				_refreshRateMeasureDialog = null;
				Choreographer.getInstance().removeFrameCallback(_refreshRateMeasureCallback);
				Log.i("hataroid", "remove choreographer callback as settings activity getting destroyed");
			}
		} catch (Exception e) {
			e.printStackTrace();
		} catch (Error e) {
			e.printStackTrace();
		}
	}

	@SuppressWarnings("deprecation")
	void linkFileSelector(String title, String prefKey, String lastDirKeyPrefix, int fileResultID, boolean allFiles, boolean selectFolder, boolean showNewFolder, String[] allowedExts)
	{
		final Settings ctx = this;
		final String dialogTitle = title;
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
				fileBrowser.putExtra(FileBrowser.CONFIG_TITLE, dialogTitle);
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
			//case FILEACTIVITYRESULT_DUMPINPUTMAPS_FOLDER:
			//{
			//	if (resultCode == RESULT_OK)
			//	{
			//		String path = data.getStringExtra(FileBrowser.RESULT_PATH);
			//		_dumpInputMap(path);
			//	}
			//	return;
			//}
		}

		if (key != null)
		{
			if (resultCode == RESULT_OK)
			{
				String resPath = data.getStringExtra(FileBrowser.RESULT_PATH);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
				Editor ed = prefs.edit();

				ed.putString(key, resPath);
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

//    void _dumpInputMap(String path)
//	{
//    	boolean done = false;
//
//    	String fname = path + "/inputmaps.txt";
//    	FileOutputStream out = null;
//    	try
//    	{
//    		String userMaps = Input.getUserInputMaps(PreferenceManager.getDefaultSharedPreferences(getApplicationContext()));
//    		out = new FileOutputStream(fname);
//    		if (out != null)
//    		{
//    			out.write(userMaps.getBytes());
//    			out.flush();
//    			out.close();
//    		}
//    		done = true;
//    	}
//    	catch (Exception e)
//    	{
//    		e.printStackTrace();
//    	}
//    	finally
//    	{
//    		try
//    		{
//    			if (out != null)
//    			{
//    				out.close();
//    			}
//    		}
//    		catch (Exception e)
//    		{
//    			e.printStackTrace();
//    		}
//    	}
//
//		if (done)
//    	{
//			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
//			alertDialog.setTitle("Saved Input Maps");
//			alertDialog.setMessage("Your input maps have been saved to:\n" + fname + "\nPlease email these to the developer if you want these in the default presets");
//			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
//			alertDialog.show();
//    	}
//    	else
//    	{
//    		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
//    		alertDialog.setTitle("Error occured");
//    		alertDialog.setMessage("There was a problem saving your input maps. Please try again or log a bug report");
//    		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
//    		alertDialog.show();
//    	}
//	}

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
					public boolean onPreferenceClick(Preference preference) {
						_onSaveStateKitKatExtSDClicked(preference);
						return true;
					}
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
			CheckBoxPreference cbitem = (CheckBoxPreference)pref;
		    boolean isChecked = cbitem.isChecked();

			{
				Preference saveFolderItem = (Preference)findPreference("pref_storage_savestate_folder");
				saveFolderItem.setEnabled(!isChecked);
			}

		    if (isChecked) {
			    File[] paths = getApplicationContext().getExternalFilesDirs(null);
			    if (paths != null && paths.length > 0) {
				    int pathIdx = paths.length - 1; // use the last one

				    boolean updatePrefs = false;
				    String savePath = paths[pathIdx].getAbsolutePath() + "/saves";
				    File newFolder = new File(savePath);
				    if (!newFolder.exists()) {
					    if (newFolder.mkdir()) {
						    updatePrefs = true;
					    }
				    } else {
					    updatePrefs = true;
				    }

				    if (updatePrefs) {
					    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
					    Editor ed = prefs.edit();
					    ed.putString("pref_storage_savestate_folder", savePath);
					    ed.commit();

					    Log.i("hataroid", "Set save folder to external app folder: " + savePath);

					    AlertDialog alertDialog = new AlertDialog.Builder(this).create();
					    alertDialog.setTitle("KIT KAT/Lollipop Warning");
					    alertDialog.setMessage("If you choose to save to External SD Card on KIT KAT+,\nWhen uninstalling Hataroid, Android will delete your saves as well.\nMake sure you manually back up your saves when uninstalling.");
					    alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() {
						    public void onClick(DialogInterface dialog, int which) {
						    }
					    });
					    alertDialog.show();
				    } else {
					    cbitem.setChecked(false);
					    {
						    Preference saveFolderItem = (Preference)findPreference("pref_storage_savestate_folder");
						    saveFolderItem.setEnabled(!isChecked);
					    }
				    }
			    }
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
	    				+ "You can also specify your own SoundFont if you want different synth sounds or higher quality instruments."
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

			cat = (Preference)findPreference(kPrefName_Midi_Hardware_Out);
			if (cat != null) { cat.setEnabled(midiOutEnabled); }
			cat = (Preference)findPreference(kPrefName_Midi_Hardware_In);
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
		boolean isAndroidTV = false;
		try
		{
			if (getPackageManager().hasSystemFeature(PackageManager.FEATURE_TELEVISION)
				|| getPackageManager().hasSystemFeature(PackageManager.FEATURE_LEANBACK)) {
				isAndroidTV = true;
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		try
		{
			if (isAndroidTV) {

				AlertDialog alertDialog = new AlertDialog.Builder(this).create();
				alertDialog.setTitle("Help / Manual");
				alertDialog.setMessage("To view the help / manual pages, please go to the following site on another device (eg. your phone or desktop computer.\n\n"
						+ "(Due to Android TV guidelines not allowing links to web content)\n\n"
						+ "sites.google.com/site/hataroid/help\n"
				);
				alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { } });
				alertDialog.setCancelable(true);
				alertDialog.setCanceledOnTouchOutside(true);
				alertDialog.show();

			} else {

				Intent myIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(kHelpSiteLink));
				startActivity(myIntent);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void _createRefreshRateMeasurer() {

		try {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
				_refreshRateMeasureCallback = new Choreographer.FrameCallback() {
					@Override
					public void doFrame(long frameTimeNanos) {
						if (_lastRefreshRateNanoTime > 0) {
							long dT = frameTimeNanos - _lastRefreshRateNanoTime;
							dT /= 10000;
							dT *= 10000;

							if (_refreshFreqMap.containsKey(dT)) {
								_refreshFreqMap.put(dT, _refreshFreqMap.get(dT) + 1);
							} else if (_refreshFreqMap.size() < kMaxFreqMapCnt) {
								_refreshFreqMap.put(dT, 1);
							} else {
								long minKey = 0;
								long minVal = 0;
								for (Map.Entry<Long,Integer> e : _refreshFreqMap.entrySet()) {
									if (minKey == 0 || e.getValue() < minVal) {
										minKey = e.getKey();
										minVal = e.getValue();
									}
								}
								if (minKey > 0) {
									_refreshFreqMap.remove(minKey);
									_refreshFreqMap.put(dT, 1);
								}
							}
						}
						_lastRefreshRateNanoTime = frameTimeNanos;
						++_sampleCount;
						if (_sampleCount < kRefreshMeasureCount) {
							Choreographer.getInstance().postFrameCallback(this);
						} else {
							if (_refreshRateMeasureDialog != null) {
								_refreshRateMeasureDialog.dismiss();
								_refreshRateMeasureDialog = null;

								// show results dialog
								_showRefreshRateResults();
							}
						}
					}
				};
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		try
		{
			CheckBoxPreference item = (CheckBoxPreference)findPreference(kPrefName_Device_VSync);
			if (item != null)
			{
				item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
					public boolean onPreferenceClick(Preference preference) {
						_onVSyncClicked(preference);
						return true;
					}
				});
			}

			item = (CheckBoxPreference)findPreference(kPrefName_Device_VSync_Autorate);
			if (item != null)
			{
				item.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
					public boolean onPreferenceClick(Preference preference) {
						_onVSyncAutoRateClicked(preference);
						return true;
					}
				});
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void _onVSyncClicked(Preference pref)
	{
		CheckBoxPreference item = (CheckBoxPreference)pref;
		if (item.isChecked()) {
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("(Experimental) Hardware Screen Sync Help (fast device needed)");
			alertDialog.setMessage("- With sync on, most games will run too fast. (most ST games runs at 50hz, but most Android screens are ~60hz)\n"
								 + "- To account for this sound needs to be sped up (pitched higher).\n"
								 + "- You can tick the 'Use Reported Hardware Refresh Rate' to use the value read from your device.\n"
								 + "- If sound skips or buffers too much, try disabling the reported rate and use the override option with the value autodetected from above or your own values\n"
								 + "- You can see if there are sound issues by enabling the Debug Sound Buffer option above\n"
			);
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
				}
			});
			alertDialog.show();
		}
	}

	void _onVSyncAutoRateClicked(Preference pref)
	{
		CheckBoxPreference item = (CheckBoxPreference)pref;
		if (!item.isChecked()) {
			return;
		}

		_lastRefreshRateNanoTime = 0;
		_sampleCount = 0;
		_refreshFreqMap.clear();

		if (_refreshRateMeasureCallback != null) {

			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Measuring Video Refresh Rate");
			alertDialog.setMessage("Please wait a few seconds...");
			alertDialog.setCancelable(false);
			alertDialog.setCanceledOnTouchOutside(false);
			alertDialog.show();

			_refreshRateMeasureDialog = alertDialog;

			Choreographer.getInstance().postFrameCallback(_refreshRateMeasureCallback);

		}
		else
		{

			_showRefreshRateResults();
		}
	}

	void _showRefreshRateResults() {

		float hardwareRefreshRate = 0;
		try {
			Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
			hardwareRefreshRate = display.getRefreshRate();
		} catch (Exception e) {
			e.printStackTrace();
		}

		String msg = "Using Refresh Rate (from driver): " + hardwareRefreshRate;

		float detectedRefreshRate = 0;
		float detectedRefreshRateAvg = 0;
		if (_refreshRateMeasureCallback != null)
		{
			if (_refreshFreqMap.size() > 0) {
				long count = 0;
				long maxKey = 0;
				long maxVal = 0;
				for (Map.Entry<Long,Integer> e : _refreshFreqMap.entrySet()) {
					detectedRefreshRateAvg += e.getValue() * e.getKey();
					if (maxKey == 0 || e.getValue() > maxVal) {
						maxKey = e.getKey();
						maxVal = e.getValue();
					}
					count += e.getValue();
				}
				if (maxKey > 0) {
					detectedRefreshRate = (float)(1000000000.0 / (double)maxKey);
				}
				detectedRefreshRateAvg = (float)(1000000000.0 / (detectedRefreshRateAvg / (double)count));

				_refreshFreqMap.clear();
			}

			if (detectedRefreshRate > 0) {
				msg += "\n\nActual Detected Refresh Rate: " + String.format("%.8f", detectedRefreshRate);
				//msg += "\nDetected Refresh Rate (Avg): " + String.format("%.8f", detectedRefreshRateAvg);

				if (Math.abs(detectedRefreshRate - hardwareRefreshRate) > 0.1f) {
					msg += "\n\n- The actual rate differs from the hardware reported rate. I recommend disabling this option and putting the detected refresh rate in the override option below instead.\n";
					msg += "- You may also want to run this test a few times to make sure the detected value is stable";
				}
			}
		}

		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
		alertDialog.setTitle("Screen Refresh Info");
		alertDialog.setMessage(msg);
		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
			}
		});
		alertDialog.show();
	}

	void linkKeyboardRegionListener()
	{
		try
		{
			ListPreference p = (ListPreference)findPreference(kPrefName_Input_KeyboardRegion);
			if (p != null)
			{
				p.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
					@Override
					public boolean onPreferenceChange(Preference preference, Object newValue) {
						try {
							if (newValue != null && HataroidActivity.instance != null) {
								String localeStr = newValue.toString();
								Input input = HataroidActivity.instance.getInput();
								input.setLocaleID(localeStr, true);

								int newLocaleID = input.getLocaleID();
								String infoMsg = null;
								if (newLocaleID == Input.kLocale_FR) {
									infoMsg = "Vous devez définir une ST TOS ROM française pour fonctionner correctement";
									infoMsg += "\n\n(A French ST TOS ROM needs to be set for this to work correctly)";
								} else if (newLocaleID == Input.kLocale_DE){
									infoMsg = "Sie müssen ein deutsches ST TOS ROM einrichten, damit es ordnungsgemäß funktioniert";
									infoMsg += "\n\n(A German ST TOS ROM needs to be set for this to work correctly)";
								}
								_showKeyboardRegionHelp(infoMsg);
							}
						} catch (Exception e) {
							e.printStackTrace();
						}
						//Log.i("hataroid", "Pref region changed: " + newValue.toString());
						return true;
					}
				});
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		catch (Error e)
		{
			e.printStackTrace();
		}
	}

	void _showKeyboardRegionHelp(String infoMsg)
	{
		if (infoMsg != null) {
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Keyboard Help");
			alertDialog.setMessage(infoMsg);
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
				}
			});
			alertDialog.show();
		}
	}


	void _checkSeekBarKeyInput(int incr, int keyCode, KeyEvent event)
	{
		try
		{
			ListView lv = getListView();
			if (lv != null)
			{
				Object obj = lv.getSelectedItem();
				if (obj != null && obj instanceof SeekBarPreference)
				{
					SeekBarPreference sbpref = (SeekBarPreference) obj;
					//sbpref.incrCurrentValue(incr, false);
					sbpref.onSeekBarKeyDown(keyCode, event);
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		//Log.i("hataroid", "****** Settings KEY EVENT");
		int c = event.getKeyCode();

		if (c == KeyEvent.KEYCODE_DPAD_LEFT)
		{
			if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0)
			{
				event.startTracking();
				return true;
			}
			else if (event.getAction() == KeyEvent.ACTION_MULTIPLE && !event.isCanceled())
			{
				_checkSeekBarKeyInput(-1 * event.getRepeatCount(), c, event);
				return true;
			}
			else if (event.getAction() == KeyEvent.ACTION_UP && !event.isCanceled())
			{
				_checkSeekBarKeyInput(-1, c, event);
				return true;
			}
		}
		else if (c == KeyEvent.KEYCODE_DPAD_RIGHT)
		{
			if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0)
			{
				event.startTracking();
				return true;
			}
			else if (event.getAction() == KeyEvent.ACTION_MULTIPLE && !event.isCanceled())
			{
				_checkSeekBarKeyInput(event.getRepeatCount(), c, event);
				return true;
			}
			else if (event.getAction() == KeyEvent.ACTION_UP && !event.isCanceled())
			{
				_checkSeekBarKeyInput(1, c, event);
				return true;
			}
		}


//		if (/*c == KeyEvent.KEYCODE_BACK ||*/ c == KeyEvent.KEYCODE_BUTTON_10) {
//			if (event.getAction() == KeyEvent.ACTION_DOWN
//					&& event.getRepeatCount() == 0) {
//				//getKeyDispatcherState().startTracking(event, this);
//				//event.startTracking();
//				//return true;
//			} else if (event.getAction() == KeyEvent.ACTION_UP) {
//					//&& getKeyDispatcherState().isTracking(event) && !event.isCanceled()) {
//				//dismiss();
//				//Log.i("hataroid", "****** BACK BUTTON from settings");
//				//onBackPressed();
//				//return true;
//			}
//			return super.dispatchKeyEvent(event);
//		}
//		else

		return super.dispatchKeyEvent(event);
	}

//	@Override
//	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
//		Log.i("hataroid", "***** PREF TREE CLICK: " + preference.getTitle());
//
//		return false;
//	}

//	@Override
//	public boolean onOptionsItemSelected(MenuItem item)
//	{
//		switch (item.getItemId())
//		{
//			case android.R.id.home:
//			{
//				finish();
//				return true;
//			}
//		}
//
//		return super.onOptionsItemSelected(item);
//	}
}
