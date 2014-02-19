package com.RetroSoft.Hataroid.Input;

import java.util.List;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class InputMapArrayAdapter extends ArrayAdapter<InputMapListItem>
{
	private Context					_c;
	private int						_id;
	private List<InputMapListItem>	_items;

	public InputMapArrayAdapter(Context context, int textViewResourceId, List<InputMapListItem> items)
	{
		super(context, textViewResourceId, items);

		_c = context;
		_id = textViewResourceId;
		_items = items;
	}
	
	public InputMapListItem getItem(int i)
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

		final InputMapListItem item = _items.get(position);
		if (item != null)
		{
			TextView t1 = (TextView) v.findViewById(R.id.EmuKeyText);
			TextView t2 = (TextView) v.findViewById(R.id.SystemKeyText);

			if (t1 != null)
			{
				t1.setText(item.getEmuKeyName());
				t1.setTextColor(Color.WHITE);
			}
			if (t2 != null)
			{
				String systemKeyName = item.getSystemKeyName();
				if (systemKeyName == null)
				{
					t2.setText("Unmapped");
					t2.setTextColor(Color.YELLOW);
				}
				else
				{
					t2.setText("Mapped to keycode: " + systemKeyName);
					t2.setTextColor(Color.GREEN);
				}
			}
		}
		return v;
	}
}
