package com.RetroSoft.Hataroid.Input;


public class InputMapListItem implements Comparable<InputMapListItem>
{
	final int [] kFlagPriorities = {VirtKeyDef.FLAG_JOY, VirtKeyDef.FLAG_MOUSEBUTTON, VirtKeyDef.FLAG_CUSTOMKEY, VirtKeyDef.FLAG_STFNKEY, VirtKeyDef.FLAG_STKEY, -1};

	VirtKeyDef	_vkDef = null;
	int[]		_systemKeys = null;
	int         _localeID = Input.kLocale_EN;
	
	public InputMapListItem(VirtKeyDef def, int[] systemKeys, int localeID)
	{
		_systemKeys = systemKeys;
		_vkDef = def;
		_localeID = localeID;
	}
	
	public String getEmuKeyName()
	{
		return (_vkDef!=null) ? _vkDef.getName(_localeID) : "";
	}
	
	public String getSystemKeyName()
	{
		if (_systemKeys == null || _systemKeys.length == 0)
		{
			return null;
		}

		String sname = "";
		for (int i = 0; i <_systemKeys.length; ++i)
		{
			if (i != 0)
			{
				sname += ", ";
			}

			int skey = _systemKeys[i];
			if (skey < AndroidKeyNames.kKeyCodeNames.length)
			{
				sname += AndroidKeyNames.kKeyCodeNames[skey] + " ";
			}

			sname += "(" + String.valueOf(skey) + ")";
		}
		
		return sname;
	}
	
	public int compareTo(InputMapListItem o)
	{
		if (_vkDef != null && o._vkDef != null)
		{
			for (int i = 0; i < kFlagPriorities.length; ++i)
			{
				int curFlag = kFlagPriorities[i];
				boolean flag1Set = ((_vkDef.flags&curFlag)!= 0);
				boolean flag2Set = ((o._vkDef.flags&curFlag)!= 0);

				if (flag1Set && !flag2Set)		{ return -1; }
				else if (!flag1Set && flag2Set)	{ return 1; }
				else if (flag1Set && flag2Set)
				{
					if (_vkDef.sort < o._vkDef.sort)		{ return -1; }
					else if (_vkDef.sort > o._vkDef.sort)	{ return 1; }
					
					int def1Len = _vkDef.getName(_localeID).length();
					int def2Len = o._vkDef.getName(_localeID).length();

					if (def1Len == 1 && def2Len > 1)		{ return -1; }
					else if (def1Len > 1 && def2Len == 1)	{ return 1; }

					return _vkDef.getName(_localeID).compareTo(o._vkDef.getName(_localeID));
				}
			}
		}
		return 0;
	}
}
