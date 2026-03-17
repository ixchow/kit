#pragma once

#include <string>
#include <cstdint>

union SDL_Event;

namespace kit {

struct Button {
	uint8_t downs = 0; //presses this update
	uint8_t ups = 0; //releases this update
	bool pressed = false; //currently pressed?

	//Buttons are created by 'create':
	struct Config {
		Config() = default;
		Config(int keycode_) : keycode(keycode_) { }
		int keycode = -1; //SDLK_* constant
		//TODO: joystick? mouse?
	};
	static Button const *create(std::string const &name, Config const &config);

	//internals:
	static void clear_events();
	static void handle_event(SDL_Event const &evt);
};

}
