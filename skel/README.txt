This is the skeleton of a kit project:

Jamfile - describes the files to build
Jamrules - references kit's Jamrules file
init.cpp - contains definitions for kit_config() and kit_mode() to set up your app
.gitignore - kit's Jamrules builds into the 'objs/' subdirectory. This .gitignore ignores that directory.

You will generally create a subclass of kit::Mode in another file (e.g., YourMode.hpp/YourMode.cpp) and instantiate it in the kit_mode() function in init.cpp .
