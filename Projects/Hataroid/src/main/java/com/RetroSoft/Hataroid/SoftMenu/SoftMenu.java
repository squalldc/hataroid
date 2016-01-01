package com.RetroSoft.Hataroid.SoftMenu;

import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.HataroidNativeLib;
import com.RetroSoft.Hataroid.R;

import java.util.LinkedList;
import java.util.List;

public class SoftMenu
{
	final int kDiskBtnIdx = 0;
	final int kPauseBtnIdx = 3;

	final int[] kMainActionIds = new int []
	{
		R.id.disk,
		R.id.reset,
		R.id.savestate,
		R.id.pause,
		R.id.settings,
		R.id.quit,
		R.id.help,
	};

	final int[][] kSubMenuActionIds = new int [][]
	{
		new int[] //R.id.disk,
		{
			R.id.floppya,
			R.id.floppyb,
			R.id.ejecta,
			R.id.ejectb,
		},
		new int[] //R.id.reset,
		{
			R.id.coldreset,
			R.id.warmreset,
		},
		new int[] //R.id.savestate,
		{
			R.id.ss_save,
			R.id.ss_load,
			R.id.ss_delete,
			R.id.ss_quicksaveslot,
		},
		null,//R.id.pause,
		null,//R.id.settings,
		null,//R.id.quit,
		null,//R.id.help,
	};

	Context					_appContext;

	HataroidActivity		_activity;
	View					_parentView;

	PopupWindow				_mainPopup;

	PopupWindow				_subPopup;
	ArrayAdapter<String>	_subPopupListAdapter;
	int						_curSubGroupIdx;

	public void create(Context context)
	{
		_appContext = context;

		int[] screenRes = getScreenDimensions();
		int scrWidth = screenRes[0];
		int scrHeight = screenRes[1];

		LayoutInflater inflater = (LayoutInflater) _appContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

		// main menu
		{
			View sfView = inflater.inflate(R.layout.softmenu_view, null);

			int popupWidth = (int)(scrWidth);
			int popupHeight = (int)(scrHeight * 0.25f);

			_mainPopup = new PopupWindow(sfView, popupWidth, popupHeight, true);
			_mainPopup.setBackgroundDrawable(new ColorDrawable(_appContext.getResources().getColor(android.R.color.transparent)));
			_mainPopup.setOutsideTouchable(true);

			final int kNumButtons = kMainActionIds.length;
			int maxBtnWidth = scrWidth / kNumButtons;
			for (int i = 0; i < kNumButtons; ++i)
			{
				final int btnIdx = i;
				final int actionIdx = kMainActionIds[i];

				ImageButton b = (ImageButton)sfView.findViewWithTag("sfBtn" + (i + 1));
				if (b != null)
				{
					b.setMaxWidth(maxBtnWidth);
					b.setOnClickListener(new View.OnClickListener() {
						public void onClick(View v) {
							_handleBtn(btnIdx, actionIdx, v);
						}
					});
				}

				TextView tv = (TextView)sfView.findViewWithTag("sfLbl" + (i + 1));
				if (tv != null)
				{
					tv.setWidth(maxBtnWidth);
					tv.setOnClickListener(new View.OnClickListener() {
						public void onClick(View v) {
							_handleBtn(btnIdx, actionIdx, v);
						}
					});
				}
			}
		}

		// sub menu
		{
			View sfView = inflater.inflate(R.layout.softmenu_sub_view, null);

			int popupWidth = (int)(scrWidth * 0.62f);
			int popupHeight = (int)(scrHeight * 0.8f);

			_subPopup = new PopupWindow(sfView, popupWidth, popupHeight, true);
			_subPopup.setBackgroundDrawable(new ColorDrawable(_appContext.getResources().getColor(android.R.color.transparent)));
			_subPopup.setOutsideTouchable(true);
			_subPopup.setHeight(WindowManager.LayoutParams.WRAP_CONTENT);

			ListView list = (ListView)sfView.findViewById(android.R.id.list);
			_subPopupListAdapter = new ArrayAdapter<String>(_appContext, R.layout.softmenu_sub_item, R.id.sfSubMenuText);
			list.setAdapter(_subPopupListAdapter);
			list.setOnItemClickListener(new AdapterView.OnItemClickListener()
			{
				public void onItemClick(AdapterView<?> parent, View view, int position, long id)
				{
					_handleSubBtn(position, view);
				}
			});
		}
	}

