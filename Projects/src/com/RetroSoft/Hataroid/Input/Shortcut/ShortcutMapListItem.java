package com.RetroSoft.Hataroid.Input.Shortcut;

import com.RetroSoft.Hataroid.Input.VirtKeyDef;


public class ShortcutMapListItem implements Comparable<ShortcutMapListItem>
{
	int			_anchor = -1;
	int			_shortcutIdx = -1;
	VirtKeyDef	_vkDef = null;

	public ShortcutMapListItem(VirtKeyDef def, int anchor, int shortcutIdx)
	{
		_anchor = anchor;
		_shortcutIdx = shortcutIdx;
		_vkDef = def;
	}
	
	public int getAnchor()		{ return _anchor; }
	public int getShortcutIdx()	{ return _shortcutIdx; }

	public void setVirtKeyDef(VirtKeyDef def) { _vkDef = def; }
	
	public String getEmuKeyName()
	{
		return (_vkDef!=null) ? _vkDef.name : "";
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
