/**
 * @file drawpad.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-02
 * 
 * Use arrow keys to move(and draw), c to cycle through colors,
 * p to toggle pen and CTRL+x to clear.
 * 
 * Needs SDL2 library and headers
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define debug(msg) fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, (msg))

typedef struct rgb_T {
	Uint8 r;
	Uint8 g;
	Uint8 b;
} rgb_T;

/* Modified wrapsdl.h as necessary, mostly same */
typedef struct state_T {
	struct {
		unsigned mouse_left : 1;
		unsigned mouse_middle : 1;
		unsigned mouse_right : 1;
		unsigned up : 1;
		unsigned down : 1;
		unsigned left : 1;
		unsigned right : 1;
		unsigned c : 1;
	} btns;
	unsigned quit : 1;
	unsigned paused : 1;
	unsigned fullscreen : 1;
	unsigned can_draw : 1;
	unsigned error : 1;
	/* Mouse cursor info */
	int x;
	int y;
	int dx;
	int dy;
	/* -- Window data fields(mostly static) -- */
	int resx;
	int resy;
	char *title;
	Uint32 win_flags;
	Uint32 ren_flags;
	SDL_Event event;
	SDL_Window *win;
	SDL_Renderer *ren;
	SDL_Texture *rentex;
} state_T;

/**
 * @brief Init SDL, Create window, renderer and texture(FULL-SIZE)
 * and show centered window
 * 
 * @param state Needs valid resolution
 * @param win_flags
 * @param ren_flags
 * @return int 0 on success, -1 otherwise
 */
static int state_create(state_T *state)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		goto scrt_err_init;

	SDL_Window *win = SDL_CreateWindow(state->title, SDL_WINDOWPOS_CENTERED,
					   SDL_WINDOWPOS_CENTERED, state->resx,
					   state->resy, state->win_flags);
	if (win == NULL)
		goto scrt_err_win;

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, state->ren_flags);
	if (ren == NULL)
		goto scrt_err_ren;

	SDL_Texture *rentex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
						SDL_TEXTUREACCESS_TARGET,
						state->resx, state->resy);
	if (rentex == NULL)
		goto scrt_err_rentex;

	state->win = win;
	state->ren = ren;
	state->rentex = rentex;
	if (state->win_flags & SDL_WINDOW_FULLSCREEN)
		state->fullscreen = 1;

	return 0;

scrt_err_rentex:
	SDL_DestroyRenderer(ren);
scrt_err_ren:
	SDL_DestroyWindow(win);
scrt_err_win:
	SDL_Quit();
scrt_err_init:
	SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
	state->quit = 1;
	return -1;
}

static void state_destroy(state_T *state)
{
	SDL_DestroyRenderer(state->ren);
	SDL_DestroyWindow(state->win);
	SDL_Quit();
	*state = (state_T){ .quit = 1 };
}

static int state_resize_rentex(state_T *state)
{
	SDL_Texture *tmp = SDL_CreateTexture(state->ren,
					     SDL_PIXELFORMAT_RGBA8888,
					     SDL_TEXTUREACCESS_TARGET,
					     state->resx, state->resy);
	if (tmp == NULL) {
		state->error = 1;
		return -1;
	}

	SDL_DestroyTexture(state->rentex);
	state->rentex = tmp;

	return 0;
}

static void handle_window_event(state_T *state)
{
	SDL_Event *ev = &(state->event);

	switch (ev->window.event) {
	case SDL_WINDOWEVENT_RESIZED:
		debug("Window resized");
		state->resx = ev->window.data1;
		state->resy = ev->window.data2;
		state_resize_rentex(state);
		break;

	case SDL_WINDOWEVENT_FOCUS_LOST:
		debug("KBD Focus lost");
		state->paused = 1;
		break;

	case SDL_WINDOWEVENT_FOCUS_GAINED:
		debug("KBD Focus gained");
		state->paused = 0;
		break;

	case SDL_WINDOWEVENT_LEAVE:
		debug("Mouse focus lost");
		break;

	case SDL_WINDOWEVENT_ENTER:
		debug("Mouse focus gained");
		break;

	default:
		break;
	}
}

