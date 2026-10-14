/*
 * Minimal Allegro 4 compatibility layer on top of SDL3.
 *
 * Implements only the subset of the Allegro 4.2 API that this Pacman
 * project uses, so the original game code builds and runs on macOS
 * (or any SDL3 platform) without modification.
 *
 * Bitmaps are stored as 0x00RRGGBB pixels, quantized to 16-bit (565)
 * precision so colour comparisons (mask colour, pixel-perfect
 * collisions) behave exactly as in the original 16bpp Allegro build.
 */
#ifndef ALLEGRO_COMPAT_H_
#define ALLEGRO_COMPAT_H_

#include <SDL3/SDL.h>
#include <stdio.h>

/* ---- core ---------------------------------------------------------- */
int  allegro_init(void);
void allegro_exit(void);
void allegro_message(const char* fmt, ...);

#define END_OF_MAIN()
#define END_OF_FUNCTION(x)
#define LOCK_VARIABLE(x)
#define LOCK_FUNCTION(x)

/* ---- timers -------------------------------------------------------- */
/* Timer units are microseconds here (Allegro uses 1193181 Hz ticks). */
#define MSEC_TO_TIMER(x) ((long)(x) * 1000L)
#define SECS_TO_TIMER(x) ((long)(x) * 1000000L)
int  install_timer(void);
int  install_int_ex(void (*proc)(void), long speed);
void rest(unsigned int ms);

/* ---- graphics ------------------------------------------------------ */
typedef struct BITMAP {
	int w, h;
	Uint32* dat; /* w*h pixels, 0x00RRGGBB */
} BITMAP;

typedef struct RGB { unsigned char r, g, b, filler; } RGB;
typedef RGB PALETTE[256];

typedef struct FONT FONT;
extern FONT* font;

extern BITMAP* screen;
extern int SCREEN_W;
extern int SCREEN_H;

#define GFX_AUTODETECT          0
#define GFX_AUTODETECT_WINDOWED 1
#define GFX_AUTODETECT_FULLSCREEN 2

void set_color_depth(int depth);
int  set_gfx_mode(int card, int w, int h, int v_w, int v_h);

BITMAP* create_bitmap(int w, int h);
void    destroy_bitmap(BITMAP* bmp);
BITMAP* load_bitmap(const char* filename, RGB* pal);

int  makecol(int r, int g, int b);
int  bitmap_mask_color(BITMAP* bmp);
int  getpixel(BITMAP* bmp, int x, int y);
void putpixel(BITMAP* bmp, int x, int y, int color);

void clear_bitmap(BITMAP* bmp);
void clear_to_color(BITMAP* bmp, int color);
void rectfill(BITMAP* bmp, int x1, int y1, int x2, int y2, int color);
void rect(BITMAP* bmp, int x1, int y1, int x2, int y2, int color);
void line(BITMAP* bmp, int x1, int y1, int x2, int y2, int color);

void blit(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy, int w, int h);
void masked_blit(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy, int w, int h);

void textprintf_ex(BITMAP* bmp, const FONT* f, int x, int y, int color, int bg, const char* fmt, ...);

/* ---- keyboard ------------------------------------------------------ */
int  install_keyboard(void);
void clear_keybuf(void);

/* key[] polls SDL events on access, so Allegro-style busy loops such as
 * `while(!key[KEY_SPACE]){}` keep working on the main thread. */
struct KeyArray { bool operator[](int scancode) const; };
extern const KeyArray key;

#define KEY_UP    SDL_SCANCODE_UP
#define KEY_DOWN  SDL_SCANCODE_DOWN
#define KEY_LEFT  SDL_SCANCODE_LEFT
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_ESC   SDL_SCANCODE_ESCAPE
#define KEY_ENTER SDL_SCANCODE_RETURN
#define KEY_SPACE SDL_SCANCODE_SPACE
#define KEY_P     SDL_SCANCODE_P
#define KEY_W     SDL_SCANCODE_W
#define KEY_L     SDL_SCANCODE_L
#define KEY_1     SDL_SCANCODE_1
#define KEY_2     SDL_SCANCODE_2

/* ---- sound --------------------------------------------------------- */
typedef struct SAMPLE SAMPLE;

#define DIGI_AUTODETECT -1
#define MIDI_NONE 0

int     install_sound(int digi, int midi, const char* cfg);
SAMPLE* load_sample(const char* filename);
void    destroy_sample(SAMPLE* spl);
int     play_sample(const SAMPLE* spl, int vol, int pan, int freq, int loop);
void    stop_sample(const SAMPLE* spl);

int  allocate_voice(const SAMPLE* spl);
void deallocate_voice(int voice);
void voice_start(int voice);
void voice_stop(int voice);
int  voice_get_position(int voice);

#endif
