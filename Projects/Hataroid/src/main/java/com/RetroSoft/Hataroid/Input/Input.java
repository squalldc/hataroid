package com.RetroSoft.Hataroid.Input;

import java.util.Dictionary;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.input.InputManager;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

import com.RetroSoft.Hataroid.Input.InputManagerCompat.InputManagerCompat;
import com.RetroSoft.Hataroid.Util.BitFlags;

public class Input implements InputManagerCompat.InputDeviceListener
{
	public final static int kLocale_EN = 0;
	public final static int kLocale_DE = 1;
	public final static int kLocale_FR = 2;

	public final static String[] kLocaleIDStrs = new String[] {
		"en", "de", "fr"
	};

	public final static int kAxis_X1			= 0;
	public final static int kAxis_Y1			= 1;
	public final static int kAxis_X2			= 2;
	public final static int kAxis_Y2			= 3;
	public final static int kNumAxis			= 4;

	public final static int kPreset_Default		= 0;
	public final static int kPreset_WiiMote		= 1;
	public final static int kPreset_NumOf		= 2;

	public final static String kPrefInputEnableInputMethod = "pref_input_device_enable_inputmethod";
	public final static String kPrefInputKeyboardRegion = "pref_input_keyboard_region";

	public final static String kPresetIDPrefix = "_preset";
	final static String [] kPresetNames = { "Default", "WiiMote" };
	final static String [] kPresetIDs = { kPresetIDPrefix+"Default", kPresetIDPrefix+"WiiMote" };

	final static int [][] kPresets = {
			{ }, //kPreset_Default
			{
				//kPreset_WiiMote
				19, VirtKeyDef.VKB_KEY_JOYUP,		//up
				20, VirtKeyDef.VKB_KEY_JOYDOWN,		//down
				21, VirtKeyDef.VKB_KEY_JOYLEFT,		//left
				22, VirtKeyDef.VKB_KEY_JOYRIGHT,	//right
				8, VirtKeyDef.VKB_KEY_SPACE,		//1
				9, VirtKeyDef.VKB_KEY_JOYFIRE,		//2
				23, VirtKeyDef.VKB_KEY_TURBOSPEED,	//a
				4, -1,								//b
				41, VirtKeyDef.VKB_KEY_SCREENPRESETS,   //-
				44, VirtKeyDef.VKB_KEY_MOUSETOGGLE,	//+
				36, VirtKeyDef.VKB_KEY_ANDROID_MENU,    //home
			}
		};

	public final static int kNumSrcKeyCodes = 256;

	private int         _curLocaleID = kLocale_EN;

	boolean		        _inputEnabled = false;

	InputMap	        _curInputMap = null;
	int []		        _srcToDestMap = null;
	int			        _numSrcInputs = 0;

	int[]               _sysKeysToDestMap = null;

	BitFlags	        _curPresses = new BitFlags(VirtKeyDef.VKB_KEY_NumOf);
	float[]		        _curAxis = new float[kNumAxis];

	boolean		        _hasDirectPresses = false;
	BitFlags	        _directPresses = new BitFlags(VirtKeyDef.VKB_KEY_NumOf);

	InputMouse	        _inputMouse = null;

//	InputManagerCompat  _inputManager = null;

	Context		        _appContext = null;
	View		        _rootView = null;

	public Input()
	{
		_curInputMap = new InputMap();
		_curInputMap.init(kNumSrcKeyCodes, _curLocaleID);
		_cacheInputMapValues();

		{
			// HACK: system keys (TODO: allow option for some src keys to map to multiple dest keys)
			_sysKeysToDestMap = new int[Input.kNumSrcKeyCodes];
			for (int i = 0; i < Input.kNumSrcKeyCodes; ++i) {
				_sysKeysToDestMap[i] = -1;
			}
			_sysKeysToDestMap[KeyEvent.KEYCODE_DPAD_UP]     = VirtKeyDef.VKB_KEY_NAVUP;
			_sysKeysToDestMap[KeyEvent.KEYCODE_DPAD_DOWN]   = VirtKeyDef.VKB_KEY_NAVDOWN;
			_sysKeysToDestMap[KeyEvent.KEYCODE_DPAD_LEFT]   = VirtKeyDef.VKB_KEY_NAVLEFT;
			_sysKeysToDestMap[KeyEvent.KEYCODE_DPAD_RIGHT]  = VirtKeyDef.VKB_KEY_NAVRIGHT;
			_sysKeysToDestMap[KeyEvent.KEYCODE_DPAD_CENTER] = VirtKeyDef.VKB_KEY_NAVBTN;
		}
	}
	
