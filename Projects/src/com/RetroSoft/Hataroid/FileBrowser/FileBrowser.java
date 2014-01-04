package com.RetroSoft.Hataroid.FileBrowser;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class FileBrowser extends ListActivity
{
	public static final String CONFIG_OPENZIPS = "Config_OpenZips";
	public static final String CONFIG_EXT = "Config_Ext";
	public static final String CONFIG_RESETST = "Config_ResetST";
	
	public static final String RESULT_PATH = "ResultPath";
	public static final String RESULT_ZIPPATH = "ResultZipPath";
	public static final String RESULT_RESETCOLD = "ResetCold";
	public static final String RESULT_RESETWARM = "ResetWarm";

	private static final String LastFloppyDirItemPathKey = "pref_storage_floppydisks_lastdir_itempath";
	private static final String LastFloppyDirItemNameKey = "pref_storage_floppydisks_lastdir_itemname";

	private static boolean		_firstCreate = true;

	private static String		_savedPath = null;
	private static int			_prevFirstVisibleItem = 0;

	private File				_curDir;
	private ZipFile				_curZipFile;

	private FileArrayAdapter	_adapter;

	private boolean				_openZips = true;
	private boolean				_resetST = true;

	private String []			_exts = null;
	private String				_root;

	private Intent				_retIntent;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_file_browser_view);
		
		parseOptions(savedInstanceState);

		_retIntent = new Intent();
		
		String lastItemName = null;
		if (_firstCreate)
		{
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
	    	_savedPath = prefs.getString(LastFloppyDirItemPathKey, "");
	    	if (_savedPath.length() == 0)
	    	{
	    		_savedPath = null;
	    	}
	    	lastItemName = prefs.getString(LastFloppyDirItemNameKey, "");
	    	if (lastItemName.length() == 0)
	    	{
	    		lastItemName = null;
	    	}
		}

		String [] rootPaths = { Environment.getExternalStorageDirectory().getPath() }; //"/sdcard/STDroid", "/sdcard", "/emmc", "/" };
		for (int i = 0 ; i < rootPaths.length; ++i)
		{
			File rootDir = new File(rootPaths[i]);
			if (rootDir.exists() && rootDir.canRead())
			{
				_root = rootDir.getAbsolutePath();
				break;
			}
		}
		
		if (_savedPath != null)
		{
			_curDir = new File(_savedPath);
		}

		if (_curDir != null && _curDir.isFile())
		{
			_curZipFile = _openZipFileList(_curDir.getPath());
			if (_curZipFile != null)
			{
				retrieveZipFileList(_curZipFile, _exts);
			}
			else
			{
				_savedPath = null;
				_prevFirstVisibleItem = 0;
			}
			_curDir = null;
		}
		if (_curZipFile == null)
		{
			if (_curDir == null || !_curDir.exists())
			{
				_curDir = new File(_root);
				_savedPath = null;
				_prevFirstVisibleItem = 0;
			}
			retrieveFileList(_curDir, _exts);
		}
		
		if (lastItemName != null)
		{
			ListView v = getListView();
			for (int i = 0; i < v.getCount(); ++i)
			{
				FileListItem item = (FileListItem)v.getItemAtPosition(i);
				if (item.getName().compareTo(lastItemName)==0)
				{
					_prevFirstVisibleItem = i;
					break;
				}
			}
		}
	}
	
	void parseOptions(Bundle savedInstanceState)
	{
		_exts = new String [] {".st", ".msa", ".dim", ".stx"};
		_openZips = true;
		_resetST = true;

		Bundle b = (savedInstanceState == null) ? getIntent().getExtras() : savedInstanceState;
		if (b != null)
		{
			String [] exts = b.getStringArray(CONFIG_EXT);
			if (exts != null)
			{
				_exts = exts;
			}
			
			_openZips = b.getBoolean(CONFIG_OPENZIPS, true);
			_resetST = b.getBoolean(CONFIG_RESETST, true);
		}
		
		if (_openZips)
		{
			int curLen = _exts.length;
			String [] curExts = _exts;
			_exts = new String [curLen + 1];
			for (int i = 0; i < curLen; ++i)
			{
				_exts[i] = curExts[i];
			}
			_exts[curLen] = ".zip";
		}
		
		setupButtonListeners();
	}
	
	@Override protected void onDestroy()
	{
		_closeZipFile(_curZipFile);
		super.onDestroy();
	}

	@Override protected void onPause()
	{
		ListView v = getListView();
		_prevFirstVisibleItem = v.getFirstVisiblePosition();

		FileListItem item = (FileListItem)v.getItemAtPosition(_prevFirstVisibleItem);
		String itemname = item.getName();
    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		Editor ed = prefs.edit();
		ed.putString(LastFloppyDirItemPathKey, (_savedPath == null) ? "" : _savedPath);
		ed.putString(LastFloppyDirItemNameKey, (itemname == null) ? "" : itemname);
		ed.commit();

		super.onPause();
	}
	
	@Override protected void onResume()
	{
		ListView v = getListView();
		v.setFastScrollEnabled(true);
		if (_prevFirstVisibleItem >= 0 && _prevFirstVisibleItem < v.getCount())
		{
			v.setSelection(_prevFirstVisibleItem);
		}

		super.onResume();
	}

	@Override public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_BACK:
			{
				String scrollItemName = _getCurPathItem(_savedPath);

				if (_curZipFile!=null)
				{
					// exiting zip file
					_closeZipFile(_curZipFile);
					_curZipFile = null;

					String parentPath = _getParentPathName(_savedPath);
					_curDir = new File(parentPath);
				}
				else if (_curDir != null)
				{
					String root = "/";//_root;
					if (root.compareToIgnoreCase(_curDir.getAbsolutePath()) == 0)
					{
						sendFinish(RESULT_CANCELED);
						return false;
					}
					else if (_curDir.getParent() != null)
					{
						_curDir = new File(_curDir.getParent());
					}
				}

				retrieveFileList(_curDir, _exts);
				_scrollToItemName(scrollItemName);
				return false;
			}
		}

		return super.onKeyDown(keyCode, event);
	}
	
	void setupButtonListeners()
	{
		findViewById(R.id.fb_closeBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				sendFinish(RESULT_CANCELED);
			}
		});
	}
	
	/*private String _getFileName(String fullname)
	{
		int s = fullname.lastIndexOf('/');
		int s2 = fullname.lastIndexOf('\\');

		int beg = (s > s2) ? (s+1) : (s2+1);
		return fullname.substring(beg);
	}
	*/
	
	private String _getParentPathName(String fullname)
	{
		if (fullname.length() == 0)
		{
			return null;
		}

		int end = fullname.length()-1;
		if (fullname.endsWith("\\") || fullname.endsWith("/")) { --end; }

		int e = fullname.lastIndexOf('\\', end);
		int e2 = fullname.lastIndexOf('/', end);
		
		if (e >=0 || e2 >= 0)
		{
			end = (e > e2) ? (e) : (e2);
		}
		
		if (end <= 1)
		{
			return null;
		}
		return fullname.substring(0, end);
	}

	private String _getCurPathItem(String fullname)
	{
		if (fullname.length() == 0)
		{
			return null;
		}

		int end = fullname.length()-1;
		if (fullname.endsWith("\\") || fullname.endsWith("/")) { --end; }

		int s = fullname.lastIndexOf('\\', end);
		int s2 = fullname.lastIndexOf('/', end);
		
		int beg = 0;
		if (s >=0 || s2 >= 0)
		{
			beg = (s > s2) ? (s+1) : (s2+1);
		}
		
		if (beg >= end)
		{
			return null;
		}
		return fullname.substring(beg, end+1);
	}

	private void _scrollToItemName(String itemName)
	{
		if (_adapter != null && itemName != null)
		{
			int numItems = _adapter.getCount();
			for (int i = 0; i < numItems; ++i)
			{
				FileListItem item = _adapter.getItem(i);
				if (itemName.compareTo(item.getName()) == 0)
				{
					getListView().setSelection(i);
					break;
				}
			}
		}
	}

	@Override protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l,  v, position, id);
		
		FileListItem item = _adapter.getItem(position);
		switch (item.getType())
		{
			case FileListItem.TYPE_DIR:
			{
				String scrollItemName = _getCurPathItem(_savedPath);

				if ((item.getName().compareTo("..")==0))
				{
					if (_curZipFile!=null)
					{
						// exiting zip file
						_closeZipFile(_curZipFile);
						_curZipFile = null;
					}
				}
								
				_curDir = new File(item.getPath());
				retrieveFileList(_curDir, _exts);

				_scrollToItemName(scrollItemName);
				break;
			}
			case FileListItem.TYPE_FILE:
			{
				String pathLower = item.getPath().toLowerCase();
				if (pathLower.endsWith(".zip"))
				{
					// don't support recursive zip files for now
					if (_curZipFile == null)
					{
						_curZipFile = _openZipFileList(item.getPath());
						if (_curZipFile != null)
						{
							if (!_isSingleDiscZip(_curZipFile, _exts))
							{
								retrieveZipFileList(_curZipFile, _exts);
								_curDir = null;
								return;
							}
						}
					}
				}

				onFileClicked(item);
				break;
			}
		}
	}
	
	private Boolean _isSingleDiscZip(ZipFile zipFile, String [] validExts)
	{
		int validDiscCount = 0;

		Enumeration<? extends ZipEntry> e = zipFile.entries();
		while (e.hasMoreElements())
		{
			ZipEntry f = e.nextElement();

			if (!f.isDirectory())
			{
				for (String ext : validExts)
				{
					String flc = f.getName().toLowerCase();
					if (!flc.endsWith(".zip") && flc.endsWith(ext))
					{
						++validDiscCount;
						if (validDiscCount > 1)
						{
							return false;
						}
						break;
					}
				}
			}
		}
		
		return true;
	}
	
	private ZipFile _openZipFileList(String pathname)
	{
		if (!_openZips)
		{
			return null;
		}

		ZipFile zf;
		try
		{
			zf = new ZipFile(pathname);
		}
		catch (IOException e)
		{
			e.printStackTrace();
			zf = null;
		}

		return zf;
	}
	
	private void _closeZipFile(ZipFile zf)
	{
		if (zf == null) return;
		
		try
		{
			_curZipFile.close();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
	}
	
	private void onFileClicked(FileListItem item)
	{
		String itemPath = item.getPath();

		_retIntent.putExtra(RESULT_PATH, itemPath);
		if (item.isZipFile())
		{
			String zipPath = item.getName();
			_retIntent.putExtra(RESULT_ZIPPATH, zipPath);
		}
		
		if (!_resetST)
		{
			sendFinish(RESULT_OK);
		}
		else
		{
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Reset ST?");
			alertDialog.setMessage("Do you want to the Reset the ST?");
			
			alertDialog.setButton("No",
					new DialogInterface.OnClickListener()
					{
						public void onClick(DialogInterface dialog, int which)
						{
							sendFinish(RESULT_OK);
						}
					}
				);
				
			alertDialog.setButton2("Yes (Cold)",
				new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int which)
					{
						_retIntent.putExtra(RESULT_RESETCOLD, true);
						sendFinish(RESULT_OK);
					}
				}
			);
	
			alertDialog.setButton3("Yes (Warm)",
				new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int which)
					{
						_retIntent.putExtra(RESULT_RESETWARM, true);
						sendFinish(RESULT_OK);
					}
				}
			);
	
			alertDialog.show();
		}
	}
	
	private void sendFinish(int resultCode)
	{
		setResult(resultCode, _retIntent);
		finish();
	}

	private void retrieveFileList(File dir, String [] validExts)
	{
//		setTitle("Current path: " + dir.getAbsolutePath());
		setCurPathText("Current path: " + dir.getAbsolutePath());

		_savedPath = dir.getAbsolutePath();
	
		String [] validExtsLower = null;
		if (validExts != null)
		{
			validExtsLower = new String [validExts.length];
			for (int i = 0; i < validExts.length; ++i)
			{
				validExtsLower[i] = validExts[i].toLowerCase();
			}
		}

		List<FileListItem> items = new ArrayList<FileListItem>();
		File [] curFiles = dir.listFiles();
		
		try
		{
			for (File f : curFiles)
			{
				if (f.isDirectory())
				{
					items.add(new FileListItem(FileListItem.TYPE_DIR, false, f.getAbsolutePath(), f.getName()));
				}
				else
				{
					boolean valid = true;
					if (validExtsLower != null)
					{
						valid = false;
						for (String ext : validExtsLower)
						{
							if (f.getName().toLowerCase().endsWith(ext))
							{
								valid = true;
								break;
							}
						}
					}
					if (valid)
					{
						items.add(new FileListItem(FileListItem.TYPE_FILE, false, f.getAbsolutePath(), f.getName()));
					}
				}
			}
			
			if (dir.getParent() != null)
			{
				items.add(new FileListItem(FileListItem.TYPE_DIR, false, dir.getParent(), ".."));
			}
		}
		catch (Exception e)
		{
		}
		
		Collections.sort(items);
		
		_adapter = new FileArrayAdapter(this, R.layout.activity_file_browser, items);
		setListAdapter(_adapter);
	}

	private void retrieveZipFileList(ZipFile zipFile, String [] validExts)
	{
//		setTitle("Current path: " + zipFile.getName());
		setCurPathText("Current path: " + zipFile.getName());

		_savedPath = zipFile.getName();
	
		String [] validExtsLower = null;
		if (validExts != null)
		{
			validExtsLower = new String [validExts.length];
			for (int i = 0; i < validExts.length; ++i)
			{
				validExtsLower[i] = validExts[i].toLowerCase();
			}
		}

		List<FileListItem> items = new ArrayList<FileListItem>();
		
		Enumeration<? extends ZipEntry> e = zipFile.entries();
		while (e.hasMoreElements())
		{
			ZipEntry f = e.nextElement();

			if (f.isDirectory())
			{
				// no need to add folders in a zip file
				//items.add(new FileListItem(FileListItem.TYPE_DIR, true, zipFile.getName(), f.getName()));
			}
			else
			{
				boolean valid = true;
				if (validExtsLower != null)
				{
					valid = false;
					for (String ext : validExtsLower)
					{
						if (f.getName().toLowerCase().endsWith(ext))
						{
							valid = true;
							break;
						}
					}
				}
				if (valid)
				{
					items.add(new FileListItem(FileListItem.TYPE_FILE, true, zipFile.getName(), f.getName()));
				}
			}
		}
		
		String parentDir = _getParentPathName(zipFile.getName());
		if (parentDir != null)
		{
			items.add(new FileListItem(FileListItem.TYPE_DIR, false, parentDir, ".."));
		}
		
		Collections.sort(items);
		
		_adapter = new FileArrayAdapter(this, R.layout.activity_file_browser, items);
		setListAdapter(_adapter);
	}
	
	void setCurPathText(String s)
	{
		View v = findViewById(R.id.fb_curPathText);
		if (v != null && v instanceof TextView)
		{
			TextView tv = (TextView)v;
			tv.setText(s);
		}
	}

}
