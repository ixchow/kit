#include "kit.hpp"
#include "gl.hpp"

#include "Button.hpp"

#include <SDL.h>

#include <iostream>
#include <chrono>

int main(int argc, char **argv) {
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


	SDL_Window *window = SDL_CreateWindow(
		kit_config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		kit_config.size.x, kit_config.size.y,
		SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
	);

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


	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	kit::set_mode(kit_mode());
	kit::commit_mode();

	std::shared_ptr< kit::Mode > const &mode = kit::get_mode();

	const kit::PointerID MouseID = 1;

	glm::uvec2 window_size = glm::uvec2(0,0);

	while (mode) {
		SDL_GL_MakeCurrent(window, context);

		{
			int w,h;
			SDL_GetWindowSize(window, &w, &h);
			window_size = glm::uvec2(w,h);
		}
		{
			int w,h;
			SDL_GL_GetDrawableSize(window, &w, &h);
			glm::uvec2 size = glm::uvec2(w,h);
			if (size != kit::display.size) {
				kit::display.size = size;
				if (size.y != 0) {
					kit::display.aspect = float(size.x) / float(size.y);
				} else {
					kit::display.aspect = 1.0f;
				}
				//TODO: look these up somehow!
				kit::display.DPI = 90.0f;
				kit::display.IPD = 1.0f / kit::display.DPI;

				glViewport(0,0,size.x,size.y);
				if (mode) {
					mode->resized();
					kit::commit_mode();
				}
			}
		}


		{
			static SDL_Event evt;
			kit::Button::clear_events();
			while (SDL_PollEvent(&evt) == 1 && mode) {
				#define MAPX( X ) ((((X) + 0.5f) / window_size.x) * 2.0f - 1.0f)
				#define MAPY( Y ) ((((Y) + 0.5f) / window_size.y) *-2.0f + 1.0f)
				if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_ENTER) {
					//std::cout << "Mouse Enter." << std::endl; //DEBUG
					kit::Pointer new_state;
					int x,y;
					Uint32 buttons = SDL_GetMouseState(&x, &y);
					new_state.at.x = MAPX(x);
					new_state.at.y = MAPY(y);
					new_state.buttons =
						  ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					kit::dispatch_pointer_action(MouseID, kit::PointerEnter, new_state);
				} else if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_LEAVE) {
					//std::cout << "Mouse Leave." << std::endl; //DEBUG
					kit::Pointer new_state;
					int x,y;
					Uint32 buttons = SDL_GetMouseState(&x, &y);
					new_state.at.x = MAPX(x);
					new_state.at.y = MAPY(y);
					new_state.buttons =
						  ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					kit::dispatch_pointer_action(MouseID, kit::PointerLeave, new_state);
				} else if (evt.type == SDL_MOUSEMOTION) {
					kit::Pointer new_state;
					new_state.at.x = MAPX(evt.motion.x);
					new_state.at.y = MAPY(evt.motion.y);
					new_state.buttons =
						  ((evt.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((evt.motion.state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((evt.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					kit::dispatch_pointer_action(MouseID, kit::PointerMove, new_state);
				} else if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP) {
					kit::Pointer new_state;
					new_state.at.x = MAPX(evt.motion.x);
					new_state.at.y = MAPY(evt.motion.y);
					new_state.buttons =
						  ((evt.button.state & SDL_BUTTON(SDL_BUTTON_LEFT)) ? kit::ButtonLeft : 0)
						| ((evt.button.state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? kit::ButtonMiddle : 0)
						| ((evt.button.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? kit::ButtonRight : 0)
					;
					kit::dispatch_pointer_action(MouseID, (evt.type == SDL_MOUSEBUTTONDOWN ? kit::PointerDown : kit::PointerUp), new_state);
				}
				if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
					if (evt.key.repeat == 0) {
						kit::Button::handle_event(evt);
					}
				}
				//exit not-so-gracefully on a "quit" message:
				if (evt.type == SDL_QUIT) {
					kit::set_mode( nullptr );
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

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);

	return 0;
}
