package com.RetroSoft.Hataroid.SaveState;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.Display;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.ListView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.Preferences.Settings;
import com.RetroSoft.Hataroid.Util.BitFlags;

public class SaveStateBrowser extends ListActivity {
	static final int kMaxSlots = 1000;

	static final int ACTIVITYRESULT_SETTINGS = 0;

	public static final int SSMODE_SAVE = 0;
	public static final int SSMODE_LOAD = 1;
	public static final int SSMODE_DELETE = 2;
	public static final int SSMODE_QUICKSAVESLOT = 3;

	public static final String CONFIG_MODE = "Config_Mode";

	public static final String RESULT_SAVESTATE_FILENAME = "Result_SaveStateFileName";
	public static final String RESULT_SAVESTATE_SLOT = "Result_SaveStateSlot";

	static final String kPrefLastLoadedSlot = "_pref_savestate_lastloadedslot";
	public static final String kPrefQuickSaveSlot = "pref_savestate_quicksaveslot";

	//private static boolean		_firstCreate = true;

	private int _mode = SSMODE_SAVE;

	private String _saveFolder = null;
	private File _curDir;
	private int _lastSavedSlot = 0;
	private int _quickSaveSlot = -1;

	private SaveStateAdapter _adapter;
	private BitFlags _usedSlots = new BitFlags(kMaxSlots);

	private Intent _retIntent;

	public static int dispWidth = 1024;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		try {
			requestWindowFeature(Window.FEATURE_NO_TITLE);

			Display display = getWindowManager().getDefaultDisplay();
			dispWidth = display.getWidth();

			// inflate and adjust layout
			LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			View layout = inflater.inflate(R.layout.savestate_view, null);
			layout.setMinimumWidth((int) (dispWidth * 0.65f));

			setContentView(layout);
		} catch (Exception e) {
			e.printStackTrace();
		}

		parseOptions(savedInstanceState);
		setupButtonListeners();

		int titleStrId = -1;
		switch (_mode) {
			case SSMODE_SAVE:
				titleStrId = R.string.savestate_save_title;
				break;
			case SSMODE_LOAD:
				titleStrId = R.string.savestate_load_title;
				break;
			case SSMODE_DELETE:
				titleStrId = R.string.savestate_delete_title;
				break;
			case SSMODE_QUICKSAVESLOT:
				titleStrId = R.string.savestate_quicksaveslot_title;
				break;
		}
		if (titleStrId >= 0) {
			String titleStr = getResources().getString(titleStrId);
			if (titleStr != null) {
				setUserTitleText(titleStr);
			}
		}

		_retIntent = new Intent();

