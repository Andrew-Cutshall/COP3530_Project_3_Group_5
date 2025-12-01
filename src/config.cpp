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

#include "config.h"
#include "dataCollection.h"



namespace Config {
	const std::string TMDB_API_KEY = [] {
		if (!changeToGitRoot()) {
			std::cerr << "FATAL: Could not access Git directory. Aborting worker.\n";
			exit(1);
		}
		try {
			std::ifstream file("assets/config.cfg");
			if (!file.is_open()) {
				throw std::runtime_error("COULD NOT OPEN CONFIG!\nCheck File Path\n");
			}
			json data = json::parse(file);
			std::string key = data.value("tmdb_api_key", "");
			if (key.empty()) {
				throw std::runtime_error("Don't forget to put your API key in assets/config.cfg!\n");
			}
			return key;
		}
		catch (const std::exception& e) {
			std::cerr << "FATAL Configuration Error: Failed to load TMDB_KEY: " << e.what() << "\n";
			exit(1);
		}
	}();

	void loadConfig() {
		if (TMDB_API_KEY.empty()) {
			throw std::runtime_error("TMDB API Key is not set!");
		}
	}
}
