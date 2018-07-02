package com.RetroSoft.Hataroid.GameDB;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.zip.CRC32;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.content.ContentValues;
import android.content.Context;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.FileBrowser.FileBrowser;

public class GameDBHelper extends SQLiteOpenHelper
{
	private static final int		DATABASE_VERSION = 3;		// Database Version
	private static final String		DATABASE_NAME = "GameDB";	// Database Name

	// Folder table
	private static final String		TABLE_FOLDERS = "folders";
	private static final String		FOLDER_KEY_ID = "id";
	private static final String		FOLDER_KEY_PATH = "path";
	//private static final String		FOLDER_KEY_ZIP = "zip";
	private static final String		TABLE_FOLDER_CREATE = "CREATE TABLE " + TABLE_FOLDERS + "("
															+ FOLDER_KEY_ID + " INTEGER PRIMARY KEY"
															+ "," + FOLDER_KEY_PATH + " TEXT"
															//+ "," + FOLDER_KEY_ZIP + " INTEGER"
															+ ")";
	
	// Files table
	private static final String		TABLE_FILES = "files";
	private static final String		FILES_KEY_ID = "id";
	private static final String		FILES_KEY_FOLDERID = "fid";
	private static final String		FILES_KEY_FILENAME = "fname";
	private static final String		FILES_KEY_GAMENAMES = "gnames";
	private static final String		FILES_KEY_RELEASEGROUP = "rgrp";
	private static final String		FILES_KEY_RELEASEID = "rid";
	private static final String		FILES_KEY_ISMULTIZIP = "mzip";
	private static final String		FILES_KEY_ZIPCOMPLETE = "zfull";
	private static final String		TABLE_FILES_CREATE = "CREATE TABLE " + TABLE_FILES + "("
															+ FILES_KEY_ID + " INTEGER PRIMARY KEY"
															+ "," + FILES_KEY_FOLDERID + " INTEGER"
															+ "," + FILES_KEY_FILENAME + " TEXT"
															+ "," + FILES_KEY_GAMENAMES + " TEXT"
															+ "," + FILES_KEY_RELEASEGROUP + " TEXT"
															+ "," + FILES_KEY_RELEASEID + " TEXT"
															+ "," + FILES_KEY_ISMULTIZIP + " INTEGER"
															+ "," + FILES_KEY_ZIPCOMPLETE + " INTEGER"
															+ ")";

	Map<Long, GameDBFile>       _refDBMap = null;
	GameDBScanner               _scanner = null;
	int                         _scanCount = 0;

	public GameDBHelper(Context context)
	{
		super(context, DATABASE_NAME, null, DATABASE_VERSION);
	}

	@Override public void onCreate(SQLiteDatabase db)
	{
		// creating required tables
		db.execSQL(TABLE_FOLDER_CREATE);
		db.execSQL(TABLE_FILES_CREATE);
	}

	@Override public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
	{
		// on upgrade drop older tables
		db.execSQL("DROP TABLE IF EXISTS " + TABLE_FOLDERS);
		db.execSQL("DROP TABLE IF EXISTS " + TABLE_FILES);

		// create new tables
		onCreate(db);
	}

	public void closeDB()
	{
		SQLiteDatabase db = this.getReadableDatabase();
		if (db != null && db.isOpen())
		{
			db.close();
		}
	}
	
