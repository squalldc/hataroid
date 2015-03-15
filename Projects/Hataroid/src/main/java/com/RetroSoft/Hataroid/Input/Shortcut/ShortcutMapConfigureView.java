package com.RetroSoft.Hataroid.Input.Shortcut;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Spinner;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.RenameInputMapView;
import com.RetroSoft.Hataroid.Input.VirtKeyDef;

public class ShortcutMapConfigureView extends ListActivity implements OnItemSelectedListener
{
	final static int SHORTCUTSELECTRESULT_KEYCODE		= 11;
	final static int RENAMESHORTCUTMAPRESULT_KEYCODE	= 12;
	
	public static final String	kPrefPrefix = "_PREFShortcutMap_";
	public final static String	kPrefLastPresetIDKey = kPrefPrefix + "Settings_LastPresetID";
	public final static String	kPrefUserPresetPrefix = kPrefPrefix + "Settings_UserPresetMap_";
	final static String			kPrefUserPresetOrder = kPrefPrefix + "Settings_UserPresetOrder";

	ShortcutMapArrayAdapter	_adapter;
	Intent					_retIntent;
	
	List<String>			_presetIDList = new LinkedList<String>();
	List<String>			_presetNameList = new LinkedList<String>();

	Map<String,ShortcutMap>	_userPresetList = new HashMap<String,ShortcutMap>();
	List<String>			_userPresetOrder = new LinkedList<String>();
	Map<String,String>		_userPresetNameList = new HashMap<String,String>();

