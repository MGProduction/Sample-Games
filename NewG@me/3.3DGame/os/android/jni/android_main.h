#ifndef _ANDROID_MAIN_H_
#define _ANDROID_MAIN_H_

void game_init(const char* data_path, const char* save_path, int width, int height, int mode);
void game_set_orientation(int portrait, int width, int height, int flip);
void game_draw(void);
void game_destroy(void);
void game_pause(void);
void game_resume(void);
void game_touch_event(int id, int action, int x, int y);
void game_key_event(int key, int code);
void game_accel_event(float x, float y, float z);
void game_restore_textures(void);

#endif // _ANDROID_MAIN_H_
