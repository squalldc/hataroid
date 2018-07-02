package com.RetroSoft.Hataroid.Util;

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
//import android.support.v7.app.ActionBar;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.RenameInputMapView;

//public class MapSetConfigureView extends AppCompatListActivity implements OnItemSelectedListener
public class MapSetConfigureView extends ListActivity implements OnItemSelectedListener
{
	public final static	String	CONFIG_MAPSETCLASSNAME	= "Config_MapSetClassName";
	public final static	String	CONFIG_TITLE			= "Config_Title";

	String					kPrefPrefix = null;
	String					kPrefLastPresetIDKey = null;
	String					kPrefUserPresetPrefix = null;
	String					kPrefUserPresetOrder = null;

	int						MAPSET_SELECT_RESULT_KEYCODE = 1;
	int						MAPSET_RENAME_RESULT_KEYCODE = 2;

	private String			_title = "";

	MapSetArrayAdapter		_adapter;
	Intent					_retIntent;
	
	List<String>			_presetIDList = new LinkedList<String>();
	List<String>			_presetNameList = new LinkedList<String>();

	Map<String,IMapSet>		_userPresetList = new HashMap<String,IMapSet>();
	List<String>			_userPresetOrder = new LinkedList<String>();
	Map<String,String>		_userPresetNameList = new HashMap<String,String>();

	int						_curPresetIdx = -1;
	String					_curPresetID = null;
	IMapSet					_curMapSet = null;
	
	String					_mapSetClassName = null;
	Class<?>				_mapSetClass =  null;
	IMapSet					_IMapSet = null;
	
	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		{
			_mapSetClassName = null;
			_mapSetClass = null;
			_IMapSet = null;

			Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
			if (b != null)
			{
				_mapSetClassName = b.getString(CONFIG_MAPSETCLASSNAME);
				try {
					_mapSetClass = Class.forName(_mapSetClassName);
					_IMapSet = (IMapSet)_mapSetClass.newInstance();
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
				} catch (InstantiationException e) {
					e.printStackTrace();
				} catch (IllegalAccessException e) {
					e.printStackTrace();
				}
			}
		}
		setContentView(_IMapSet.get_ViewLayoutResID());
		
		kPrefPrefix = _IMapSet.get_PREF_PREFIX();
		kPrefLastPresetIDKey = kPrefPrefix + "Settings_LastPresetID";
		kPrefUserPresetPrefix = kPrefPrefix + "Settings_UserPresetMap_";
		kPrefUserPresetOrder = kPrefPrefix + "Settings_UserPresetOrder";

		MAPSET_SELECT_RESULT_KEYCODE = _IMapSet.get_MAPVALUE_RESULT_KEYCODE();
		MAPSET_RENAME_RESULT_KEYCODE = _IMapSet.get_RENAME_MAPSET_RESULT_KEYCODE();

		_parseOptions(savedInstanceState);

//		try
//		{
//			ActionBar actionBar = getSupportActionBar();
//
//			LayoutInflater mInflater = LayoutInflater.from(this);
//			View customView = mInflater.inflate(R.layout.presetlist_actionbar, null);
//
//			actionBar.setCustomView(customView);
//			actionBar.setDisplayShowCustomEnabled(true);
//
//			TextView tv = (TextView)customView.findViewById(R.id.ab_title);
//			tv.setText(_title);
//		}
//		catch (Exception e)
//		{
//			e.printStackTrace();
//		}

		try
		{
			View mView = this.findViewById(_IMapSet.get_ViewLayoutID());

			TextView title = (TextView)mView.findViewById(R.id.ab_title);
			title.setText(_title);

			ImageButton navBackBtn = (ImageButton)mView.findViewById(R.id.nav_back);
			navBackBtn.setOnClickListener(new View.OnClickListener() {
				public void onClick(View view) {
					sendFinish(RESULT_OK);
				}
			});
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		getListView().setItemsCanFocus(true);

		_setupButtonListeners();

		_retIntent = new Intent();
		
		_readSavedPrefs();
		
		_updatePresetListUIItem();
		_refreshCurSelection();
	}
	
