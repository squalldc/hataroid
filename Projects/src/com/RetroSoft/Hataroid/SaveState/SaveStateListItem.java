package com.RetroSoft.Hataroid.SaveState;

public class SaveStateListItem implements Comparable<SaveStateListItem>
{
	//private String		_basePath;
	private String		_path;
	private String		_name;
	private boolean		_dummyItem;
	private long		_lastModified;
	
	private int			_slotID;
	private String		_saveName;
	
	private boolean		_sortFirstItem = false;
	private boolean		_isQuickSaveSlot = false;
	
	public SaveStateListItem(String path, String name, long lastModified, boolean dummyItem, int firstSlotID, int quickSaveSlotID)
	{
		int extPos;
		
		//extPos = path.lastIndexOf(".");
		//_basePath = (extPos >= 0) ? path.substring(0, extPos+1) : path;
		_path = path;
		_name = name;
		_lastModified = lastModified;
		_dummyItem = dummyItem;
		
		_slotID = -1;
		
		extPos = name.lastIndexOf(".");
		_saveName = (extPos > 0) ? name.substring(0, extPos) : name;
		
		try
		{
			int saveNameLen = _saveName.length();
			if (saveNameLen > 4)
			{
				String slotStr = _saveName.substring(0, 3);
				int curSlotID = Integer.parseInt(slotStr);
				String curSaveName = _saveName.substring(4, saveNameLen);
				
				_slotID = curSlotID;
				_saveName = curSaveName;
			}
		}
		catch (Exception e)
		{
		}

		_sortFirstItem = (_slotID == firstSlotID);
		_isQuickSaveSlot = !_dummyItem && (_slotID == quickSaveSlotID);
	}

	public String getPath()					{ return _path; }
	//public String getBasePath()			{ return _basePath; }
	public String getName()					{ return _name; }
	public long getLastModified()			{ return _lastModified; }
	public boolean isDummyItem()			{ return _dummyItem; }
	public boolean isFirstSlotItem()		{ return _sortFirstItem; }
	public boolean isQuickSaveSlotItem()	{ return _isQuickSaveSlot; }
	
	public int getSlotID()					{ return _slotID; }
	public String getSaveName()				{ return _saveName; }

	public int compareTo(SaveStateListItem o)
	{
		if (_dummyItem && !o._dummyItem)				{ return -1; }
		else if (!_dummyItem && o._dummyItem)			{ return 1; }
		
		if (_sortFirstItem && !o._sortFirstItem)		{ return -1; }
		else if (!_sortFirstItem && o._sortFirstItem)	{ return 1; }
		
		if (_lastModified > o._lastModified)			{ return -1; }
		else if (_lastModified < o._lastModified)		{ return 1; }
		
	    return _name.compareToIgnoreCase(o._name);
	}
}
