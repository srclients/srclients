/*
 * 
 */
package com.example.srclient;

import android.widget.ImageView;
import android.widget.SimpleAdapter.ViewBinder;
import android.view.View;
import android.graphics.drawable.Drawable;

// TODO: Auto-generated Javadoc
/**
 * The Class AgendaViewBinder.
 */
public class AgendaViewBinder implements ViewBinder 
{
	
	/**
	 * Instantiates a new agenda view binder.
	 */
	public AgendaViewBinder() {	}
	
	/* (non-Javadoc)
	 * @see android.widget.SimpleAdapter.ViewBinder#setViewValue(android.view.View, java.lang.Object, java.lang.String)
	 */
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
