package com.marcogiorgini.World2D;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;
import android.opengl.GLU; 
import java.lang.Thread;

public class GameRenderer implements GLSurfaceView.Renderer 
{
	private boolean mInit=false;
	private float mWidth;
	private float mHeight;
	private long startTime;
	
	public int isPortrait(float w, float h)
	{
		if (w>h)
			return 1;
		return 0;		
	}
	
	public GameRenderer() 
	{
		super();
		startTime = System.currentTimeMillis();
	}
	
	public void onDrawFrame(GL10 gl) 
	{
		/* Compute a new frame. After this call completes, Android will perform a eglSwapBuffers */
		long endTime = System.currentTimeMillis();
	    long dt = endTime - startTime;
	    if (dt < 33)
	    {
			try 
			{
				Thread.sleep(33 - dt);
			} 
			catch (InterruptedException e) 
			{
				//e.printStackTrace();
			}
	    }
	    startTime = System.currentTimeMillis();	
		GameJNI.drawGame();
	}
	
	public void onSurfaceChanged(GL10 gl, int width, int height) 
	{
		/* When the screen is rotated or the keyboard slides in and in other cases this is called even
		 * when the activity is not recreated or so. Prevent that we reinitialize the game engine.
		 */
		//float nearD=10.0f;
		float nearD=2.5f;
		float farD=250.0f;
		//float angleP=45.0f;
		float angleP=30.0f;
		float ratioP;
		
		int portrait = isPortrait(width, height);
		
		mWidth = width;
		mHeight = height;
		
		gl.glViewport(0,0,width,height);						// Make our viewport the whole window
		if (mInit==false)
		{
			gl.glMatrixMode(GL10.GL_PROJECTION);						// Select The Projection Matrix
			gl.glLoadIdentity();									// Reset The Projection Matrix
	
			ratioP=(float)width/(float)height;
			GLU.gluPerspective(gl, angleP,ratioP, nearD, farD);
		}
		GameJNI.setOrientationGame(portrait, width, height, 0);
		if (mInit==false)
		{
			GameJNI.initGame(Game.GAME_DATA_FOLDER, Game.GAME_SAVE_FOLDER, width, height, 0);
			mInit = true;
		}
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		
		if (mInit==true)
		{
			gl.glViewport(0,0,(int)mWidth,(int)mHeight);						// Make our viewport the whole window
			gl.glMatrixMode(GL10.GL_PROJECTION);						// Select The Projection Matrix
			gl.glLoadIdentity();									// Reset The Projection Matrix
			GameJNI.restoreGameTextures();
		}
	}
}
