package com.RetroSoft.Hataroid.Midi;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;

import com.RetroSoft.Hataroid.HataroidNativeLib;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.Util.IMapSet;
import com.RetroSoft.Hataroid.Util.IMapSetListItem;
import com.RetroSoft.Hataroid.Util.MapSetConfigureView;

public class InstrPatchMap implements IMapSet
{
	public final static int kInstrMaxChannels		= 16;
//	public final static int kDefaultPercChannel		= 10;

	public final static int[] kDefaultInstrPatch = {
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
	};

	public final static int[] kKarateKid2InstrPatch = {
		11,
		50,
		34,
		61,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
	};

	final static String kPresetIDPrefix = "_preset";
	final static String [] kPresetNames = { "Default", "Karate Kid 2" };
	final static String [] kPresetIDs = { kPresetIDPrefix+"Default", kPresetIDPrefix+"KKid2" };
	final static int[][] kPresets = { kDefaultInstrPatch, kKarateKid2InstrPatch };
	public static String getKK2PresetPatchID() { return kPresetIDs[1]; }

	int[] _chanInstrMap = null;

	//---- IMapSet implementation
	final static int MAPVALUE_RESULT_KEYCODE			= 1;
	final static int RENAME_MAPSET_RESULT_KEYCODE		= 2;
	public final static String kBasePrefPrefix			= "_PREFMapSet_";
	public final static String kPrefPrefix				= kBasePrefPrefix + "Instr_";
	final static String kDeleteDialogTitle				= "Delete Instrument Map?";
	final static String DDeleteDialogMsg				= "Are you sure you want to delete this instrument map?";
	final static String kNewMapSetNamePrefix			= "my_instrument_map";

	
	public int get_MAPVALUE_RESULT_KEYCODE()		{ return MAPVALUE_RESULT_KEYCODE; }
	public int get_RENAME_MAPSET_RESULT_KEYCODE()	{ return RENAME_MAPSET_RESULT_KEYCODE; }

	public String get_PREF_PREFIX()					{ return kPrefPrefix; }
	
	public String get_DeleteDialogTitle()			{ return kDeleteDialogTitle; }
	public String get_DeleteDialogMessage()			{ return DDeleteDialogMsg; }
	public String get_NewMapSetNamePrefix()			{ return kNewMapSetNamePrefix; }

	public int get_ViewLayoutResID()				{ return R.layout.midi_instrument_patch_view; }
	public int get_ItemLayoutResID()				{ return R.layout.midi_instrument_patch_item; }

	public int getNumPresets()						{ return kPresetIDs.length; }
	public String getPresetID(int idx)				{ return (idx>=0 && idx<kPresetIDs.length) ? kPresetIDs[idx] : null; }
	public String getPresetName(int idx)			{ return (idx>=0 && idx<kPresetNames.length) ? kPresetNames[idx] : null; }
	public boolean isSystemPreset(String presetID)	{ return (presetID != null) ? presetID.startsWith(kPresetIDPrefix) : false; }
	
	public void initDefault()						{ initFromPreset(kPresetIDs[0]); }

	public void initFromPreset(String presetID)
	{
		int presetIdx = 0;
		for (int i = 0; i < kPresetIDs.length; ++i)
		{
			if (presetID.compareTo(kPresetIDs[i]) == 0)
			{
				presetIdx = i;
				break;
			}
		}
		
		_chanInstrMap = new int [kInstrMaxChannels];
		for (int i = 0; i < kInstrMaxChannels; ++i)
		{
			_chanInstrMap[i] = kPresets[presetIdx][i];
		}
	}

