/**
 * @file wrapsdl.h
 * @author Amiy Kumar
 * @brief SDL wrapper template
 * @version 0.1
 * @date 2022-02-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SDL_EVENT_WRAPPER_H_
#define SDL_EVENT_WRAPPER_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL.h>

#define debug(msg) fprintf(stderr, "%d: [DEBUG] %s\n", __LINE__, (msg))

typedef struct rgb_T {
	Uint8 r;
	Uint8 g;
	Uint8 b;
} rgb_T;

typedef struct state_T {
	struct {
		// unsigned lctrl : 1;
		// unsigned rctrl : 1;
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
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s\n",
				SDL_GetError());
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

#endif
