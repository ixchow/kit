#include "kit-SDL2-osx.hpp"

#include "kit.hpp"

#include <iostream>

#import <AppKit/AppKit.h>

id eventMonitor = nil;
NSWindow __unsafe_unretained *window = nil;

void kit::osx::start_pointer_handling(void *window_) {//NSWindow __unsafe_unretained *window_) {
	assert(window_);
	assert(eventMonitor == nil);
	window = (decltype(window))(window_);
	NSEvent.mouseCoalescingEnabled = false; //ask for high-resolution mouse/tablet info

	eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskAny handler:^(NSEvent *evt){
		//get_loc is a helper to get x,y, (z = pressure) from event
		auto get_loc = [&]() -> glm::vec3 {
			NSPoint loc = evt.locationInWindow;
			NSRect frame = window.frame;
			if (evt.window == nil) {
				loc.x -= frame.origin.x;
				loc.y -= frame.origin.y;
			}
			loc.x = (loc.x / frame.size.width) * 2.0f - 1.0f;
			loc.y = (loc.y / frame.size.height) * 2.0f - 1.0f;
			float pressure = evt.pressure;
			return glm::vec3(loc.x, loc.y, pressure);
		};
		auto get_button = [&]() -> kit::PointerButton {
			NSInteger b = evt.buttonNumber;
			if      (b == 0) return kit::ButtonLeft;
			else if (b == 1) return kit::ButtonRight;
			else if (b == 2) return kit::ButtonMiddle; //not sure if this is actually the case
			else return kit::PointerButton(0);
		};
		auto get_id = [&]() -> kit::PointerID {
			NSEventSubtype subtype = evt.subtype;
			if (subtype == NSEventSubtypeTabletPoint) {
				return evt.deviceID ^ 0xff000000;
			} else if (subtype == NSEventSubtypeTabletProximity) {
				return evt.deviceID ^ 0xff000000;
			} else {
				return kit::PointerID(1);
			}
		};


		switch ([evt type]) {
			case NSEventTypeLeftMouseDown:
			case NSEventTypeRightMouseDown:
			case NSEventTypeOtherMouseDown: {
				kit::PointerID pointer = get_id();

				Pointer state;
				{ //look up old state (for incremental button tracking):
					auto f = kit::pointers.find(pointer);
					if (f != kit::pointers.end()) state = f->second;
				}

				state.buttons |= get_button();

				glm::vec3 loc = get_loc();
				state.at = glm::vec2(loc);
				state.pressure = loc.z;
				//std::cout << "Down: " << buttonNumber << " at " << loc.x << " " << loc.y << " " << loc.z << std::endl;
				kit::dispatch_pointer_action(pointer, kit::PointerDown, state);
				break;
			}
			case NSEventTypeLeftMouseUp:
			case NSEventTypeRightMouseUp:
			case NSEventTypeOtherMouseUp: {
				kit::PointerID pointer = get_id();

				Pointer state;
				{ //look up old state (for incremental button tracking):
					auto f = kit::pointers.find(pointer);
					if (f != kit::pointers.end()) state = f->second;
				}

				state.buttons &= ~(get_button());

				glm::vec3 loc = get_loc();
				state.at = glm::vec2(loc);
				state.pressure = loc.z;
				//std::cout << "Up: " << buttonNumber << " at " << loc.x << " " << loc.y << " " << loc.z << std::endl;
				kit::dispatch_pointer_action(pointer, kit::PointerUp, state);
				break;
			}
			case NSEventTypeLeftMouseDragged:
			case NSEventTypeRightMouseDragged:
			case NSEventTypeOtherMouseDragged:
			case NSEventTypeMouseMoved: {
				kit::PointerID pointer = get_id();


				bool entering = false;
				Pointer state;
				{ //look up old state (for incremental button tracking):
					auto f = kit::pointers.find(pointer);
					if (f != kit::pointers.end()) state = f->second;
					else entering = true;
				}

				glm::vec3 loc = get_loc();
				state.at = glm::vec2(loc);
				state.pressure = loc.z;

				if (evt.subtype == NSEventSubtypeTabletProximity) {
					if (evt.enteringProximity) {
						kit::dispatch_pointer_action(pointer, kit::PointerEnter, state);
					} else {
						kit::dispatch_pointer_action(pointer, kit::PointerLeave, state);
					}
				} else {
					if (entering) { //this is a hack because I don't have event handling code to track mouse stuff
						kit::dispatch_pointer_action(pointer, kit::PointerEnter, state);
					} else {
						kit::dispatch_pointer_action(pointer, kit::PointerMove, state);
					}
				}
				break;
			}
			default: {
				break;
			}
		}
		return evt;
	}];
}

bool kit::osx::is_pointer_handling() {
	return eventMonitor != nil;
}

void kit::osx::stop_pointer_handling() {
	assert(eventMonitor != nil);
	[NSEvent removeMonitor:eventMonitor];
	eventMonitor = nil;
}
