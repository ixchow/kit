#include "Button.hpp"

#include <SDL.h>

#include <list>
#include <map>
#include <stdexcept>

namespace kit {

struct ButtonInternal : public Button {
	std::string name;
	Button::Config config;
};

struct ButtonList : std::list< ButtonInternal > {
	std::multimap< int, ButtonInternal * > by_scancode;
	std::map< std::string, ButtonInternal * > by_name;
	static ButtonList &instance() {
		static ButtonList list;
		return list;
	}
};

Button const *Button::create(std::string const &name, Config const &config) {
	static ButtonList &list = ButtonList::instance();
	if (list.by_name.count(name)) {
		throw std::runtime_error("Two buttons with the name '" + name + "' created.");
	}
	list.emplace_back();
	list.back().name = name;
	list.back().config = config;

	list.by_name.insert(std::make_pair(name, &list.back()));
	list.by_scancode.insert(std::make_pair(config.scancode, &list.back()));

	return &list.back();
}

void Button::clear_events() {
	static ButtonList &list = ButtonList::instance();
	for (auto &b : list) {
		b.downs = 0;
		b.ups = 0;
	}
}

void Button::handle_event(SDL_Event const &evt) {
	static ButtonList &list = ButtonList::instance();
	if ((evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) && evt.key.repeat == 0) {
		auto r = list.by_scancode.equal_range(evt.key.keysym.scancode);
		for (auto bi = r.first; bi != r.second; ++bi) {
			if (evt.type == SDL_KEYDOWN) {
				bi->second->downs += 1;
			} else {
				bi->second->ups += 1;
			}
			bi->second->pressed = (evt.key.state == SDL_PRESSED);
		}
	}
}

}