	void _cacheInputMapValues()
	{
		_srcToDestMap = _curInputMap.srcToDestMap;
		_numSrcInputs = _srcToDestMap.length;
	}
	
	public void init(Context appContext)
	{
		_appContext = appContext;

//		try {
//			_inputManager = InputManagerCompat.Factory.getInputManager(_appContext);
//			_inputManager.registerInputDeviceListener(this, null);
//		} catch (Exception e) {
//			_inputManager = null;
//			e.printStackTrace();
//		} catch (Error e) {
//			_inputManager = null;
//			e.printStackTrace();
//		}
	}
	
	public void initMouse(View rootView)
	{
		_rootView = rootView;

		if (android.os.Build.VERSION.SDK_INT >= 12)
		{
			try
			{
				_inputMouse = new InputMouse();
				rootView.setOnGenericMotionListener(_inputMouse);
			}
			catch (Error e)
			{
				_inputMouse = null;
			}
			catch (Exception e)
			{
				_inputMouse = null;
			}
		}
	}

	public void enableNewMouse(boolean enableNewMouse)
	{
		try
		{
			if (_inputMouse != null)
			{
				_inputMouse.setEnabled(enableNewMouse);
			}
		}
		catch (Error e)
		{
		}
		catch (Exception e)
		{
		}
	}

	public int getLocaleID() { return _curLocaleID; }
	public void setLocaleID(String localeStr, boolean updateMaps)
	{
		int newLocaleID = findLocaleID(localeStr);
		if (newLocaleID >= 0 && newLocaleID != _curLocaleID) {
			_curLocaleID = newLocaleID;

			if (updateMaps) {
				// remap changed keys
				if (_curInputMap != null) {
					_curInputMap.autoRemapRegionKeys(_curLocaleID);
				}
			}
		}
	}
	
	public InputMouse getInputMouse() { return _inputMouse; }

	public InputMap getCurInputMap() { return _curInputMap; }

	public int findLocaleID(String localeStr)
	{
		if (localeStr != null && localeStr.length() > 0) {
			for (int i = 0; i < kLocaleIDStrs.length; ++i) {
				if (localeStr.compareTo(kLocaleIDStrs[i]) == 0) {
					return i;
				}
			}
		}
		return -1;
	}

