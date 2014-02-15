package com.RetroSoft.Hataroid.Input;

import java.util.HashMap;
import java.util.Map;

import android.annotation.SuppressLint;

@SuppressLint("UseSparseArrays")
public class InputMap
{
	public int [] srcToDestMap = null;
	public Map<Integer, Integer> destToSrcMap = null;
	public int numDestInputs = 0;

	public InputMap() { }
	
	public void clear()
	{
		for (int i = 0; i < srcToDestMap.length; ++i)
		{
			srcToDestMap[i] = -1;
		}
		destToSrcMap.clear();
	}
	
	public void addDefaults()
	{
		for (int i = 0; i < VirtKeyDef.kDefs.length; ++i)
		{
			VirtKeyDef vk = VirtKeyDef.kDefs[i];
			addKeyMapEntry((int)vk.androidKeycode, vk.id);
		}
	}
	
	public void init(int numSrcInputs_)
	{
		srcToDestMap = new int [numSrcInputs_];
		destToSrcMap = new HashMap<Integer, Integer>();
		numDestInputs = VirtKeyDef.VKB_KEY_NumOf;

		// reset
		clear();

		// default map
		addDefaults();
	}
	
	public void initFromArray(int numSrcInputs_, int [] inputMap)
	{
		init(numSrcInputs_);

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
				destToSrcMap.remove(removeKey);
			}
		}

		srcToDestMap[srcKey] = -1;

		return true;
	}

	public boolean addKeyMapEntry(int srcKey, int destKey)
	{
		if (srcKey < 0 || srcKey >= srcToDestMap.length || destKey < 0 || destKey >= numDestInputs)
		{
			return false;
		}

		if (destToSrcMap.containsKey(destKey))
		{
			int androidKeyCode = destToSrcMap.get(destKey);
			srcToDestMap[androidKeyCode] = -1;
		}
		
		if (srcToDestMap[srcKey] >= 0)
		{
			int removeKey = srcToDestMap[srcKey];
			if (destToSrcMap.containsKey(removeKey))
			{
				destToSrcMap.remove(removeKey);
			}
		}

		srcToDestMap[srcKey] = destKey;
		destToSrcMap.put(destKey, srcKey);
		
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
}
