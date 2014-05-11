package com.RetroSoft.Hataroid.FileBrowser;


public class FileListItem implements Comparable<FileListItem>
{
	public static final int TYPE_FILE				= 0;
	public static final int TYPE_DIR				= 1;

	private int			_type;
	private String		_path;
	private String		_name;
	private String		_displayName;
	
	private boolean		_isZipFile;
	private boolean		_isDBEntry;
	
	public FileListItem(int type, boolean isZipFile, String path, String name, String displayName, boolean isDBEntry)
	{
		_type = type;
		_path = path;
		_name = name;
		_displayName = (displayName != null) ? displayName : name;
		_isZipFile = isZipFile;
		_isDBEntry = isDBEntry;
	}
	
	public int getType()				{ return _type; }
	public String getPath()				{ return _path; }
	public String getName()				{ return _name; }
	public String getDisplayName()		{ return _displayName; }
	public boolean isZipFile()			{ return _isZipFile; }
	public boolean isDBEntry()			{ return _isDBEntry; }

	public boolean isDir()			{ return _type == TYPE_DIR; }
	
	public int compareTo(FileListItem o)
	{
		if (_displayName.compareToIgnoreCase("..") == 0)			{ return -1; }
		else if (o._displayName.compareToIgnoreCase("..") == 0)		{ return 1; }

		if (_type != o._type)
		{
			return (_type == TYPE_DIR) ? -1 : 1;
		}
	    return _displayName.compareToIgnoreCase(o._displayName);
	}
}
