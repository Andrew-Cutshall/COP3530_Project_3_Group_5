#pragma once
#include <SFML\Graphics.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <format>
#include <stdexcept> 
#include <filesystem>
#include <utility>
#include <cstdlib>
#include <thread>
#include "window.h"
#include "graph.h"   
#include "bfh.h"
#include "dijkstra.h" 
#include "dataCollection.h"

class Window {
public:
	Window(sf::RenderWindow& mainWindow);

	int loadAllResources();

	int renderMainMenu();

	sf::RenderTexture& renderMainMenuTexture();

	//std::tuple<sf::Sprite, sf::Text, sf::FloatRect> drawTextBox(int x, int y, std::string input);
	sf::Vector2f getMousePosition();
	//Checks and returns whether left and right mouse button is clicked.
	bool leftMouseClicked();
	bool rightMouseClicked();

private:
	sf::RenderWindow& mainWindow;
	sf::RenderTexture drawnTexture;
	sf::Vector2u windowSize;
	sf::Texture texture;
	sf::Font scFont;
	sf::Font bnFont;
	float scale;

	sf::Vector2f mousePosition;
	bool leftClick;
	bool rightClick;
};