#pragma once

#include <string>

extern std::string data_path_subdir; //pasted into every data_path before 'suffix'; e.g., "data/", default is "" (nothing)
std::string data_path(std::string const &suffix);
std::string user_path(std::string const &suffix);
