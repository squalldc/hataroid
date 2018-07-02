package com.RetroSoft.Hataroid.SoftMenu;

import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.HataroidActivity;
import com.RetroSoft.Hataroid.HataroidNativeLib;
import com.RetroSoft.Hataroid.Input.InputMap;
import com.RetroSoft.Hataroid.R;
import com.RetroSoft.Hataroid.Mod.CustomPopup;

import java.util.LinkedList;
import java.util.List;

public class SoftMenu
{
	final int kDiskBtnIdx = 0;
	final int kPauseBtnIdx = 3;
	final int kMouseJoyIdx = 7;
	final int kTurboIdx = 9;

	final int[] kMainActionIds = new int []
	{
		R.id.disk,
		R.id.reset,
		R.id.savestate,
		R.id.pause,
		R.id.settings,
		R.id.quit,
		R.id.help,
		R.id.mousejoy,
		R.id.keybd,
		R.id.speedtoggle,
		R.id.screensize,
		R.id.shortcuts,
		0, // unused (padding)
		0, // unused (padding)
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

		null,//R.id.mousejoy,
		null,//R.id.keybd,
		null,//R.id.speedtoggle,
		new int[] //R.id.screensize,
		{
			R.id.screen_preset_0,
			R.id.screen_preset_1,
			R.id.screen_preset_2,
			R.id.screen_preset_3,
			R.id.screen_preset_4,
			R.id.screen_preset_5,
		},
		new int[] //R.id.shortcuts,
		{
			R.id.skey_1,
			R.id.skey_2,
			R.id.skey_3,
			R.id.skey_4,
			R.id.skey_5,
		},
		null,//unused (padding)
		null,//unused (padding)
	};

	Context					_appContext;

	HataroidActivity		_activity;
	View					_parentView;
	InputMap				_inputMap = null;

	CustomPopup				_mainPopup;

	CustomPopup				_subPopup;
	ArrayAdapter<String>	_subPopupListAdapter;
	int						_curSubGroupIdx;

	boolean                 _unpauseEmuOnDismiss = true;

//	boolean					_isShowing = false;