static void handle_keydown(state_T *state)
{
	switch (state->event.key.keysym.sym) {
	case SDLK_ESCAPE:
		debug("ESC pressed, quit");
		state->quit = 1;
		break;

	case SDLK_F11:
		debug("Toggle fullscreen");
		/* Does not change resolution */
		state->fullscreen = !state->fullscreen;
		state->win_flags |= SDL_WINDOW_FULLSCREEN * state->fullscreen;
		SDL_SetWindowFullscreen(state->win, SDL_WINDOW_FULLSCREEN *
							    state->fullscreen);
		state_resize_rentex(state);
		break;

	case SDLK_UP:
		state->btns.up = 1;
		break;

	case SDLK_DOWN:
		state->btns.down = 1;
		break;

	case SDLK_LEFT:
		state->btns.left = 1;
		break;

	case SDLK_RIGHT:
		state->btns.right = 1;
		break;

	case SDLK_c:
		state->btns.c = 1;
		break;

	case SDLK_x:
		if (state->event.key.keysym.mod & KMOD_CTRL) {
			SDL_SetRenderTarget(state->ren, state->rentex);
			SDL_SetRenderDrawColor(state->ren, 0, 0, 0, 255);
			SDL_RenderClear(state->ren);
			SDL_SetRenderTarget(state->ren, NULL);
		}
		break;

	case SDLK_p:
		state->can_draw = !state->can_draw;

	default:
		break;
	}
}

static void handle_keyup(state_T *state)
{
	switch (state->event.key.keysym.sym) {
	case SDLK_UP:
		state->btns.up = 0;
		break;

	case SDLK_DOWN:
		state->btns.down = 0;
		break;

	case SDLK_LEFT:
		state->btns.left = 0;
		break;

	case SDLK_RIGHT:
		state->btns.right = 0;
		break;

	case SDLK_c:
		state->btns.c = 0;

	default:
		break;
	}
}

static void handle_mouse_button_down(state_T *state)
{
	switch (state->event.button.button) {
	case SDL_BUTTON_LEFT:
		state->btns.mouse_left = 1;
		break;

	case SDL_BUTTON_MIDDLE:
		state->btns.mouse_middle = 1;
		break;

	case SDL_BUTTON_RIGHT:
		state->btns.mouse_right = 1;
		break;

	default:
		break;
	}
}
static void handle_mouse_button_up(state_T *state)
{
	switch (state->event.button.button) {
	case SDL_BUTTON_LEFT:
		state->btns.mouse_left = 0;
		break;

	case SDL_BUTTON_MIDDLE:
		state->btns.mouse_middle = 0;
		break;

	case SDL_BUTTON_RIGHT:
		state->btns.mouse_right = 0;
		break;

	default:
		break;
	}
}

static void handle_mouse_motion(state_T *state)
{
	state->x = state->event.motion.x;
	state->y = state->event.motion.y;
	state->dx = state->event.motion.xrel;
	state->dy = state->event.motion.yrel;
}

static void handle_events(state_T *state)
{
	SDL_Event *ev = &(state->event);

	/* Reset values corresponding to events */
	state->dx = 0;
	state->dy = 0;

	while (SDL_PollEvent(ev) && !state->error) {
		switch (ev->type) {
		case SDL_QUIT:
			debug("Window closed, quit");
			state->quit = 1;
			break;

		case SDL_WINDOWEVENT:
			handle_window_event(state);
			break;

		case SDL_KEYDOWN:
			handle_keydown(state);
			break;

		case SDL_KEYUP:
			handle_keyup(state);
			break;

		case SDL_MOUSEBUTTONUP:
			handle_mouse_button_up(state);
			break;

		case SDL_MOUSEMOTION:
			handle_mouse_motion(state);
			break;

		case SDL_MOUSEBUTTONDOWN:
			handle_mouse_button_down(state);
			break;

		case SDL_FINGERMOTION:
			/* Unhandeled */
			break;

		default:
			break;
		}
	}
}

