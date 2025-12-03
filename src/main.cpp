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
#include "dataCollection.h"
#include "config.h"

//		Organization of Files:
// src/			        : All source code files
//     main.cpp         : Main entry point of the program
//     window.h/cpp     : Window creation and management
//     graph.h/cpp      : Graph data structure implementation, graph is weighted, and can be called unweighted. Nodes are weighted for visual usage
//     bfs.h/cpp	    : Breadth-First Search algorithm implementation
//     dijkstra.h/cpp   : Dijkstra's algorithm implementation
//     config.h/cpp     : Handles config settings
//     dataCollection.h : Data collection and file creation from TMDB API, also collects images for use in window. Used to manage vector of actors as well.
// ----------------------------------------------------------------------------------------------------------------
// 
// assets/              : All assets used in the program (mostly images for U/I)
//        images/       : All images used in the program
//        data.     : Data file created from dataCollection.h for use in the program (As of 5:50PM on 11/25/2025, have not decided on file format)



//Main function
int main() {
	/*
	if (combineDatabaseYears(1900, 2012)) {
		std::cout << "Database combined successfully.\n";
	}
	else {
		std::cerr << "Error combining database.\n";
		return 1;
	}
	*/

	//SQLite::Database db = openMainDatabase();
	//loadActorDataFromDB(db);

	
	//Data Collection Code - Uncomment to run data collection separately
	const std::string yearPath = "assets/yearStatus.csv";
	runWorker(yearPath);
	/*
	============================================================
	//referenced https://www.sfml-dev.org/
	//Color Palette: https://colorswall.com/palette/27237
	*/

	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "A Study of Actor Networks: Shortest Hops vs. Strongest Connections", sf::State::Windowed);
	Window mainWindow(window);
	if (mainWindow.loadAllResources() == 0) {
		std::cout << "All Resources Loaded!\n";
	}
	/*
	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}
		window.clear();
		if (mainWindow.leftMouseClicked()) {
			std::cout << "Left Mouse Clicked at: " << sf::Mouse::getPosition(window).x << ", " << sf::Mouse::getPosition(window).y << "\n";
		}
		mainWindow.renderMainMenu();

		window.display();

		

	}
	*/
	return 0;
}




