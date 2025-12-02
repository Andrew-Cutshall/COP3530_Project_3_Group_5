#ifndef BFH_H
#define BFH_H

#include "graph.h"
#include <vector>
#include <chrono>
#include <unordered_map>

//=====================================================================================
//                          Path Result Structure
//=====================================================================================
// Stores the result of a pathfinding operation
struct PathResult {
    std::vector<int> path;              // Sequence of actor IDs from start to end
    std::vector<std::string> actorNames; // Names corresponding to the path
    int hopCount;                        // Number of hops (edges) in the path
    int totalWeight;                     // Sum of edge weights along the path
    double executionTimeMs;              // Time taken to find the path (milliseconds)
    bool pathExists;                     // Whether a path was found

    PathResult()
        : hopCount(0), totalWeight(0), executionTimeMs(0.0), pathExists(false) {
    }
};

//=====================================================================================
//                          BFS Class
//=====================================================================================
// Implements Breadth-First Search to find shortest path by hop count
// (Ignores edge weights - finds path with fewest intermediate actors)
class BFS {
public:
    // Find shortest path from startActorId to endActorId
    // Returns PathResult with the path information
    static PathResult findShortestPath(const Graph& graph, int startActorId, int endActorId);

    // Helper method to print the path nicely
    static void printPath(const PathResult& result);

private:
    // Reconstruct the path from parent map
    static std::vector<int> reconstructPath(
        const std::unordered_map<int, int>& parent,
        int startActorId,
        int endActorId
    );

    // Calculate total weight of a path
    static int calculatePathWeight(const Graph& graph, const std::vector<int>& path);
};

#endif // BFH_H