	public void create(Context context, boolean showShortcuts)
	{
		_appContext = context;

		int[] screenRes = getScreenDimensions();
		int scrWidth = screenRes[0];
		int scrHeight = screenRes[1];

		LayoutInflater inflater = (LayoutInflater) _appContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

		// main menu
		{
			//View sfView = inflater.inflate(showShortcuts ? R.layout.softmenutv_view : R.layout.softmenu_view, null);
			View sfView = inflater.inflate(R.layout.softmenutv_view, null);

			int popupWidth = (int)(scrWidth);
			//int popupHeight = (int)(scrHeight * (showShortcuts?0.5f:0.25f));
			int popupHeight = (int)(scrHeight * 0.5f);

			_mainPopup = new CustomPopup(sfView, popupWidth, popupHeight, true);
			_mainPopup.setBackgroundDrawable(new ColorDrawable(_appContext.getResources().getColor(android.R.color.transparent)));
			_mainPopup.setOutsideTouchable(true);
			_mainPopup.setHeight(WindowManager.LayoutParams.WRAP_CONTENT);

			_mainPopup.setOnDismissListener(new CustomPopup.OnDismissListener() {
				public void onDismiss() { _onMainPopupDismiss(); }
			});

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
					if (actionIdx != 0)
					{
						b.setOnClickListener(new View.OnClickListener() {
							public void onClick(View v) {
								_handleBtn(btnIdx, actionIdx, v);
							}
						});
						b.setOnFocusChangeListener(new View.OnFocusChangeListener() {
							public void onFocusChange(View v, boolean hasFocus) {
								_handleFocusChange(btnIdx, actionIdx, v, hasFocus);
							}
						});
					}
					else
					{
						b.setVisibility(View.INVISIBLE);
					}
				}

				TextView tv = (TextView)sfView.findViewWithTag("sfLbl" + (i + 1));
				if (tv != null)
				{
					tv.setWidth(maxBtnWidth);
					if (actionIdx != 0)
					{
						tv.setOnClickListener(new View.OnClickListener() {
						public void onClick(View v) {
							_handleBtn(btnIdx, actionIdx, v);
						}
					});
					}
					else
					{
						tv.setVisibility(View.INVISIBLE);
					}
				}
			}
		}

		// sub menu
		{
			View sfView = inflater.inflate(R.layout.softmenu_sub_view, null);

			int popupWidth = (int)(scrWidth * 0.62f);
			int popupHeight = (int)(scrHeight * 0.8f);

			_subPopup = new CustomPopup(sfView, popupWidth, popupHeight, true);
			_subPopup.setBackgroundDrawable(new ColorDrawable(_appContext.getResources().getColor(android.R.color.transparent)));
			_subPopup.setOutsideTouchable(true);
			_subPopup.setHeight(WindowManager.LayoutParams.WRAP_CONTENT);

			_subPopup.setOnDismissListener(new CustomPopup.OnDismissListener() {
				public void onDismiss() { _onSubPopupDismiss(); }
			});

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

	public void _handleFocusChange(int btnIdx, int actionIdx, View v, Boolean hasFocus)
	{
		try {
			ImageButton b = (v != null && v instanceof ImageButton) ? (ImageButton) v : null;
			if (hasFocus) {
				b.setBackgroundColor(Color.argb(204, 255, 165, 30));
			} else {
				b.setBackgroundColor(Color.argb(204, 0, 0, 0));
			}
		}
		catch (Exception e)
		{
		}

		//Log.i("hataroid", "****** On Focus Change: btn: " + btnIdx + " action: " + actionIdx +", hasFocus: " + hasFocus);
	}

	public void _onMainPopupDismiss()
	{
		if (_unpauseEmuOnDismiss)
		{
			HataroidNativeLib.emulatorSetEmuPausedSoftMenu(false);
		}
	}

	public void _onSubPopupDismiss()
	{
		if (_unpauseEmuOnDismiss)
		{
			HataroidNativeLib.emulatorSetEmuPausedSoftMenu(false);
		}
	}

	public void _handleBtn(int btnIdx, int actionIdx, View v)
	{
		int[] subMenu = kSubMenuActionIds[btnIdx];
		if (subMenu != null)
		{
			_unpauseEmuOnDismiss = false;
			showSubMenu(btnIdx, actionIdx);
		}
		else
		{
			_unpauseEmuOnDismiss = true;
			_activity.onSoftMenuItemSelected(actionIdx);
			_parentView = null;
			_activity = null;
		}

		_mainPopup.dismiss();

		_unpauseEmuOnDismiss = true;
	}

	public void _handleSubBtn(int itemIdx, View v)
	{
		int btnIdx = _curSubGroupIdx;
		int[] subMenu = kSubMenuActionIds[btnIdx];
		if (subMenu != null)
		{
			_activity.onSoftMenuItemSelected(subMenu[itemIdx]);
		}
		_unpauseEmuOnDismiss = true;

		_parentView = null;
		_activity = null;

		_subPopup.dismiss();

		_unpauseEmuOnDismiss = true;
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
			case R.id.screensize:
			{
				items.add("Best Fit");
				items.add("Stretch");
				items.add("x1");
				items.add("x2");
				items.add("x3");
				items.add("x4");
				break;
			}
			case R.id.shortcuts:
			{
				items.add("Y");
				items.add("N");
				items.add("1");
				items.add("2");
				items.add("Space");
				break;
			}
		}

		_curSubGroupIdx = btnIdx;
		for (int i = 0; i < items.size(); ++i)
		{
			_subPopupListAdapter.add(items.get(i));
		}

		_subPopup.showAtLocation(_parentView, Gravity.CENTER, 0, 0);
		_setInputMap(_subPopup, _inputMap);
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

	LinearLayout        _tvMenuImageLayout = null;
	LinearLayout        _tvMenuTextLayout = null;

	public void prepare(boolean showShortcuts)
	{
		try
		{
			{
				View sfView = _mainPopup.getContentView();
				LinearLayout ml = (LinearLayout)sfView.findViewWithTag("mainLayout");

				if (ml != null) {
					if (showShortcuts) {
						if (_tvMenuImageLayout != null) {
							ml.addView(_tvMenuImageLayout, 1);
							_tvMenuImageLayout = null;
						}
						if (_tvMenuTextLayout != null) {
							ml.addView(_tvMenuTextLayout, 2);
							_tvMenuTextLayout = null;
						}
					} else {
						if (_tvMenuImageLayout == null) {
							LinearLayout ll = (LinearLayout)ml.findViewWithTag("tvMenuImages");
							if (ll != null) {
								_tvMenuImageLayout = ll;
								ml.removeView(ll);
							}
						}
						if (_tvMenuTextLayout == null) {
							LinearLayout ll = (LinearLayout)ml.findViewWithTag("tvMenuText");
							if (ll != null) {
								_tvMenuTextLayout = ll;
								ml.removeView(ll);
							}
						}
					}
				}

				_mainPopup.setHeight(WindowManager.LayoutParams.WRAP_CONTENT);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

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

			// update mouse/joy toggle
			{
				View sfView = _mainPopup.getContentView();
				ImageButton b = (ImageButton)sfView.findViewWithTag("sfBtn" + (kMouseJoyIdx + 1));
				boolean mouseActive = HataroidNativeLib.emulatorGetMouseActive();
				if (b != null)
				{
					if (mouseActive)
					{
						b.setImageResource(R.drawable.st_joy);
					}
					else
					{
						b.setImageResource(R.drawable.st_mouse);
					}
				}

				TextView v = (TextView)sfView.findViewWithTag("sfLbl" + (kMouseJoyIdx + 1));
				if (v != null)
				{
					String s = _appContext.getResources().getString(mouseActive ? R.string.SetJoystick : R.string.SetMouse);
					v.setText(s);
				}
			}

			// update speed toggle
			{
				View sfView = _mainPopup.getContentView();
				ImageButton b = (ImageButton)sfView.findViewWithTag("sfBtn" + (kTurboIdx + 1));
				boolean turboActive = HataroidNativeLib.emulatorGetTurboMode();
				if (b != null)
				{
					if (turboActive)
					{
						b.setImageResource(R.drawable.spd_normal);
					}
					else
					{
						b.setImageResource(R.drawable.spd_turbo);
					}
				}

				TextView v = (TextView)sfView.findViewWithTag("sfLbl" + (kTurboIdx + 1));
				if (v != null)
				{
					String s = _appContext.getResources().getString(turboActive ? R.string.SetNormalSpeed : R.string.SetTurboSpeed);
					v.setText(s);
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}

//	public boolean isShowing()
//	{
//		return _isShowing;
//	}
//
//	public void hide()
//	{
//		_isShowing = false;
//
//		if (_mainPopup.isShowing())
//		{
//			_mainPopup.dismiss();
//		}
//		if (_subPopup.isShowing())
//		{
//			_subPopup.dismiss();
//		}
//
//		_parentView = null;
//		_activity = null;
//	}

	public void show(HataroidActivity activity, View parentView, int openSubMenuIdx, InputMap inputMap)
	{
//		_isShowing = true;

		HataroidNativeLib.emulatorSetEmuPausedSoftMenu(true);
		_unpauseEmuOnDismiss = true;

		_inputMap = inputMap;
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
			_setInputMap(_mainPopup, _inputMap);
		}
	}

	void _setInputMap(CustomPopup customPopup, InputMap inputMap)
	{
		if (customPopup != null)
		{
			customPopup.setInputMap(inputMap);
		}
	}

}