	public void setupOptionsFromPrefs(SharedPreferences prefs)
	{
    	Map<String,?> allPrefs = prefs.getAll();
    	
    	_inputEnabled = false;

		try
		{
			_curLocaleID = kLocale_EN;

			Object val = allPrefs.get(kPrefInputKeyboardRegion);
			if (val != null) {
				int savedLocaleID = findLocaleID(val.toString());
				if (savedLocaleID >= 0) {
					_curLocaleID = savedLocaleID;
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}

    	_curInputMap = new InputMap();
		_curInputMap.init(kNumSrcKeyCodes, _curLocaleID);

		{
			Object val = allPrefs.get(kPrefInputEnableInputMethod);
			if (val != null) {
				String sval = val.toString();
				_inputEnabled = !(sval.compareTo("false") == 0 || sval.compareTo("0") == 0);
			}
		}

    	if (_inputEnabled)
		{
			String lastPresetID = prefs.getString(InputMapConfigureView.kPrefLastPresetIDKey, null);
			if (lastPresetID != null)
			{
				// see if system preset
				if (isSystemPreset(lastPresetID))
				{
					// system preset
					int [] presetArray = getPresetArray(getPresetID(lastPresetID));
					if (presetArray != null)
					{
						_curInputMap = new InputMap();
						_curInputMap.initFromArray(Input.kNumSrcKeyCodes, presetArray, _curLocaleID);

						Log.i("hataroid", "Setting input map: " + lastPresetID);
					}
				}
				else
				{
					String key = InputMapConfigureView.kPrefUserPresetPrefix + lastPresetID;
					String prefStr = prefs.getString(key, null);

					Map<String,Object> result = new HashMap<String,Object>();
					if (Input.decodeInputMapPref(prefStr, _curLocaleID, result))
					{
						_curInputMap = (InputMap)result.get("map");

						Log.i("hataroid", "Setting input map: " + result.get("name") + " (" + lastPresetID + ")");
					}
				}
			}
		}

    	_curPresses.clearAll();

		_hasDirectPresses = false;
		_directPresses.clearAll();

		clearAxis();

		_cacheInputMapValues();
	}

	public boolean dispatchKeyEvent(KeyEvent event, boolean[] softMenuRes)
	{
		softMenuRes[0] = false;

		if (!_inputEnabled)
		{
			return false;
		}
		
		int action = event.getAction();
		int c = event.getKeyCode();
		if (c == KeyEvent.KEYCODE_MENU) // let parent handle
		{
			return false;
		}

		//Log.i("hataroid", "CONFIGURE keydown: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

		if (action == KeyEvent.ACTION_DOWN || action == KeyEvent.ACTION_MULTIPLE)
		{
			if (c >= 0)
			{
				boolean handled = false;
				if (c < _numSrcInputs)
				{
					//InputDevice idev = event.getDevice();
					//if (idev != null)
					//{
					//	Log.i("hataroid", "id: " + idev.getId() + ", n: " + (idev.getName()!=null?idev.getName():""));
					//}

					int im = _srcToDestMap[c];
					if (im >= 0) {
						if (im == VirtKeyDef.VKB_KEY_ANDROID_MENU) {
							softMenuRes[0] = true;
						} else {
							//Log.i("hataroid", "keydown: " + AndroidKeyNames.kKeyCodeNames[c] + ", vkb: " + VirtKeyDef.kDefs[im].getName(0));
							_curPresses.setBit(im);
						}
						handled = true;
					}
				}
				if (c < _sysKeysToDestMap.length)
				{
					int im = _sysKeysToDestMap[c];
					if (im >= 0) {
						if (im == VirtKeyDef.VKB_KEY_ANDROID_MENU) {
							softMenuRes[0] = true;
						} else {
							_curPresses.setBit(im);
						}
						handled = true;
					}
				}

				if (handled) {
					return true;
				}
			}
		}
		else if (action == KeyEvent.ACTION_UP)
		{
			//InputDevice idev = event.getDevice();
			//if (idev != null)
			//{
			//	Log.i("hataroid", "id: " + idev.getId() + ", n: " + (idev.getName()!=null?idev.getName():""));
			//}

			if (c >= 0)
			{
				boolean handled = false;
				if (c < _numSrcInputs)
				{
					int im = _srcToDestMap[c];
					if (im >= 0) {
						if (im == VirtKeyDef.VKB_KEY_ANDROID_MENU) {
							softMenuRes[0] = true;
						} else {
							_curPresses.clearBit(im);
						}
						handled = true;
					}
				}

				if (c < _sysKeysToDestMap.length)
				{
					int im = _sysKeysToDestMap[c];
					if (im >= 0) {
						if (im == VirtKeyDef.VKB_KEY_ANDROID_MENU) {
							softMenuRes[0] = true;
						} else {
							_curPresses.clearBit(im);
						}
						handled = true;
					}
				}

				if (handled) {
					return true;
				}
			}
		}
		
		return false;
	}

//	public void sendBackKeyEvent()
//	{
//		if (_rootView != null)
//		{
//			_rootView.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK));
//			_rootView.dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_BACK));
//		}
//	}

	public boolean dispatchGenericMotionEvent(MotionEvent event)
	{
//		if (_inputManager != null) {
//			try {
//				_inputManager.onGenericMotionEvent(event);
//			} catch (Exception e) {
//				e.printStackTrace();
//			} catch (Error e) {
//				e.printStackTrace();
//			}
//		}

		// Check that the event came from a game controller
		if ((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)
				//&& event.getAction() == MotionEvent.ACTION_MOVE)
		{
			// Process all historical movement samples in the batch
			{
				//final int historySize = event.getHistorySize();

				// Process the movements starting from the earliest historical position in the batch
				//for (int i = 0; i < historySize; i++) {
				//	processJoystickInput(event, i); // Process the event at historical position i
				//}
			}

			// Process the current movement sample in the batch (position -1)
			processJoystickInput(event, -1);
			//return true;
			return false; // allow input to go through normal input as well
		}

		return false;
	}

	private static float getCenteredAxis(MotionEvent event, InputDevice device, int axis, int historyPos)
	{
		final InputDevice.MotionRange range = device.getMotionRange(axis, event.getSource());

		// A joystick at rest does not always report an absolute position of (0,0).
		// Use the getFlat() method to determine the range of values bounding the joystick axis center.
		if (range != null)
		{
			final float kMinFlat = 0.2f; // TODO: user option
			float flat = range.getFlat();
			if (flat < kMinFlat) { flat = kMinFlat; }

			final float value =  historyPos < 0 ? event.getAxisValue(axis) : event.getHistoricalAxisValue(axis, historyPos);

			// Ignore axis values that are within the 'flat' region of the joystick axis center.
			float maxVal = range.getMax();
			float rangeVal = maxVal - flat;
			if (rangeVal > 0.0f)
			{
				// return nomalized value
				if (value > flat)
				{
					return (value - flat) / rangeVal;
				}
				else if (value < -flat)
				{
					return (value + flat) / rangeVal;
				}
			}
		}
		return 0;
	}

	private void processJoystickInput(MotionEvent event, int historyPos)
	{
		InputDevice mInputDevice = event.getDevice();
		if (mInputDevice == null)
		{
			clearAxis();
		}
		else
		{
			_curAxis[kAxis_X1] = getCenteredAxis(event, mInputDevice, MotionEvent.AXIS_X, historyPos);
			_curAxis[kAxis_Y1] = getCenteredAxis(event, mInputDevice, MotionEvent.AXIS_Y, historyPos);
			_curAxis[kAxis_X2] = getCenteredAxis(event, mInputDevice, MotionEvent.AXIS_Z, historyPos);
			_curAxis[kAxis_Y2] = getCenteredAxis(event, mInputDevice, MotionEvent.AXIS_RZ, historyPos);
		}
	}

	public void clearAxis()
	{
		for (int i = 0; i < kNumAxis; ++i)
		{
			_curAxis[i] = 0;
		}
	}

	/*
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (!_inputEnabled)
		{
			return false;
		}

		int c = event.getKeyCode();
		//Log.i("hataroid", "keydown: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());

		if (c >= 0 && c < _numSrcInputs)
		{
			//InputDevice idev = event.getDevice();
			//if (idev != null)
			//{
			//	Log.i("hataroid", "id: " + idev.getId() + ", n: " + (idev.getName()!=null?idev.getName():""));
			//}
			
			int im = _srcToDestMap[c];
			if (im >= 0)
			{
				_curPresses.setBit(im);
				return true;
			}
		}
		
		return false;
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		if (!_inputEnabled)
		{
			return false;
		}

		int c = event.getKeyCode();
		//Log.i("hataroid", "keyup: " + String.valueOf(c) + ", sc: " + event.getScanCode() + ", id: " + event.getDeviceId());
		
		if (c >= 0 && c < _numSrcInputs)
		{
			//InputDevice idev = event.getDevice();
			//if (idev != null)
			//{
			//	Log.i("hataroid", "id: " + idev.getId() + ", n: " + (idev.getName()!=null?idev.getName():""));
			//}
	
			int im = _srcToDestMap[c];
			if (im >= 0)
			{
				_curPresses.clearBit(im);
				return true;
			}
		}

		return false;
	}
	*/
	
	public BitFlags getKeyPresses()
	{
		return _curPresses;
	}
	public float[] getCurAxis()
	{
		return _curAxis;
	}

	public boolean hasDirectPresses()	{ return _hasDirectPresses; }
	public BitFlags getDirectPresses()	{ return _directPresses; }
	public void clearDirectPresses()	{ _directPresses.clearAll(); _hasDirectPresses = false; }
	public void setDirectPress(int virtKeyID)
	{
		_directPresses.setBit(virtKeyID);
		_hasDirectPresses = true;
	}
	
	public void showInputMethodSelector()
	{
		InputMethodManager imm = (InputMethodManager)_appContext.getSystemService(Context.INPUT_METHOD_SERVICE);
		imm.showInputMethodPicker();
	}
	
	public static int getPresetID(String presetName)
	{
		for (int i = 0; i < kPreset_NumOf; ++i)
		{
			if (kPresetIDs[i].compareTo(presetName) == 0)
			{
				return i;
			}
		}
		return -1;
	}
	
	public static String getPresetName(int presetID)
	{
		if (presetID >=0 && presetID < kPreset_NumOf)
		{
			return kPresetNames[presetID];
		}
		return null;
	}
	
	public static String getPresetID(int presetID)
	{
		if (presetID >=0 && presetID < kPreset_NumOf)
		{
			return kPresetIDs[presetID];
		}
		return null;
	}

	public static int [] getPresetArray(int presetID)
	{
		if (presetID >=0 && presetID < kPreset_NumOf)
		{
			return kPresets[presetID];
		}
		return null;
	}
	
	public static boolean isSystemPreset(String presetID)
	{
		if (presetID != null)
		{
			return presetID.startsWith(Input.kPresetIDPrefix);
		}
		
		return false;
	}
	
	public static boolean decodeInputMapPref(String prefVal, int localeID, Map<String,Object> result)
	{
		if (prefVal == null)
		{
			return false;
		}

		boolean error = false;
		try
		{
			String [] info = prefVal.split(",");
			if (info.length == 0)
			{
				error = true;
			}
			else
			{
				InputMap map = new InputMap();
				map.init(Input.kNumSrcKeyCodes, localeID);
				map.clear();

				String name = info[0].trim();
				for (int i = 1; i < info.length-1; i += 2)
				{
					int srcKey = Integer.parseInt(info[i].toString().trim());
					int destKey = Integer.parseInt(info[i+1].toString().trim());
					map.addKeyMapEntry(srcKey, destKey);
				}

				map.autoRemapRegionKeys(localeID);
				
				result.put("name", name);
				result.put("map", map);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			error = true;
		}
		
		return !error;
	}
	
//	public static String getUserInputMaps(SharedPreferences prefs)
//	{
//		String result = "";
//    	Map<String,?> allPrefs = prefs.getAll();
//		for (Map.Entry<String,?> entry : allPrefs.entrySet())
//		{
//			String key = entry.getKey();
//			if (key.startsWith(InputMapConfigureView.kPrefUserPresetPrefix))
//			{
//				result += entry.getValue().toString() + "\n";
//			}
//		}
//		return result;
//	}

	public static void storeEnableInputMap(boolean enabled, Context ctx)
	{
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(ctx);

		SharedPreferences.Editor ed = prefs.edit();
		ed.putBoolean(kPrefInputEnableInputMethod, enabled);
		ed.commit();
	}

	public Map<String,Integer>       _devDescJoyMap = new LinkedHashMap<String, Integer>();
	public Map<Integer,Integer>      _devIDJoyMap = new LinkedHashMap<Integer, Integer>();

	void _resetDevJoyMap() {
		_devDescJoyMap.clear();
		_devIDJoyMap.clear();
	}

	// 0 = joystick player 1, 1 = joystick player 2
	int _getJoyPlayer(int deviceId) {

		Integer playerID = _devIDJoyMap.get(deviceId);
		if (playerID == null) {
			// try to map device descripter to current deviceId
			//if (_devDescJoyMap.containsKey()) {
			//}
		}

		if (playerID == null || _devIDJoyMap.size() <= 1) { // always return 1st player if only one device connected
			return 0;
		}
		return playerID.intValue();
	}

	@Override public void onInputDeviceAdded(int deviceId) {
//		if (_inputManager != null) {
//			InputDevice idev = _inputManager.getInputDevice(deviceId);
//			Log.i("hataroid", "input device added. id: " + deviceId + ", desc: " + _inputManager.getDeviceDescriptor(idev));
//		}
	}

	// This is an unusual case. Input devices don't typically change, but they
	// certainly can --- for example a device may have different modes.
	@Override public void onInputDeviceChanged(int deviceId) {
//		if (_inputManager != null) {
//			InputDevice idev = _inputManager.getInputDevice(deviceId);
//			Log.i("hataroid", "input device changed. id: " + deviceId + ", desc: " + _inputManager.getDeviceDescriptor(idev));
//		}
	}


	@Override  public void onInputDeviceRemoved(int deviceId) {
//		if (_inputManager != null) {
//			InputDevice idev = _inputManager.getInputDevice(deviceId);
//			Log.i("hataroid", "input device removed. id: " + deviceId + ", desc: " + _inputManager.getDeviceDescriptor(idev));
//		}
	}

	public void onPause()
	{
//		try {
//			if (_inputManager != null) {
//				_inputManager.onPause();
//			}
//		} catch (Exception e) {
//			e.printStackTrace();
//		} catch (Error e) {
//			e.printStackTrace();
//		}
	}

	public void onResume()
	{
//		try {
//			if (_inputManager != null) {
//				_inputManager.onResume();
//			}
//		} catch (Exception e) {
//			e.printStackTrace();
//		} catch (Error e) {
//			e.printStackTrace();
//		}
	}
}
