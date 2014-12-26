package com.RetroSoft.Hataroid.Input.Shortcut;

import java.util.HashMap;
import java.util.Map;

import android.content.SharedPreferences;

import com.RetroSoft.Hataroid.Input.VirtKeyDef;

public class ShortcutMap
{
	public final static String	kDynPrefKeyName = "_pref_dyn_shortcut_map";
	
	public final static int kAnchorTR		= 0;
	public final static int kAnchorBR		= 1;
	public final static int kNumAnchors		= 2;

	public final static String [] kAnchorNames = {
		"Top Right",
		"Bottom Right",
	};
	
	// KEEP IN SYNC WITH JNI SIDE
	public final static int [] kMaxKeys = {
		6, // kAnchorTR
		4, // kAnchorBR
	};

	public final static int [][] kDefaultPreset = {
		// kAnchorTR
		{
			VirtKeyDef.VKB_KEY_TURBOSPEED,
			VirtKeyDef.VKB_KEY_Y,
			VirtKeyDef.VKB_KEY_N,
			VirtKeyDef.VKB_KEY_1,
			VirtKeyDef.VKB_KEY_ANDROID_MENU,
			-1,//VirtKeyDef.VKB_KEY_RETURN,
		},
		// kAnchorBR
		{
			VirtKeyDef.VKB_KEY_SPACE,
			VirtKeyDef.VKB_KEY_LEFTSHIFT,
			VirtKeyDef.VKB_KEY_ALTERNATE,
			-1,//VirtKeyDef.VKB_KEY_CONTROL,
		}
	};

	public final static String kPresetIDPrefix = "_preset";
	final static String [] kPresetNames = { "Default" };
	final static String [] kPresetIDs = { kPresetIDPrefix+"Default" };

	int [][] _shortcutMap = null;
	
	public ShortcutMap()
	{
		_shortcutMap = new int [kNumAnchors][];
		for (int i = 0; i < kNumAnchors; ++i)
		{
			_shortcutMap[i] = new int [kMaxKeys[i]];
			for (int k = 0; k < kMaxKeys[i]; ++k)
			{
				_shortcutMap[i][k] = kDefaultPreset[i][k];
			}
		}
	}
	
	public void clear()
	{
		for (int i = 0; i < kNumAnchors; ++i)
		{
			for (int k = 0; k < kMaxKeys[i]; ++k)
			{
				_shortcutMap[i][k] = -1;
			}
		}
	}

	public int [][] getCurMap() { return _shortcutMap; }
	
	public boolean removeShortcutEntry(int anchor, int shortcutIdx)
	{
		if (anchor >= 0 && anchor < kNumAnchors && shortcutIdx >= 0 && shortcutIdx < kMaxKeys[anchor])
		{
			_shortcutMap[anchor][shortcutIdx] = -1;
			return true;
		}
		return false;
	}

	public boolean addShortcutEntry(int anchor, int shortcutIdx, int emuKey)
	{
		if (anchor >= 0 && anchor < kNumAnchors && shortcutIdx >= 0 && shortcutIdx < kMaxKeys[anchor])
		{
			_shortcutMap[anchor][shortcutIdx] = emuKey;
			return true;
		}
		return false;
	}

	public static String getPresetID() { return kPresetIDs[0]; }
	public static String getPresetName() { return kPresetNames[0]; }
	public static boolean isSystemPreset(String presetID)
	{
		if (presetID != null)
		{
			return presetID.startsWith(ShortcutMap.kPresetIDPrefix);
		}
		
		return false;
	}

	String encodePrefString(String name)
	{
		String s = (name==null) ? "unknown" : name;
		for (int a = 0; a < _shortcutMap.length; ++a)
		{
			for (int k = 0; k < _shortcutMap[a].length; ++k)
			{
				int vkId = _shortcutMap[a][k];
				s += "," + a + "," + k + "," + vkId;
			}
		}

		return s;
	}

	public static boolean decodeShortcutMapPref(String prefVal, Map<String,Object> result)
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
				ShortcutMap map = new ShortcutMap();
				map.clear();

				String name = info[0].trim();
				for (int i = 1; i < info.length-2; i += 3)
				{
					int anchor = Integer.parseInt(info[i].toString().trim());
					int shortcutIdx = Integer.parseInt(info[i+1].toString().trim());
					int emuKey = Integer.parseInt(info[i+2].toString().trim());
					map.addShortcutEntry(anchor, shortcutIdx, emuKey);
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

	public static void getSelectedOptionFromPrefs(SharedPreferences prefs, Map<String,?> allPrefs, Map<String,String> dynOptions)
	{
		ShortcutMap m = null;

		if (prefs != null && allPrefs != null)
		{
			String lastPresetID = prefs.getString(ShortcutMapConfigureView.kPrefLastPresetIDKey, null);
			if (lastPresetID != null && !isSystemPreset(lastPresetID))
			{
				String key = ShortcutMapConfigureView.kPrefUserPresetPrefix + lastPresetID;
				String prefStr = prefs.getString(key, null);
				if (prefStr != null)
				{
					try
					{
						Map<String,Object> result = new HashMap<String,Object>();
						boolean error = !decodeShortcutMapPref(prefStr, result);
						if (!error)
						{
							m = (ShortcutMap)result.get("map");
						}
					}
					catch (Exception e)
					{
						e.printStackTrace();
					}
				}
			}
		}
		
		if (m == null)
		{
			m = new ShortcutMap();
		}

		// setup our dynamic options
		String dynPref = "";

		int n = 0;
		for (int i = 0; i < kNumAnchors; ++i)
		{
			for (int k = 0; k < kMaxKeys[i]; ++k)
			{
				dynPref += ((n==0)?"":",") + (m._shortcutMap[i][k]);
				++n;
			}
		}
		
		dynOptions.put(kDynPrefKeyName, dynPref);
	}
}
