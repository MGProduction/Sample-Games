//***********************************************************************//
//	
// FILE:   android_main.c
// DESC:
//	AUTHOR: Andrea Capitani
//                                                                       //
//***********************************************************************//


#include "const.h"
#include "g_main.h"
#include "android_main.h"
#include "android_jni.h"

void game_init(const char* data_path, const char* save_path, int width, int height, int mode)
{
	GAME_init(&g_W, data_path, (float)width, (float)height);
}

void game_set_orientation(int portrait, int width, int height, int flip)
{
	/* os_video_w=width;
	 os_video_h=height;
	 os_portrait=portrait;
	 os_flip=flip;
	 SetPerspective();
	 */
}

void game_draw(void)
{
	 GAME_loop(&g_W);
	 //os_BACK_pressed = 0;
}

void game_destroy(void)
{
	 GAME_reset(&g_W);
}

void game_pause(void)
{
	//GAME_gotosleep();
	GAME_pause(&g_W);
}

void game_resume(void)
{
	//GAME_wakeup();
	GAME_resume(&g_W);
}

void game_restore_textures(void)
{
	GAME_restore_textures(&g_W);
}

void game_touch_event(int id, int action, int x, int y)
{
	//os_np = max(os_np, id);
	//os_np = 10;
	if (os_np<id)
		os_np = id;
	if (!os_portrait)
	{
		x = (int)os_video_w-y;
		y = x;
	}
	os_x[id] = x;
	os_y[id] = y;
	
	if ((action==2)&&((os_status[id]==0)||(os_status[id]==3)))
		os_status[id] = 1;
	else
		os_status[id] = action;
}

void game_key_event(int key, int code)
{
	if (((key==3)||(key==4))&&(code==0))
	{
		//os_BACK_pressed = 1;
		GAME_handle_back_pressed(&g_W);
	}
}

void game_accel_event(float x, float y, float z)
{
	os_roll = y;
	os_pitch = x;
	os_Z = z;
}

/////////////////////////////////////////////////////////////////////////
void   GC_leaderboard_show_usingID(int iLeaderboard)
{
	// todo!!!
}

void os_openURL(const char*url)
{
#if defined(OS_ANDROID)
	jni_open_url(url);
#endif
}

void os_updateOrientation()
{
	// todo!!!
}

int    os_getLanguage()
{
	// todo!!!
	return 0;
}

