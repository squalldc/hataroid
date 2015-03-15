package com.RetroSoft.Hataroid.GameDB;

import java.io.File;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.res.AssetManager;

import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;

public class GameDBScanner extends Thread
{
	final int kModeNone			= 0;
	final int kModeScan			= 1;
	final int kModeClear		= 2;

	int					_mode = kModeNone;

	GameDBHelper		_gameDB = null;
	AssetManager		_assets = null;
	String				_path = null;
	String[]			_exts = null;
	boolean				_recurse = false;

	IGameDBScanner		_scanInterface = null;
	ProgressDialog		_scanDialog = null;
	volatile boolean	_creatingScanDialog = false;
	volatile boolean	_scanMessagePending = false;
	volatile long		_prevProgressTime = 0;

	volatile boolean	_complete = false;
	boolean				_result = false;
	
	public void initScan(GameDBHelper gameDB, AssetManager assets, String path, String[] exts, boolean recurse, IGameDBScanner scanInterface)
	{
		_mode = kModeScan;
		
		_gameDB = gameDB;
		_assets = assets;
		_path = path;
		_exts = exts;
		_recurse = recurse;
		_scanInterface = scanInterface;
		
		_result = true;
	}
	
	public void initClear(GameDBHelper gameDB, String path, boolean recurse, IGameDBScanner scanInterface)
	{
		_mode = kModeClear;

		_gameDB = gameDB;
		_assets = null;
		_path = path;
		_exts = null;
		_recurse = recurse;
		_scanInterface = scanInterface;
		
		_result = true;
	}

	void _createProgressDialog()
	{
		if (_scanInterface != null)
		{
			_creatingScanDialog = true;

			final Activity owner = _scanInterface.getGameDBScanActivity();
			owner.runOnUiThread(new Runnable() {
				public void run()
				{
					_scanDialog = new ProgressDialog(owner);
					
					String titleStr = (_mode == kModeScan) ? "GameDB: Scanning files..." : "GameDB: Clearing database...";
					_scanDialog.setTitle(titleStr);
	    			_scanDialog.setMessage("");
					_scanDialog.setCancelable(false);
					_scanDialog.setIndeterminate(true);
					_scanDialog.show();

					_creatingScanDialog = false;
				}
	    	});
		}
	}
	
	void _destroyProgressDialog()
	{
		while (_creatingScanDialog)
		{
			try
			{
				Thread.sleep(100);
			}
			catch (Exception e)
			{
			}
		}

		if (_scanDialog != null)
		{
			_scanDialog.cancel();
			_scanDialog = null;
		}
	}
	
	public void clearInterface()
	{
		_destroyProgressDialog();
		_scanInterface = null;
	}
	
	public boolean setInterface(IGameDBScanner scanInterface)
	{
		clearInterface();
		
		if (!_complete)
		{
			_scanInterface = scanInterface;
			_createProgressDialog();
			
			return true;
		}

		return false;
	}
	
	public boolean isComplete() { return _complete; }

	public void run()
	{
		_createProgressDialog();

		switch (_mode)
		{
			case kModeScan:
			{
				_runScan();
				break;
			}
			case kModeClear:
			{
				_runClear();
				break;
			}
			default:
			{
				break;
			}
		}

		_gameDB.scanComplete();
		
		_destroyProgressDialog();
		if (_scanInterface != null)
		{
			_scanInterface.onGameDBScanComplete();
		}

		_scanInterface = null;
		_complete = true;

		//Log.i(HataroidActivity.LOG_TAG, "DONE*****************************");
	}

