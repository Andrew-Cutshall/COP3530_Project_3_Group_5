#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <SQLiteCpp/SQLiteCpp.h>

// Forward declarations
struct Actor;
struct Edge;
class Graph;

//=====================================================================================
//                              Actor Structure
//=====================================================================================
// Represents an actor node in the graph
struct Actor {
    int id;
    std::string name;

    Actor() : id(-1), name("") {}
    Actor(int actorId, const std::string& actorName)
        : id(actorId), name(actorName) {
    }
};

//=====================================================================================
//                              Edge Structure
//=====================================================================================
// Represents a weighted edge between two actors
// Weight = number of movies they've worked together on
struct Edge {
    int targetActorId;
    int weight; // Number of collaborations

    Edge() : targetActorId(-1), weight(0) {}
    Edge(int target, int w) : targetActorId(target), weight(w) {}
};

//=====================================================================================
//                              Graph Class
//=====================================================================================
// Adjacency list representation of the actor collaboration network
class Graph {
private:
    // Map: actor_id -> Actor data
    std::unordered_map<int, Actor> actors;

    // Map: actor_id -> list of edges (neighbors)
    std::unordered_map<int, std::vector<Edge>> adjList;

    // Track max weight for potential normalization
    int maxWeight;

public:
    // Constructor
    Graph();

    // Destructor
    ~Graph();

    //=====================================================================================
    //                          Graph Building Methods
    //=====================================================================================

    // Load the entire graph from database
    void loadFromDatabase(SQLite::Database& db);

    // Add a single actor to the graph
    void addActor(int actorId, const std::string& actorName);

    // Add an edge between two actors with a weight
    void addEdge(int actor1Id, int actor2Id, int weight);

    //=====================================================================================
    //                          Query Methods
    //=====================================================================================

    // Check if an actor exists in the graph
    bool hasActor(int actorId) const;

    // Get actor by ID
    const Actor* getActor(int actorId) const;

    // Get actor by name (case-insensitive search)
    const Actor* getActorByName(const std::string& name) const;

    // Search for actors by partial name match
    std::vector<Actor> searchActorsByName(const std::string& partialName) const;

    // Get all neighbors of an actor
    const std::vector<Edge>* getNeighbors(int actorId) const;

    // Get weight between two actors (0 if no edge)
    int getEdgeWeight(int actor1Id, int actor2Id) const;

    // Get total number of actors
    size_t getActorCount() const;

    // Get total number of edges
    size_t getEdgeCount() const;

    // Get maximum weight in the graph
    int getMaxWeight() const;

    //=====================================================================================
    //                          Utility Methods
    //=====================================================================================

    // Print graph statistics
    void printStatistics() const;

    // Clear all data from the graph
    void clear();
};

#endif // GRAPH_H