	public void _handleBtn(int btnIdx, int actionIdx, View v)
	{
		int[] subMenu = kSubMenuActionIds[btnIdx];
		if (subMenu != null)
		{
			showSubMenu(btnIdx, actionIdx);
		}
		else
		{
			_activity.onSoftMenuItemSelected(actionIdx);
			_parentView = null;
			_activity = null;
		}

		_mainPopup.dismiss();
	}

	public void _handleSubBtn(int itemIdx, View v)
	{
		int btnIdx = _curSubGroupIdx;
		int[] subMenu = kSubMenuActionIds[btnIdx];
		if (subMenu != null)
		{
			_activity.onSoftMenuItemSelected(subMenu[itemIdx]);
		}

		_parentView = null;
		_activity = null;

		_subPopup.dismiss();
	}

	void showSubMenu(int btnIdx, int actionIdx)
	{
		_subPopupListAdapter.clear();

		List<String> items = new LinkedList<String>();
		switch (actionIdx)
		{
			case R.id.disk:
			{
				items.add("Insert Floppy A");
				items.add("Insert Floppy B");

				try
				{
					String[] ejectTitle = { "Eject Floppy A", "Eject Floppy B" };
					for (int i = 0; i < 2; ++i)
					{
						String filename = HataroidNativeLib.emulatorGetCurFloppy(i);
						String zipname = HataroidNativeLib.emulatorGetCurFloppyZip(i);

						if (filename.length() == 0 && zipname.length() == 0)
						{
							//item.setEnabled(false);
							items.add(ejectTitle[i]);
						}
						else
						{
							//item.setEnabled(true);
							String [] fileSplit = filename.split("/");
							String t = ejectTitle[i] + " (" + fileSplit[fileSplit.length-1] + (zipname.length()>0 ? ("/" + zipname) : "") + ")";
							items.add(t);
						}
					}
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}

				break;
			}
			case R.id.reset:
			{
				items.add("Cold Reset");
				items.add("Warm Reset");
				break;
			}
			case R.id.savestate:
			{
				items.add("Save");
				items.add("Load");
				items.add("Delete");
				items.add("Select Quick Save Slot");
				break;
			}
		}

		_curSubGroupIdx = btnIdx;
		for (int i = 0; i < items.size(); ++i)
		{
			_subPopupListAdapter.add(items.get(i));
		}

		_subPopup.showAtLocation(_parentView, Gravity.CENTER, 0, 0);
	}

	int[] getScreenDimensions()
	{
		int[] res = new int [2];
		DisplayMetrics displaymetrics = new DisplayMetrics();

		WindowManager wm = (WindowManager)_appContext.getSystemService(Context.WINDOW_SERVICE);
		wm.getDefaultDisplay().getMetrics(displaymetrics);

		res[0] = displaymetrics.widthPixels;
		res[1] = displaymetrics.heightPixels;
		return res;
	}

	public void prepare()
	{
		try
		{
			// update pause/unpause option strings
			{
				View sfView = _mainPopup.getContentView();
				TextView v = (TextView)sfView.findViewWithTag("sfLbl" + (kPauseBtnIdx + 1));
				if (v != null)
				{
					boolean paused = HataroidNativeLib.emulatorGetUserPaused();

					String s = _appContext.getResources().getString(paused ? R.string.Unpause : R.string.Pause);
					v.setText(s);
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public void show(HataroidActivity activity, View parentView, int openSubMenuIdx)
	{
		_activity = activity;
		_parentView = parentView;

		int subGroupBtnIdx = -1;
		switch (openSubMenuIdx)
		{
			case 1: { subGroupBtnIdx = kDiskBtnIdx; break; }
		}
		if (subGroupBtnIdx >= 0 && kSubMenuActionIds[subGroupBtnIdx] != null)
		{
			showSubMenu(subGroupBtnIdx, kMainActionIds[subGroupBtnIdx]);
		}
		else
		{
			//_mainPopup.update(800, 800);
			_mainPopup.showAtLocation(_parentView, Gravity.LEFT | Gravity.BOTTOM, 0, 0);
		}
	}

}
