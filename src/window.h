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


private:
	sf::RenderWindow& mainWindow;
	sf::RenderTexture drawnTexture;
	sf::Vector2u windowSize;
	sf::Texture texture;
	sf::Font scFont;
	sf::Font bnFont;
};