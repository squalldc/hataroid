package com.RetroSoft.Hataroid.SaveState;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class SaveStateAdapter extends ArrayAdapter<SaveStateListItem>
{
	private Context					_c;
	private int						_id;
	private List<SaveStateListItem>	_items;

	public SaveStateAdapter(Context context, int textViewResourceId, List<SaveStateListItem> items)
	{
		super(context, textViewResourceId, items);

		_c = context;
		_id = textViewResourceId;
		_items = items;
	}
	
	public SaveStateListItem getItem(int i)
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

		final SaveStateListItem item = _items.get(position);
		if (item != null)
		{
			TextView tvSlot = (TextView) v.findViewById(R.id.slotText);
			TextView tvDate = (TextView) v.findViewById(R.id.dateText);
			TextView tvName = (TextView) v.findViewById(R.id.nameText);
			ImageView i1 = (ImageView) v.findViewById(R.id.imageView1);
			
			boolean isDummyItem = item.isDummyItem();

			int color = Color.WHITE;
			if (item.isFirstSlotItem())
			{
				color = Color.GREEN;
			}
			else if (isDummyItem)
			{
				color = Color.YELLOW;
			}
			else if (item.isQuickSaveSlotItem())
			{
				color = Color.CYAN;
			}

			if (tvSlot != null)
			{
				int slot = item.getSlotID();
				if (isDummyItem)
				{
					tvSlot.setText("");
					tvSlot.setEnabled(false);
				}
				else
				{
					//tvSlot.setText((slot<0)?"-":String.valueOf(slot+1));
					tvSlot.setText((slot<0)?"---":String.format("%03d", slot+1));
				}
				tvSlot.setTextColor(color);
			}
			if (tvDate != null)
			{
				String dateStr = "";
				if (!isDummyItem)
				{
					SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm");
					dateStr = sdf.format(new Date(item.getLastModified()));
					tvDate.setMaxLines(1);
				}
				else
				{
					tvDate.setMaxHeight(8);
					tvDate.setEnabled(false);
				}
				tvDate.setText(dateStr);
				tvDate.setTextColor(color);
			}
			if (tvName != null)
			{
				if (item.isQuickSaveSlotItem())
				{
					tvName.setText("[QuickSave] " + item.getSaveName());
				}
				else
				{
					tvName.setText(item.getSaveName());
				}
				tvName.setTextColor(color);
			}

			if (i1 != null)
			{
				boolean clearImage = true;
				if (!item.isDummyItem())
				{
					SaveMetaData metaData = new SaveMetaData();
					if (_readSaveMetaFile(item.getPath(), metaData))
					{
						i1.setImageBitmap(metaData.bmp);
						i1.setScaleType(ScaleType.FIT_CENTER);
						clearImage = false;
					}
				}

				if (clearImage)
				{
					i1.setMinimumWidth(80);
					i1.setImageDrawable(null);
				}
			}
		}
		return v;
	}
	
	boolean _readSaveMetaFile(String filePath, SaveMetaData metaData)
	{
		if (!filePath.toLowerCase().endsWith(".ss"))
		{
			return false;
		}
		
		final int kInitHeaderSize = 8;
		
		File file = new File(filePath);
		int fileLen = (int)file.length();
		if (fileLen < kInitHeaderSize)
		{
			return false;
		}

		boolean result = false;
		InputStream istream = null;
		try
		{
			for (int i = 0; i < 1; ++i)
			{
				int bytesRemain = fileLen;
				istream = new BufferedInputStream(new FileInputStream(file));
	
				byte[] initHeaderBuf = _readBytes(istream, kInitHeaderSize);
				bytesRemain -= kInitHeaderSize;
				if (initHeaderBuf == null)
				{
					break;
				}
				
				// parse init header
				{
					ByteBuffer buf = ByteBuffer.wrap(initHeaderBuf);
					buf.order(ByteOrder.LITTLE_ENDIAN);
					IntBuffer intBuf = buf.asIntBuffer();
					
					int hdr = intBuf.get();
					if (hdr != 0x74737368)
					{
						break;
					}
					
					int headerSize = intBuf.get();
					if (headerSize > bytesRemain)
					{
						break;
					}
					
					metaData.header = _readBytes(istream, headerSize);
					bytesRemain -= headerSize;
					if (metaData.header == null)
					{
						break;
					}
				}
				
				// parse header
				{
					ByteBuffer buf = ByteBuffer.wrap(metaData.header);
					buf.order(ByteOrder.LITTLE_ENDIAN);
					IntBuffer intBuf = buf.asIntBuffer();
					
					metaData.version = intBuf.get();
					metaData.width = intBuf.get();
					metaData.height = intBuf.get();
					
					int pixCount = metaData.width * metaData.height;
					int pixSize = pixCount << 1;
					if (pixSize > bytesRemain)
					{
						break;
					}

					metaData.bmpData = _readBytes(istream, pixSize);
					bytesRemain -= pixSize;
					if (metaData.bmpData == null)
					{
						break;
					}
				}
				
				// parse bitmap data
				{
					ByteBuffer buf = ByteBuffer.wrap(metaData.bmpData);
					//buf.order(ByteOrder.LITTLE_ENDIAN);

					metaData.bmp = Bitmap.createBitmap(metaData.width, metaData.height, Config.RGB_565);
					metaData.bmp.copyPixelsFromBuffer(buf);

					metaData.bmpData = null;	// no longer needed
					
					result = true;
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			result = false;
		}
		
		if (istream != null)
		{
			try
			{
				istream.close();
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
			istream = null;
		}
		
		metaData.header = null;		// no longer needed
		metaData.bmpData = null;	// no longer needed
		if (!result)
		{
			metaData.bmp = null;
		}
		
		return result;
    }

	byte[] _readBytes(InputStream istream, int len)
	{
		byte[] buf = null;

		try
		{
			buf = new byte [len];

			int totalBytesRead = 0;
			while(totalBytesRead < len)
			{
				int bytesRemaining = len - totalBytesRead;
				int bytesRead = istream.read(buf, totalBytesRead, bytesRemaining); 
				if (bytesRead > 0)
				{
					totalBytesRead += bytesRead;
				}
				else
				{
					break;
				}
			}

			if (totalBytesRead != len)
			{
				return null;
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			buf = null;
		}
		
		return buf;
	}
	
	private class SaveMetaData
	{
		public byte[]	header = null;
		public byte[]	bmpData = null;
		
		@SuppressWarnings("unused")
		public int		version = 0;
		
		public int		width = 0;
		public int		height = 0;

		public Bitmap	bmp = null;
	}
}
