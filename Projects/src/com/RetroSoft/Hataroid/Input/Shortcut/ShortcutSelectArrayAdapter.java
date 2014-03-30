package com.RetroSoft.Hataroid.Input.Shortcut;

import java.util.List;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class ShortcutSelectArrayAdapter extends ArrayAdapter<ShortcutSelectListItem>
{
	private Context							_c;
	private int								_id;
	private List<ShortcutSelectListItem>	_items;

	public ShortcutSelectArrayAdapter(Context context, int textViewResourceId, List<ShortcutSelectListItem> items)
	{
		super(context, textViewResourceId, items);

		_c = context;
		_id = textViewResourceId;
		_items = items;
	}
	
	public ShortcutSelectListItem getItem(int i)
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

		final ShortcutSelectListItem item = _items.get(position);
		if (item != null)
		{
			TextView t1 = (TextView) v.findViewById(R.id.EmuKeyText);

			if (t1 != null)
			{
				t1.setText(item.getEmuKeyName());
				t1.setTextColor(Color.WHITE);
			}
		}
		return v;
	}
}
