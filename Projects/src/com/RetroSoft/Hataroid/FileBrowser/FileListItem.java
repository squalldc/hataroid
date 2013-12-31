package com.RetroSoft.Hataroid.FileBrowser;


public class FileListItem implements Comparable<FileListItem>
{
	public static final int TYPE_FILE				= 0;
	public static final int TYPE_DIR				= 1;

	private int			_type;
	private String		_path;
	private String		_name;
	
	private Boolean		_isZipFile;
	
	public FileListItem(int type, Boolean isZipFile, String path, String name)
	{
		_type = type;
		_path = path;
		_name = name;
		_isZipFile = isZipFile;
	}
	
	public int getType()				{ return _type; }
	public String getPath()				{ return _path; }
	public String getName()				{ return _name; }
	public Boolean isZipFile()			{ return _isZipFile; }

	public boolean isDir()			{ return _type == TYPE_DIR; }
	
	public int compareTo(FileListItem o)
	{
		if (_name.compareToIgnoreCase("..") == 0)			{ return -1; }
		else if (o._name.compareToIgnoreCase("..") == 0)	{ return 1; }

		if (_type != o._type)
		{
			return (_type == TYPE_DIR) ? -1 : 1;
		}
	    return _name.compareToIgnoreCase(o._name);
	}
}
