#pragma once

#include "gl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <list>
#include <functional>
#include <string>

//Kit apps must define two things:
//(1) Configuration function. Called before GL context exists, requests configuration:
namespace kit {
	struct Config {
		//requested context:
		glm::uvec2 size = glm::uvec2(100, 100);
		bool resizable = true; 
		//requested window title:
		std::string title = "kit2";
		//desired time between frames in various states:
		float hidden_update  = 0.0f;
		float visible_update = 0.0f;
		float active_update  = 0.0f;
	};
}
kit::Config kit_config(); //**YOU MUST DEFINE THIS***

//(2) main entrypoint. Called after everything is initialized. Should return a mode to run, or nullptr if something hasn't been set up properly.
namespace kit {
	struct Mode;
}
std::shared_ptr< kit::Mode > kit_mode(); //**YOU MUST DEFINE THIS***


#ifdef KIT_RAW_SDL_EVENTS
union SDL_Event;
#endif

namespace kit {
	//command-line arguments. First is the name of the program.
	// (may not exist on all platforms)
	extern std::vector< std::string > args;

	enum State : uint8_t {
		Stopped = 0, //App isn't running (or fully set up yet).
			//Stopped -> Hidden after initialization completes.
		Hidden  = 1, //App is running but not on screen; e.g. minimized
			//Hidden -> Visible when window shown.
			//Hidden -> Stopped when terminated.
		Visible = 2, //App is running and visible, but doesn't have input focus.
			//Visible -> Active when window gains input focus.
			//Visible -> Hidden when window minimized / switched away from.
		Active  = 3, //App is running, visible, and has input focus.
			//Active -> Visible when input focus lost.
			
	};
	extern State state;

	//information about the current display (== opengl context) is stored in kit::display:
	struct Display {
		glm::uvec2 size = glm::uvec2(0);
		#ifdef KIT_RAW_SDL_EVENTS
		glm::uvec2 window_size = glm::uvec2(0);
		#endif
		float aspect = 1.0f; //aspect ratio (x / y)
		float pixel_ratio = 1.0f; //size of a layout pixel in display pixels (1.0 for non-HighDPI devices)
		float DPI = std::numeric_limits< float >::quiet_NaN();
		float IPD = std::numeric_limits< float >::quiet_NaN();
	};
	extern Display display;

	//Pointer/Touch input is handled through Pointer structures:
	typedef uint32_t PointerID;
	enum PointerAction : uint8_t {
		PointerEnter, //enters window/proximity/focus (will start getting Move events)
		PointerLeave, //left window/proximity/focus (will stop getting Move events)
		PointerDown, //pressed/touched
		PointerUp,   //released/lifted
		PointerMove,
	};
	enum PointerButton : uint8_t {
		ButtonLeft = (1 << 0),
		ButtonMiddle = (1 << 1),
		ButtonRight = (1 << 2),
		WheelDown = (1 << 3),
		WheelUp = (1 << 4),
		Touch = ButtonLeft,
	};

	//information about all currently tracked pointers:
	struct Pointer {
		/*enum Type {
			Mouse,
			ScreenFinger, PadFinger,
			ScreenStylus, PadStylus,
		} type = Mouse;*/

		uint8_t buttons = 0; //bits indicating state of buttons

		//'at' is in normalized [-1,-1]x[-1,1] display coordinates
		glm::vec2 at = glm::vec2(std::numeric_limits< float >::quiet_NaN());

		//pressure is in the 0.0f .. 1.0f range; for devices that don't support pressure, will be 0.0f if buttons are up and 1.0f if buttons are down.
		float pressure = 0.0f;
	};
	extern std::unordered_map< PointerID, Pointer > pointers;

	struct Mode : public std::enable_shared_from_this< Mode > {
		virtual ~Mode() { };
		//called before 'update' or 'draw' if the display dimensions have changed:
		virtual void resized() { }
		//update will be called periodically (see config.*_tick)
		virtual void update(float elapsed) { }
		//called after update() when app is visible; must fill current fb
		virtual void draw() { }

		//called just before pointer values change:
		virtual void pointer_action(PointerID pointer, PointerAction action, Pointer const &old_state, Pointer const &new_state) { }
		
		//if you want this function, make sure -DKIT_RAW_SDL_EVENTS is passed to all compiled code;
		//if using the Jamfiles, you can do this by setting KIT_RAW_SDL_EVENTS=1
		#ifdef KIT_RAW_SDL_EVENTS
		virtual void handle_event(SDL_Event const &) { }
		#endif
	};


	//set_mode will set the current mode. This will involve Leave and Enter events on mouse pointers, and Leave events on fingers.
	//note that calls to set_mode from mode callbacks don't take effect until after said callbacks have finished executing.
	void set_mode(std::shared_ptr< Mode > const &mode);

	//internal stuff:
	std::shared_ptr< Mode > const &get_mode();
	void commit_mode();
	void dispatch_pointer_action(PointerID pointer, PointerAction action, Pointer const &new_state);
} //namespace kit
