
package com.marcogiorgini.Zaxxon;

public class GameJNI 
{
	public static native void initGame(String game_path, String write_path, int video_width, int video_height, int mode);
	public static native void setOrientationGame(int portrait, int width, int height, int flip);
	public static native void drawGame();
	public static native void destroyGame();
	public static native void pauseGame();
	public static native void resumeGame();
	public static native void restoreGameTextures();
	
	public static native void keyEvent(int key, int state);
	public static native void touchEvent(int id, int action, int x, int y);
	public static native void accelEvent(float x, float y, float z);
	
	public static void exit_app(int code)
	{
		app.exitApp(code);
	}
	
	public static void open_url(String url)
	{
		app.openUrl(url);
	}
	
	public static Game app = null;
    
	static 
	{
		System.loadLibrary("openal");
		System.loadLibrary("zaxxon");
	}
}