	public void loadDBMap(AssetManager assetMgr)
	{
		if (_refDBMap != null)
		{
			return; // already loaded
		}

		_refDBMap = new HashMap<Long, GameDBFile>();
		//_refDBMap.clear();

		InputStream stream = null;
		BufferedReader in = null;
		try
		{
			stream = assetMgr.open("text/stgames.txt");
			in = new BufferedReader(new InputStreamReader(stream));
			String line = null;
			while ((line = in.readLine()) != null)
			{
				line = line.trim();
				if (line.length() == 0)
				{
					continue;
				}
				
				if (line.startsWith("#"))
				{
					continue;
				}

				String [] entry = line.split(":");
				if (entry.length < 4)
				{
					Log.i(HataroidActivity.LOG_TAG, "GamesDB: Invalid entry: '" + line + "'");
					continue;
				}
				
				String rgrp = entry[0].trim();
				String rid = entry[1].trim();

				String crcs = entry[2].trim();
				if (crcs.length() == 0)
				{
					//Log.i(HataroidActivity.LOG_TAG, "GamesDB: Empty crc, ignoring: '" + line + "'");
					continue;
				}
				String [] crclist = crcs.split(" ");
				
				String gameNames = entry[3];
				for (int i = 4; i < entry.length; ++i)
				{
					gameNames += ":" + entry[i];
				}
				gameNames = gameNames.trim();

				GameDBFile dbFile = new GameDBFile();
				
				dbFile.dbID = -1;
				dbFile.folderID = -1;

				dbFile.fileName = null;
				dbFile.gameNames = new String [] { gameNames };
				dbFile.releaseGroup = rgrp;
				dbFile.releaseID = rid;
				dbFile.isMultiZip = false;
				dbFile.zipComplete = false;

				for (int i = 0; i < crclist.length; ++i)
				{
					String crc = crclist[i].trim();
					if (crc.length() > 0)
					{
						try
						{
							Long crc32;
							if (crc.toLowerCase().startsWith("0x"))
							{
								crc32 = Long.decode(crc);
							}
							else
							{
								crc32 = Long.decode("0x"+crc);
							}
							//if (_refDBMap.containsKey(crc32))
							//{
							//	Log.i(HataroidActivity.LOG_TAG, "GamesDB: Warning: Duplicate crc entry: '" + crc + "'" + ": Name: " + gameNames);
							//}
							_refDBMap.put(crc32, dbFile);
						}
						catch (Exception e)
						{
						}
					}
				}

				//Log.i(HataroidActivity.LOG_TAG, "GamesDB: Loaded entry: '" + line + "'");
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		finally
		{
			try
			{
				if (in != null)
				{
					in.close();
					in = null;
				}
				if (stream != null)
				{
					stream.close();
					stream = null;
				}
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
		}
	}

	public long addFolder(String folderPath/*, boolean isZip*/)
	{
		SQLiteDatabase db = this.getWritableDatabase();

		ContentValues values = new ContentValues();
		values.put(FOLDER_KEY_PATH, folderPath);
		//values.put(FOLDER_KEY_ZIP, isZip ? 1 : 0);

		// insert row
		long folderID = db.insert(TABLE_FOLDERS, null, values);
		return folderID;
	}	

	public GameDBFolder findFolder(String path)
	{
		SQLiteDatabase db = this.getReadableDatabase();

		String selectQuery = "SELECT * FROM " + TABLE_FOLDERS + " WHERE " + FOLDER_KEY_PATH + " = \"" + path + "\"";
		Cursor c = db.rawQuery(selectQuery, null);

		if (c != null && c.moveToFirst())
		{
			GameDBFolder dbFolder = new GameDBFolder();
			dbFolder.dbID = c.getLong(c.getColumnIndex(FOLDER_KEY_ID));
			dbFolder.folderPath = c.getString(c.getColumnIndex(FOLDER_KEY_PATH));
			//dbFolder.isZip = c.getInt(c.getColumnIndex(FOLDER_KEY_ZIP)) != 0;

			return dbFolder;
		}

		return null;
	}
	
	public List<GameDBFolder> matchFolders(String pathPrefix)
	{
		SQLiteDatabase db = this.getReadableDatabase();

		String selectQuery = "SELECT * FROM " + TABLE_FOLDERS + " WHERE " + FOLDER_KEY_PATH + " LIKE \"" + pathPrefix + "%\"";
		Cursor c = db.rawQuery(selectQuery, null);

		List<GameDBFolder> results = new LinkedList<GameDBFolder>();
		if (c != null && c.moveToFirst())
		{
			do
			{
				GameDBFolder dbFolder = new GameDBFolder();
				dbFolder.dbID = c.getLong(c.getColumnIndex(FOLDER_KEY_ID));
				dbFolder.folderPath = c.getString(c.getColumnIndex(FOLDER_KEY_PATH));
				//dbFolder.isZip = c.getInt(c.getColumnIndex(FOLDER_KEY_ZIP)) != 0;
				
				results.add(dbFolder);
			} while (c.moveToNext());
		}

		return results;
	}
	
	public void deleteFolder(String folderPath, boolean deleteFiles)
	{
		GameDBFolder dbFolder = findFolder(folderPath);
		if (dbFolder != null)
		{
			deleteFolder(dbFolder.dbID, deleteFiles);
		}
	}

	public void deleteFolder(long folderID, boolean deleteFiles)
	{
		if (deleteFiles)
		{
			deleteFolderEntries(folderID);
		}

		SQLiteDatabase db = this.getWritableDatabase();
		db.delete(TABLE_FOLDERS, FOLDER_KEY_ID + " = ?", new String[] { String.valueOf(folderID) });
	}
	
	public void deleteFolderEntries(String folderPath)
	{
		GameDBFolder dbFolder = findFolder(folderPath);
		if (dbFolder != null)
		{
			deleteFolderEntries(dbFolder.dbID);
		}
	}

	public void deleteFolderEntries(long folderID)
	{
		SQLiteDatabase db = this.getWritableDatabase();
		db.delete(TABLE_FILES, FILES_KEY_FOLDERID + " = ?", new String[] { String.valueOf(folderID) });
	}
	
	public Map<String,GameDBFile> getFiles(String folderPath)
	{
		GameDBFolder dbFolder = findFolder(folderPath);
		if (dbFolder != null)
		{
			return getFiles(dbFolder.dbID);
		}
		return new LinkedHashMap<String, GameDBFile>();
	}
	
	public void updateFile(String folderPath, String fileName, GameDBFile newFile)
	{
		SQLiteDatabase db = this.getWritableDatabase();

		long folderID = -1;
		GameDBFolder dbFolder = findFolder(folderPath);
		if (dbFolder != null)
		{
			folderID = dbFolder.dbID;
			db.delete(TABLE_FILES, FILES_KEY_FOLDERID + " = ? AND " + FILES_KEY_FILENAME + " = ?", new String[] { String.valueOf(folderID), fileName });
		}
		else
		{
			folderID = addFolder(folderPath/*, isZip*/);
		}
		
		if (folderID >= 0)
		{
			GameDBFile dbf = newFile;

			ContentValues values = new ContentValues();
			values.put(FILES_KEY_FOLDERID, folderID);
			values.put(FILES_KEY_FILENAME, dbf.fileName);
			values.put(FILES_KEY_GAMENAMES, dbf.gameNames[0]);
			values.put(FILES_KEY_RELEASEGROUP, dbf.releaseGroup);
			values.put(FILES_KEY_RELEASEID, dbf.releaseID);
			values.put(FILES_KEY_ISMULTIZIP, dbf.isMultiZip?1:0);
			values.put(FILES_KEY_ZIPCOMPLETE, dbf.zipComplete?1:0);

			db.insert(TABLE_FILES, null, values);
		}
	}

	public Map<String,GameDBFile> getFiles(long folderID)
	{
		Map<String, GameDBFile> dbFiles = new LinkedHashMap<String, GameDBFile>();
		String selectQuery = "SELECT * FROM " + TABLE_FILES + " WHERE " + FILES_KEY_FOLDERID + " = " + String.valueOf(folderID);

		SQLiteDatabase db = this.getReadableDatabase();
		Cursor c = db.rawQuery(selectQuery, null);

		if (c != null && c.moveToFirst())
		{
			do
			{
				GameDBFile dbf = new GameDBFile();
				dbf.dbID = c.getLong(c.getColumnIndex(FILES_KEY_ID));
				dbf.folderID = c.getLong(c.getColumnIndex(FILES_KEY_FOLDERID));
				
				dbf.fileName = c.getString(c.getColumnIndex(FILES_KEY_FILENAME));
				dbf.releaseGroup = c.getString(c.getColumnIndex(FILES_KEY_RELEASEGROUP));
				dbf.releaseID = c.getString(c.getColumnIndex(FILES_KEY_RELEASEID));
				dbf.isMultiZip = c.getInt(c.getColumnIndex(FILES_KEY_ISMULTIZIP)) != 0;
				dbf.zipComplete = c.getInt(c.getColumnIndex(FILES_KEY_ZIPCOMPLETE)) != 0;

				String gameList = c.getString(c.getColumnIndex(FILES_KEY_GAMENAMES));
				dbf.gameNames = gameList.split("; ");

				dbFiles.put(dbf.fileName, dbf);
			} while (c.moveToNext());
		}
		return dbFiles;
	}
	
	public boolean addFiles(String path, /*boolean isZip,*/ List<GameDBFile> files, boolean deletePrevFolderEntries, GameDBScanner scanner)
	{
		boolean result = true;

		SQLiteDatabase db = this.getWritableDatabase();

		if (files.size() > 0)
		{
			String[] shortPathSplit = path.split("/");
			String spath = shortPathSplit[shortPathSplit.length-1];
			
			if (deletePrevFolderEntries)
			{
				setCurProgressText("Removing old entries: " + spath, scanner, false);
				GameDBFolder dbFolder = findFolder(path);
				if (dbFolder != null)
				{
					deleteFolder(dbFolder.dbID, true);
				}
			}

			setCurProgressText("Adding files to db: " + spath, scanner, false);
			db.beginTransaction();
			try
			{
				long folderID = -1;
				GameDBFolder dbFolder = findFolder(path);
				if (dbFolder == null)
				{
					folderID = addFolder(path/*, isZip*/);
				}
				else
				{
					folderID = dbFolder.dbID;
				}
				if (folderID >= 0)
				{
					for (int i = 0; i < files.size(); ++i)
					{
						GameDBFile dbf = files.get(i);
	
						ContentValues values = new ContentValues();
						values.put(FILES_KEY_FOLDERID, folderID);
						values.put(FILES_KEY_FILENAME, dbf.fileName);
						values.put(FILES_KEY_GAMENAMES, dbf.gameNames[0]);
						values.put(FILES_KEY_RELEASEGROUP, dbf.releaseGroup);
						values.put(FILES_KEY_RELEASEID, dbf.releaseID);
						values.put(FILES_KEY_ISMULTIZIP, dbf.isMultiZip?1:0);
						values.put(FILES_KEY_ZIPCOMPLETE, dbf.zipComplete?1:0);
	
						db.insert(TABLE_FILES, null, values);
						//db.yieldIfContendedSafely(); // In case you do larger updates
					}
				}
				else
				{
					result = false;
				}
				db.setTransactionSuccessful();
			}
			catch (Exception e)
			{
				result = false;
			}
			finally
			{
				db.endTransaction();
			}

			setCurProgressText("Files added to db: " + spath, scanner, false);
		}
		return result;
	}
	
	public List<GameDBFile> scanFile(String filePath, String fileName, String[] validExts, GameDBScanner scanner, boolean[] isMultiDiscZip, boolean[] zipComplete)
	{
		List<GameDBFile> matchedFiles = new LinkedList<GameDBFile>();
		
		isMultiDiscZip[0] = false;
		zipComplete[0] = false;

		++_scanCount;
		String scanMsgPrefix = "Scanning " + _scanCount + ": ";
		
		if (filePath.toLowerCase().endsWith(".zip"))
		{
			ZipFile zf = null;
			try
			{
				setCurProgressText(scanMsgPrefix + fileName, scanner, false);

				zf = new ZipFile(filePath);
				int [] numValidDiscs = new int [1];

				ZipEntry firstDisc = getNumSTDisks(zf, validExts, numValidDiscs);
				if (numValidDiscs[0] == 1 && firstDisc != null)
				{
					setCurProgressText(scanMsgPrefix + firstDisc.getName(), scanner, false);

					Long crc32 = crc32ZipFile(zf, firstDisc);
					GameDBFile dbFile = _createFromRefDB(crc32, fileName);
					if (dbFile != null)
					{
						setCurProgressText("Matched: " + firstDisc.getName(), scanner, false);
						matchedFiles.add(dbFile);
					}
				}
				else if (numValidDiscs[0] > 1)
				{
					isMultiDiscZip[0] = true;

					int numMatchedSubFiles = 0;

					Enumeration<? extends ZipEntry> entries = zf.entries();
					while (entries.hasMoreElements())
					{
						ZipEntry f = entries.nextElement();

						if (!f.isDirectory())
						{
							String flc = f.getName().toLowerCase();
							if (!flc.endsWith(".zip")) // no recurse zips
							{
								for (String ext : validExts)
								{
									if (flc.endsWith(ext))
									{
										setCurProgressText(scanMsgPrefix + f.getName(), scanner, false);

										Long crc32 = crc32ZipFile(zf, f);
										GameDBFile dbFile = _createFromRefDB(crc32, f.getName());
										if (dbFile != null)
										{
											setCurProgressText("Matched: " + f.getName(), scanner, false);
											matchedFiles.add(dbFile);
											++numMatchedSubFiles;
										}
										break;
									}
								}
							}
						}
					}
					
					zipComplete[0] = numMatchedSubFiles == numValidDiscs[0];
				}
			}
			catch (Exception e)
			{
			}
			finally
			{
				FileBrowser._closeZipFile(zf);
				zf = null;
			}
		}
		else
		{
			try
			{
				setCurProgressText(scanMsgPrefix + fileName, scanner, false);

				Long crc32 = crc32File(filePath);
				GameDBFile dbFile = _createFromRefDB(crc32, fileName);
				if (dbFile != null)
				{
					setCurProgressText("Matched: " + fileName, scanner, false);
					matchedFiles.add(dbFile);
				}
			}
			catch (Exception e)
			{
			}
		}

		return matchedFiles;
	}
	
	GameDBFile _createFromRefDB(Long crc32, String filename)
	{
		if (_refDBMap.containsKey(crc32))
		{
			GameDBFile refFile = _refDBMap.get(crc32);
			GameDBFile dbFile = new GameDBFile();

			dbFile.fileName = filename;
			dbFile.gameNames = refFile.gameNames;
			dbFile.releaseGroup = refFile.releaseGroup;
			dbFile.releaseID = refFile.releaseID;
			dbFile.isMultiZip = false;
			dbFile.zipComplete = false;

			return dbFile;
		}
		return null;
	}
	
	public GameDBFile _createMultiZipParent(String filename, boolean zipComplete)
	{
		GameDBFile dbFile = new GameDBFile();

		dbFile.fileName = filename;
		dbFile.gameNames = new String[] {""};
		dbFile.releaseGroup = "";
		dbFile.releaseID = "";
		dbFile.isMultiZip = true;
		dbFile.zipComplete = zipComplete;

		return dbFile;
	}

	public Long crc32ZipFile(ZipFile zf, ZipEntry f)
	{
		Long crc32 = f.getCrc();
		if (crc32 == -1)
		{
			// scan file ourselves
			InputStream is = null;
			try
			{
				is = zf.getInputStream(f);
				if (is != null)
				{
					crc32 = crc32Stream(is);
				}
			}
			catch (Exception e)
			{
			}
			finally
			{
				if (is != null)
				{
					try { is.close(); }
					catch (Exception ei) {}
					is = null;
				}
			}
		}
		return crc32;
	}
	
	public long crc32File(String filePath) throws IOException
	{
		long crc32 = -1;

		InputStream inputStream = null;
		try
		{
			inputStream = new BufferedInputStream(new FileInputStream(filePath));
			crc32 = crc32Stream(inputStream);
		}
		catch (Exception e)
		{
		}
		finally
		{
			if (inputStream != null)
			{
				inputStream.close();
			}
		}

		return crc32;
	}
	
	public long crc32Stream(InputStream inputStream) throws IOException
	{
		CRC32 crc = new CRC32();
		byte[] buffer = new byte[1024];
		int read;
		while ((read = inputStream.read(buffer)) != -1)
		{
			crc.update(buffer, 0, read);
		}
		return crc.getValue();
	}

	public void scanFolder(AssetManager assets, String path, String [] exts, boolean recurse, IGameDBScanner scanInterface)
	{
		if (_scanner == null)
		{
			_scanner = new GameDBScanner();
			_scanner.initScan(this, assets, path, exts, recurse, scanInterface);
			_scanner.start();
		}
	}
	
	public void clearFolder(String path, boolean recurse, IGameDBScanner scanInterface)
	{
		if (_scanner == null)
		{
			_scanner = new GameDBScanner();
			_scanner.initClear(this, path, recurse, scanInterface);
			_scanner.start();
		}
	}

	public void scanComplete()
	{
		_scanner = null;
	}

	public void clearScanInterface()
	{
		if (_scanner != null)
		{
			_scanner.clearInterface();
		}
	}
	
	public boolean setScanInterface(IGameDBScanner scanInterface)
	{
		if (_scanner != null)
		{
			return _scanner.setInterface(scanInterface);
		}

		return false;
	}

	public void initScan() {
		_scanCount = 0;
	}

	public void setCurProgressText(String msg, GameDBScanner scanner, boolean force)
	{
		//Log.i(HataroidActivity.LOG_TAG, msg);
		if (scanner != null)
		{
			scanner.onGameDBScanProgress(msg, force);
		}
	}

	// return first entry
	public static ZipEntry getNumSTDisks(ZipFile zf, String[] validExts, int[] numEntries)
	{
		ZipEntry firstEntry = null;
		int validDiscCount = 0;

		//isPasti[0] = false;

		Enumeration<? extends ZipEntry> e = zf.entries();
		while (e.hasMoreElements())
		{
			ZipEntry f = e.nextElement();

			if (!f.isDirectory())
			{
				String flc = f.getName().toLowerCase();
				if (!flc.endsWith(".zip")) // no recurse zips
				{
					//if (flc.endsWith(".stx"))
					//{
					//	isPasti[0] = true;
					//}

					for (String ext : validExts)
					{
						if (flc.endsWith(ext))
						{
							if (firstEntry == null)
							{
								firstEntry = f;
							}
							++validDiscCount;
							break;
						}
					}
				}
			}
		}

		numEntries[0] = validDiscCount;
		return firstEntry;
	}
}
