#include "kit.hpp"
#include "gl.hpp"

#ifdef __APPLE__
#include "kit-SDL3-osx.hpp"
#endif

#include "Button.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include <iostream>
#include <chrono>

int main(int argc, char **argv) {
#ifdef _WIN32
	try {
#endif
	kit::args.resize(argc);
	for (int arg = 0; arg < argc; ++arg) {
		kit::args[arg] = argv[arg];
	}

	kit::state = kit::Stopped;

	kit::Config kit_config = ::kit_config(); //app sets config

	kit::state = kit::Hidden;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);


	SDL_Window *window;
	if (kit_config.fullscreen) {
		window = SDL_CreateWindow(
			kit_config.title.c_str(),
			100, 100,
			SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY
		);

		SDL_DisplayMode got;
		if (!SDL_GetClosestFullscreenDisplayMode(0, kit_config.size.x, kit_config.size.y, 0.0f, true, &got)) {
			throw std::runtime_error("Failed to set fullscreen mode.");
		}
		std::cout << "Got " << got.w << " x " << got.h << " @ " << got.refresh_rate << "Hz." << std::endl;
		if (!SDL_SetWindowFullscreenMode(window, &got)) {
			throw std::runtime_error(std::string("Failed to set display mode: ") + SDL_GetError());
		}

		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

		int w,h;
		SDL_GetWindowSize(window, &w, &h);
		std::cout << "Window size is " << w << "x" << h << std::endl;
		SDL_GetWindowSizeInPixels(window, &w, &h);
		std::cout << "Drawable size is " << w << "x" << h << std::endl;

	} else {
		window = SDL_CreateWindow(
			kit_config.title.c_str(),
			kit_config.size.x, kit_config.size.y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY
			| (kit_config.resizable ? SDL_WINDOW_RESIZABLE : 0)
		);
	}

	if (!window) {
		throw std::runtime_error(std::string("Error creating SDL window: ") + SDL_GetError());
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		throw std::runtime_error(std::string("Error creating OpenGL context: ") + SDL_GetError());
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	init_gl_shims();
	#endif


	if (!SDL_GL_SetSwapInterval(-1)) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (!SDL_GL_SetSwapInterval(1)) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	glm::uvec2 window_size = glm::uvec2(0,0);
	auto update_window_size = [&]() {
		int w,h;
		SDL_GetWindowSize(window, &w, &h);
		window_size = glm::uvec2(w,h);
		#ifdef KIT_RAW_SDL_EVENTS
		kit::display.window_size = window_size;
		#endif
	};
	update_window_size();

	auto update_drawable_size = [&]() {
		int w,h;
		SDL_GetWindowSizeInPixels(window, &w, &h);
		glm::uvec2 size = glm::uvec2(w,h);
		if (size != kit::display.size) {
			kit::display.size = size;
			if (size.y != 0) {
				kit::display.aspect = float(size.x) / float(size.y);
			} else {
				kit::display.aspect = 1.0f;
			}
			kit::display.pixel_ratio = float(size.x) / float(window_size.x);
			//TODO: look these up somehow!
			kit::display.DPI = 90.0f * kit::display.pixel_ratio;
			kit::display.IPD = 1.0f / kit::display.DPI;
			return true;
		} else {
			return false;
		}
	};
	update_drawable_size();

	/*std::cout << "Display size: " << kit::display.size.x << " x " << kit::display.size.y << std::endl;
	std::cout << "Display aspect: " << kit::display.aspect << std::endl;*/

	kit::set_mode(kit_mode());
	kit::commit_mode();

	std::shared_ptr< kit::Mode > const &mode = kit::get_mode();

	#ifndef __APPLE__
	const kit::PointerID MouseID = 1;
	#endif

	#ifdef __APPLE__
	{
		SDL_PropertiesID props = SDL_GetWindowProperties(window);
		void *window = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
		if (!props || !window) {
			std::cerr << "Error getting window info: " << SDL_GetError() << std::endl;
		}
		kit::osx::start_pointer_handling(window);
	}
	#endif

	while (mode) {
		SDL_GL_MakeCurrent(window, context);

		update_window_size();

		if (update_drawable_size()) {
			glViewport(0,0,kit::display.size.x,kit::display.size.y);
			if (mode) {
				mode->resized();
				kit::commit_mode();
			}
		}


		{
			static SDL_Event evt;
			kit::Button::clear_events();
			while (SDL_PollEvent(&evt) == 1 && mode) {
				#ifdef KIT_RAW_SDL_EVENTS
				mode->handle_event(evt);
				kit::commit_mode();
				if (mode == nullptr) break;
				#endif

				//Pointer events are handled by kit-SDL2-osx.mm on OSX
				#ifndef __APPLE__
				#define MAPX( X ) ((((X) + 0.5f) / window_size.x) * 2.0f - 1.0f)
				#define MAPY( Y ) ((((Y) + 0.5f) / window_size.y) *-2.0f + 1.0f)
				if (evt.type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
					//std::cout << "Mouse Enter." << std::endl; //DEBUG
					kit::Pointer new_state;
					float x,y;
					Uint32 buttons = SDL_GetMouseState(&x, &y);
					new_state.at.x = MAPX(x);
					new_state.at.y = MAPY(y);
					new_state.buttons =
						  ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					new_state.pressure = (new_state.buttons ? 1.0f : 0.0f);
					kit::dispatch_pointer_action(MouseID, kit::PointerEnter, new_state);
					kit::commit_mode();
				} else if (evt.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
					//std::cout << "Mouse Leave." << std::endl; //DEBUG
					kit::Pointer new_state;
					float x,y;
					Uint32 buttons = SDL_GetMouseState(&x, &y);
					new_state.at.x = MAPX(x);
					new_state.at.y = MAPY(y);
					new_state.buttons =
						  ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					new_state.pressure = (new_state.buttons ? 1.0f : 0.0f);
					kit::dispatch_pointer_action(MouseID, kit::PointerLeave, new_state);
					kit::commit_mode();
				} else if (evt.type == SDL_EVENT_MOUSE_MOTION) {
					kit::Pointer new_state;
					new_state.at.x = MAPX(evt.motion.x);
					new_state.at.y = MAPY(evt.motion.y);
					new_state.buttons =
						  ((evt.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((evt.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((evt.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					new_state.pressure = (new_state.buttons ? 1.0f : 0.0f);
					kit::dispatch_pointer_action(MouseID, kit::PointerMove, new_state);
					kit::commit_mode();
				} else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
					kit::Pointer new_state;
					new_state.at.x = MAPX(evt.motion.x);
					new_state.at.y = MAPY(evt.motion.y);
					float x,y;
					Uint32 buttons = SDL_GetMouseState(&x, &y);
					new_state.buttons =
						  ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((buttons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					new_state.pressure = (new_state.buttons ? 1.0f : 0.0f);
					kit::dispatch_pointer_action(MouseID, (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? kit::PointerDown : kit::PointerUp), new_state);
					kit::commit_mode();
				}
				#endif // __APPLE__
				if (evt.type == SDL_EVENT_KEY_DOWN || evt.type == SDL_EVENT_KEY_UP) {
					if (evt.key.repeat == 0) {
						kit::Button::handle_event(evt);
						kit::commit_mode();
					}
				}
				//exit not-so-gracefully on a "quit" message:
				if (evt.type == SDL_EVENT_QUIT) {
					kit::set_mode( nullptr );
					kit::commit_mode();
				}
			}
		}
		float elapsed;
		{
			static auto then = std::chrono::steady_clock::now();
			auto now = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration< float >(now - then).count();
			//TODO: wait if updating too quickly.
			then = now;
		}
		if (mode) {
			mode->update(elapsed);
			kit::commit_mode();
		}
		if (mode) {
			mode->draw();
			kit::commit_mode();
		}
		SDL_GL_SwapWindow(window);
	}

	#ifdef __APPLE__
	kit::osx::stop_pointer_handling();
	#endif

	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);

#ifdef _WIN32
	} catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		throw e;
	}
#endif

	return 0;
}
