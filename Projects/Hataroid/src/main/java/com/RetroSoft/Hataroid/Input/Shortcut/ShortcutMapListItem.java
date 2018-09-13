package com.RetroSoft.Hataroid.Input.Shortcut;

import com.RetroSoft.Hataroid.Input.Input;
import com.RetroSoft.Hataroid.Input.InputMap;
import com.RetroSoft.Hataroid.Input.VirtKeyDef;


public class ShortcutMapListItem implements Comparable<ShortcutMapListItem>
{
	int			_anchor = -1;
	int			_shortcutIdx = -1;
	int         _localeID = Input.kLocale_EN;
	VirtKeyDef	_vkDef = null;

	public ShortcutMapListItem(VirtKeyDef def, int anchor, int shortcutIdx, int localeID)
	{
		_anchor = anchor;
		_shortcutIdx = shortcutIdx;
		_localeID = localeID;
		_vkDef = def;
	}
	
	public int getAnchor()		{ return _anchor; }
	public int getShortcutIdx()	{ return _shortcutIdx; }

	public void setVirtKeyDef(VirtKeyDef def) { _vkDef = def; }
	
	public String getEmuKeyName()
	{
		return (_vkDef!=null) ? _vkDef.getName(_localeID) : "";
	}
	
	public String getShortcutName()
	{
		if ((_anchor < 0 || _anchor >= ShortcutMap.kNumAnchors) ||  _shortcutIdx < 0)
		{
			return null;
		}
		
		return ShortcutMap.kAnchorNames[_anchor] + " Key " + String.valueOf(_shortcutIdx+1);
	}
	
	public int compareTo(ShortcutMapListItem o)
	{
		if (_anchor == o._anchor)
		{
			return (_shortcutIdx < o._shortcutIdx) ? -1 : 1;
		}
		return (_anchor < o._anchor) ? -1 : 1;
	}
}
