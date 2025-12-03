//SFML Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

// Your project headers
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


Window::Window(sf::RenderWindow& setWindow) : mainWindow(setWindow) {
	windowSize = mainWindow.getSize();
    drawnTexture.resize(windowSize);
    float xScreenResolution = windowSize.x;
    float yScreenResolution = windowSize.y;
    scale = (float)xScreenResolution / (float)yScreenResolution;
}

int Window::loadAllResources() {
	int errorCode = 0;
    if (!bnFont.openFromFile("Fonts/BebasNeue_Regular.ttf")) {
        std::cerr << "Error:: Could not load font file::" << std::endl;
        errorCode += 1;
    }
    if (!scFont.openFromFile("Fonts/Safilms_Crawford.otf")) {
        std::cerr << "Error:: Could not load font file::" << std::endl;
        errorCode += 1;
    }
    if (!texture.loadFromFile("Sprites/clapperboard.png")) {
        std::cerr << "Error:: Could not load image file::" << std::endl;
        errorCode += 1;
    }

	return errorCode;
}

int Window::renderMainMenu() {
    mainWindow.clear(sf::Color(0, 10, 20));
    sf::Sprite menu(renderMainMenuTexture().getTexture());
	menu.setPosition(sf::Vector2f(0, 0));
    mainWindow.draw(menu);
    return 0;
}

sf::RenderTexture& Window::renderMainMenuTexture() {
    drawnTexture.clear();
    sf::Vector2u size = windowSize;

    sf::Text text(scFont);
    text.setString("Find Paths");
    text.setCharacterSize(80);
    text.setFillColor(sf::Color::White);
    //text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    text.setPosition(sf::Vector2f(size.x / 9, size.y / 15));

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

    sf::VertexArray square(sf::PrimitiveType::TriangleStrip, 4);

    // define the position of the squares's points
    square[0].position = sf::Vector2f(size.x * .25, 0);
    square[1].position = sf::Vector2f(size.x, 0);
    square[2].position = sf::Vector2f(size.x * .25, size.y);
    square[3].position = sf::Vector2f(size.x, size.y);


    // define the color of the square's points
    square[0].color = sf::Color(0, 20, 41);
    square[1].color = sf::Color(0, 10, 20);
    square[2].color = sf::Color(0, 41, 82);
    square[3].color = sf::Color(0, 31, 61);

    sf::Sprite clapper(texture);
    clapper.setScale(sf::Vector2f(scale + 1.0f, scale + 1.0f));
    clapper.setPosition(sf::Vector2f(0, 0));

    sf::RectangleShape topActorRectangle({ 500.f, 50.f });
    topActorRectangle.setFillColor(sf::Color(0, 41, 82));
    topActorRectangle.setOutlineThickness(10);
    topActorRectangle.setOutlineColor(sf::Color(0, 82, 163));
    topActorRectangle.setPosition(sf::Vector2f(size.x / 20, size.y / 5));

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
    dijRectangle.setPosition(sf::Vector2f(size.x / 20, size.y - size.y / 2.5));

    sf::RectangleShape keyRectangle({ 500.f, 100.f });
    keyRectangle.setFillColor(sf::Color(0, 41, 82));
    keyRectangle.setOutlineThickness(10);
    keyRectangle.setOutlineColor(sf::Color(0, 82, 163));
    keyRectangle.setPosition(sf::Vector2f(size.x / 20, size.y - size.y / 4));

    sf::RectangleShape noteRectangle({ 500.f, 100.f });
    noteRectangle.setFillColor(sf::Color(0, 41, 82));
    noteRectangle.setOutlineThickness(10);
    noteRectangle.setOutlineColor(sf::Color(0, 82, 163));
    noteRectangle.setPosition(sf::Vector2f(size.x / 20, size.y - size.y / 5.7));

    drawnTexture.draw(text);
    drawnTexture.draw(square);
    drawnTexture.draw(actor1);
    drawnTexture.draw(actor2);
    drawnTexture.draw(clapper);
    drawnTexture.draw(topActorRectangle);
    drawnTexture.draw(botActorRectangle);
    drawnTexture.draw(resultText);
    drawnTexture.draw(bfsRectangle);
    drawnTexture.draw(dijRectangle);
    drawnTexture.draw(keyRectangle);
    drawnTexture.draw(noteRectangle);
    drawnTexture.draw(note);
	drawnTexture.display(); //Don't forget to display the texture after drawing
    return drawnTexture;
}

sf::Vector2f Window::getMousePosition() {
    mousePosition = sf::Vector2f(sf::Mouse::getPosition(mainWindow));
    return mousePosition;
}

bool Window::leftMouseClicked() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        leftClick = true;
    }
    else {
        leftClick = false;
    }
    return leftClick;
}

bool Window::rightMouseClicked() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        rightClick = true;
    }
    else {
        rightClick = false;
    }
    return rightClick;
}













// Note: This is just example/reference code for Andrew to integrate
// You'll need to adapt this to your actual window rendering system

/*
Example integration (pseudocode - adapt to your actual implementation):

void YourWindowClass::renderPaths(Graph& graph, int startId, int endId) {
    // Get paths
    PathResult bfsPath = BFS::findShortestPath(graph, startId, endId);
    PathResult dijkPath = Dijkstra::findStrongestPath(graph, startId, endId);

    if (!bfsPath.pathExists || !dijkPath.pathExists) {
        // Display error message
        return;
    }

    // Draw BFS path in GREEN
    sf::Color bfsColor(0, 255, 0, 180);
    for (size_t i = 0; i < bfsPath.path.size() - 1; i++) {
        // You'll need to implement drawEdge based on your visualization
        // This is just showing the concept
        drawEdge(bfsPath.path[i], bfsPath.path[i+1], bfsColor, 3.0f);
    }

    // Draw Dijkstra path in RED
    sf::Color dijkColor(255, 0, 0, 180);
    for (size_t i = 0; i < dijkPath.path.size() - 1; i++) {
        drawEdge(dijkPath.path[i], dijkPath.path[i+1], dijkColor, 3.0f);
    }

    // Display statistics (adapt to your text rendering)
    displayText("BFS: " + std::to_string(bfsPath.hopCount) + " degrees",
               sf::Vector2f(10, 10), sf::Color::Green);
    displayText("Dijkstra: " + std::to_string(dijkPath.hopCount) + " degrees",
               sf::Vector2f(10, 40), sf::Color::Red);
}
*/


