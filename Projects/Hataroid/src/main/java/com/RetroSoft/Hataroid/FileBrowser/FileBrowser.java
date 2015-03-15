package com.RetroSoft.Hataroid.FileBrowser;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.app.Activity;
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
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.GameDB.GameDBFile;
import com.RetroSoft.Hataroid.GameDB.GameDBHelper;
import com.RetroSoft.Hataroid.GameDB.IGameDBScanner;
import com.RetroSoft.Hataroid.Input.RenameInputMapView;

public class FileBrowser extends ListActivity implements IGameDBScanner
{
	public static final String CONFIG_OPENZIPS = "Config_OpenZips";
	public static final String CONFIG_EXT = "Config_Ext";
	public static final String CONFIG_RESETST = "Config_ResetST";
	public static final String CONFIG_REFRESHDB = "Config_RefreshDB";
	public static final String CONFIG_SELECTFOLDER = "Config_SelectFolder";
	public static final String CONFIG_NEWFOLDER = "Config_NewFolder";
	public static final String CONFIG_PREFLASTITEMPATH = "Config_PrefLastItemPath";
	public static final String CONFIG_PREFLASTITEMNAME = "Config_PrefLastItemName";
	
	public static final String RESULT_PATH = "ResultPath";
	public static final String RESULT_ZIPPATH = "ResultZipPath";
	public static final String RESULT_DISPLAYNAME = "ResultDisplayName";
	public static final String RESULT_RESETCOLD = "ResetCold";
	public static final String RESULT_RESETWARM = "ResetWarm";

	private static final int	ACTIVITYRESULT_NEWFOLDERNAME				= 1;

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
	private boolean				_refreshDB = false;
	private boolean				_selectFolder = false;
	private boolean				_newFolder = false;
	private String				_prefLastItemPathKey = LastFloppyDirItemPathKey;
	private String				_prefLastItemNameKey = LastFloppyDirItemNameKey;
	
	private SelectFolderClickListener	_selectFolderListener = null;

	private String []			_exts = null;
	private String				_root = "/";

	private Intent				_retIntent;
	
