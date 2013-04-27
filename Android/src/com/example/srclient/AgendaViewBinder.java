package com.example.srclient;

import android.widget.ImageView;
import android.widget.SimpleAdapter.ViewBinder;
import android.view.View;
import android.graphics.drawable.Drawable;

public class AgendaViewBinder implements ViewBinder 
{
	public AgendaViewBinder() {	}
	
	@Override
    public boolean setViewValue(View view, Object data,String textRepresentation) 
    {
    	if((view instanceof ImageView) & (data instanceof Drawable)) 
    	{
    		ImageView iv = (ImageView) view;
    		Drawable drawable = (Drawable) data;
    		iv.setImageDrawable(drawable);
    		return true;
    	}
    	return false;
    }

}
