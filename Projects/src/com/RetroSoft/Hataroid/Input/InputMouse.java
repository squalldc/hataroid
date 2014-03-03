package com.RetroSoft.Hataroid.Input;

import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnGenericMotionListener;

public class InputMouse implements OnGenericMotionListener
{
	float		_curMouseX = 0;
	float		_curMouseY = 0;

	public float getMouseX() { return _curMouseX; }
	public float getMouseY() { return _curMouseY; }
	
	public boolean onGenericMotion(View v, MotionEvent e)
	{
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
}