	private View				_refreshDBBtnView = null;
	private View				_clearDBBtnView = null;

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_file_browser_view);
		
		parseOptions(savedInstanceState);
		setupButtonListeners();

		_retIntent = new Intent();
		
		String lastItemName = null;
		if (_firstCreate)
		{
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
	    	_savedPath = prefs.getString(_prefLastItemPathKey, "");
	    	if (_savedPath.length() == 0)
	    	{
	    		_savedPath = null;
	    	}
	    	lastItemName = prefs.getString(_prefLastItemNameKey, "");
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
			_setCurDir(_savedPath);
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
			_setCurDir(null);
		}
		if (_curZipFile == null)
		{
			if (_curDir == null || !_curDir.exists())
			{
				_setCurDir(_root);
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
				if (item.getDisplayName().compareTo(lastItemName)==0)
				{
					_prevFirstVisibleItem = i;
					break;
				}
			}
		}

		HataroidActivity ha = HataroidActivity.instance;
		if (ha != null)
		{
			ha.setGameDBScannerInterface(this);
		}
	}
	
	@Override protected void onSaveInstanceState(Bundle outState)
	{
		try
		{
			if (_exts != null)
			{
				int numExts = 0;
				for (int i = 0; i < _exts.length; ++i)
				{
					numExts += (_exts[i].compareTo(".zip") != 0) ? 1 : 0;
				}
				if (numExts > 0)
				{
					String [] exts = new String [numExts];
					for (int i = 0, e = 0; i < _exts.length; ++i)
					{
						if (_exts[i].compareTo(".zip") != 0)
						{
							exts[e] = _exts[i];
							++e;
						}
					}
					outState.putStringArray(CONFIG_EXT, exts);
				}
			}
			
			outState.putBoolean(CONFIG_OPENZIPS, _openZips);
			outState.putBoolean(CONFIG_RESETST, _resetST);
			outState.putBoolean(CONFIG_REFRESHDB, _refreshDB);
			outState.putBoolean(CONFIG_SELECTFOLDER, _selectFolder);
			outState.putBoolean(CONFIG_NEWFOLDER, _newFolder);
		}
		catch (Exception e)
		{
			e.printStackTrace();
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
			_refreshDB = b.getBoolean(CONFIG_REFRESHDB, false);
			_selectFolder = b.getBoolean(CONFIG_SELECTFOLDER, false);
			_newFolder = b.getBoolean(CONFIG_NEWFOLDER, false);
			
			_prefLastItemPathKey = b.getString(CONFIG_PREFLASTITEMPATH);
			_prefLastItemNameKey = b.getString(CONFIG_PREFLASTITEMNAME);
		}

		if (_prefLastItemPathKey == null) { _prefLastItemPathKey = LastFloppyDirItemPathKey; }
		if (_prefLastItemNameKey == null) { _prefLastItemNameKey = LastFloppyDirItemNameKey; }
		
		if (_openZips && !_allowAllFiles())
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
	}
	
	@Override protected void onDestroy()
	{
		HataroidActivity ha = HataroidActivity.instance;
		if (ha != null)
		{
			ha.clearGameDBScannerInterface();
		}

		if (_selectFolderListener != null)
		{
			_selectFolderListener.deinit();
		}
			
		_closeZipFile(_curZipFile);
		_curZipFile = null;

		super.onDestroy();
	}

	@Override protected void onPause()
	{
		ListView v = getListView();
		_prevFirstVisibleItem = v.getFirstVisiblePosition();

		FileListItem item = (FileListItem)v.getItemAtPosition(_prevFirstVisibleItem);
		if (item != null && _prefLastItemPathKey != null && _prefLastItemNameKey != null)
		{
			String itemname = item.getDisplayName();
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			Editor ed = prefs.edit();
			ed.putString(_prefLastItemPathKey, (_savedPath == null) ? "" : _savedPath);
			ed.putString(_prefLastItemNameKey, (itemname == null) ? "" : itemname);
			ed.commit();
		}

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
					_setCurDir(parentPath);
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
						_setCurDir(_curDir.getParent());
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

		try
		{
			int enabledButtonCount = 0;
			
			View selectBtnView = findViewById(R.id.fb_selectBtn);
			if (selectBtnView != null)
			{
				if (_selectFolder)
				{
					if (_selectFolderListener == null)
					{
						_selectFolderListener = new SelectFolderClickListener(this);
						selectBtnView.setOnClickListener(_selectFolderListener);
					}
					++enabledButtonCount;
				}
				else
				{
					selectBtnView.setEnabled(false);
					selectBtnView.setVisibility(View.INVISIBLE);
					((ViewGroup)selectBtnView.getParent()).removeView(selectBtnView);
				}
			}

			View newFolderBtnView = findViewById(R.id.fb_newFolderBtn);
			if (newFolderBtnView != null)
			{
				if (_newFolder)
				{
					newFolderBtnView.setOnClickListener(new OnClickListener() { public void onClick(View v) { _onNewFolderBtnClicked(); }});
					++enabledButtonCount;
				}
				else
				{
					newFolderBtnView.setEnabled(false);
					newFolderBtnView.setVisibility(View.INVISIBLE);
					((ViewGroup)newFolderBtnView.getParent()).removeView(newFolderBtnView);
				}
			}
			
			_refreshDBBtnView = null;
			_clearDBBtnView = null;
			View refreshDBBtnView = findViewById(R.id.fb_refreshDBBtn);
			View clearDBBtnView = findViewById(R.id.fb_clearDBBtn);
			if (refreshDBBtnView != null)
			{
				if (_refreshDB)
				{
					refreshDBBtnView.setOnClickListener(new OnClickListener() { public void onClick(View v) { _onRefreshDBClicked(); }});
					_refreshDBBtnView = refreshDBBtnView;

					if (clearDBBtnView != null)
					{
						clearDBBtnView.setOnClickListener(new OnClickListener() { public void onClick(View v) { _onClearDBClicked(); }});
						_clearDBBtnView = clearDBBtnView;
					}

					++enabledButtonCount;
				}
				else
				{
					refreshDBBtnView.setEnabled(false);
					refreshDBBtnView.setVisibility(View.INVISIBLE);
					((ViewGroup)refreshDBBtnView.getParent()).removeView(refreshDBBtnView);

					if (clearDBBtnView != null)
					{
						clearDBBtnView.setEnabled(false);
						clearDBBtnView.setVisibility(View.INVISIBLE);
						((ViewGroup)clearDBBtnView.getParent()).removeView(clearDBBtnView);
					}
				}
			}

			if (enabledButtonCount == 0)
			{
				View buttonLayoutView = findViewById(R.id.bottomButtonsLayout);
				if (buttonLayoutView != null)
				{
					((ViewGroup)buttonLayoutView.getParent()).removeView(buttonLayoutView);
				}
			}

			ListView fileListView = (ListView)findViewById(android.R.id.list);
			if (fileListView != null)
			{
				Object lpObj = fileListView.getLayoutParams();
				if (lpObj instanceof LinearLayout.LayoutParams)
				{
					LinearLayout.LayoutParams lp = (LinearLayout.LayoutParams)lpObj;
					lp.weight = (_selectFolder || _newFolder) ? 0.9f : 0.95f;
					fileListView.setLayoutParams(lp);
					View root = fileListView.getRootView();
					if (root != null)
					{
						root.requestLayout();
					}
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	boolean _allowAllFiles()
	{
		boolean allFiles = false;
		if (_exts!=null && _exts.length == 1 && _exts[0].compareTo("*")==0) // HACK: should support rexp instead
		{
			allFiles = true;
		}
		return allFiles;
	}

	
	
	/*private String _getFileName(String fullname)
	{
		int s = fullname.lastIndexOf('/');
		int s2 = fullname.lastIndexOf('\\');

		int beg = (s > s2) ? (s+1) : (s2+1);
		return fullname.substring(beg);
	}
	*/
	
	public static String _getParentPathName(String fullname)
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

				_setCurDir(item.getPath());
				retrieveFileList(_curDir, _exts);

				_scrollToItemName(scrollItemName);
				break;
			}
			case FileListItem.TYPE_FILE:
			{
				boolean isPastiFile = false;

				String pathLower = item.getPath().toLowerCase();
				String nameLower = item.getName().toLowerCase();
				if (pathLower.endsWith(".zip"))
				{
					// don't support recursive zip files for now
					if (!item.isDBEntry() && _curZipFile == null)
					{
						ZipFile zf = _openZipFileList(item.getPath());
						if (zf != null)
						{
							boolean [] isPastiResult = new boolean [1];
							if (!_isSingleDiscZip(zf, _exts, isPastiResult))
							{
								_curZipFile = zf;
								retrieveZipFileList(_curZipFile, _exts);
								_setCurDir(null);
								return;
							}
							else
							{
								if (isPastiResult[0])
								{
									isPastiFile = true;
								}

								_closeZipFile(zf);
								zf = null;
							}
						}
					}
					else if (nameLower.endsWith(".stx"))
					{
						isPastiFile = true;
					}
				}
				else if (nameLower.endsWith(".stx"))
				{
					isPastiFile = true;
				}
				
				if (isPastiFile)
				{
					_showPastiAlert();
				}
				else
				{
					onFileClicked(item);
				}
				break;
			}
		}
	}
	
	void _showPastiAlert()
	{
		AlertDialog alertDialog = new AlertDialog.Builder(this).create();
		alertDialog.setTitle("Pasti (.stx) is not supported");
		alertDialog.setMessage("Pasti support is currently being worked on");
		alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener() { public void onClick(DialogInterface dialog, int which) { } });
		alertDialog.show();
	}
	
	private boolean _isSingleDiscZip(ZipFile zipFile, String [] validExts, boolean [] isPasti)
	{
		int validDiscCount = 0;

		isPasti[0] = false;

		Enumeration<? extends ZipEntry> e = zipFile.entries();
		while (e.hasMoreElements())
		{
			ZipEntry f = e.nextElement();

			if (!f.isDirectory())
			{
				String flc = f.getName().toLowerCase();
				if (!flc.endsWith(".zip"))
				{
					if (flc.endsWith(".stx"))
					{
						isPasti[0] = true;
					}

					if (_allowAllFiles()) // HACK: should allow rexp
					{
						++validDiscCount;
						if (validDiscCount > 1) { return false; }
					}
					else
					{
						for (String ext : validExts)
						{
							if (flc.endsWith(ext))
							{
								++validDiscCount;
								if (validDiscCount > 1) { return false; }
								break;
							}
						}
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
	
	public static void _closeZipFile(ZipFile zf)
	{
		if (zf == null) return;
		
		try
		{
			zf.close();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
	}
	
	public void onSelectButtonClicked()
	{
		if (_selectFolder)
		{
			String selectedPath = null;
			if (_curDir != null)
			{
				if (_curDir.isDirectory())
				{
					selectedPath = _curDir.getPath();
				}
			}
	
			if (selectedPath != null)
			{
				_retIntent.putExtra(RESULT_PATH, selectedPath);
				sendFinish(RESULT_OK);
			}
		}
	}

	private void onFileClicked(FileListItem item)
	{
		if (_selectFolder)
		{
			return;
		}

		String itemPath = item.getPath();

		_retIntent.putExtra(RESULT_PATH, itemPath);
		if (item.isZipFile())
		{
			String zipPath = item.getName();
			_retIntent.putExtra(RESULT_ZIPPATH, zipPath);
		}
		_retIntent.putExtra(RESULT_DISPLAYNAME, item.getDisplayName());
		
		if (!_resetST)
		{
			sendFinish(RESULT_OK);
		}
		else
		{
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Reset ST?");
			alertDialog.setMessage("Do you want to the Reset the ST?");
			
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "No",
					new DialogInterface.OnClickListener()
					{
						public void onClick(DialogInterface dialog, int which)
						{
							sendFinish(RESULT_OK);
						}
					}
				);
				
			alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Yes (Cold)",
				new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int which)
					{
						_retIntent.putExtra(RESULT_RESETCOLD, true);
						sendFinish(RESULT_OK);
					}
				}
			);
	
			alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "Yes (Warm)",
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
		
		GameDBHelper gameDB = null;
		Map<String,GameDBFile> dbFiles = null;
		HataroidActivity ha = HataroidActivity.instance;
		if (ha != null)
		{
			gameDB = ha.getGameDB();
			dbFiles = gameDB.getFiles(dir.getAbsolutePath());
		}

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
					items.add(new FileListItem(FileListItem.TYPE_DIR, false, f.getAbsolutePath(), f.getName(), null, false));
				}
				else
				{
					boolean showNormal = true;
					
					GameDBFile dbf = dbFiles.get(f.getName());
					if (dbf != null)
					{
						showNormal = false;
						
						if (!dbf.isMultiZip)
						{
							for (int i = 0; i < dbf.gameNames.length; ++i)
							{
								items.add(new FileListItem(FileListItem.TYPE_FILE, false, f.getAbsolutePath(), f.getName(), dbf.gameNames[i], true));
							}
						}
						else if (dbf.zipComplete)
						{
							Map<String,GameDBFile> dbZipFiles = gameDB.getFiles(f.getAbsolutePath()); // gameDB is valid at this point
							Iterator<Map.Entry<String, GameDBFile>> entries = dbZipFiles.entrySet().iterator();
							while (entries.hasNext())
							{
							    Map.Entry<String, GameDBFile> entry = entries.next();
							    GameDBFile zipEntry = entry.getValue();
								for (int i = 0; i < zipEntry.gameNames.length; ++i)
								{
									items.add(new FileListItem(FileListItem.TYPE_FILE, true, f.getAbsolutePath(), zipEntry.fileName, zipEntry.gameNames[i], true));
								}
							}
						}
						else
						{
							showNormal = true;
						}
					}
					
					if (showNormal)
					{
						boolean valid = true;
						if (!_allowAllFiles() && validExtsLower != null)
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
							items.add(new FileListItem(FileListItem.TYPE_FILE, false, f.getAbsolutePath(), f.getName(), null, false));
						}
					}
				}
			}
			
			if (dir.getParent() != null)
			{
				items.add(new FileListItem(FileListItem.TYPE_DIR, false, dir.getParent(), "..", null, false));
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

		GameDBHelper gameDB = null;
		Map<String,GameDBFile> dbFiles = null;
		HataroidActivity ha = HataroidActivity.instance;
		if (ha != null)
		{
			gameDB = ha.getGameDB();
			dbFiles = gameDB.getFiles(zipFile.getName());
		}

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
				boolean showNormal = true;
				
				GameDBFile dbf = dbFiles.get(f.getName());
				if (dbf != null)
				{
					showNormal = false;
					
					for (int i = 0; i < dbf.gameNames.length; ++i)
					{
						items.add(new FileListItem(FileListItem.TYPE_FILE, true, zipFile.getName(), f.getName(), dbf.gameNames[i], true));
					}
				}
				
				if (showNormal)
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
						items.add(new FileListItem(FileListItem.TYPE_FILE, true, zipFile.getName(), f.getName(), null, false));
					}
				}
			}
		}
		
		String parentDir = _getParentPathName(zipFile.getName());
		if (parentDir != null)
		{
			items.add(new FileListItem(FileListItem.TYPE_DIR, false, parentDir, "..", null, false));
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

	void _onNewFolderBtnClicked()
	{
        Intent view = new Intent(this, RenameInputMapView.class);
        view.putExtra(RenameInputMapView.CONFIG_CURNAME, "saves");
        startActivityForResult(view, ACTIVITYRESULT_NEWFOLDERNAME);
	}
	
	void _onRefreshDBClicked()
	{
		HataroidActivity ha = HataroidActivity.instance;
		if (ha != null)
		{
			if (_curDir != null)
			{
				ha.refreshGameDB(_curDir.getAbsolutePath(), _exts, true, this);
			}
			else if (_curZipFile != null)
			{
				ha.refreshGameDB(_curZipFile.getName(), _exts, true, this);
			}
		}
	}
	
	void _onClearDBClicked()
	{
		HataroidActivity ha = HataroidActivity.instance;
		if (ha != null)
		{
			if (_curDir != null)
			{
				ha.clearGameDB(_curDir.getAbsolutePath(), true, this);
			}
			else if (_curZipFile != null)
			{
				ha.clearGameDB(_curZipFile.getName(), true, this);
			}
		}
	}
	
	public Activity getGameDBScanActivity() { return this; }
	public void onGameDBScanComplete()
	{
		if (_refreshDB && (_curDir != null || _curZipFile != null))
		{
			FileBrowser.this.runOnUiThread(new Runnable() {
				public void run()
				{
					FileBrowser.this._refreshFileList();
				}
			});
		}
	}
	
	void _refreshFileList()
	{
		if (_refreshDB)
		{
			if (_curDir != null)
			{
				FileBrowser.this.retrieveFileList(_curDir, _exts);
			}
			else if (_curZipFile != null)
			{
				FileBrowser.this.retrieveZipFileList(_curZipFile, _exts);
			}
		}
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			case ACTIVITYRESULT_NEWFOLDERNAME:
			{
				if (resultCode == RESULT_OK)
				{
					String newFolderName = data.getStringExtra(RenameInputMapView.RESULT_NAME);
					if (newFolderName != null)
					{
						newFolderName = newFolderName.trim();
					}
					if (newFolderName != null && newFolderName.length() > 0
					 && _curDir != null && _curDir.exists())
					{
						String folderPath = _curDir.getAbsolutePath() + "/" + newFolderName;
						File newFolder = new File(folderPath);
						if (!newFolder.exists())
						{
							if (newFolder.mkdir())
							{
								// refresh
								retrieveFileList(_curDir, _exts);
							}
						}
					}
				}
				break;
			}
		}
	}

	void _setCurDir(String path)
	{
		_curDir = (path != null) ? new File(path) : null;
		
		if (_refreshDBBtnView != null)
		{
			boolean showDBRefresh = _refreshDB && ((_curDir != null && _curDir.isDirectory()) || (_curZipFile != null));
			_refreshDBBtnView.setVisibility(showDBRefresh ? View.VISIBLE : View.INVISIBLE);
		}
		if (_clearDBBtnView != null)
		{
			boolean showDBRefresh = _refreshDB && ((_curDir != null && _curDir.isDirectory()) || (_curZipFile != null));
			_clearDBBtnView.setVisibility(showDBRefresh ? View.VISIBLE : View.INVISIBLE);
		}
	}
}

class SelectFolderClickListener implements OnClickListener
{
	FileBrowser _fb = null;
	public SelectFolderClickListener(FileBrowser fb)
	{
		_fb = fb;
	}
	public void deinit()
	{
		_fb = null;
	}
	
	public void onClick(View v) {
		if (_fb != null)
		{
			_fb.onSelectButtonClicked();
		}
	}
}