static const rgb_T COLORS[] = {
	{ 255, 0, 0 },	 { 255, 5, 0 },	  { 255, 11, 0 },  { 255, 17, 0 },
	{ 255, 23, 0 },	 { 255, 29, 0 },  { 255, 35, 0 },  { 255, 41, 0 },
	{ 255, 47, 0 },	 { 255, 53, 0 },  { 255, 59, 0 },  { 255, 65, 0 },
	{ 255, 71, 0 },	 { 255, 77, 0 },  { 255, 83, 0 },  { 255, 89, 0 },
	{ 255, 95, 0 },	 { 255, 101, 0 }, { 255, 107, 0 }, { 255, 113, 0 },
	{ 255, 119, 0 }, { 255, 125, 0 }, { 255, 131, 0 }, { 255, 137, 0 },
	{ 255, 143, 0 }, { 255, 149, 0 }, { 255, 155, 0 }, { 255, 161, 0 },
	{ 255, 167, 0 }, { 255, 173, 0 }, { 255, 179, 0 }, { 255, 185, 0 },
	{ 255, 191, 0 }, { 255, 197, 0 }, { 255, 203, 0 }, { 255, 209, 0 },
	{ 255, 215, 0 }, { 255, 221, 0 }, { 255, 227, 0 }, { 255, 233, 0 },
	{ 255, 239, 0 }, { 255, 245, 0 }, { 255, 251, 0 }, { 253, 255, 0 },
	{ 247, 255, 0 }, { 241, 255, 0 }, { 235, 255, 0 }, { 229, 255, 0 },
	{ 223, 255, 0 }, { 217, 255, 0 }, { 211, 255, 0 }, { 205, 255, 0 },
	{ 199, 255, 0 }, { 193, 255, 0 }, { 187, 255, 0 }, { 181, 255, 0 },
	{ 175, 255, 0 }, { 169, 255, 0 }, { 163, 255, 0 }, { 157, 255, 0 },
	{ 151, 255, 0 }, { 145, 255, 0 }, { 139, 255, 0 }, { 133, 255, 0 },
	{ 127, 255, 0 }, { 121, 255, 0 }, { 115, 255, 0 }, { 109, 255, 0 },
	{ 103, 255, 0 }, { 97, 255, 0 },  { 91, 255, 0 },  { 85, 255, 0 },
	{ 79, 255, 0 },	 { 73, 255, 0 },  { 67, 255, 0 },  { 61, 255, 0 },
	{ 55, 255, 0 },	 { 49, 255, 0 },  { 43, 255, 0 },  { 37, 255, 0 },
	{ 31, 255, 0 },	 { 25, 255, 0 },  { 19, 255, 0 },  { 13, 255, 0 },
	{ 7, 255, 0 },	 { 1, 255, 0 },	  { 0, 255, 3 },   { 0, 255, 9 },
	{ 0, 255, 15 },	 { 0, 255, 21 },  { 0, 255, 27 },  { 0, 255, 33 },
	{ 0, 255, 39 },	 { 0, 255, 45 },  { 0, 255, 51 },  { 0, 255, 57 },
	{ 0, 255, 63 },	 { 0, 255, 69 },  { 0, 255, 75 },  { 0, 255, 81 },
	{ 0, 255, 87 },	 { 0, 255, 93 },  { 0, 255, 99 },  { 0, 255, 105 },
	{ 0, 255, 111 }, { 0, 255, 117 }, { 0, 255, 123 }, { 0, 255, 129 },
	{ 0, 255, 135 }, { 0, 255, 141 }, { 0, 255, 147 }, { 0, 255, 153 },
	{ 0, 255, 159 }, { 0, 255, 165 }, { 0, 255, 171 }, { 0, 255, 177 },
	{ 0, 255, 183 }, { 0, 255, 189 }, { 0, 255, 195 }, { 0, 255, 201 },
	{ 0, 255, 207 }, { 0, 255, 213 }, { 0, 255, 219 }, { 0, 255, 225 },
	{ 0, 255, 231 }, { 0, 255, 237 }, { 0, 255, 243 }, { 0, 255, 249 },
	{ 0, 255, 255 }, { 0, 249, 255 }, { 0, 243, 255 }, { 0, 237, 255 },
	{ 0, 231, 255 }, { 0, 225, 255 }, { 0, 219, 255 }, { 0, 213, 255 },
	{ 0, 207, 255 }, { 0, 201, 255 }, { 0, 195, 255 }, { 0, 189, 255 },
	{ 0, 183, 255 }, { 0, 177, 255 }, { 0, 171, 255 }, { 0, 165, 255 },
	{ 0, 159, 255 }, { 0, 153, 255 }, { 0, 147, 255 }, { 0, 141, 255 },
	{ 0, 135, 255 }, { 0, 129, 255 }, { 0, 123, 255 }, { 0, 117, 255 },
	{ 0, 111, 255 }, { 0, 105, 255 }, { 0, 99, 255 },  { 0, 93, 255 },
	{ 0, 87, 255 },	 { 0, 81, 255 },  { 0, 75, 255 },  { 0, 69, 255 },
	{ 0, 63, 255 },	 { 0, 57, 255 },  { 0, 51, 255 },  { 0, 45, 255 },
	{ 0, 39, 255 },	 { 0, 33, 255 },  { 0, 27, 255 },  { 0, 21, 255 },
	{ 0, 15, 255 },	 { 0, 9, 255 },	  { 0, 3, 255 },   { 1, 0, 255 },
	{ 7, 0, 255 },	 { 13, 0, 255 },  { 19, 0, 255 },  { 25, 0, 255 },
	{ 31, 0, 255 },	 { 37, 0, 255 },  { 43, 0, 255 },  { 49, 0, 255 },
	{ 55, 0, 255 },	 { 61, 0, 255 },  { 67, 0, 255 },  { 73, 0, 255 },
	{ 79, 0, 255 },	 { 85, 0, 255 },  { 91, 0, 255 },  { 97, 0, 255 },
	{ 103, 0, 255 }, { 109, 0, 255 }, { 115, 0, 255 }, { 121, 0, 255 },
	{ 127, 0, 255 }, { 133, 0, 255 }, { 139, 0, 255 }, { 145, 0, 255 },
	{ 151, 0, 255 }, { 157, 0, 255 }, { 163, 0, 255 }, { 169, 0, 255 },
	{ 175, 0, 255 }, { 181, 0, 255 }, { 187, 0, 255 }, { 193, 0, 255 },
	{ 199, 0, 255 }, { 205, 0, 255 }, { 211, 0, 255 }, { 217, 0, 255 },
	{ 223, 0, 255 }, { 229, 0, 255 }, { 235, 0, 255 }, { 241, 0, 255 },
	{ 247, 0, 255 }, { 253, 0, 255 }, { 255, 0, 251 }, { 255, 0, 245 },
	{ 255, 0, 239 }, { 255, 0, 233 }, { 255, 0, 227 }, { 255, 0, 221 },
	{ 255, 0, 215 }, { 255, 0, 209 }, { 255, 0, 203 }, { 255, 0, 197 },
	{ 255, 0, 191 }, { 255, 0, 185 }, { 255, 0, 179 }, { 255, 0, 173 },
	{ 255, 0, 167 }, { 255, 0, 161 }, { 255, 0, 155 }, { 255, 0, 149 },
	{ 255, 0, 143 }, { 255, 0, 137 }, { 255, 0, 131 }, { 255, 0, 125 },
	{ 255, 0, 119 }, { 255, 0, 113 }, { 255, 0, 107 }, { 255, 0, 101 },
	{ 255, 0, 95 },	 { 255, 0, 89 },  { 255, 0, 83 },  { 255, 0, 77 },
	{ 255, 0, 71 },	 { 255, 0, 65 },  { 255, 0, 59 },  { 255, 0, 53 },
	{ 255, 0, 47 },	 { 255, 0, 41 },  { 255, 0, 35 },  { 255, 0, 29 },
	{ 255, 0, 23 },	 { 255, 0, 17 },  { 255, 0, 11 },  { 255, 0, 5 },
};
enum { NCOLORS = sizeof(COLORS) / sizeof(COLORS[0]) };

