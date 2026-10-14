/*
 * SDL3 implementation of the Allegro 4 subset declared in allegro.h.
 */
#include "allegro.h"
#include "font8x8.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <map>
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

BITMAP* screen = NULL;
int SCREEN_W = 0;
int SCREEN_H = 0;
FONT* font = NULL; /* only one built-in font; the pointer is never dereferenced */
const KeyArray key = KeyArray();

static SDL_Window*   g_window   = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture*  g_texture  = NULL;

/* ---- core ---------------------------------------------------------- */

int allegro_init(void)
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return -1;
	}
	/* The game loads assets via relative paths ("./images/...").
	   If launched from elsewhere (e.g. Finder), switch to the
	   executable's directory when the assets live there. */
	const char* base = SDL_GetBasePath();
	if (base) {
		std::string images = std::string(base) + "images";
		struct stat st;
		if (stat(images.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			chdir(base);
	}
	return 0;
}

void allegro_exit(void)
{
	SDL_Quit();
}

void allegro_message(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Pacman", buf, g_window);
}

/* ---- timers -------------------------------------------------------- */

int install_timer(void)
{
	return SDL_InitSubSystem(SDL_INIT_EVENTS) ? 0 : -1;
}

#ifdef __EMSCRIPTEN__
/* A browser page has no timer threads: run the callbacks from the main
   thread, catching up on elapsed time whenever the program waits
   (rest) or reads the keyboard. */
struct PolledTimer { void (*proc)(void); Uint64 period_ns, next_ns; };
static std::vector<PolledTimer> g_timers;

static void RunTimers(void)
{
	Uint64 now = SDL_GetTicksNS();
	for (size_t i = 0; i < g_timers.size(); i++) {
		PolledTimer& t = g_timers[i];
		int calls = 0;
		while (now >= t.next_ns && calls++ < 10) {  /* don't spiral after a stall */
			t.proc();
			t.next_ns += t.period_ns;
		}
		if (now >= t.next_ns) t.next_ns = now + t.period_ns;
	}
}

int install_int_ex(void (*proc)(void), long speed)
{
	PolledTimer t;
	t.proc = proc;
	t.period_ns = (Uint64)(speed > 0 ? speed : 1) * 1000;
	t.next_ns = SDL_GetTicksNS() + t.period_ns;
	g_timers.push_back(t);
	return 0;
}

/* Yields to the browser (needs -sASYNCIFY) so the page stays responsive. */
void rest(unsigned int ms)
{
	RunTimers();
	emscripten_sleep(ms);
	RunTimers();
}
#else
static Uint32 TimerTrampoline(void* userdata, SDL_TimerID, Uint32 interval)
{
	((void (*)(void))userdata)();
	return interval;
}

int install_int_ex(void (*proc)(void), long speed)
{
	Uint32 ms = (Uint32)(speed / 1000);
	if (ms < 1) ms = 1;
	return SDL_AddTimer(ms, TimerTrampoline, (void*)proc) ? 0 : -1;
}

void rest(unsigned int ms) { SDL_Delay(ms); }
#endif

/* ---- graphics ------------------------------------------------------ */

/* Round a colour to what a 16bpp (565) Allegro bitmap would store. */
static inline Uint32 Quantize(int r, int g, int b)
{
	r &= 0xFF; g &= 0xFF; b &= 0xFF;
	r = (r & 0xF8) | (r >> 5);
	g = (g & 0xFC) | (g >> 6);
	b = (b & 0xF8) | (b >> 5);
	return ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
}

int makecol(int r, int g, int b) { return (int)Quantize(r, g, b); }

static void Present(void)
{
	if (!g_renderer) return;
	SDL_UpdateTexture(g_texture, NULL, screen->dat, screen->w * (int)sizeof(Uint32));
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	SDL_RenderClear(g_renderer);
	SDL_RenderTexture(g_renderer, g_texture, NULL, NULL);
	SDL_RenderPresent(g_renderer);

	/* Debug aid: PACMAN_SCREENSHOT=out.bmp saves the frame shown
	   PACMAN_SCREENSHOT_SECS seconds (default 3) after startup. */
	static const char* shot = getenv("PACMAN_SCREENSHOT");
	static Uint64 shot_at = SDL_GetTicks() +
		1000 * (getenv("PACMAN_SCREENSHOT_SECS") ? atoi(getenv("PACMAN_SCREENSHOT_SECS")) : 3);
	if (shot && SDL_GetTicks() >= shot_at) {
		SDL_Surface* s = SDL_CreateSurfaceFrom(screen->w, screen->h, SDL_PIXELFORMAT_XRGB8888,
		                                       screen->dat, screen->w * (int)sizeof(Uint32));
		if (s) { SDL_SaveBMP(s, shot); SDL_DestroySurface(s); }
		shot = NULL;
	}
}

/* Drawing directly to `screen` is shown immediately, as in Allegro. */
static inline void Touched(BITMAP* bmp)
{
	if (bmp == screen) Present();
}

void set_color_depth(int) {}

int set_gfx_mode(int card, int w, int h, int, int)
{
	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
	if (card == GFX_AUTODETECT_FULLSCREEN) flags |= SDL_WINDOW_FULLSCREEN;
	if (!SDL_CreateWindowAndRenderer("Pacman", w, h, flags, &g_window, &g_renderer)) {
		fprintf(stderr, "SDL_CreateWindowAndRenderer failed: %s\n", SDL_GetError());
		return -1;
	}
	SDL_SetRenderVSync(g_renderer, 1);
	SDL_SetRenderLogicalPresentation(g_renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	SDL_SetTextureScaleMode(g_texture, SDL_SCALEMODE_NEAREST);
	SCREEN_W = w;
	SCREEN_H = h;
	screen = create_bitmap(w, h);
	Present();
	return 0;
}

BITMAP* create_bitmap(int w, int h)
{
	BITMAP* b = (BITMAP*)malloc(sizeof(BITMAP));
	b->w = w;
	b->h = h;
	b->dat = (Uint32*)calloc((size_t)w * h, sizeof(Uint32));
	return b;
}

void destroy_bitmap(BITMAP* bmp)
{
	if (!bmp) return;
	free(bmp->dat);
	free(bmp);
}

BITMAP* load_bitmap(const char* filename, RGB*)
{
	SDL_Surface* raw = SDL_LoadBMP(filename);
	if (!raw) {
		fprintf(stderr, "load_bitmap(%s): %s\n", filename, SDL_GetError());
		return NULL;
	}
	SDL_Surface* s = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_XRGB8888);
	SDL_DestroySurface(raw);
	if (!s) return NULL;

	BITMAP* b = create_bitmap(s->w, s->h);
	for (int y = 0; y < s->h; y++) {
		const Uint32* row = (const Uint32*)((const Uint8*)s->pixels + y * s->pitch);
		for (int x = 0; x < s->w; x++) {
			Uint32 p = row[x];
			b->dat[y * b->w + x] = Quantize((p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF);
		}
	}
	SDL_DestroySurface(s);
	return b;
}

int bitmap_mask_color(BITMAP*) { return makecol(255, 0, 255); }

int getpixel(BITMAP* bmp, int x, int y)
{
	if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h) return -1;
	return (int)bmp->dat[y * bmp->w + x];
}

static inline void PutPixelRaw(BITMAP* bmp, int x, int y, Uint32 c)
{
	if (x >= 0 && y >= 0 && x < bmp->w && y < bmp->h)
		bmp->dat[y * bmp->w + x] = c;
}

void putpixel(BITMAP* bmp, int x, int y, int color)
{
	PutPixelRaw(bmp, x, y, (Uint32)color);
	Touched(bmp);
}

void clear_to_color(BITMAP* bmp, int color)
{
	for (int i = 0; i < bmp->w * bmp->h; i++) bmp->dat[i] = (Uint32)color;
	Touched(bmp);
}

void clear_bitmap(BITMAP* bmp) { clear_to_color(bmp, 0); }

void rectfill(BITMAP* bmp, int x1, int y1, int x2, int y2, int color)
{
	if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
	if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
	for (int y = y1; y <= y2; y++)
		for (int x = x1; x <= x2; x++)
			PutPixelRaw(bmp, x, y, (Uint32)color);
	Touched(bmp);
}

static void LineRaw(BITMAP* bmp, int x1, int y1, int x2, int y2, Uint32 c)
{
	int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;
	for (;;) {
		PutPixelRaw(bmp, x1, y1, c);
		if (x1 == x2 && y1 == y2) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x1 += sx; }
		if (e2 <= dx) { err += dx; y1 += sy; }
	}
}

