#pragma once
#include <nlohmann/json.hpp>    //For JSON parsing
//A bunch of C++ built in libraries
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept> 
#include <filesystem>
#include <utility>
#include <cstdlib>
#include <thread>
#include <stop_token>
#include <chrono>

using json = nlohmann::json;

namespace Config {
	extern const std::string TMDB_API_KEY;

	void loadConfig();
}