static inline int real_mod(int x, int y)
{
	return ((x % y) + y) % y;
}

int main()
{
	state_T st = {
		.can_draw = 1,
		.resx = 900,
		.resy = 600,
		.title = "Draw Pad",
		.win_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE,
		.ren_flags = SDL_RENDERER_ACCELERATED |
			     SDL_RENDERER_PRESENTVSYNC,
	};
	int err = state_create(&st);
	if (err)
		goto init_err;

	SDL_Point at = { .x = st.resx / 2, .y = st.resy / 2 };
	rgb_T col = { 0 };
	int pen_cc = 0;

	while (!st.quit) {
		handle_events(&st);
		if (st.error) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s\n",
					SDL_GetError());
			break;
		}

		/* Draw on texture to save history */
		col = COLORS[pen_cc];
		SDL_SetRenderTarget(st.ren, st.rentex);
		SDL_SetRenderDrawColor(st.ren, col.r, col.g, col.b, 255);

		/* Move the pen according to arrow keys pressed */
		at.x += (-1 * st.btns.left);
		at.x += (1 * st.btns.right);
		at.x = real_mod(at.x, st.resx);

		at.y += (-1 * st.btns.up);
		at.y += (1 * st.btns.down);
		at.y = real_mod(at.y, st.resy);

		if (st.btns.c)
			pen_cc = (pen_cc + 1) % NCOLORS;

		if (st.btns.mouse_left) {
			at.x = st.x;
			at.y = st.y;
		}

		/* Draw if not being dragged and pen is active */
		if (st.can_draw && !st.btns.mouse_left)
			SDL_RenderDrawPoint(st.ren, at.x, at.y);

		SDL_SetRenderTarget(st.ren, NULL);
		SDL_RenderCopy(st.ren, st.rentex, NULL, NULL);

		/* Draw White cursor */
		SDL_Rect cur = { .x = at.x - 2, .y = at.y - 2, .h = 5, .w = 5 };
		SDL_SetRenderDrawColor(st.ren, 255, 255, 255, 255);
		SDL_RenderFillRect(st.ren, &cur);

		SDL_RenderPresent(st.ren);
		SDL_SetRenderDrawColor(st.ren, 0, 0, 0, 0);
		SDL_RenderClear(st.ren);
	}

	state_destroy(&st);

	return 0;

init_err:
	printf("ERROR\n");
	return EXIT_FAILURE;
}
