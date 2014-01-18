package com.RetroSoft.Hataroid.Help;

import java.io.IOException;
import java.io.InputStream;
import java.util.Scanner;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.text.Html;
import android.text.method.ScrollingMovementMethod;
import android.view.Display;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.TextView;

import com.RetroSoft.Hataroid.R;

public class HelpActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_help);
        
		InputStream stream = null;
		String helpText = "";
		try
		{
			stream = getAssets().open("text/help.html");
			Scanner s = new Scanner(stream).useDelimiter("\\A");
		    helpText = s.hasNext() ? s.next() : "";
			stream.close();
		}
		catch (IOException e) { }
	    
		View v = findViewById(R.id.helpTextView);
		if (v != null && v instanceof TextView)
		{
			TextView tv = (TextView)v;
			tv.setText(Html.fromHtml(helpText));
	        tv.setMovementMethod(new ScrollingMovementMethod());
	        //tv.setMovementMethod(new LinkMovementMethod());
	        tv.setClickable(false);
	        tv.setLongClickable(false);
        }
	    
	    findViewById(R.id.help_closeBtn).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				finish();
			}
		});

	    try
	    {
		    Display display = getWindowManager().getDefaultDisplay();
		    int dw = display.getWidth();
		    int dh = display.getHeight();
		    
		    Bitmap src = BitmapFactory.decodeResource(getApplicationContext().getResources(), R.drawable.atari_full);
		    int sw = src.getWidth();
		    int sh = src.getHeight();
		    
		    float saspect = (float)sw/(float)sh;
		    float daspect = (float)dw/(float)dh;
		    
		    int nw = sw;
		    int nh = sh;
		    int px = 0, py = 0;
		    
		    if (daspect > saspect)
		    {
		    	nh = dh;
		    	nw = (int)(dh * saspect);
		    	px = (dw-nw) >> 1;
		    }
		    else
		    {
		    	nw = dw;
		    	nh = (int)(nw / saspect);
		    	px = (dh-nh) >> 1;
		    }
		    
		    Bitmap newBmp = Bitmap.createScaledBitmap(src, nw, nh, true);

		    View bv = findViewById(R.id.helpBkgView);
			if (bv != null && bv instanceof ImageView)
			{
				ImageView iv = (ImageView)bv;
				iv.setImageBitmap(newBmp);
				iv.setPadding(px, py, px, py);
			}
	    }
		catch (Exception e) { }
    }
}
