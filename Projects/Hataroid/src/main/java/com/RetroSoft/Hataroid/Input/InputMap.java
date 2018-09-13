package com.RetroSoft.Hataroid.Input;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.annotation.SuppressLint;
import android.util.Log;

@SuppressLint("UseSparseArrays")
public class InputMap
{
	public int []                       srcToDestMap = null;
	public Map<Integer, List<Integer>>  destToSrcMap = null;
	public int                          numDestInputs = 0;

	public InputMap() { }
	
	public void clear()
	{
		for (int i = 0; i < srcToDestMap.length; ++i)
		{
			srcToDestMap[i] = -1;
		}
		destToSrcMap.clear();
	}
	
	private void addDefaults(int localeID)
	{
		for (int i = 0; i < VirtKeyDef.kDefs.length; ++i)
		{
			VirtKeyDef vk = VirtKeyDef.kDefs[i];
			addKeyMapEntry((int)vk.getAndroidKeyCode(localeID), vk.id);
		}
	}
	
	public void init(int numSrcInputs_, int localeID)
	{
		srcToDestMap = new int [numSrcInputs_];
		destToSrcMap = new HashMap<Integer, List<Integer>>();
		numDestInputs = VirtKeyDef.VKB_KEY_NumOf;

		// reset
		clear();

		// default map
		addDefaults(localeID);
	}
	
	public void initFromArray(int numSrcInputs_, int [] inputMap, int localeID)
	{
		init(numSrcInputs_, localeID);

		if (inputMap == null) { return; }
		
		// combine our chosen preset map
		for (int i = 0; i < inputMap.length-1; i += 2)
		{
			int srcKey = inputMap[i];
			int destKey = inputMap[i+1];
			if (destKey < 0)
			{
				removeKeyMapEntry(srcKey);
			}
			else
			{
				addKeyMapEntry(srcKey, destKey);
			}
		}

		autoRemapRegionKeys(localeID);
	}
	
	public boolean removeKeyMapEntry(int srcKey)
	{
		if (srcKey < 0 || srcKey >= srcToDestMap.length)
		{
			return false;
		}
		
		if (srcToDestMap[srcKey] >= 0)
		{
			int removeKey = srcToDestMap[srcKey];
			if (destToSrcMap.containsKey(removeKey))
			{
				List<Integer> sysKeys = destToSrcMap.get(removeKey);
				int sysKeyIdx = sysKeys.indexOf(srcKey);
				if (sysKeyIdx >= 0)
				{
					sysKeys.remove(sysKeyIdx);
				}
			}
		}

		srcToDestMap[srcKey] = -1;

		return true;
	}

	public boolean removeDestKeyMapEntry(int destKey)
	{
		List<Integer> sysKeys = destToSrcMap.get(destKey);
		if (sysKeys != null)
		{
			for (int i = 0; i < sysKeys.size(); ++i)
			{
				int srcKey = sysKeys.get(i);
				if (srcKey >= 0 && srcKey < srcToDestMap.length)
				{
					srcToDestMap[srcKey] = -1;
				}
			}

			sysKeys.clear();
		}

		return true;
	}

	public boolean addKeyMapEntry(int srcKey, int destKey)
	{
		if (srcKey < 0 || srcKey >= srcToDestMap.length || destKey < 0 || destKey >= numDestInputs)
		{
			return false;
		}
		
		removeKeyMapEntry(srcKey);

		srcToDestMap[srcKey] = destKey;
		
		List<Integer> sysKeysList = destToSrcMap.get(destKey);
		if (sysKeysList == null)
		{
			sysKeysList = new LinkedList<Integer>();
			destToSrcMap.put(destKey, sysKeysList);
		}
		if (sysKeysList.indexOf(srcKey) == -1)
		{
			sysKeysList.add(srcKey);
		}

		return true;
	}

	String encodePrefString(String name)
	{
		String s = (name==null) ? "unknown" : name;
		for (int i = 0; i < srcToDestMap.length; ++i)
		{
			int destKey = srcToDestMap[i];
			if (destKey >= 0)
			{
				s += "," + i + "," + destKey;
			}
		}
		return s;
	}

	public void autoRemapRegionKeys(int localeID)
	{
		// (src) android key id -> (dst) virt key id
		try
		{
			HashSet<Integer>    remapIDs = new HashSet<Integer>();
			List<Integer>       removeIDs = new ArrayList<Integer>();

			for (int i = VirtKeyDef.VKB_KEY_F1; i < VirtKeyDef.VKB_KEY_KEYBOARDTOGGLE; ++i) // first st key to the last st key on the virt keyboard
			{
				VirtKeyDef vk = VirtKeyDef.kDefs[i];
				if ((vk.flags & VirtKeyDef.FLAG_REGION_REMAP) != 0)
				{
					//Log.i("hataroid", "remapping: \"" + vk.name[0] + "\" to \"" + vk.name[localeID] + "\"");
					remapIDs.clear();

					int desiredAndroidKeyID = vk.getAndroidKeyCode(localeID);

					for (int j = 0; j < vk.androidKeycode.length; ++j) {
						if (j != localeID) {
							remapIDs.add((int)vk.androidKeycode[j]);
						}
					}

					int curAndroidKeyID = -1;
					List<Integer> sysKeys = destToSrcMap.get(i);
					if (sysKeys != null)
					{
						removeIDs.clear();
						for (int skey : sysKeys) {
							if (skey == desiredAndroidKeyID) {
								curAndroidKeyID = skey;
							} else if (remapIDs.contains(skey)) {
								removeIDs.add(skey);
							}
						}
						for (int rkey : removeIDs)
						{
							removeKeyMapEntry(rkey);
							//Log.i("hataroid", "regionremap: removing \"" + AndroidKeyNames.kKeyCodeNames[rkey] + "\" for \"" + vk.name[0] + "\" (mapped key: \"" + vk.name[localeID]+ "\")");
						}
					}

					if (curAndroidKeyID != desiredAndroidKeyID) {
						if (curAndroidKeyID != -1) {
							removeKeyMapEntry(curAndroidKeyID);
							//Log.i("hataroid", "regionremap: removing \"" + AndroidKeyNames.kKeyCodeNames[curAndroidKeyID] + "\" for \"" + vk.name[0] + "\" (mapped key: \"" + vk.name[localeID]+ "\")");
						}
						if (desiredAndroidKeyID != -1) {
							addKeyMapEntry(desiredAndroidKeyID, i);
							//Log.i("hataroid", "regionremap: adding\"" + AndroidKeyNames.kKeyCodeNames[desiredAndroidKeyID] + "\" for \"" + vk.name[0] + "\" (mapped key: \"" + vk.name[localeID]+ "\")");
						}
					}
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
}
