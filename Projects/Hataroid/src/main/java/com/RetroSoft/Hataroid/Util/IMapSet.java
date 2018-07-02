package com.RetroSoft.Hataroid.Util;

import java.util.List;
import java.util.Map;

public interface IMapSet
{
	public int get_MAPVALUE_RESULT_KEYCODE();		// eg. 1
	public int get_RENAME_MAPSET_RESULT_KEYCODE();	// eg. 2
	public String get_PREF_PREFIX();				// eg. "_PREFShortcutMap_"
	
	public String get_DeleteDialogTitle();			// eg. "Delete Shortcut Map?"
	public String get_DeleteDialogMessage();		// eg. "Are you sure you want to delete this shortcut map?"
	public String get_NewMapSetNamePrefix();		// eg. "my_shortcut_map";

	public int get_ViewLayoutResID();				// eg. R.layout.shortcutmap_view
	public int get_ViewLayoutID();					// eg. R.id.shortcutmap_view
	public int get_ItemLayoutResID();				// eg. R.layout.shortcutmap_item

	public int getNumPresets();
	public String getPresetID(int idx);
	public String getPresetName(int idx);
	public boolean isSystemPreset(String presetID);
	
	public void initFromPreset(String presetID);
	public void initDefault();

	public boolean decodeMapSetPref(String prefVal, Map<String,Object> result);
	public String encodePrefString(String name);

	public List<IMapSetListItem> buildMapSetListItems();

	public boolean updateEntry(boolean unMap, IMapSetListItem srcItem, IMapSetListItem newItemVal);
	public void updateListItem(IMapSetListItem item);

	public void setupButtonListeners(MapSetConfigureView view);
}
