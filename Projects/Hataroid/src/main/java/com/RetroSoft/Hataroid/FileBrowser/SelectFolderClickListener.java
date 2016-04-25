package com.RetroSoft.Hataroid.FileBrowser;

import android.view.View;

class SelectFolderClickListener implements View.OnClickListener
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