	void _runScan()
	{
		_gameDB.setCurProgressText("Loading Game DB...", this, true);
		_gameDB.loadDBMap(_assets);
		
		try
		{
			Map<String,List<GameDBFile>> matchedFiles = new LinkedHashMap<String, List<GameDBFile>>();

			String [] validExtsLower = null;
			if (_exts != null)
			{
				validExtsLower = new String [_exts.length];
				for (int i = 0; i < _exts.length; ++i)
				{
					validExtsLower[i] = _exts[i].toLowerCase();
				}
			}

			List<String> checkPaths = new LinkedList<String>();
			checkPaths.add(_path);
			while (checkPaths.size() > 0)
			{
				String curPath = checkPaths.get(0);
				checkPaths.remove(0);
				
				File curDir = new File(curPath);
				if (curDir != null)
				{
					List<GameDBFile> curPathDBFiles = new LinkedList<GameDBFile>();

					if (curPath.toLowerCase().endsWith(".zip")) // multi-disc zips
					{
						boolean[] isMultiDiscZip = new boolean [1];
						boolean[] zipComplete = new boolean [1];
						List<GameDBFile> dbFiles = _gameDB.scanFile(curDir.getAbsolutePath(), curDir.getName(), validExtsLower, this, isMultiDiscZip, zipComplete);
						if (dbFiles != null && dbFiles.size() > 0 && isMultiDiscZip[0])
						{
							curPathDBFiles.addAll(dbFiles);

							String parentPath = FileBrowser._getParentPathName(curDir.getAbsolutePath());
							String zipName = curDir.getName();
							GameDBFile multiZipParent = _gameDB._createMultiZipParent(zipName, zipComplete[0]);
							_gameDB.updateFile(parentPath, zipName, multiZipParent);
						}

						//checkPaths.add(curDir.getAbsolutePath()); // just scan parent folder which will include this
					}
					else
					{
						File [] curFiles = curDir.listFiles();
						for (File f : curFiles)
						{
							if (f.isDirectory())
							{
								if (_recurse)
								{
									checkPaths.add(f.getAbsolutePath());
								}
							}
							else
							{
								boolean valid = true;
								String flc = f.getName().toLowerCase();
								if (validExtsLower != null)
								{
									valid = false;
									for (String ext : validExtsLower)
									{
										if (flc.endsWith(ext))
										{
											valid = true;
											break;
										}
									}
								}
								if (valid)
								{
									boolean[] isMultiDiscZip = new boolean [1];
									boolean[] zipComplete = new boolean [1];
									List<GameDBFile> dbFiles = _gameDB.scanFile(f.getAbsolutePath(), f.getName(), validExtsLower, this, isMultiDiscZip, zipComplete);
									if (dbFiles != null && dbFiles.size() > 0)
									{
										if (isMultiDiscZip[0])
										{
											matchedFiles.put(f.getAbsolutePath(), dbFiles);

											GameDBFile multiZipParent = _gameDB._createMultiZipParent(f.getName(), zipComplete[0]);
											curPathDBFiles.add(multiZipParent);
										}
										else
										{
											curPathDBFiles.addAll(dbFiles);
										}
									}
								}
							}
						}
					}
					
					if (curPathDBFiles.size() > 0)
					{
						matchedFiles.put(curPath, curPathDBFiles);
					}
				}
			}
			
			// add to db
			Iterator<Map.Entry<String,List<GameDBFile>>> entries = matchedFiles.entrySet().iterator();
			while (entries.hasNext())
			{
				Map.Entry<String,List<GameDBFile>> entry = entries.next();
				String path = entry.getKey();
				List<GameDBFile> files = entry.getValue();
				_gameDB.addFiles(path, files, true, this);
			}
		}
		catch (Exception e)
		{
			_result = false;
		}
	}

	
	void _runClear()
	{
		_gameDB.setCurProgressText("Clearing entries...", this, true);

		try
		{
			if (!_recurse)
			{
				_gameDB.deleteFolder(_path, true);
			}
			else
			{
				List<GameDBFolder> folders = _gameDB.matchFolders(_path);
				for (int i = 0; i < folders.size(); ++i)
				{
					GameDBFolder f = folders.get(i);
					String fpath = f.folderPath;
					String [] fpathSplit = fpath.split("/");
					_gameDB.setCurProgressText("Clearing " + fpathSplit[fpathSplit.length-1], this, false);
					_gameDB.deleteFolder(f.dbID, true);
				}
			}
		}
		catch (Exception e)
		{
			_result = false;
		}
	}
	
	public void onGameDBScanProgress(String statusMsg, boolean force)
	{
		try
		{
			if (_scanInterface != null && _scanDialog != null && !_scanMessagePending)
			{
				long curTime = System.currentTimeMillis();
				if (/*force ||*/ ((curTime - _prevProgressTime) > 1000))
				{
					_scanMessagePending = true;
	
					final String msg = statusMsg;
					_scanInterface.getGameDBScanActivity().runOnUiThread(new Runnable() {
						public void run()
						{
							if (_scanDialog != null)
							{
								_scanDialog.setMessage(msg);
							}
							_scanMessagePending = false;
							_prevProgressTime = System.currentTimeMillis();
						}
					});
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