	@Override protected void onSaveInstanceState(Bundle outState)
	{
		try
		{
			outState.putString(CONFIG_MAPSETCLASSNAME, _mapSetClassName);
			outState.putString(CONFIG_TITLE, _title);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
	
	void _parseOptions(Bundle savedInstanceState)
	{
		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			_title = b.getString(CONFIG_TITLE);
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

		// user defined maps
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
						error = !_IMapSet.decodeMapSetPref(entry.getValue().toString(), result);
						if (!error)
						{
							_userPresetNameList.put(id, (String)result.get("name"));
							_userPresetList.put(id, (IMapSet)result.get("map"));
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
		
		for (int i = 0; i < _IMapSet.getNumPresets(); ++i)
		{
			_presetIDList.add(_IMapSet.getPresetID(i));
			_presetNameList.add(_IMapSet.getPresetName(i) + " (read only)");
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

//	@Override
//	public boolean onOptionsItemSelected(MenuItem item)
//	{
//		switch (item.getItemId())
//		{
//			case android.R.id.home:
//			{
//				sendFinish(RESULT_OK);
//				return true;
//			}
//		}
//
//		return super.onOptionsItemSelected(item);
//	}

	void _setupButtonListeners()
	{
//		findViewById(R.id.im_closeBtn).setOnClickListener(new OnClickListener() {
//			public void onClick(View v) {
//				sendFinish(RESULT_OK);
//			}
//		});

//		ActionBar actionBar = null;
//		View actionBarView = null;
//		try
//		{
//			actionBar = getSupportActionBar();
//			if (actionBar != null)
//			{
//				actionBarView = actionBar.getCustomView();
//			}
//		}
//		catch (Exception e)
//		{
//			e.printStackTrace();
//		}
		View actionBarView = this.findViewById(_IMapSet.get_ViewLayoutID());

		try
		{
			if (actionBarView != null)
			{
				View spinner = findViewById(R.id.im_presetSpinner);
				spinner.setNextFocusUpId(R.id.im_showKeyboardBtn);

				View vb = null;

				vb = actionBarView.findViewById(R.id.im_deleteBtn);
				vb.setNextFocusDownId(R.id.im_presetSpinner);
				vb.setOnClickListener(new OnClickListener() {
					public void onClick(View v) {
						_onDeleteClicked();
					}
				});

				vb = actionBarView.findViewById(R.id.im_renameBtn);
				vb.setNextFocusDownId(R.id.im_presetSpinner);
				vb.setOnClickListener(new OnClickListener() {
					public void onClick(View v) {
						_onRenameClicked();
					}
				});

				vb = actionBarView.findViewById(R.id.im_newBtn);
				vb.setNextFocusDownId(R.id.im_presetSpinner);
				vb.setOnClickListener(new OnClickListener() {
					public void onClick(View v) {
						_onNewClicked();
					}
				});

				vb = actionBarView.findViewById(R.id.im_showKeyboardBtn);
				vb.setNextFocusDownId(R.id.im_presetSpinner);
				vb.setNextFocusRightId(R.id.im_presetSpinner);
				//vb.setNextFocusForwardId(R.id.im_presetSpinner);
				vb.setOnClickListener(new OnClickListener() {
					public void onClick(View v) {
						// show input method selector
						Input input = HataroidActivity.instance.getInput();
						input.showInputMethodSelector();
					}
				});
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		_IMapSet.setupButtonListeners(this);
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
	
	void _setupMapSetItems()
	{
		_curMapSet = null;
		if (_curPresetID == null)
		{
			return;
		}

		if (_IMapSet.isSystemPreset(_curPresetID))
		{
			// system preset
			try {
				_curMapSet = (IMapSet)_mapSetClass.newInstance();
			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}
			_curMapSet.initFromPreset(_curPresetID);
		}
		else
		{
			// user preset
			_curMapSet = _userPresetList.get(_curPresetID);
			if (_curMapSet == null)
			{
				// create new one
				try {
					_curMapSet = (IMapSet)_mapSetClass.newInstance();
				} catch (InstantiationException e) {
					e.printStackTrace();
				} catch (IllegalAccessException e) {
					e.printStackTrace();
				}
				_curMapSet.initDefault();
				_userPresetList.put(_curPresetID, _curMapSet);
			}
		}

		if (_curMapSet != null)
		{
			List<IMapSetListItem> items = _curMapSet.buildMapSetListItems();
			int itemLayout = _IMapSet.get_ItemLayoutResID();
			
			_adapter = new MapSetArrayAdapter(this, itemLayout, items);
			setListAdapter(_adapter);
		}

		_setModifyButtonsEnabled(!_IMapSet.isSystemPreset(_curPresetID));
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l,  v, position, id);
		
		IMapSetListItem item = _adapter.getItem(position);
		onMapBtnClicked(item);
	}

	public void onMapBtnClicked(IMapSetListItem item)
	{
		if (!_verifyCanModifyCurPreset())
		{
			return;
		}

		showMapSetSelectDialog(item);
	}

	public void onUnMapBtnClicked(IMapSetListItem item)
	{
		if (!_verifyCanModifyCurPreset())
		{
			return;
		}

		String mapId = _curPresetID;
		IMapSetListItem srcItem = item;
		IMapSetListItem newItemVal = null;
		boolean unMap = true;

		_updateMapSetItemEntry(mapId, srcItem, newItemVal, unMap);
	}


	boolean _verifyCanModifyCurPreset()
	{
		if (_IMapSet.isSystemPreset(_curPresetID))
		{
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Built-in PRESET");
			alertDialog.setMessage("This preset is read only. If you want to change the settings, please click the NEW button below or choose a different preset.");
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
			alertDialog.show();
			return false;
		}
		return true;
	}

	public void onItemSelected(AdapterView<?> parent, View view, int pos, long id)
	{
		if (pos >= 0 && pos < _presetIDList.size())
		{
			_curPresetIdx = pos;
			_curPresetID = _presetIDList.get(_curPresetIdx);
			_setupMapSetItems();
			
			_storeSelectedMapSetID(_curPresetID);
		}
    }

    public void onNothingSelected(AdapterView<?> parent) {
    }
    
    private void sendFinish(int resultCode)
	{
		setResult(resultCode, _retIntent);
		finish();
	}

    public void showMapSetSelectDialog(IMapSetListItem mapSetItem)
    {
		final MapSetConfigureView ctx = this;
		final int resultID = MAPSET_SELECT_RESULT_KEYCODE;

        Intent view = new Intent(ctx, MapSetSelectView.class);
        view.putExtra(MapSetSelectView.CONFIG_MAPSET_ITEM, mapSetItem);
        view.putExtra(MapSetSelectView.CONFIG_MAPSETID, _curPresetID);
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
    	
		final MapSetConfigureView ctx = this;
		final int resultID = MAPSET_RENAME_RESULT_KEYCODE;

        Intent view = new Intent(ctx, RenameInputMapView.class);
        view.putExtra(RenameInputMapView.CONFIG_CURNAME, curName);
        ctx.startActivityForResult(view, resultID);
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if (requestCode == MAPSET_SELECT_RESULT_KEYCODE)
		{
			if (resultCode == RESULT_OK)
			{
				String mapId = data.getStringExtra(MapSetSelectView.RESULT_MAPSETID);
				IMapSetListItem srcItem = (IMapSetListItem)data.getSerializableExtra(MapSetSelectView.RESULT_MAPSET_SRCITEM);
				IMapSetListItem newItemVal = (IMapSetListItem)data.getSerializableExtra(MapSetSelectView.RESULT_MAPSET_NEWITEMVAL);
				boolean unMap = data.getBooleanExtra(MapSetSelectView.RESULT_UNMAP, false);

				_updateMapSetItemEntry(mapId, srcItem, newItemVal, unMap);
			}
		}
		else if (requestCode == MAPSET_RENAME_RESULT_KEYCODE)
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

					_storeMapSet(_curPresetID);
				}
			}
		}
	}

	boolean _updateMapSetItemEntry(String mapId, IMapSetListItem srcItem, IMapSetListItem newItemVal, boolean unMap)
	{
		if (_curMapSet == null || _adapter == null)
		{
			return false;
		}

		if (mapId == null || mapId.compareTo(_curPresetID) != 0
				|| srcItem == null)
		{
			return false;
		}

		if (_curMapSet.updateEntry(unMap, srcItem, newItemVal))
		{
			// update list items
			int numItems = _adapter.getCount();
			for (int i = 0; i < numItems; ++i)
			{
				IMapSetListItem li = _adapter.getItem(i);
				if (li.isSameItemKey(srcItem))
				{
					_curMapSet.updateListItem(li);
					break;
				}
			}

			_adapter.notifyDataSetChanged();
			_storeMapSet(_curPresetID);

			return true;
		}

		return false;
	}

	boolean _showingDeleteConfirm = false;
	void _onDeleteClicked()
	{
		if (_IMapSet.isSystemPreset(_curPresetID))
		{
			return;
		}

		if (!_showingDeleteConfirm)
		{
			String title = _IMapSet.get_DeleteDialogTitle();
			String msg = _IMapSet.get_DeleteDialogMessage();
			
			_showingDeleteConfirm = true;
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle(title);
			alertDialog.setMessage(msg);
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; } });
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { _showingDeleteConfirm = false; _tryDeleteMapSet(); } });
			alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() { public void onCancel(DialogInterface dialog) { _showingDeleteConfirm = false; }});
			alertDialog.show();
		}
	}
	
	void _tryDeleteMapSet()
	{
		if (_curPresetID == null || _IMapSet.isSystemPreset(_curPresetID))
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
		if (_IMapSet.isSystemPreset(_curPresetID))
		{
			return;
		}
		
		showRenameDialog();
	}

	void _onNewClicked()
	{
		// get unique name
		final String newNamePrefix = _IMapSet.get_NewMapSetNamePrefix();
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
		IMapSet map = null;
		try {
			map = (IMapSet)_mapSetClass.newInstance();
		} catch (InstantiationException e) {
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			e.printStackTrace();
		}
		map.initDefault();

		_userPresetList.put(newPresetID, map);
		_userPresetOrder.add(newPresetID);
		_userPresetNameList.put(newPresetID, newPresetName);

		_storeMapSet(newPresetID);

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

	void _storeSelectedMapSetID(String id)
	{
		if (id != null)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

			Editor ed = prefs.edit();
			ed.putString(kPrefLastPresetIDKey, id);
			ed.commit();
		}
	}

	void _storeMapSet(String id)
	{
		if (id != null && _userPresetList.containsKey(id))
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
			
			Editor ed = prefs.edit();

			_updateUserPresetOrderPrefs(prefs, ed);
			
			String itemKey = kPrefUserPresetPrefix + id;
			IMapSet map = _userPresetList.get(id);
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

	public IMapSet getCurMapSet()
	{
		return _curMapSet;
	}

	public String getCurPresetID()
	{
		return _curPresetID;
	}
}