void line(BITMAP* bmp, int x1, int y1, int x2, int y2, int color)
{
	LineRaw(bmp, x1, y1, x2, y2, (Uint32)color);
	Touched(bmp);
}

void rect(BITMAP* bmp, int x1, int y1, int x2, int y2, int color)
{
	LineRaw(bmp, x1, y1, x2, y1, (Uint32)color);
	LineRaw(bmp, x2, y1, x2, y2, (Uint32)color);
	LineRaw(bmp, x2, y2, x1, y2, (Uint32)color);
	LineRaw(bmp, x1, y2, x1, y1, (Uint32)color);
	Touched(bmp);
}

static void BlitImpl(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy, int w, int h, bool masked)
{
	if (!src || !dst) return;
	const Uint32 mask = (Uint32)makecol(255, 0, 255);
	/* Use a temporary copy when blitting a bitmap onto itself. */
	std::vector<Uint32> tmp;
	const Uint32* sdat = src->dat;
	if (src == dst) { tmp.assign(src->dat, src->dat + src->w * src->h); sdat = &tmp[0]; }
	for (int y = 0; y < h; y++) {
		int syy = sy + y, dyy = dy + y;
		if (syy < 0 || syy >= src->h || dyy < 0 || dyy >= dst->h) continue;
		for (int x = 0; x < w; x++) {
			int sxx = sx + x, dxx = dx + x;
			if (sxx < 0 || sxx >= src->w || dxx < 0 || dxx >= dst->w) continue;
			Uint32 p = sdat[syy * src->w + sxx];
			if (masked && p == mask) continue;
			dst->dat[dyy * dst->w + dxx] = p;
		}
	}
	Touched(dst);
}

