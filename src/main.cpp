//SFML Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

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

//Header Files
#include "window.h"
#include "graph.h"
#include "bfh.h"
#include "dijkstra.h"
#include "otherFunctions.h"
#include "dataCollection.h"

//		Organization of Files:
// src/			        : All source code files
//     main.cpp         : Main entry point of the program
//     window.h/cpp     : Window creation and management
//     graph.h/cpp      : Graph data structure implementation, graph is weighted, and can be called unweighted. Nodes are weighted for visual usage
//     bfs.h/cpp	    : Breadth-First Search algorithm implementation
//     dijkstra.h/cpp   : Dijkstra's algorithm implementation
//     apiKey.h         : Contains the API key for TMDB API (not included in repository for security reasons, will be provided, or can be put in by user)
//     otherFunctions.h : Helpers and other functions that don't fit in other files
// 
// -----This file works as it's own program to create the file for data analysis, not used in the main program-----
//     dataCollection.h : Data collection and file creation from TMDB API, also collects images for use in window
// ----------------------------------------------------------------------------------------------------------------
// 
// assets/              : All assets used in the program (mostly images for U/I)
//        images/       : All images used in the program
//        data.smth     : Data file created from dataCollection.h for use in the program (As of 5:50PM on 11/25/2025, have not decided on file format)



//Main function
int main() {
    try {
        SQLite::Database db("assets/movieData.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        std::cout << "Database opened successfully!" << std::endl;
        setupDatabase(db);
        std::cout << "Total Actor Count: " << getTotalActors(db) << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "\nCRITICAL ERROR: Application terminated due to database failure: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}


/*
//Main 2 is used for interal testing, main() will be handling window creation and logic
int main2() {
	std::cout << "Hello World from main2!" << std::endl;
	return 0;
}
*/