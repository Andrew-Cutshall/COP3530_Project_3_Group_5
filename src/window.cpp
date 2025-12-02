//SFML Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

// Your project headers
#include "window.h"
#include "graph.h"           // ← ADD THIS
#include "bfh.h"             // ← ADD THIS
#include "dijkstra.h"        // ← ADD THIS
#include "dataCollection.h"

#include <iostream>
#include <string>

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