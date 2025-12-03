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
// 
// -----This file works as it's own program to create the file for data analysis, not used in the main program-----
//     dataCollection.h : Data collection and file creation from TMDB API, also collects images for use in window
// ----------------------------------------------------------------------------------------------------------------
// 
// assets/              : All assets used in the program (mostly images for U/I)
//        images/       : All images used in the program
//        data.     : Data file created from dataCollection.h for use in the program (As of 5:50PM on 11/25/2025, have not decided on file format)



//Main function
int main() {
	/*const std::string yearPath = "assets/yearStatus.csv";
	runWorker(yearPath);*/


	//referenced https://www.sfml-dev.org/
	//Color Palette: https://colorswall.com/palette/27237

	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "A Study of Actor Networks: Shortest Hops vs. Strongest Connections", sf::State::Windowed);
	
	//get the size of the window
	sf::Vector2u size = window.getSize();

	// Safilms Crawford Font - https://www.1001freefonts.com/safilms-crawford.font
	sf::Font scFont;
	if (!scFont.openFromFile("Fonts/Safilms_Crawford.otf")) {
		std::cerr << "Error:: Could not load font file::" << std::endl;
	}

	// BebasNeue_Regular Font - https://www.1001freefonts.com/bebas-neue.font
	sf::Font bnFont;
	if (!bnFont.openFromFile("Fonts/BebasNeue_Regular.ttf")) {
		std::cerr << "Error:: Could not load font file::" << std::endl;
	}


	sf::Text text(scFont);
	text.setString("Find Paths");
	text.setCharacterSize(80);
	text.setFillColor(sf::Color::White);
	//text.setStyle(sf::Text::Bold | sf::Text::Underlined);
	text.setPosition(sf::Vector2f(size.x/9 , size.y/15));


	sf::Text actor1(bnFont);
	actor1.setString("Actor 1");
	actor1.setCharacterSize(28);
	actor1.setFillColor(sf::Color::White);
	actor1.setPosition(sf::Vector2f(size.x / 18, size.y / 6));

	sf::Text actor2(bnFont);
	actor2.setString("Actor 2");
	actor2.setCharacterSize(28);
	actor2.setFillColor(sf::Color::White);
	actor2.setPosition(sf::Vector2f(size.x / 18, size.y / 4));

	sf::Text resultText(bnFont);
	resultText.setString("Results");
	resultText.setCharacterSize(28);
	resultText.setFillColor(sf::Color::White);
	resultText.setPosition(sf::Vector2f(size.x / 18, size.y / 2.2));


	sf::Text note(bnFont);
	note.setString("Note: BFS finds the fastest path by hop count.\n  Dijkstra finds the strongest path through actors\n who worked tohether more frequently.");
	note.setCharacterSize(28);
	note.setFillColor(sf::Color::White);
	note.setPosition(sf::Vector2f(size.x / 18, size.y - size.y / 5.7));

	

	//creating gradient background
	sf::VertexArray square(sf::PrimitiveType::TriangleStrip, 4);

	// define the position of the squares's points
	square[0].position = sf::Vector2f(size.x*.25, 0);
	square[1].position = sf::Vector2f(size.x, 0);
	square[2].position = sf::Vector2f(size.x*.25, size.y);
	square[3].position = sf::Vector2f(size.x, size.y);


	// define the color of the square's points
	square[0].color = sf::Color(0, 20, 41);
	square[1].color = sf::Color(0, 10, 20);
	square[2].color = sf::Color(0, 41, 82);
	square[3].color = sf::Color(0, 31, 61);


	sf::Texture texture;
	if (!texture.loadFromFile("Sprites/clapperboard.png")) {
		std::cerr << "Error:: Could not load image file::" << std::endl;
	}

	sf::Sprite clapper(texture);
	clapper.setScale(sf::Vector2f(0.09, 0.09));
	clapper.setPosition(sf::Vector2f(0, 0));




	sf::RectangleShape topActorRectangle({ 500.f, 50.f });
	topActorRectangle.setFillColor(sf::Color(0, 41, 82));
	topActorRectangle.setOutlineThickness(10);
	topActorRectangle.setOutlineColor(sf::Color(0, 82, 163));
	topActorRectangle.setPosition(sf::Vector2f(size.x / 20, size.y/5));

	sf::RectangleShape botActorRectangle({ 500.f, 50.f });
	botActorRectangle.setFillColor(sf::Color(0, 41, 82));
	botActorRectangle.setOutlineThickness(10);
	botActorRectangle.setOutlineColor(sf::Color(0, 82, 163));
	botActorRectangle.setPosition(sf::Vector2f(size.x / 20, size.y / 3.5));

	sf::RectangleShape bfsRectangle({ 500.f, 175.f });
	bfsRectangle.setFillColor(sf::Color(0, 41, 82));
	bfsRectangle.setOutlineThickness(10);
	bfsRectangle.setOutlineColor(sf::Color(0, 82, 163));
	bfsRectangle.setPosition(sf::Vector2f(size.x / 20, size.y / 2.05));

	sf::RectangleShape dijRectangle({ 500.f, 175.f });
	dijRectangle.setFillColor(sf::Color(0, 41, 82));
	dijRectangle.setOutlineThickness(10);
	dijRectangle.setOutlineColor(sf::Color(0, 82, 163));
	dijRectangle.setPosition(sf::Vector2f(size.x / 20, size.y-size.y/2.5));

	sf::RectangleShape keyRectangle({ 500.f, 100.f });
	keyRectangle.setFillColor(sf::Color(0, 41, 82));
	keyRectangle.setOutlineThickness(10);
	keyRectangle.setOutlineColor(sf::Color(0, 82, 163));
	keyRectangle.setPosition(sf::Vector2f(size.x / 20, size.y - size.y / 4));

	sf::RectangleShape noteRectangle({ 500.f, 100.f });
	noteRectangle.setFillColor(sf::Color(0, 41, 82));
	noteRectangle.setOutlineThickness(10);
	noteRectangle.setOutlineColor(sf::Color(0, 82, 163));
	noteRectangle.setPosition(sf::Vector2f(size.x / 20, size.y - size.y/5.7));
	

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		window.clear(sf::Color(0, 10, 20));

		window.draw(text);

		window.draw(square);

		window.draw(actor1);

		window.draw(actor2);

		window.draw(clapper);

		window.draw(topActorRectangle);

		window.draw(botActorRectangle);

		window.draw(resultText);

		window.draw(bfsRectangle);

		window.draw(dijRectangle);

		window.draw(keyRectangle);

		

		window.draw(noteRectangle);

		window.draw(note);

		window.display();

		

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
