package com.marcogiorgini.Zaxxon;


import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.KeyEvent;
import android.view.MotionEvent;

class GameView extends GLSurfaceView 
{
	public GameRenderer mGameRenderer = null;
	
	private int touchX[] = new int[10];
	private int touchY[] = new int[10];
	private int touchStatus[] = new int[10];
	
	public GameView(Context context){
		super(context);
			
		for (int i=0; i<10; i++)
		{
			touchX[i] = 0;
			touchY[i] = 0;
			touchStatus[i] = 0;
		} 
		
		mGameRenderer = new GameRenderer();
		setRenderer(mGameRenderer);

		setFocusable(true);
		setFocusableInTouchMode(true);
		
	}
	
	@Override
	public void onPause()
	{
		super.onPause();
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) 
	{
		int qKeyCode = androidKeyCodeToQk(keyCode, event);
		if ((keyCode==KeyEvent.KEYCODE_VOLUME_DOWN)||(keyCode==KeyEvent.KEYCODE_VOLUME_UP))
			return super.onKeyDown(keyCode, event);
		else
			return queueKeyEvent(qKeyCode, 1);
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) 
	{
		int qKeyCode = androidKeyCodeToQk(keyCode, event);
		return queueKeyEvent(qKeyCode, 0);
	}
	
	@SuppressWarnings("deprecation")
	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
	   int action = event.getAction();
	   int ptrId = event.getPointerId(0);
	   if(event.getPointerCount() > 1)
	      ptrId = (action & MotionEvent.ACTION_POINTER_ID_MASK) >> MotionEvent.ACTION_POINTER_ID_SHIFT;
	   action = action & MotionEvent.ACTION_MASK;
	   if(action < 7 && action > 4)
	      action = action - 5;
	 
	   if( action == MotionEvent.ACTION_DOWN )
	   {
	      for( int i = 0; i < event.getPointerCount(); i++ )
	      {
	         float x = event.getX(i);
	         float y = event.getY(i);
	 
	         int pid = event.getPointerId(i);
	         if ((pid>=0)&&(pid<10))
	         {
	        	 touchX[pid] = (int)x;
	        	 touchY[pid] = (int)y;
	         }
	      }
	      if ((ptrId>=0)&&(ptrId<10))
	    	  touchStatus[ptrId] = 1;
	   }
	   if( action == MotionEvent.ACTION_MOVE )
	   {
	      for( int i = 0; i < event.getPointerCount(); i++ )
	      {
	         float x = event.getX(i);
	         float y = event.getY(i);
	 
	         int pid = event.getPointerId(i);
	         if ((pid>=0)&&(pid<10))
	         {
	        	 touchX[pid] = (int)x;
	        	 touchY[pid] = (int)y;
	         }
	      }
	      if ((ptrId>=0)&&(ptrId<10))
	    	  touchStatus[ptrId] = 2;
	   }
	   if( action == MotionEvent.ACTION_UP )
	   {
		   if( event.getPointerCount() == 1 )
	         for( int i = 0; i < 10; i++ )
	            touchStatus[i] = 0;
	      if ((ptrId>=0)&&(ptrId<10))
	    	  touchStatus[ptrId] = 3;
	   }
	   if( action == MotionEvent.ACTION_CANCEL )
	   {
	      if( event.getPointerCount() == 1 )
	         for( int i = 0; i < 10; i++ )
	            touchStatus[i] = 0;
	      if ((ptrId>=0)&&(ptrId<10))
	    	  touchStatus[ptrId] = 3;
	   }
	   return queueMotionEvent(touchStatus, touchX, touchY);
	}
	
	@Override
	public boolean onTrackballEvent(MotionEvent event) 
	{
		return queueTrackballEvent(event.getAction(), event.getX(), event.getY());
	}

	public boolean queueKeyEvent(final int qKeyCode, final int state)
	{
		if (qKeyCode == 0) 
			return true;

		queueEvent(new Runnable()
		{
            public void run() 
            {
        		GameJNI.keyEvent(qKeyCode, state);
            }
		});
		return true;
	}

	private boolean queueMotionEvent(final int[] action, final int x[], final int y[])
	{
		queueEvent(new Runnable()
		{
            public void run() 
            {
        		for (int i=0; i<10; i++)
            	{
            		GameJNI.touchEvent(i, action[i], x[i], y[i]);
            	}
            }
		});
        return true;
	}
	
	private boolean queueTrackballEvent(final int action, final float x, final float y)
	{
		return true;
	}

	private static final int QK_HOME = 3;
	private static final int QK_BACK = 4;
	
	private static final int QK_ENTER = 13;
	//private static final int QK_ESCAPE = 27;
	private static final int QK_BACKSPACE = 127;
	private static final int QK_LEFT = 134;
	private static final int QK_RIGHT = 135;
	private static final int QK_UP = 132;
	private static final int QK_DOWN = 133;
	private static final int QK_CTRL = 137;
	private static final int QK_SHIFT = 138;
	private static final int QK_CONSOLE = 340;

	private static final int QK_F1 = 145;
	private static final int QK_F2 = 146;
	private static final int QK_F3 = 147;
	
	private int androidKeyCodeToQk(int aKeyCode, KeyEvent event)
	{	
		/* Convert non-ASCII keys by hand */
		switch(aKeyCode)
		{
			/* For now map the focus buttons to F1 and let the user remap it in game.
			 * This should allow some basic movement on the Nexus One if people map it to forward.
			 * At least on the Milestone the camera button itself is shared with the Focus one. You have
			 * to press focus first and then you hit camera, this leads to the following event sequence which
			 * I don't handle right now: focus_down -> camera_down -> camera_up -> focus_up.
			 */
			case KeyEvent.KEYCODE_FOCUS:
				return QK_F1;
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				return QK_F2;
			case KeyEvent.KEYCODE_VOLUME_UP:
				return QK_F3;
			case KeyEvent.KEYCODE_DPAD_UP:
				return QK_UP;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				return QK_DOWN;
			case KeyEvent.KEYCODE_DPAD_LEFT:
				return QK_LEFT;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				return QK_RIGHT;
			case KeyEvent.KEYCODE_DPAD_CENTER:
				/* Center is useful for shooting if you only use the keyboard */
				return QK_CTRL;
			case KeyEvent.KEYCODE_ENTER:
				return QK_ENTER;
			case KeyEvent.KEYCODE_SEARCH:
				return QK_CONSOLE;
			//case KeyEvent.KEYCODE_BACK:
			//	return QK_ESCAPE;
			case KeyEvent.KEYCODE_DEL:
				return QK_BACKSPACE;
			case KeyEvent.KEYCODE_ALT_LEFT:
				return QK_CTRL;
			case KeyEvent.KEYCODE_SHIFT_LEFT:
				return QK_SHIFT;
			case KeyEvent.KEYCODE_HOME:
				return QK_HOME;
			case KeyEvent.KEYCODE_BACK:
				return QK_BACK;
		}

		int uchar = event.getUnicodeChar();
		if(uchar < 127)
			return uchar;

		return 0;
	}
}