void blit(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy, int w, int h)
{
	BlitImpl(src, dst, sx, sy, dx, dy, w, h, false);
}

void masked_blit(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy, int w, int h)
{
	BlitImpl(src, dst, sx, sy, dx, dy, w, h, true);
}

void textprintf_ex(BITMAP* bmp, const FONT*, int x, int y, int color, int bg, const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	for (const char* c = buf; *c; c++, x += 8) {
		unsigned char ch = (unsigned char)*c;
		const unsigned char* glyph = font8x8_basic[(ch >= 0x20 && ch <= 0x7E) ? ch - 0x20 : '?' - 0x20];
		for (int row = 0; row < 8; row++)
			for (int col = 0; col < 8; col++) {
				if (glyph[row] & (1 << col))
					PutPixelRaw(bmp, x + col, y + row, (Uint32)color);
				else if (bg != -1)
					PutPixelRaw(bmp, x + col, y + row, (Uint32)bg);
			}
	}
	Touched(bmp);
}

/* ---- keyboard ------------------------------------------------------ */

int install_keyboard(void) { return 0; }
void clear_keybuf(void) {}

/* Keys pressed since key[] last looked at them. A tap shorter than the
   game's polling interval would otherwise be missed entirely. */
static bool g_tapped[SDL_SCANCODE_COUNT];

static void PumpEvents(void)
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_EVENT_KEY_DOWN:
			if (!e.key.repeat && e.key.scancode < SDL_SCANCODE_COUNT) g_tapped[e.key.scancode] = true;
			break;
		case SDL_EVENT_QUIT:
			SDL_Quit();
			exit(0);
		case SDL_EVENT_WINDOW_EXPOSED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			Present();
			break;
		default:
			break;
		}
	}
}

bool KeyArray::operator[](int scancode) const
{
	/* Allegro programs spin on key[] waiting for input. Pump events
	   on every query, and back off briefly when queried in a tight
	   loop so those waits don't burn a whole CPU core. */
	static Uint64 last_ns = 0;
	static int queries = 0;
	Uint64 now = SDL_GetTicksNS();
	if (now - last_ns < 1000000) {
		if (++queries > 1000) { rest(1); queries = 0; }
	} else {
		queries = 0;
	}
	last_ns = now;

	PumpEvents();
	const bool* state = SDL_GetKeyboardState(NULL);
	/* Held now, or tapped since the last time this key was checked. */
	bool down = state[scancode] || g_tapped[scancode];
	g_tapped[scancode] = false;
	return down;
}

/* ---- sound --------------------------------------------------------- */

struct SAMPLE {
	SDL_AudioSpec spec;
	Uint8* buf;
	Uint32 len;
};

