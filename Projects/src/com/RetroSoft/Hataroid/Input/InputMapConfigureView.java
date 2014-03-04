package com.RetroSoft.Hataroid.Input;

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

public class InputMapConfigureView extends ListActivity implements OnItemSelectedListener
{
	final static int INPUTCAPTURERESULT_KEYCODE		= 1;
	final static int RENAMEINPUTMAPRESULT_KEYCODE	= 2;
	
	public static final String	kPrefPrefix = "_PREFInputMap_";
	public final static String	kPrefLastPresetIDKey = kPrefPrefix + "Settings_LastPresetID";
	public final static String	kPrefUserPresetPrefix = kPrefPrefix + "Settings_UserPresetMap_";
	final static String			kPrefUserPresetOrder = kPrefPrefix + "Settings_UserPresetOrder";

	InputMapArrayAdapter	_adapter;
	Intent					_retIntent;
	
	List<String>			_presetIDList = new LinkedList<String>();
	List<String>			_presetNameList = new LinkedList<String>();

	Map<String,InputMap>	_userPresetList = new HashMap<String,InputMap>();
	List<String>			_userPresetOrder = new LinkedList<String>();
	Map<String,String>		_userPresetNameList = new HashMap<String,String>();

	int						_curPresetIdx = -1;
	String					_curPresetID = null;
	InputMap				_curInputMap = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.configureinputmap_view);
		
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
						error = !Input.decodeInputMapPref(entry.getValue().toString(), result);
						if (!error)
						{
							_userPresetNameList.put(id, (String)result.get("name"));
							_userPresetList.put(id, (InputMap)result.get("map"));
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
		
		for (int i = 0; i < Input.kPreset_NumOf; ++i)
		{
			_presetIDList.add(Input.getPresetID(i));
			_presetNameList.add(Input.getPresetName(i) + " (read only)");
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
	
	void _setupInputMapItems()
	{
		_curInputMap = null;
		if (_curPresetID == null)
		{
			return;
		}

		if (Input.isSystemPreset(_curPresetID))
		{
			// system preset
			int [] presetArray = Input.getPresetArray(Input.getPresetID(_curPresetID));
			if (presetArray != null)
			{
				_curInputMap = new InputMap();
				_curInputMap.initFromArray(Input.kNumSrcKeyCodes, presetArray);
			}
		}
		else
		{
			// user preset
			_curInputMap = _userPresetList.get(_curPresetID);
			if (_curInputMap == null)
			{
				// create new one
				_curInputMap = new InputMap();
				_curInputMap.init(Input.kNumSrcKeyCodes);
				_userPresetList.put(_curPresetID, _curInputMap);
			}
		}

		if (_curInputMap != null)
		{
			Map<Integer, Integer> emuToSystemMap = _curInputMap.destToSrcMap;
	
			List<InputMapListItem> items = new ArrayList<InputMapListItem>();
			for (int i = 0; i < VirtKeyDef.VKB_KEY_NumOf; ++i)
			{
				VirtKeyDef vkDef = VirtKeyDef.kDefs[i];
				if (vkDef.config > 0)
				{
					int systemKey = emuToSystemMap.containsKey(vkDef.id) ? emuToSystemMap.get(vkDef.id): -1;
					items.add(new InputMapListItem(vkDef, systemKey));
				}
			}
			Collections.sort(items);
			
			_adapter = new InputMapArrayAdapter(this, R.layout.configureinputmap_item, items);
			setListAdapter(_adapter);
		}

		_setModifyButtonsEnabled(!Input.isSystemPreset(_curPresetID));
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l,  v, position, id);
		
		if (Input.isSystemPreset(_curPresetID))
		{
			return;
		}
		
		InputMapListItem item = _adapter.getItem(position);
		showGetKeycodeDialog(item);
	}

	public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
		if (pos >= 0 && pos < _presetIDList.size())
		{
			_curPresetIdx = pos;
			_curPresetID = _presetIDList.get(_curPresetIdx);
			_setupInputMapItems();
			
			_storeSelectedInputMapID(_curPresetID);
		}
    }

    public void onNothingSelected(AdapterView<?> parent) {
    }
    
    private void sendFinish(int resultCode)
	{
		setResult(resultCode, _retIntent);
		finish();
	}

    public void showGetKeycodeDialog(InputMapListItem scanItem)
    {
		final InputMapConfigureView ctx = this;
		final int resultID = INPUTCAPTURERESULT_KEYCODE;

        Intent view = new Intent(ctx, InputCaptureView.class);
        view.putExtra(InputCaptureView.CONFIG_EMUKEY, scanItem._vkDef.id);
        view.putExtra(InputCaptureView.CONFIG_SYSTEMKEY, scanItem._systemKey);
        view.putExtra(InputCaptureView.CONFIG_MAPID, _curPresetID);
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
    	
		final InputMapConfigureView ctx = this;
		final int resultID = RENAMEINPUTMAPRESULT_KEYCODE;

        Intent view = new Intent(ctx, RenameInputMapView.class);
        view.putExtra(RenameInputMapView.CONFIG_CURNAME, curName);
        ctx.startActivityForResult(view, resultID);
    }
    
	@Override protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			case INPUTCAPTURERESULT_KEYCODE:
			{
				if (resultCode == RESULT_OK)
				{
					if (_curInputMap == null || _adapter == null)
					{
						break;
					}
					
					int prevEmuKey = data.getIntExtra(InputCaptureView.RESULT_PREVEMUKEY, -1);
					int prevSystemKey = data.getIntExtra(InputCaptureView.RESULT_PREVSYSTEMKEY, -1);
					String prevMapId= data.getStringExtra(InputCaptureView.RESULT_PREVMAPID);
					
					if (prevEmuKey < 0  || prevMapId == null || prevMapId.compareTo(_curPresetID) != 0)
					{
						break;
					}
					
					boolean unMap = data.getBooleanExtra(InputCaptureView.RESULT_UNMAP, false);
					int systemKey = data.getIntExtra(InputCaptureView.RESULT_KEYCODE, -1);
					if (unMap)
					{
						_curInputMap.removeKeyMapEntry(prevSystemKey);
					}
					else if (systemKey >= 0)
					{
						_curInputMap.addKeyMapEntry(systemKey, prevEmuKey);
					}
					
					// update list items
					{
						int numItems = _adapter.getCount();
						Map<Integer, Integer> destToSrcMap = _curInputMap.destToSrcMap;
						if (destToSrcMap != null)
						{
							for (int i = 0; i < numItems; ++i)
							{
								InputMapListItem li = _adapter.getItem(i);
								if (li != null && destToSrcMap.containsKey(li._vkDef.id))
								{
									li._systemKey = _curInputMap.destToSrcMap.get(li._vkDef.id);
								}
								else
								{
									li._systemKey = -1;
								}
							}
						}
					}
					
					_adapter.notifyDataSetChanged();
					
					_storeInputMap(_curPresetID);
				}
				break;
			}
			case RENAMEINPUTMAPRESULT_KEYCODE:
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

						_storeInputMap(_curPresetID);
					}
				}
				break;
			}
		}
	}

	boolean _showingDeleteConfirm = false;
	void _onDeleteClicked()
	{
		if (Input.isSystemPreset(_curPresetID))
		{
			return;
		}

		if (!_showingDeleteConfirm)
		{
			_showingDeleteConfirm = true;
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Delete Input Map?");
			alertDialog.setMessage("Are you sure you want to delete this input map?");
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; } });
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; _tryDeleteInputMap(); } });
			alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { _showingDeleteConfirm = false; }});
			alertDialog.show();
		}
	}
	
	void _tryDeleteInputMap()
	{
		if (_curPresetID == null || Input.isSystemPreset(_curPresetID))
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
		if (Input.isSystemPreset(_curPresetID))
		{
			return;
		}
		
		showRenameDialog();
	}

	void _onNewClicked()
	{
		// get unique name
		final String newNamePrefix = "my_input_map";
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
		InputMap map = new InputMap();
		map.init(Input.kNumSrcKeyCodes);
		_userPresetList.put(newPresetID, map);
		_userPresetOrder.add(newPresetID);
		_userPresetNameList.put(newPresetID, newPresetName);

		_storeInputMap(newPresetID);

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

	void _storeSelectedInputMapID(String id)
	{
		if (id != null)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

			Editor ed = prefs.edit();
			ed.putString(kPrefLastPresetIDKey, id);
			ed.commit();
		}
	}

	void _storeInputMap(String id)
	{
		if (id != null && _userPresetList.containsKey(id))
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
			
			Editor ed = prefs.edit();

			_updateUserPresetOrderPrefs(prefs, ed);
			
			String itemKey = kPrefUserPresetPrefix + id;
			InputMap map = _userPresetList.get(id);
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
