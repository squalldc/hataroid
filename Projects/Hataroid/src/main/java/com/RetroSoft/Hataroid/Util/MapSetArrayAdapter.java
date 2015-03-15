package com.RetroSoft.Hataroid.Util;

import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;

public class MapSetArrayAdapter extends ArrayAdapter<IMapSetListItem>
{
	private Context						_c;
	private int							_id;
	private List<IMapSetListItem>		_items;

	public MapSetArrayAdapter(Context context, int textViewResourceId, List<IMapSetListItem> items)
	{
		super(context, textViewResourceId, items);

		_c = context;
		_id = textViewResourceId;
		_items = items;
	}
	
	public IMapSetListItem getItem(int i)
	{
		if (_items != null && i < _items.size())
		{
			return _items.get(i);
		}
		return null;
	}
	
	@Override public View getView(int position, View convertView, ViewGroup parent)
	{
		View v = convertView;
		
		if (v == null)
		{
			LayoutInflater vi = (LayoutInflater)_c.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			v = vi.inflate(_id, null);
		}

		final IMapSetListItem item = _items.get(position);
		if (item != null)
		{
			item.formatItemView(v);
		}
		return v;
	}
}
