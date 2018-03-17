//TODO: #include "YourMode.hpp"

#include <kit/kit.hpp>
#include <kit/Load.hpp>

#include <fstream>
#include <sstream>

kit::Config kit_config() {
	kit::Config config;
	config.size = glm::uvec2(1280, 720);
	config.title = "kit skel"; //TODO: your title

	return config;
}

std::shared_ptr< kit::Mode > kit_mode() {
	kit::call_load_functions(); //only needed if using kit::Load< > for resource loading

	//you can parse arguments here:
	//for (uint32_t argi = 1; argi < kit::args.size(); ++argi) ...

	return nullptr;
	//TODO: return std::make_shared< YourMode >();
}

