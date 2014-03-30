package com.RetroSoft.Hataroid.Input.Shortcut;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;

import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.Input.VirtKeyDef;

public class ShortcutSelectView extends ListActivity
{
	public static final String CONFIG_ANCHOR		= "Config_Anchor";
	public static final String CONFIG_SHORTCUTIDX	= "Config_ShortcutIdx";
	public static final String CONFIG_MAPID			= "Config_MapID";
	
	public static final String RESULT_UNMAP			= "ResultUnmap";

	public static final String RESULT_EMUKEY		= "ResultEmuKey";

	public static final String RESULT_ANCHOR		= "ResultAnchor";
	public static final String RESULT_SHORTCUTIDX	= "ResultShortcutIdx";
	public static final String RESULT_MAPID			= "ResultMapID";

	int			_anchor = -1;
	int			_shortcutIdx = -1;
	String		_mapID = null;

	Intent		_retIntent = null;

	ShortcutSelectArrayAdapter _adapter;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shortcutkeyselect_view);
		
		_parseOptions(savedInstanceState);
		_retIntent = new Intent();
		
		setupButtonListeners();
		
		_setupEmuKeyList();
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

	@Override protected void onSaveInstanceState(Bundle outState)
	{
		try
		{
			outState.putInt(CONFIG_ANCHOR, _anchor);
			outState.putInt(CONFIG_SHORTCUTIDX, _shortcutIdx);
			outState.putString(CONFIG_MAPID, _mapID);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void _parseOptions(Bundle savedInstanceState)
	{
		_anchor = -1;
		_shortcutIdx = -1;
		_mapID = null;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			_anchor = b.getInt(CONFIG_ANCHOR, -1);
			_shortcutIdx = b.getInt(CONFIG_SHORTCUTIDX, -1);
			_mapID = b.getString(CONFIG_MAPID);
		}
	}

	void setupButtonListeners()
	{
		View vw = findViewById(R.id.ic_cancelBtn);
		vw.setFocusable(false);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				sendFinish(RESULT_CANCELED);
			}
		});
		
		vw = findViewById(R.id.ic_unmapBtn);
		vw.setFocusable(false);
		vw.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_retIntent.putExtra(RESULT_UNMAP, true);
				sendFinish(RESULT_OK);
			}
		});
	}

	private void sendFinish(int resultCode)
	{
		_retIntent.putExtra(RESULT_ANCHOR, _anchor);
		_retIntent.putExtra(RESULT_SHORTCUTIDX, _shortcutIdx);
		_retIntent.putExtra(RESULT_MAPID, _mapID);

		setResult(resultCode, _retIntent);
		finish();
	}
	
	void _setupEmuKeyList()
	{
		List<ShortcutSelectListItem> items = new ArrayList<ShortcutSelectListItem>();
		for (int i = 0; i < VirtKeyDef.VKB_KEY_NumOf; ++i)
		{
			VirtKeyDef vkDef = VirtKeyDef.kDefs[i];
			if (vkDef.scut > 0)
			{
				items.add(new ShortcutSelectListItem(vkDef));
			}
		}
		Collections.sort(items);
		
		_adapter = new ShortcutSelectArrayAdapter(this, R.layout.shortcutkeyselect_item, items);
		setListAdapter(_adapter);
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l,  v, position, id);
		
		ShortcutSelectListItem item = _adapter.getItem(position);
		int emuKeyIdx = item.getKeyDef().id;

		_retIntent.putExtra(RESULT_EMUKEY, emuKeyIdx);
		sendFinish(RESULT_OK);
	}
}
