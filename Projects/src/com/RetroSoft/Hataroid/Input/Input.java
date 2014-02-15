package com.RetroSoft.Hataroid.Input;

import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;

import com.RetroSoft.Hataroid.Util.BitFlags;

public class Input
{
	public final static int kPreset_Default		= 0;
	public final static int kPreset_WiiMote		= 1;
	public final static int kPreset_NumOf		= 2;
	
	public final static String kPresetIDPrefix = "_preset";
	final static String [] kPresetNames = { "Default", "WiiMote" };
	final static String [] kPresetIDs = { kPresetIDPrefix+"Default", kPresetIDPrefix+"WiiMote" };

	final static int [][] kPresets = {
			{ }, //kPreset_Default
			{
				//kPreset_WiiMote
				19, VirtKeyDef.VKB_KEY_JOYUP,		//up
				20, VirtKeyDef.VKB_KEY_JOYDOWN,		//down
				21, VirtKeyDef.VKB_KEY_JOYLEFT,		//left
				22, VirtKeyDef.VKB_KEY_JOYRIGHT,	//right
				8, VirtKeyDef.VKB_KEY_SPACE,		//1
				9, VirtKeyDef.VKB_KEY_JOYFIRE,		//2
				23, VirtKeyDef.VKB_KEY_TURBOSPEED,	//a
				4, -1,								//b
				41, VirtKeyDef.VKB_KEY_SCREENPRESETS,	//-
				44, VirtKeyDef.VKB_KEY_MOUSETOGGLE,	//+
				36, VirtKeyDef.VKB_KEY_TOGGLEUI,	//home
			}
		};
	
	public final static int kNumSrcKeyCodes = 256;

	boolean		_inputEnabled = false;
	
	InputMap	_curInputMap = null;
	int []		_srcToDestMap = null;
	int			_numSrcInputs = 0;

	BitFlags _curPresses = new BitFlags(VirtKeyDef.VKB_KEY_NumOf);
	
	Context _appContext = null;
	
	public Input()
	{
		_curInputMap = new InputMap();
		_curInputMap.init(kNumSrcKeyCodes);
		_cacheInputMapValues();
	}
	
	void _cacheInputMapValues()
	{
		_srcToDestMap = _curInputMap.srcToDestMap;
		_numSrcInputs = _srcToDestMap.length;
	}
	
	public void init(Context appContext)
	{
		_appContext = appContext;
	}
	
	public void setupOptionsFromPrefs(SharedPreferences prefs)
	{
    	Map<String,?> allPrefs = prefs.getAll();
    	
    	_inputEnabled = false;

    	_curInputMap = new InputMap();
		_curInputMap.init(kNumSrcKeyCodes);

		Object val = allPrefs.get("pref_input_device_enable_inputmethod");
    	if (val != null)
    	{
    		String sval = val.toString();
    		_inputEnabled = !(sval.compareTo("false")==0 || sval.compareTo("0")==0);
    	}

    	if (_inputEnabled)
		{
			String lastPresetID = prefs.getString(InputMapConfigureView.kPrefLastPresetIDKey, null);
			if (lastPresetID != null)
			{
				// see if system preset
				if (isSystemPreset(lastPresetID))
				{
					// system preset
					int [] presetArray = getPresetArray(getPresetID(lastPresetID));
					if (presetArray != null)
					{
						_curInputMap = new InputMap();
						_curInputMap.initFromArray(Input.kNumSrcKeyCodes, presetArray);

						Log.i("hataroid", "Setting input map: " + lastPresetID);
					}
				}
				else
				{
					String key = InputMapConfigureView.kPrefUserPresetPrefix + lastPresetID;
					String prefStr = prefs.getString(key, null);

					Map<String,Object> result = new HashMap<String,Object>();
					if (Input.decodeInputMapPref(prefStr, result))
					{
						_curInputMap = (InputMap)result.get("map");

						Log.i("hataroid", "Setting input map: " + result.get("name") + " (" + lastPresetID + ")");
					}
				}
			}
		}

    	_curPresses.clearAll();
		_cacheInputMapValues();
	}
	
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (!_inputEnabled)
		{
			return false;
		}

		int c = event.getKeyCode();
		//Log.i("hataroid", "keydown: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

		if (c >= 0 && c < _numSrcInputs)
		{
			//InputDevice idev = event.getDevice();
			//if (idev != null)
			//{
			//	Log.i("hataroid", "id: " + idev.getId() + ", n: " + (idev.getName()!=null?idev.getName():""));
			//}
			
			int im = _srcToDestMap[c];
			if (im >= 0)
			{
				_curPresses.setBit(im);
				return true;
			}
		}
		
		return false;
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		if (!_inputEnabled)
		{
			return false;
		}

		int c = event.getKeyCode();
		//Log.i("hataroid", "keyup: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());
		
		if (c >= 0 && c < _numSrcInputs)
		{
			//InputDevice idev = event.getDevice();
			//if (idev != null)
			//{
			//	Log.i("hataroid", "id: " + idev.getId() + ", n: " + (idev.getName()!=null?idev.getName():""));
			//}
	
			int im = _srcToDestMap[c];
			if (im >= 0)
			{
				_curPresses.clearBit(im);
				return true;
			}
		}

		return false;
	}
	
	public BitFlags getKeyPresses()
	{
		return _curPresses;
	}
	
	public void showInputMethodSelector()
	{
		InputMethodManager imm = (InputMethodManager)_appContext.getSystemService(Context.INPUT_METHOD_SERVICE);
		imm.showInputMethodPicker();
	}
	
	public static int getPresetID(String presetName)
	{
		for (int i = 0; i < kPreset_NumOf; ++i)
		{
			if (kPresetIDs[i].compareTo(presetName) == 0)
			{
				return i;
			}
		}
		return -1;
	}
	
	public static String getPresetName(int presetID)
	{
		if (presetID >=0 && presetID < kPreset_NumOf)
		{
			return kPresetNames[presetID];
		}
		return null;
	}
	
	public static String getPresetID(int presetID)
	{
		if (presetID >=0 && presetID < kPreset_NumOf)
		{
			return kPresetIDs[presetID];
		}
		return null;
	}

	public static int [] getPresetArray(int presetID)
	{
		if (presetID >=0 && presetID < kPreset_NumOf)
		{
			return kPresets[presetID];
		}
		return null;
	}
	
	public static boolean isSystemPreset(String presetID)
	{
		if (presetID != null)
		{
			return presetID.startsWith(Input.kPresetIDPrefix);
		}
		
		return false;
	}
	
	public static boolean decodeInputMapPref(String prefVal, Map<String,Object> result)
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
				InputMap map = new InputMap();
				map.init(Input.kNumSrcKeyCodes);
				map.clear();

				String name = info[0];
				for (int i = 1; i < info.length-1; i += 2)
				{
					int srcKey = Integer.parseInt(info[i].toString());
					int destKey = Integer.parseInt(info[i+1].toString());
					map.addKeyMapEntry(srcKey, destKey);
				}
				
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
	
	public static String getUserInputMaps(SharedPreferences prefs)
	{
		String result = "";
    	Map<String,?> allPrefs = prefs.getAll();
		for (Map.Entry<String,?> entry : allPrefs.entrySet())
		{
			String key = entry.getKey();
			if (key.startsWith(InputMapConfigureView.kPrefUserPresetPrefix))
			{
				result += entry.getValue().toString() + "\n";
			}
		}
		return result;
	}
}