		//if (_firstCreate)
		{
			try {
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
				_saveFolder = prefs.getString(Settings.kPrefName_SaveState_Folder, null);
				_lastSavedSlot = prefs.getInt(kPrefLastLoadedSlot, 0);
				_quickSaveSlot = prefs.getInt(kPrefQuickSaveSlot, -1);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		String invalidSaveFolderMessage = "Unable to access save state folder. Please check your settings and then retry";
		if (_saveFolder == null || _saveFolder.length() == 0) {
			_saveFolder = null;
			invalidSaveFolderMessage = "Please set up the Save State Folder in the Settings first and then retry";
		} else {
			String testFolder = _saveFolder;
			_saveFolder = null;
			try {
				File rootDir = new File(testFolder);
				if (rootDir.exists() && rootDir.canRead()) {
					_saveFolder = rootDir.getAbsolutePath();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		if (_saveFolder != null) {
			_curDir = new File(_saveFolder);
			_retrieveSaveFiles(_curDir);
		} else {
			_showErrorDialog(invalidSaveFolderMessage);
		}
	}

	void _showErrorDialog(String msg) {
		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
		alertDialog.setTitle("Invalid Save State Folder");
		alertDialog.setMessage(msg);
		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				_showSettingsDialog();
			}
		});
		alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
			public void onCancel(DialogInterface dialog) {
				sendFinish(RESULT_CANCELED);
			}
		});
		alertDialog.show();
	}

	void _showSettingsDialog() {
		Intent settings = new Intent(this, Settings.class);
		startActivityForResult(settings, ACTIVITYRESULT_SETTINGS);
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		try {
			outState.putInt(CONFIG_MODE, _mode);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void parseOptions(Bundle savedInstanceState) {
		_mode = SSMODE_SAVE;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null) {
			_mode = b.getInt(CONFIG_MODE, SSMODE_SAVE);
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		int action = event.getAction();
		int keyCode = event.getKeyCode();

		if (action == KeyEvent.ACTION_UP) {
			switch (keyCode) {
				case KeyEvent.KEYCODE_BACK: {
					sendFinish(RESULT_CANCELED);
					return true;
				}
			}
		} else if (action == KeyEvent.ACTION_DOWN) {
			if (keyCode == KeyEvent.KEYCODE_BACK) {
				return true;
			}
		}
		return super.dispatchKeyEvent(event);
	}

	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	@Override public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				sendFinish(RESULT_CANCELED);
				return true;
			}
		}

		return super.onKeyDown(keyCode, event);
	}

	void setupButtonListeners()
	{
		findViewById(R.id.ss_closeBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				sendFinish(RESULT_CANCELED);
			}
		});
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			case ACTIVITYRESULT_SETTINGS:
			{
				sendFinish(RESULT_CANCELED);
				break;
			}
		}
	}

	private void sendFinish(int resultCode)
	{
		setResult(resultCode, _retIntent);
		finish();
	}

	private void _retrieveSaveFiles(File dir)
	{
		List<SaveStateListItem> items = new ArrayList<SaveStateListItem>();
		File [] curFiles = dir.listFiles();
		
		try
		{
			_usedSlots.clearAll();
			
			for (File f : curFiles)
			{
				if (!f.isDirectory())
				{
					if (f.getName().toLowerCase().endsWith(".ss"))
					{
						SaveStateListItem item = new SaveStateListItem(f.getAbsolutePath(), f.getName(), f.lastModified(), false, _lastSavedSlot, _quickSaveSlot);
						int itemSlot = item.getSlotID();
						if (itemSlot >= 0 && itemSlot < kMaxSlots)
						{
							_usedSlots.setBit(itemSlot);
						}
						items.add(item);
					}
				}
			}
			
			String dummyItemName = null;
			switch (_mode)
			{
				case SSMODE_SAVE:
				case SSMODE_QUICKSAVESLOT:
				{
					dummyItemName = "New Save Slot";
					break;
				}
				case SSMODE_LOAD:
				case SSMODE_DELETE:
				{
					if (items.size() == 0)
					{
						dummyItemName = "No Save States Found";
					}
					break;
				}
			}
			
			if (dummyItemName != null)
			{
				items.add(new SaveStateListItem(dummyItemName, dummyItemName, 0, true, _lastSavedSlot, _quickSaveSlot));
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		
		Collections.sort(items);
		
		_adapter = new SaveStateAdapter(this, R.layout.savestate_item, items);
		setListAdapter(_adapter);
	}

	void setUserTitleText(String s)
	{
		View v = findViewById(R.id.ss_title);
		if (v != null && v instanceof TextView)
		{
			TextView tv = (TextView)v;
			tv.setText(s);
		}
	}
	
	void _updateLastSavedSlot(int slotID)
	{
		try
		{
			_lastSavedSlot = slotID;
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			Editor ed = prefs.edit();
			ed.putInt(kPrefLastLoadedSlot, _lastSavedSlot);
			ed.commit();
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l,  v, position, id);

		SaveStateListItem item = _adapter.getItem(position);

		switch (_mode)
		{
			case SSMODE_SAVE:
			{
				String itemPath = item.getPath();
				int itemSlot = item.getSlotID();

				if (item.isDummyItem())
				{
					itemSlot = _usedSlots.findFirstEmptyBit();
					if (itemSlot >= kMaxSlots)
					{
						// show error, max slots reached
						break;
					}
					itemPath = _curDir.getAbsolutePath() + "/" + String.format("%03d", itemSlot) + "_new.ss";
				}

				_updateLastSavedSlot(itemSlot);
				
				_retIntent.putExtra(RESULT_SAVESTATE_FILENAME, itemPath);
				_retIntent.putExtra(RESULT_SAVESTATE_SLOT, itemSlot);
				sendFinish(RESULT_OK);
				break;
			}
			case SSMODE_LOAD:
			{
				if (!item.isDummyItem())
				{
					String itemPath = item.getPath();
					int itemSlot = item.getSlotID();

					_updateLastSavedSlot(itemSlot);

					_retIntent.putExtra(RESULT_SAVESTATE_FILENAME, itemPath);
					_retIntent.putExtra(RESULT_SAVESTATE_SLOT, itemSlot);
					sendFinish(RESULT_OK);
				}
				break;
			}
			case SSMODE_DELETE:
			{
				if (!item.isDummyItem())
				{
					String itemPath = item.getPath();
					int itemSlot = item.getSlotID();

					_onDeleteClicked(position, itemPath, itemSlot);
				}
				break;
			}
			case SSMODE_QUICKSAVESLOT:
			{
				int itemSlot = item.getSlotID();

				if (item.isDummyItem())
				{
					itemSlot = _usedSlots.findFirstEmptyBit();
					if (itemSlot >= kMaxSlots)
					{
						// show error, max slots reached
						break;
					}
				}
				
				// set quick save slot
				{
					_quickSaveSlot = itemSlot;
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
					Editor ed = prefs.edit();
					ed.putInt(kPrefQuickSaveSlot, _quickSaveSlot);
					ed.commit();
				}

				sendFinish(RESULT_OK);
				break;
			}
		}
	}

	boolean _showingDeleteConfirm = false;
	void _onDeleteClicked(int listItemID_, String itemPath, int itemSlot)
	{
		if (!_showingDeleteConfirm)
		{
			final String deletePath = itemPath;
			final int deleteItemSlot = itemSlot;
			final int listItemID = listItemID_;
			
			_showingDeleteConfirm = true;
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Delete Save State?");
			alertDialog.setMessage("Are you sure you want to delete this save state?");
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; } });
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; _tryDeleteSaveState(listItemID, deletePath, deleteItemSlot); } });
			alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { _showingDeleteConfirm = false; }});
			alertDialog.show();
		}
	}
	
	void _tryDeleteSaveState(int listItemID, String itemPath, int itemSlot)
	{
		try
		{
			// meta file
			{
				File file = new File(itemPath);
				/*boolean deleted =*/ file.delete(); // TODO: check result
			}

			// save file
			if (itemSlot >= 0)
			{
				int baseDirPos = itemPath.lastIndexOf("/");
				String baseDir = (baseDirPos >= 0) ? itemPath.substring(0, baseDirPos+1) : itemPath;
				String saveFilePath = baseDir + "/" + String.format("%03d", itemSlot) + ".sav";

				File file = new File(saveFilePath);
				/*boolean deleted =*/ file.delete(); // TODO: check result
			}
			
			// reset quick save
			if (itemSlot == _quickSaveSlot)
			{
				_quickSaveSlot = -1;
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
				Editor ed = prefs.edit();
				ed.putInt(kPrefQuickSaveSlot, _quickSaveSlot);
				ed.commit();
			}
			
			// refresh list
			_adapter.remove(_adapter.getItem(listItemID));
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
