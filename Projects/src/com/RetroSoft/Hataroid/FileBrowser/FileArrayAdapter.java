package com.RetroSoft.Hataroid.FileBrowser;

import java.util.List;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class FileArrayAdapter extends ArrayAdapter<FileListItem>
{
	private Context				_c;
	private int					_id;
	private List<FileListItem>	_items;

	public FileArrayAdapter(Context context, int textViewResourceId, List<FileListItem> items)
	{
		super(context, textViewResourceId, items);

		_c = context;
		_id = textViewResourceId;
		_items = items;
	}
	
	public FileListItem getItem(int i)
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

		final FileListItem item = _items.get(position);
		if (item != null)
		{
			TextView t1 = (TextView) v.findViewById(R.id.TextView01);
			ImageView i1 = (ImageView) v.findViewById(R.id.imageView1);

			if (t1 != null)
			{
				t1.setText(item.getName());
				t1.setTextColor(Color.WHITE);
			}
			if (i1 != null)
			{
				if (item.isDir())
				{
					i1.setImageResource(R.drawable.stfolder);
				}
				else if (item.getName().toLowerCase().endsWith(".st"))
				{
					i1.setImageResource(R.drawable.stfloppy);
				}
				else
				{
					i1.setImageResource(R.drawable.stdoc);
				}
			}
		}
		return v;
	}
}
