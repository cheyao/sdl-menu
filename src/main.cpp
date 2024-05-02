#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "menu.h"

int SDL_AppInit(void **appstate, int argc, char **argv) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("SDL3 menu example", 1024, 768,
					      SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		SDL_Log("Window could not be created: %s", SDL_GetError());
		return 1;
	}

	initMenu(window);

	*appstate = window;

	return 0;
	(void)argc;
	(void)argv;
}

int SDL_AppIterate(void *appstate) {
	return 0;
	(void)appstate;
}

int SDL_AppEvent(void *appstate, const SDL_Event *event) {
	if (event->type == SDL_EVENT_QUIT) {
		return 1;
	}

	return 0;
	(void)appstate;
}

void SDL_AppQuit(void *appstate) {
	SDL_DestroyWindow(static_cast<SDL_Window *>(appstate));
	SDL_Quit();
}

