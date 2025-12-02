#include "graph.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <format>

//=====================================================================================
//                          Constructor & Destructor
//=====================================================================================

Graph::Graph() : maxWeight(0) {
    // Initialize empty graph
}

Graph::~Graph() {
    clear();
}

//=====================================================================================
//                          Graph Building Methods
//=====================================================================================

void Graph::loadFromDatabase(SQLite::Database& db) {
    std::cout << "Loading graph from database...\n";

    try {
        // Step 1: Load all actors
        SQLite::Statement actorQuery(db, "SELECT actor_id, actor_name FROM Actors;");
        int actorCount = 0;

        while (actorQuery.executeStep()) {
            int actorId = actorQuery.getColumn(0).getInt();
            std::string actorName = actorQuery.getColumn(1).getString();
            addActor(actorId, actorName);
            actorCount++;

            if (actorCount % 10000 == 0) {
                std::cout << std::format("Loaded {} actors...\n", actorCount);
            }
        }

        std::cout << std::format("Loaded {} actors total.\n", actorCount);

        // Step 2: Load all edges
        SQLite::Statement edgeQuery(db,
            "SELECT actor1_id, actor2_id, weight FROM Actor_Edges;");
        int edgeCount = 0;

        while (edgeQuery.executeStep()) {
            int actor1Id = edgeQuery.getColumn(0).getInt();
            int actor2Id = edgeQuery.getColumn(1).getInt();
            int weight = edgeQuery.getColumn(2).getInt();

            addEdge(actor1Id, actor2Id, weight);
            edgeCount++;

            if (edgeCount % 50000 == 0) {
                std::cout << std::format("Loaded {} edges...\n", edgeCount);
            }
        }

        std::cout << std::format("Loaded {} edges total.\n", edgeCount);
        std::cout << "Graph loading complete!\n";
        printStatistics();
    }
    catch (const std::exception& e) {
        std::cerr << std::format("Error loading graph from database: {}\n", e.what());
        throw;
    }
}

void Graph::addActor(int actorId, const std::string& actorName) {
    if (actors.find(actorId) == actors.end()) {
        actors[actorId] = Actor(actorId, actorName);
        adjList[actorId] = std::vector<Edge>(); // Initialize empty neighbor list
    }
}

void Graph::addEdge(int actor1Id, int actor2Id, int weight) {
    // Ensure both actors exist
    if (!hasActor(actor1Id) || !hasActor(actor2Id)) {
        std::cerr << std::format("Warning: Attempting to add edge between non-existent actors ({}, {})\n",
            actor1Id, actor2Id);
        return;
    }

    // Add bidirectional edge (undirected graph)
    adjList[actor1Id].push_back(Edge(actor2Id, weight));
    adjList[actor2Id].push_back(Edge(actor1Id, weight));

    // Track maximum weight
    if (weight > maxWeight) {
        maxWeight = weight;
    }
}

//=====================================================================================
//                          Query Methods
//=====================================================================================

bool Graph::hasActor(int actorId) const {
    return actors.find(actorId) != actors.end();
}

const Actor* Graph::getActor(int actorId) const {
    auto it = actors.find(actorId);
    if (it != actors.end()) {
        return &(it->second);
    }
    return nullptr;
}

const Actor* Graph::getActorByName(const std::string& name) const {
    // Convert search name to lowercase for case-insensitive comparison
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    for (const auto& pair : actors) {
        std::string actorNameLower = pair.second.name;
        std::transform(actorNameLower.begin(), actorNameLower.end(),
            actorNameLower.begin(), ::tolower);

        if (actorNameLower == lowerName) {
            return &(pair.second);
        }
    }

    return nullptr;
}

std::vector<Actor> Graph::searchActorsByName(const std::string& partialName) const {
    std::vector<Actor> results;

    // Convert search term to lowercase
    std::string lowerPartial = partialName;
    std::transform(lowerPartial.begin(), lowerPartial.end(),
        lowerPartial.begin(), ::tolower);

    for (const auto& pair : actors) {
        std::string actorNameLower = pair.second.name;
        std::transform(actorNameLower.begin(), actorNameLower.end(),
            actorNameLower.begin(), ::tolower);

        // Check if the partial name is contained in the actor's name
        if (actorNameLower.find(lowerPartial) != std::string::npos) {
            results.push_back(pair.second);
        }
    }

    return results;
}

const std::vector<Edge>* Graph::getNeighbors(int actorId) const {
    auto it = adjList.find(actorId);
    if (it != adjList.end()) {
        return &(it->second);
    }
    return nullptr;
}

int Graph::getEdgeWeight(int actor1Id, int actor2Id) const {
    const std::vector<Edge>* neighbors = getNeighbors(actor1Id);
    if (neighbors == nullptr) {
        return 0;
    }

    for (const Edge& edge : *neighbors) {
        if (edge.targetActorId == actor2Id) {
            return edge.weight;
        }
    }

    return 0;
}

size_t Graph::getActorCount() const {
    return actors.size();
}

size_t Graph::getEdgeCount() const {
    size_t count = 0;
    for (const auto& pair : adjList) {
        count += pair.second.size();
    }
    return count / 2; // Divide by 2 because edges are bidirectional
}

int Graph::getMaxWeight() const {
    return maxWeight;
}

//=====================================================================================
//                          Utility Methods
//=====================================================================================

void Graph::printStatistics() const {
    std::cout << "\n=== Graph Statistics ===\n";
    std::cout << std::format("Total Actors: {}\n", getActorCount());
    std::cout << std::format("Total Edges: {}\n", getEdgeCount());
    std::cout << std::format("Maximum Edge Weight: {}\n", maxWeight);

    // Calculate average degree
    if (!actors.empty()) {
        double avgDegree = (2.0 * getEdgeCount()) / actors.size();
        std::cout << std::format("Average Degree: {:.2f}\n", avgDegree);
    }

    std::cout << "========================\n\n";
}

void Graph::clear() {
    actors.clear();
    adjList.clear();
    maxWeight = 0;
}