	public boolean decodeMapSetPref(String prefVal, Map<String,Object> result)
	{
		if (prefVal == null)
		{
			return false;
		}

		boolean error = false;
		try
		{
			String [] info = prefVal.split(",");
			if (info.length == 0)
			{
				error = true;
			}
			else
			{
				InstrPatchMap map = new InstrPatchMap();
				map.initDefault();

				String name = info[0].trim();
				for (int i = 1; i < info.length-1; i += 2)
				{
					int chan = Integer.parseInt(info[i].toString().trim());
					int instr = Integer.parseInt(info[i+1].toString().trim());
					map._addMapEntry(chan, instr);
				}
				
				if (name == null) { name = "unknown"; }

				result.put("name", name);
				result.put("map", map);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			error = true;
		}
		
		return !error;
	}

	public String encodePrefString(String name)
	{
		String s = (name==null) ? "unknown" : name;
		for (int i = 0; i < kInstrMaxChannels; ++i)
		{
			s += "," + i + "," + _chanInstrMap[i];
		}
		return s;
	}


	public List<IMapSetListItem> buildMapSetListItems()
	{
		List<IMapSetListItem> items = new ArrayList<IMapSetListItem>();

		for (int i = 0; i < kInstrMaxChannels; ++i)
		{
			InstrMapListItem item = new InstrMapListItem(i, _chanInstrMap[i]);
			items.add(item);
		}

		Collections.sort(items);
		return items;
	}

	public boolean updateEntry(boolean unMap, IMapSetListItem srcItem, IMapSetListItem newItemVal)
	{
		InstrMapListItem item = (InstrMapListItem)srcItem;
		if (item != null)
		{
			if (unMap)
			{
				return _removeMapEntry(item._chan);
			}
			else
			{
				InstrItem itemVal = (InstrItem)newItemVal;
				if (itemVal != null)
				{
					return _addMapEntry(item._chan, itemVal._instr);
				}
			}
		}
		return false;
	}

	public void updateListItem(IMapSetListItem li)
	{
		if (_chanInstrMap != null)
		{
			InstrMapListItem item = (InstrMapListItem)li;
			int chan = item._chan;
			if (chan >= 0 && chan < kInstrMaxChannels)
			{
				item._instr = _chanInstrMap[chan];
			}
		}
	}
	
	public void setupButtonListeners(MapSetConfigureView view)
	{
		final MapSetConfigureView msView = view;
		view.findViewById(R.id.im_sendBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				_onSendClicked(msView);
			}
		});
	}
	
	public void _onSendClicked(MapSetConfigureView view)
	{
		if (view != null)
		{
			InstrPatchMap patchMap = (InstrPatchMap)view.getCurMapSet();
			if (patchMap != null)
			{
				patchMap.sendPatch(view);
			}
		}
	}

	public void sendPatchDirect()
	{
		Log.i("hataroid", "SENDING MIDI PATCH");
		if (_chanInstrMap != null)
		{
			HataroidNativeLib.emulatorSendMidiInstrPatch(_chanInstrMap);
		}
	}

	//---- IMapSet implementation
	
	void sendPatch(MapSetConfigureView view)
	{
		Log.i("hataroid", "SENDING MIDI PATCH");
		if (_chanInstrMap != null)
		{
			HataroidNativeLib.emulatorSendMidiInstrPatch(_chanInstrMap);

			String msgExtra = "";
			try
			{
				String curPresetID = view.getCurPresetID();
				if (curPresetID != null && curPresetID.equals(kPresetIDPrefix+"KKid2"))
				{
					msgExtra = "\n\nKarate Kid 2 resets the instruments on the title screen\n- please enable the option to ignore program changes\n\nKarate Kid 2 GM output is offkey (as it's intended for a cz101\n- please enable the Mute ST sound option or Transpose the key by a tone";
				}
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}

			AlertDialog alertDialog = new AlertDialog.Builder(view).create();
    		alertDialog.setTitle("Patch Sent");
    		alertDialog.setMessage("The instrument patch was successfully sent to the synth." + msgExtra);
    		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) {  } });
    		alertDialog.show();
		}
	}

	void _clear()
	{
		for (int i = 0; i < kInstrMaxChannels; ++i)
		{
			_chanInstrMap[i] = -1;
		}
	}

	boolean _removeMapEntry(int chan)
	{
		if (_chanInstrMap != null && chan >= 0 && chan < kInstrMaxChannels)
		{
			_chanInstrMap[chan] = -1;
			return true;
		}
		return false;
	}

	boolean _addMapEntry(int chan, int instr)
	{
		if (_chanInstrMap != null && chan >= 0 && chan < kInstrMaxChannels && instr >= 0 && instr < GM1Instruments.INSTR_NUM_OF)
		{
			_chanInstrMap[chan] = instr;
			return true;
		}
		return false;
	}
}

