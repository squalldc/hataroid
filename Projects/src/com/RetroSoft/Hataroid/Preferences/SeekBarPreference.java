// from http://robobunny.com/wp/2013/08/24/android-seekbar-preference-v2/

package com.RetroSoft.Hataroid.Preferences;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.preference.Preference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class SeekBarPreference extends Preference implements OnSeekBarChangeListener {
	
	private final String TAG = getClass().getName();
	
	private static final String ANDROIDNS="http://schemas.android.com/apk/res/android";
	private static final String APPLICATIONNS="http://robobunny.com";
	private static final int DEFAULT_VALUE = 50;
	
	private int mMaxValue      = 100;
	private int mMinValue      = 0;
	private int mInterval      = 1;
	private int mCurrentValue;
	private String mUnitsLeft  = "";
	private String mUnitsRight = "";
	private SeekBar mSeekBar;
	
	private TextView mStatusText;

	boolean _storeValue = false;

	public SeekBarPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		initPreference(context, attrs);
	}

	public SeekBarPreference(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		initPreference(context, attrs);
	}

	private void initPreference(Context context, AttributeSet attrs) {
		setValuesFromXml(attrs);
		mSeekBar = new SeekBar(context, attrs);
		mSeekBar.setMax(mMaxValue - mMinValue);
		mSeekBar.setOnSeekBarChangeListener(this);
		
		setWidgetLayoutResource(R.layout.seek_bar_preference);
	}
	
	private void setValuesFromXml(AttributeSet attrs) {
		// try int
		boolean tryString = false;
		try { mMaxValue = attrs.getAttributeIntValue(ANDROIDNS, "max", 100); } catch (Exception e) { tryString = true; }
		if (tryString) { try { mMaxValue = (attrs.getAttributeValue(ANDROIDNS, "max")!=null) ? Integer.parseInt(attrs.getAttributeValue(ANDROIDNS, "max")) : 100; } catch (Exception e) { } }

		tryString = false;
		try { mMinValue = attrs.getAttributeIntValue(APPLICATIONNS, "min", 0); } catch (Exception e) { tryString = true; }
		if (tryString) { try { mMinValue = (attrs.getAttributeValue(APPLICATIONNS, "min")!=null) ? Integer.parseInt(attrs.getAttributeValue(APPLICATIONNS, "min")) : 0; } catch (Exception e) { } }
		
		mUnitsLeft = getAttributeStringValue(attrs, APPLICATIONNS, "unitsLeft", "");
		String units = getAttributeStringValue(attrs, APPLICATIONNS, "units", "");
		mUnitsRight = getAttributeStringValue(attrs, APPLICATIONNS, "unitsRight", units);
		
		try {
			String newInterval = attrs.getAttributeValue(APPLICATIONNS, "interval");
			if(newInterval != null)
				mInterval = Integer.parseInt(newInterval);
		}
		catch(Exception e) {
			Log.e(TAG, "Invalid interval value", e);
		}
		
	}
	
	private String getAttributeStringValue(AttributeSet attrs, String namespace, String name, String defaultValue) {
		String value = attrs.getAttributeValue(namespace, name);
		if(value == null)
			value = defaultValue;
		
		return value;
	}
	
	@Override
	protected View onCreateView(ViewGroup parent) {
		View view = super.onCreateView(parent);
		
		// The basic preference layout puts the widget frame to the right of the title and summary,
		// so we need to change it a bit - the seekbar should be under them.
		LinearLayout layout = (LinearLayout) view;
		layout.setOrientation(LinearLayout.VERTICAL);
		
		return view;
	}
	
	@Override
	public void onBindView(View view) {
		super.onBindView(view);

		try {
			// move our seekbar to the new view we've been given
			ViewParent oldContainer = mSeekBar.getParent();
			ViewGroup newContainer = (ViewGroup) view.findViewById(R.id.seekBarPrefBarContainer);
			
			if (oldContainer != newContainer) {
				// remove the seekbar from the old view
				if (oldContainer != null) {
					((ViewGroup) oldContainer).removeView(mSeekBar);
				}
				// remove the existing seekbar (there may not be one) and add ours
				newContainer.removeAllViews();
				newContainer.addView(mSeekBar, ViewGroup.LayoutParams.FILL_PARENT,
				ViewGroup.LayoutParams.WRAP_CONTENT);
			}
		}
		catch(Exception ex) {
			Log.e(TAG, "Error binding view: " + ex.toString());
		}
		
		//if dependency is false from the beginning, disable the seek bar
		if (view != null && !view.isEnabled())
		{
			mSeekBar.setEnabled(false);
		}
		
		updateView(view);
	}
    
    	/**
	 * Update a SeekBarPreference view with our current state
	 * @param view
	 */
	protected void updateView(View view) {

		try {
			mStatusText = (TextView) view.findViewById(R.id.seekBarPrefValue);

			mStatusText.setText(String.valueOf(mCurrentValue));
			mStatusText.setMinimumWidth(30);
			
			mSeekBar.setProgress(mCurrentValue - mMinValue);

			TextView unitsRight = (TextView)view.findViewById(R.id.seekBarPrefUnitsRight);
			unitsRight.setText(mUnitsRight);
			unitsRight.setTextColor(Color.WHITE);
			
			TextView unitsLeft = (TextView)view.findViewById(R.id.seekBarPrefUnitsLeft);
			unitsLeft.setText(mUnitsLeft);
			unitsLeft.setTextColor(Color.WHITE);
		}
		catch(Exception e) {
			Log.e(TAG, "Error updating seek bar preference", e);
		}
		
	}
	
	//@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		if (mSeekBar != seekBar)
		{
			return;
		}

		int newValue = progress + mMinValue;
		
		if(newValue > mMaxValue)
			newValue = mMaxValue;
		else if(newValue < mMinValue)
			newValue = mMinValue;
		else if(mInterval != 1 && newValue % mInterval != 0)
			newValue = Math.round(((float)newValue)/mInterval)*mInterval;  
		
		// change rejected, revert to the previous value
		if(!callChangeListener(newValue)){
			seekBar.setProgress(mCurrentValue - mMinValue); 
			return; 
		}

		// change accepted, store it
		mCurrentValue = newValue;
		mStatusText.setText(String.valueOf(newValue));
		
		//boolean tryString = false;
		//try { persistInt(newValue); } catch (Exception e) { tryString = true; }
		//if (tryString) { try { persistString(String.valueOf(newValue)); } catch (Exception e) { }}
		
		_storeValue = true;
	}
	
	//@Override
	public void onStartTrackingTouch(SeekBar seekBar) {}

	//@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		if (seekBar == mSeekBar)
		{
			notifyChanged();
			
			if (_storeValue)
			{
				boolean tryString = false;
				try { persistInt(mCurrentValue); } catch (Exception e) { tryString = true; }
				if (tryString) { try { persistString(String.valueOf(mCurrentValue)); } catch (Exception e) { }}
				_storeValue = false;
			}
		}
	}


	@Override 
	protected Object onGetDefaultValue(TypedArray ta, int index){
		int defaultValue = 0;
		boolean tryString = false;
		try { defaultValue = ta.getInteger(index, DEFAULT_VALUE); } catch (Exception e) { tryString = true; }
		if (tryString) { try { defaultValue = (ta.getString(index)!=null) ? Integer.parseInt(ta.getString(index)) : DEFAULT_VALUE; } catch (Exception e) {}}

		return defaultValue;
	}

	@Override
	protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {

		if(restoreValue) {
			boolean tryString = false;
			try { mCurrentValue = getPersistedInt(mCurrentValue); } catch (Exception e) { tryString = true; }
			if (tryString) { try { mCurrentValue = Integer.parseInt(getPersistedString(String.valueOf(mCurrentValue))); } catch (Exception e) {} }
		}
		else {
			boolean tryString = false;

			int temp = 0;
			try {
				temp = (Integer)defaultValue;
				persistInt(temp);
				mCurrentValue = temp;
			}
			catch(Exception ex) {
				tryString = true;
				Log.e(TAG, "Invalid default value: " + defaultValue.toString());
			}
			
			if (tryString)
			{
				try {
					temp = Integer.parseInt((String)defaultValue);
					persistString(String.valueOf(temp));
					mCurrentValue = temp;
				}
				catch(Exception ex) {
				}
			}
		}
		
	}
	
	/**
	* make sure that the seekbar is disabled if the preference is disabled
	*/
	@Override
	public void setEnabled(boolean enabled) {
		super.setEnabled(enabled);
		mSeekBar.setEnabled(enabled);
	}
	
	@Override
	public void onDependencyChanged(Preference dependency, boolean disableDependent) {
		super.onDependencyChanged(dependency, disableDependent);
		
		//Disable movement of seek bar when dependency is false
		if (mSeekBar != null)
		{
			mSeekBar.setEnabled(!disableDependent);
		}
	}
}
