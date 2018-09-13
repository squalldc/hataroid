package com.RetroSoft.Hataroid.Input.Shortcut;

import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.InputMap;
import com.RetroSoft.Hataroid.Input.VirtKeyDef;


public class ShortcutSelectListItem implements Comparable<ShortcutSelectListItem>
{
	final int [] kFlagPriorities = {VirtKeyDef.FLAG_JOY, VirtKeyDef.FLAG_MOUSEBUTTON, VirtKeyDef.FLAG_CUSTOMKEY, VirtKeyDef.FLAG_STFNKEY, VirtKeyDef.FLAG_STKEY, -1};

	int         _localeID = Input.kLocale_EN;
	VirtKeyDef	_vkDef = null;

	public ShortcutSelectListItem(VirtKeyDef def, int localeID)
	{
		_vkDef = def;
		_localeID = localeID;
	}
	
	public String getEmuKeyName()
	{
		return (_vkDef!=null) ? _vkDef.getName(_localeID) : "";
	}
	
	public VirtKeyDef getKeyDef()
	{
		return _vkDef;
	}
	
	public int compareTo(ShortcutSelectListItem o)
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
