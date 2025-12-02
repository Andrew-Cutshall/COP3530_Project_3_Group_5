#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include "bfh.h"  // Reuse PathResult structure
#include <vector>

//=====================================================================================
//                          Dijkstra Class
//=====================================================================================
// Implements Dijkstra's Algorithm to find the path with strongest collaborations
// Uses inverted weights: higher collaboration count = lower cost
// This finds the path that maximizes total collaboration strength
class Dijkstra {
public:
    // Find path with strongest collaborations from startActorId to endActorId
    // Returns PathResult with the path information
    static PathResult findStrongestPath(const Graph& graph, int startActorId, int endActorId);

    // Helper method to print the path nicely
    static void printPath(const PathResult& result);

private:
    // Node structure for priority queue
    struct Node {
        int actorId;
        double cost;  // Inverted weight (lower cost = stronger collaboration)

        Node(int id, double c) : actorId(id), cost(c) {}

        // For priority queue (min-heap based on cost)
        bool operator>(const Node& other) const {
            return cost > other.cost;
        }
    };

    // Reconstruct the path from parent map
    static std::vector<int> reconstructPath(
        const std::unordered_map<int, int>& parent,
        int startActorId,
        int endActorId
    );

    // Calculate total weight of a path
    static int calculatePathWeight(const Graph& graph, const std::vector<int>& path);

    // Convert weight to cost (inverted)
    // Higher weight (more collaborations) = lower cost
    static double weightToCost(int weight, int maxWeight);
};

#endif // DIJKSTRA_H
