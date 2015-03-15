package com.RetroSoft.Hataroid.Input;

import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnGenericMotionListener;

public class InputMouse implements OnGenericMotionListener
{
	float		_curMouseX = 0;
	float		_curMouseY = 0;
	
	boolean		_disabled = false;

	public float getMouseX() { return _curMouseX; }
	public float getMouseY() { return _curMouseY; }
	
	public boolean onGenericMotion(View v, MotionEvent e)
	{
		if (_disabled)
		{
			return false;
		}

		int pointerIdx = 0;
		if (e.getToolType(pointerIdx) == MotionEvent.TOOL_TYPE_MOUSE)
		{
			_curMouseX = e.getX(pointerIdx);
			_curMouseY = e.getY(pointerIdx);
			//Log.i("hataroid", "mouse(" + pointerIdx + "): " + _curMouseX + ", " + _curMouseY);
			return true;
		}
		return false;
	}

	public void setEnabled(boolean enabled)
	{
		_disabled = !enabled;
	}
}
