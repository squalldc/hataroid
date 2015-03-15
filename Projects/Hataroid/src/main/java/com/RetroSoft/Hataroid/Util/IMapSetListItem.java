package com.RetroSoft.Hataroid.Util;

import java.io.Serializable;
import java.util.List;

import android.view.View;

public interface IMapSetListItem extends Serializable, Comparable<IMapSetListItem>
{
	public int get_ViewLayoutResID();				// eg. R.layout.shortcutkeyselect_view
	public int get_ItemLayoutResID();				// eg. R.layout.shortcutkeyselect_item

	public void formatItemView(View v);

	public List<IMapSetListItem> buildListItems();
	public boolean isSameItemKey(IMapSetListItem other);
}