	int						_curPresetIdx = -1;
	String					_curPresetID = null;
	ShortcutMap				_curShortcutMap = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shortcutmap_view);
		
		_parseOptions(savedInstanceState);
		_setupButtonListeners();

		_retIntent = new Intent();
		
		_readSavedPrefs();
		
		_updatePresetListUIItem();
		_refreshCurSelection();
	}
	
	void _parseOptions(Bundle savedInstanceState)
	{
		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
		}
	}
	
	void _readSavedPrefs()
	{
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		
		// order
		{
			_userPresetOrder.clear();
			String presetOrderStr = prefs.getString(kPrefUserPresetOrder, null);
			if (presetOrderStr != null)
			{
				String [] data = presetOrderStr.split(",");
				if (data != null && data.length > 0)
				{
					for (int i = 0; i < data.length; ++i)
					{
						_userPresetOrder.add(data[i]);
					}
				}
			}
		}
		
		List<String> invalidEntries = new LinkedList<String>();

		// user input maps
		{
			_userPresetList.clear();
			_userPresetNameList.clear();
			
	    	Map<String,?> allPrefs = prefs.getAll();
			for (Map.Entry<String,?> entry : allPrefs.entrySet())
			{
				String key = entry.getKey();
				if (key.startsWith(kPrefUserPresetPrefix))
				{
					boolean error = false;
					String id = key.replace(kPrefUserPresetPrefix, "");
					if (!_userPresetOrder.contains(id))
					{
						if (!invalidEntries.contains(key))
						{
							invalidEntries.add(key);
						}
						continue;
					}
					
					try
					{
						Map<String,Object> result = new HashMap<String,Object>();
						error = !ShortcutMap.decodeShortcutMapPref(entry.getValue().toString(), result);
						if (!error)
						{
							_userPresetNameList.put(id, (String)result.get("name"));
							_userPresetList.put(id, (ShortcutMap)result.get("map"));
						}
					}
					catch (Exception e)
					{
						e.printStackTrace();
						error = true;
					}
					
					if (error)
					{
						if (!invalidEntries.contains(key))
						{
							invalidEntries.add(key);
						}
					}
				}
			}
		}

		// remove invalid entries
		{
			for (int i = 0; i < _userPresetOrder.size(); ++i)
			{
				String id = _userPresetOrder.get(i);
				if (!_userPresetList.containsKey(id))
				{
					String key = kPrefUserPresetPrefix + id;
					if (!invalidEntries.contains(key))
					{
						invalidEntries.add(key);
					}
					_userPresetOrder.remove(i);
					--i;
				}
			}
			if (invalidEntries.size() > 0)
			{
				Editor ed = prefs.edit();
				for (int i = 0; i < invalidEntries.size(); ++i)
				{
					ed.remove(invalidEntries.get(i));
				}
				ed.commit();
			}
		}

		// cur selected map
		{
			_setupPresetList();

			_curPresetIdx = 0;
			_curPresetID = _presetIDList.get(_curPresetIdx);

			String lastPresetID = prefs.getString(kPrefLastPresetIDKey, null);
			if (lastPresetID != null)
			{
				int idx = _presetIDList.indexOf(lastPresetID);
				if (idx >= 0)
				{
					_curPresetIdx = idx;
					_curPresetID = lastPresetID;
				}
			}
		}
	}
	
	void _setupPresetList()
	{
		_presetIDList.clear();
		_presetNameList.clear();
		
		{
			_presetIDList.add(ShortcutMap.getPresetID());
			_presetNameList.add(ShortcutMap.getPresetName() + " (read only)");
		}
		
		for (int i = 0; i < _userPresetOrder.size(); ++i)
		{
			String id = _userPresetOrder.get(i);
			_presetIDList.add(id);
			_presetNameList.add(_userPresetNameList.get(id));
		}
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

	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				sendFinish(RESULT_CANCELED);
			}
		}

		return super.onKeyDown(keyCode, event);
	}

	void _setupButtonListeners()
	{
		findViewById(R.id.im_closeBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				sendFinish(RESULT_OK);
			}
		});

		findViewById(R.id.im_deleteBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_onDeleteClicked();
			}
		});

		findViewById(R.id.im_renameBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_onRenameClicked();
			}
		});

		findViewById(R.id.im_newBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_onNewClicked();
			}
		});

		findViewById(R.id.im_showKeyboardBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				// show input method selector
				Input input = HataroidActivity.instance.getInput();
				input.showInputMethodSelector();
			}
		});
	}
	
	void _setModifyButtonsEnabled(boolean enabled)
	{
		Button b = (Button)findViewById(R.id.im_deleteBtn);
		if (b != null)
		{
			b.setEnabled(enabled);
		}
		
		b = (Button)findViewById(R.id.im_renameBtn);
		if (b != null)
		{
			b.setEnabled(enabled);
		}
	}

	void _updatePresetListUIItem()
	{
		Spinner spList = (Spinner)findViewById(R.id.im_presetSpinner);
		if (spList != null)
		{
			ArrayAdapter<String> presetListAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item);
			for (int i = 0; i < _presetIDList.size(); ++i)
			{
				presetListAdapter.add(_presetNameList.get(i));
			}

			spList.setAdapter(presetListAdapter);
			spList.setOnItemSelectedListener(this);
		}
	}
	
	void _setupShortcutMapItems()
	{
		_curShortcutMap = null;
		if (_curPresetID == null)
		{
			return;
		}

		if (ShortcutMap.isSystemPreset(_curPresetID))
		{
			// system preset
			_curShortcutMap = new ShortcutMap();
		}
		else
		{
			// user preset
			_curShortcutMap = _userPresetList.get(_curPresetID);
			if (_curShortcutMap == null)
			{
				// create new one
				_curShortcutMap = new ShortcutMap();
				_userPresetList.put(_curPresetID, _curShortcutMap);
			}
		}

		if (_curShortcutMap != null)
		{
			int [][] smap = _curShortcutMap.getCurMap();

			List<ShortcutMapListItem> items = new ArrayList<ShortcutMapListItem>();
			for (int a = 0; a < smap.length; ++a)
			{
				for (int k = 0; k < smap[a].length; ++k)
				{
					VirtKeyDef vkDef = null;
					int vkId = smap[a][k];
					if (vkId >= 0 && vkId < VirtKeyDef.kDefs.length)
					{
						vkDef = VirtKeyDef.kDefs[vkId];
					}
					items.add(new ShortcutMapListItem(vkDef, a, k));
				}
			}
			Collections.sort(items);
			
			_adapter = new ShortcutMapArrayAdapter(this, R.layout.shortcutmap_item, items);
			setListAdapter(_adapter);
		}

		_setModifyButtonsEnabled(!ShortcutMap.isSystemPreset(_curPresetID));
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l,  v, position, id);
		
		if (ShortcutMap.isSystemPreset(_curPresetID))
		{
    		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
    		alertDialog.setTitle("Built-in PRESET");
    		alertDialog.setMessage("This preset is read only. If you want to change the settings, please click the NEW button below or choose a different preset.");
    		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
    		alertDialog.show();
			return;
		}
		
		ShortcutMapListItem item = _adapter.getItem(position);
		showShortcutSelectDialog(item);
	}

	public void onItemSelected(AdapterView<?> parent, View view, int pos, long id)
	{
		if (pos >= 0 && pos < _presetIDList.size())
		{
			_curPresetIdx = pos;
			_curPresetID = _presetIDList.get(_curPresetIdx);
			_setupShortcutMapItems();
			
			_storeSelectedShortcutMapID(_curPresetID);
		}
    }

    public void onNothingSelected(AdapterView<?> parent) {
    }
    
    private void sendFinish(int resultCode)
	{
		setResult(resultCode, _retIntent);
		finish();
	}

    public void showShortcutSelectDialog(ShortcutMapListItem shortcutItem)
    {
		final ShortcutMapConfigureView ctx = this;
		final int resultID = SHORTCUTSELECTRESULT_KEYCODE;

        Intent view = new Intent(ctx, ShortcutSelectView.class);
        view.putExtra(ShortcutSelectView.CONFIG_ANCHOR, shortcutItem.getAnchor());
        view.putExtra(ShortcutSelectView.CONFIG_SHORTCUTIDX, shortcutItem.getShortcutIdx());
        view.putExtra(ShortcutSelectView.CONFIG_MAPID, _curPresetID);
        ctx.startActivityForResult(view, resultID);
	}

    public void showRenameDialog()
    {
    	if (_curPresetIdx < 0 || _curPresetIdx >= _presetIDList.size())
    	{
    		return;
    	}
    	String curName = _presetNameList.get(_curPresetIdx);
    	if (curName == null)
    	{
    		return;
    	}
    	
		final ShortcutMapConfigureView ctx = this;
		final int resultID = RENAMESHORTCUTMAPRESULT_KEYCODE;

        Intent view = new Intent(ctx, RenameInputMapView.class);
        view.putExtra(RenameInputMapView.CONFIG_CURNAME, curName);
        ctx.startActivityForResult(view, resultID);
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			case SHORTCUTSELECTRESULT_KEYCODE:
			{
				if (resultCode == RESULT_OK)
				{
					if (_curShortcutMap == null || _adapter == null)
					{
						break;
					}
					
					int anchor = data.getIntExtra(ShortcutSelectView.RESULT_ANCHOR, -1);
					int shortcutIdx = data.getIntExtra(ShortcutSelectView.RESULT_SHORTCUTIDX, -1);
					String mapId = data.getStringExtra(ShortcutSelectView.RESULT_MAPID);
					
					if (anchor < 0 || anchor >= ShortcutMap.kNumAnchors
					 || shortcutIdx < 0 || shortcutIdx >= ShortcutMap.kMaxKeys[anchor]
					 || mapId == null || mapId.compareTo(_curPresetID) != 0)
					{
						break;
					}
					
					boolean unMap = data.getBooleanExtra(ShortcutSelectView.RESULT_UNMAP, false);
					int emuKey = data.getIntExtra(ShortcutSelectView.RESULT_EMUKEY, -1);
					if (unMap)
					{
						emuKey = -1;
						_curShortcutMap.removeShortcutEntry(anchor, shortcutIdx);
					}
					else if (emuKey >= 0)
					{
						_curShortcutMap.addShortcutEntry(anchor, shortcutIdx, emuKey);
					}
					
					// update list items
					{
						int numItems = _adapter.getCount();
						for (int i = 0; i < numItems; ++i)
						{
							ShortcutMapListItem li = _adapter.getItem(i);
							if (li != null && li.getAnchor() == anchor && li.getShortcutIdx() == shortcutIdx)
							{
								VirtKeyDef def = (emuKey >= 0) ? VirtKeyDef.kDefs[emuKey] : null;
								li.setVirtKeyDef(def);
								break;
							}
						}
					}
					
					_adapter.notifyDataSetChanged();
					
					_storeShortcutMap(_curPresetID);
				}
				break;
			}

			case RENAMESHORTCUTMAPRESULT_KEYCODE:
			{
				if (resultCode == RESULT_OK)
				{
					String newName = data.getStringExtra(RenameInputMapView.RESULT_NAME);
					if (newName != null)
					{
						newName = newName.trim();
						newName = newName.replaceAll(",", ".");
					}
					if (newName != null && newName.length() > 0 && _curPresetIdx >= 0 && _curPresetIdx < _presetIDList.size())
					{
						int presetIdx = _curPresetIdx;
						
						String curName = _presetNameList.get(_curPresetIdx);
						if (curName.compareTo(newName) != 0)
						{
							_presetNameList.set(_curPresetIdx, newName);
							_userPresetNameList.put(_curPresetID, newName);
							_updatePresetListUIItem();

							Spinner spList = (Spinner)findViewById(R.id.im_presetSpinner);
							if (spList != null)
							{
								spList.setSelection(presetIdx);
							}
						}

						_storeShortcutMap(_curPresetID);
					}
				}
				break;
			}
		}
	}

	boolean _showingDeleteConfirm = false;
	void _onDeleteClicked()
	{
		if (ShortcutMap.isSystemPreset(_curPresetID))
		{
			return;
		}

		if (!_showingDeleteConfirm)
		{
			_showingDeleteConfirm = true;
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Delete Shortcut Map?");
			alertDialog.setMessage("Are you sure you want to delete this shortcut map?");
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; } });
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; _tryDeleteShortcutMap(); } });
			alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { _showingDeleteConfirm = false; }});
			alertDialog.show();
		}
	}
	
	void _tryDeleteShortcutMap()
	{
		if (_curPresetID == null || ShortcutMap.isSystemPreset(_curPresetID))
		{
			return;
		}

		int presetPos = _curPresetIdx;
		if (presetPos >= 0)
		{
			int newSelection = -1;

			_presetIDList.remove(presetPos);
			_presetNameList.remove(presetPos);
			
			if (_userPresetList.containsKey(_curPresetID))
			{
				_userPresetList.remove(_curPresetID);
			}
			if (_userPresetNameList.containsKey(_curPresetID))
			{
				_userPresetNameList.remove(_curPresetID);
			}
			int presetOrderIdx = _userPresetOrder.indexOf(_curPresetID);
			if (presetOrderIdx >= 0)
			{
				_userPresetOrder.remove(presetOrderIdx);
			}
			
			_storeItemDeleted(_curPresetID);

			_curPresetID = null;
			newSelection = presetPos - 1;
			if (newSelection < 0 && _presetIDList.size() > 0)
			{
				newSelection = 0;
			}
			
			_curPresetIdx = newSelection;
			_updatePresetListUIItem();
			_refreshCurSelection();
		}
	}

	void _onRenameClicked()
	{
		if (ShortcutMap.isSystemPreset(_curPresetID))
		{
			return;
		}
		
		showRenameDialog();
	}

	void _onNewClicked()
	{
		// get unique name
		final String newNamePrefix = "my_shortcut_map";
		int nextID = 1;
		for (int i = 0; i < _presetNameList.size(); ++i)
		{
			String [] nameSplit = _presetNameList.get(i).split(" ");
			if (nameSplit != null && nameSplit.length == 2 && nameSplit[0].compareTo(newNamePrefix) == 0)
			{
				try
				{
					int curID = Integer.parseInt(nameSplit[1]);
					if (nextID <= curID) { nextID = curID + 1;; }
				}
				catch (Exception e) {}
			}
		}
		
		String newPresetID = java.util.UUID.randomUUID().toString();
		String newPresetName = newNamePrefix + " " + nextID;

		_presetIDList.add(newPresetID);
		_presetNameList.add(newPresetName);
		
		// create new input map
		ShortcutMap map = new ShortcutMap();
		_userPresetList.put(newPresetID, map);
		_userPresetOrder.add(newPresetID);
		_userPresetNameList.put(newPresetID, newPresetName);

		_storeShortcutMap(newPresetID);

		int newSelection = _presetIDList.size()-1;
		_curPresetIdx = newSelection;
		_updatePresetListUIItem();
		_refreshCurSelection();
	}

	void _refreshCurSelection()
	{
		if (_curPresetIdx >= 0)
		{
			Spinner spList = (Spinner)findViewById(R.id.im_presetSpinner);
			if (spList != null)
			{
				spList.setSelection(_curPresetIdx);
			}
		}
	}

	void _storeSelectedShortcutMapID(String id)
	{
		if (id != null)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

			Editor ed = prefs.edit();
			ed.putString(kPrefLastPresetIDKey, id);
			ed.commit();
		}
	}

	void _storeShortcutMap(String id)
	{
		if (id != null && _userPresetList.containsKey(id))
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
			
			Editor ed = prefs.edit();

			_updateUserPresetOrderPrefs(prefs, ed);
			
			String itemKey = kPrefUserPresetPrefix + id;
			ShortcutMap map = _userPresetList.get(id);
			String mapName = _userPresetNameList.get(id);
			String mapStr = map.encodePrefString(mapName);
			if (mapStr != null)
			{
				ed.putString(itemKey, mapStr);
			}

			ed.commit();
		}
	}

	void _storeItemDeleted(String id)
	{
		if (id != null)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
			
			Editor ed = prefs.edit();

			_updateUserPresetOrderPrefs(prefs, ed);
			
			String itemKey = kPrefUserPresetPrefix + id;
			if (prefs.contains(itemKey))
			{
				ed.remove(itemKey);
			}
			
			ed.commit();
		}
	}
	
	void _updateUserPresetOrderPrefs(SharedPreferences prefs, Editor ed)
	{
		String order = null;
		if (_userPresetOrder.size() > 0)
		{
			order = _userPresetOrder.get(0);
			for (int i = 1; i < _userPresetOrder.size(); ++i)
			{
				order += "," + _userPresetOrder.get(i);
			}
		}

		if (order == null)
		{
			if (prefs.contains(kPrefUserPresetOrder))
			{
				ed.remove(kPrefUserPresetOrder);
			}
		}
		else
		{
			ed.putString(kPrefUserPresetOrder, order);
		}
	}
}