struct Voice {
	const SAMPLE* spl;
	SDL_AudioStream* stream;
	bool looping;
};

static SDL_AudioDeviceID g_audio_dev = 0;
static std::map<int, Voice*> g_voices;
static std::map<const SAMPLE*, int> g_sample_voice; /* voices used by play_sample */
static int g_next_voice = 0;

int install_sound(int, int, const char*)
{
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return -1;
	g_audio_dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	return g_audio_dev ? 0 : -1;
}

SAMPLE* load_sample(const char* filename)
{
	SAMPLE* s = (SAMPLE*)calloc(1, sizeof(SAMPLE));
	if (!SDL_LoadWAV(filename, &s->spec, &s->buf, &s->len)) {
		fprintf(stderr, "load_sample(%s): %s\n", filename, SDL_GetError());
		free(s);
		return NULL;
	}
	return s;
}

void destroy_sample(SAMPLE* spl)
{
	if (!spl) return;
	SDL_free(spl->buf);
	free(spl);
}

/* Refill looping voices whenever the device asks for more data. */
static void SDLCALL VoiceRefill(void* userdata, SDL_AudioStream* stream, int additional, int)
{
	Voice* v = (Voice*)userdata;
	if (v->looping && additional > 0)
		SDL_PutAudioStreamData(stream, v->spl->buf, (int)v->spl->len);
}

int allocate_voice(const SAMPLE* spl)
{
	if (!spl || !g_audio_dev) return -1;
	SDL_AudioSpec dev_spec;
	if (!SDL_GetAudioDeviceFormat(g_audio_dev, &dev_spec, NULL)) return -1;
	Voice* v = new Voice();
	v->spl = spl;
	v->looping = false;
	v->stream = SDL_CreateAudioStream(&spl->spec, &dev_spec);
	if (!v->stream || !SDL_BindAudioStream(g_audio_dev, v->stream)) {
		if (v->stream) SDL_DestroyAudioStream(v->stream);
		delete v;
		return -1;
	}
	SDL_SetAudioStreamGetCallback(v->stream, VoiceRefill, v);
	int id = g_next_voice++;
	g_voices[id] = v;
	return id;
}

void deallocate_voice(int voice)
{
	std::map<int, Voice*>::iterator i = g_voices.find(voice);
	if (i == g_voices.end()) return;
	SDL_DestroyAudioStream(i->second->stream);
	delete i->second;
	g_voices.erase(i);
}

void voice_start(int voice)
{
	std::map<int, Voice*>::iterator i = g_voices.find(voice);
	if (i == g_voices.end()) return;
	Voice* v = i->second;
	SDL_ClearAudioStream(v->stream);
	SDL_PutAudioStreamData(v->stream, v->spl->buf, (int)v->spl->len);
}

void voice_stop(int voice)
{
	std::map<int, Voice*>::iterator i = g_voices.find(voice);
	if (i == g_voices.end()) return;
	i->second->looping = false;
	SDL_ClearAudioStream(i->second->stream);
}

int voice_get_position(int voice)
{
	std::map<int, Voice*>::iterator i = g_voices.find(voice);
	if (i == g_voices.end()) return -1;
	Voice* v = i->second;
	return (v->looping || SDL_GetAudioStreamQueued(v->stream) > 0) ? 0 : -1;
}

int play_sample(const SAMPLE* spl, int vol, int, int, int loop)
{
	if (!spl) return -1;
	int id;
	std::map<const SAMPLE*, int>::iterator i = g_sample_voice.find(spl);
	if (i != g_sample_voice.end())
		id = i->second;
	else {
		id = allocate_voice(spl);
		if (id < 0) return -1;
		g_sample_voice[spl] = id;
	}
	Voice* v = g_voices[id];
	SDL_SetAudioStreamGain(v->stream, vol / 255.0f);
	SDL_LockAudioStream(v->stream);
	v->looping = loop != 0;
	SDL_UnlockAudioStream(v->stream);
	voice_start(id);
	return id;
}

void stop_sample(const SAMPLE* spl)
{
	std::map<const SAMPLE*, int>::iterator i = g_sample_voice.find(spl);
	if (i == g_sample_voice.end()) return;
	Voice* v = g_voices[i->second];
	SDL_LockAudioStream(v->stream);
	v->looping = false;
	SDL_UnlockAudioStream(v->stream);
	SDL_ClearAudioStream(v->stream);
}