class InstrMapListItem implements IMapSetListItem
{
	private static final long serialVersionUID = 1L;

	public int get_ViewLayoutResID() { return R.layout.midi_instrument_select_view; }
	public int get_ItemLayoutResID() { return R.layout.midi_instrument_select_item; }

	int _chan = -1;
	int _instr = -1;
	
	public InstrMapListItem(int chan, int instr)
	{
		_chan = chan;
		_instr = instr;
	}

	public void formatItemView(View v)
	{
		TextView chanView = (TextView) v.findViewById(R.id.ChannelText);
		TextView instrView = (TextView) v.findViewById(R.id.InstrumentText);

		if (chanView != null)
		{
			chanView.setText("Channel " + (_chan+1));
			chanView.setTextColor(Color.WHITE);
		}

		if (instrView != null)
		{
			String instrName = null;
			if (_instr >= 0 && _instr < GM1Instruments.INSTR_NUM_OF)
			{
				instrName = GM1Instruments.kInstrNames[_instr];
			}

			if (instrName == null || instrName.length() == 0)
			{
				instrView.setText("- Default -");
				instrView.setTextColor(Color.YELLOW);
			}
			else
			{
				instrView.setText("[" + _instr + "] " + instrName);
				instrView.setTextColor(Color.GREEN);
			}
		}
	}

	public int compareTo(IMapSetListItem item)
	{
		InstrMapListItem o = (InstrMapListItem)item;
		if (_chan == o._chan)
		{
			return (_instr < o._instr) ? -1 : 1;
		}
		return (_chan < o._chan) ? -1 : 1;
	}

	public List<IMapSetListItem> buildListItems()
	{
		List<IMapSetListItem> items = new ArrayList<IMapSetListItem>();

		for (int i = 0; i < GM1Instruments.INSTR_NUM_OF; ++i)
		{
			InstrItem item = new InstrItem(i);
			items.add(item);
		}

		Collections.sort(items);
		return items;
	}

	public boolean isSameItemKey(IMapSetListItem other)
	{
		InstrMapListItem o = (InstrMapListItem)other;
		return (_chan == o._chan);
	}
}

class InstrItem implements IMapSetListItem, Comparable<IMapSetListItem>
{
	private static final long serialVersionUID = 1L;

	public int get_ViewLayoutResID() { return -1; } // leaf item, unused
	public int get_ItemLayoutResID() { return -1; } // leaf item, unused
	public List<IMapSetListItem> buildListItems() { return null; } // leaf item, unused
	public boolean isSameItemKey(IMapSetListItem other) { return true; } // leaf item, unused

	int _instr = -1;
	
	public InstrItem(int instr)
	{
		_instr = instr;
	}

	public void formatItemView(View v)
	{
		TextView instrView = (TextView) v.findViewById(R.id.InstrumentText);

		if (instrView != null)
		{
			String instrName = null;
			if (_instr >= 0 && _instr < GM1Instruments.INSTR_NUM_OF)
			{
				instrName = GM1Instruments.kInstrNames[_instr];
			}

			instrView.setTextColor(Color.WHITE);
			if (instrName == null || instrName.length() == 0)
			{
				instrView.setText("[" + _instr + "] " + "Unknown");
			}
			else
			{
				instrView.setText("[" + _instr + "] " + instrName);
			}
		}
	}

	public int compareTo(IMapSetListItem item)
	{
		InstrItem o = (InstrItem)item;
		return (_instr < o._instr) ? -1 : 1;
	}
}
