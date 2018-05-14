#include "kit.hpp"

#include <iostream>

namespace kit {
	std::vector< std::string > args;
	State state;
	Display display;
	Config config;
	std::unordered_map< PointerID, Pointer > pointers;
}

namespace {
	std::shared_ptr< kit::Mode > current_mode = nullptr;
	std::shared_ptr< kit::Mode > pending_mode = nullptr;
}

void kit::set_mode(std::shared_ptr< Mode > const &new_mode) {
	pending_mode = new_mode;
}

std::shared_ptr< kit::Mode > const &kit::get_mode() {
	return current_mode;
}

void kit::commit_mode() {
	if (current_mode == pending_mode) return;

	std::unordered_map< PointerID, Pointer > old_pointers = pointers;

	for (auto p : old_pointers) {
		dispatch_pointer_action(p.first, PointerLeave, p.second);
	}
	assert(pointers.empty());

	current_mode = pending_mode;

	for (auto p : old_pointers) {
		dispatch_pointer_action(p.first, PointerEnter, p.second);
	}
}

void kit::dispatch_pointer_action(PointerID pointer, PointerAction action, Pointer const &new_state) {
	if (action == PointerEnter) {
		//code shouldn't dispatch_pointer_action if pointer exists already:
		if (pointers.count(pointer)) {
			std::cerr << "WARNING: got an Enter action for an already-present pointer." << std::endl;
			return;
		}
		if (current_mode) {
			current_mode->pointer_action(pointer, action, new_state, new_state);
		}
		pointers.insert(std::make_pair(pointer, new_state));
		return;
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - -
	} else if (action == PointerLeave) {
		auto p = pointers.find(pointer);
		if (p == pointers.end()) {
			std::cerr << "WARNING: got a Leave action for a pointer that isn't present." << std::endl;
			return;
		}
		Pointer const &old_state = p->second;
		if (new_state.at != old_state.at) {
			std::cerr << "WARNING: pointer moved during Leave action." << std::endl;
		}
		if (new_state.buttons != old_state.buttons) {
			std::cerr << "WARNING: pointer buttons changed during Leave action." << std::endl;
		}
		if (current_mode) {
			current_mode->pointer_action(pointer, action, old_state, new_state);
		}
		pointers.erase(p);
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - -
	} else if (action == PointerDown) {
		auto p = pointers.find(pointer);
		if (p == pointers.end()) {
			std::cerr << "WARNING: got a Down action for a pointer that isn't present." << std::endl;
			return;
		}
		Pointer const &old_state = p->second;
		if (new_state.at != old_state.at) {
			std::cerr << "WARNING: pointer moved during Down action." << std::endl;
		}
		if (new_state.buttons == old_state.buttons) {
			std::cerr << "WARNING: no button change during Down action." << std::endl;
		}
		if ((old_state.buttons ^ (new_state.buttons & old_state.buttons)) != 0) {
			std::cerr << "WARNING: buttons lifted during Down action." << std::endl;
		}

		if (current_mode) {
			current_mode->pointer_action(pointer, action, old_state, new_state);
		}
		p->second = new_state;
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - -
	} else if (action == PointerUp) {
		auto p = pointers.find(pointer);
		if (p == pointers.end()) {
			std::cerr << "WARNING: got a Up action for a pointer that isn't present." << std::endl;
			return;
		}
		Pointer const &old_state = p->second;
		if (new_state.at != old_state.at) {
			std::cerr << "WARNING: pointer moved during Up action." << std::endl;
		}
		if (new_state.buttons == old_state.buttons) {
			std::cerr << "WARNING: no button change during Up action." << std::endl;
		}
		if ((new_state.buttons ^ (new_state.buttons & old_state.buttons)) != 0) {
			std::cerr << "WARNING: buttons pressed during Up action." << std::endl;
		}

		if (current_mode) {
			current_mode->pointer_action(pointer, action, old_state, new_state);
		}
		p->second = new_state;
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - -
	} else if (action == PointerMove) {
		auto p = pointers.find(pointer);
		if (p == pointers.end()) {
			std::cerr << "WARNING: got a Move action for a pointer that isn't present." << std::endl;
			return;
		}
		Pointer const &old_state = p->second;
		if (new_state.at == old_state.at && new_state.pressure == old_state.pressure) {
			std::cerr << "WARNING: pointer didn't move during Move action." << std::endl;
		}
		if (new_state.buttons != old_state.buttons) {
			std::cerr << "WARNING: button change during Move action." << std::endl;
		}

		if (current_mode) {
			current_mode->pointer_action(pointer, action, old_state, new_state);
		}
		p->second = new_state;

	} else {
		assert(0 && "Invalid PointerAction");
	}
}
