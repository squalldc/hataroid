package com.RetroSoft.Hataroid.GameDB;

import android.app.Activity;

public interface IGameDBScanner
{
	public Activity getGameDBScanActivity();

	public void onGameDBScanComplete();
	//public void onGameDBScanProgress(String statusMsg);
}
