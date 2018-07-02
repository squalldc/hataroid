package com.RetroSoft.Hataroid.Util;

import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;

import com.RetroSoft.Hataroid.R;

public class MapSetSelectView extends ListActivity
{
	public static final String CONFIG_MAPSETID				= "Config_MapSetID";
	public static final String CONFIG_MAPSET_ITEM			= "Config_MapSetItem";

	public static final String RESULT_MAPSETID				= "Result_MapSetID";
	public static final String RESULT_UNMAP					= "Result_Unmap";
	public static final String RESULT_MAPSET_SRCITEM		= "Result_MapSetSrcItem";
	public static final String RESULT_MAPSET_NEWITEMVAL		= "Result_MapSetNewItemVal";
	
	
	IMapSetListItem		_item = null;
	IMapSetListItem		_newItemVal = null;

	String				_mapSetID = null;

	Intent				_retIntent = null;

	MapSetArrayAdapter	_adapter = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		_parseOptions(savedInstanceState);

		setContentView(_item.get_ViewLayoutResID());
		
		_retIntent = new Intent();
		
		setupButtonListeners();
		_setupItemsList();
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
			outState.putSerializable(CONFIG_MAPSET_ITEM, _item);
			outState.putString(CONFIG_MAPSETID, _mapSetID);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	void _parseOptions(Bundle savedInstanceState)
	{
		_item = null;
		_mapSetID = null;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			try
			{
				_item = (IMapSetListItem)b.getSerializable(CONFIG_MAPSET_ITEM);
				_mapSetID = b.getString(CONFIG_MAPSETID);
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
		}
	}

	void setupButtonListeners()
	{
//		View vw = findViewById(R.id.ic_cancelBtn);
//		vw.setFocusable(false);
//		vw.setOnClickListener(new OnClickListener() {
//			public void onClick(View v) {
//				sendFinish(RESULT_CANCELED);
//			}
//		});
		
//		vw = findViewById(R.id.ic_unmapBtn);
//		vw.setFocusable(false);
//		vw.setOnClickListener(new OnClickListener() {
//			public void onClick(View v) {
//				_retIntent.putExtra(RESULT_UNMAP, true);
//				sendFinish(RESULT_OK);
//			}
//		});
	}

	private void sendFinish(int resultCode)
	{
		_retIntent.putExtra(RESULT_MAPSETID, _mapSetID);
		_retIntent.putExtra(RESULT_MAPSET_SRCITEM, _item);

		setResult(resultCode, _retIntent);
		finish();
	}
	
	void _setupItemsList()
	{
		List<IMapSetListItem> items = _item.buildListItems();

		int itemLayout = _item.get_ItemLayoutResID();
		
		_adapter = new MapSetArrayAdapter(this, itemLayout, items);
		setListAdapter(_adapter);
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);
		
		IMapSetListItem itemVal = _adapter.getItem(position);
		_retIntent.putExtra(RESULT_MAPSET_NEWITEMVAL, itemVal);

		sendFinish(RESULT_OK);
	}
}
