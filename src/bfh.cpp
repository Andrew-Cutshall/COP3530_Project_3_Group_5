#include "bfh.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <format>
#include <algorithm>

//=====================================================================================
//                          BFS Implementation
//=====================================================================================

PathResult BFS::findShortestPath(const Graph& graph, int startActorId, int endActorId) {
    auto startTime = std::chrono::high_resolution_clock::now();

    PathResult result;

    // Validate input
    if (!graph.hasActor(startActorId)) {
        std::cerr << std::format("Error: Start actor ID {} not found in graph.\n", startActorId);
        return result;
    }

    if (!graph.hasActor(endActorId)) {
        std::cerr << std::format("Error: End actor ID {} not found in graph.\n", endActorId);
        return result;
    }

    // Special case: start and end are the same
    if (startActorId == endActorId) {
        result.path.push_back(startActorId);
        const Actor* actor = graph.getActor(startActorId);
        if (actor) {
            result.actorNames.push_back(actor->name);
        }
        result.hopCount = 0;
        result.totalWeight = 0;
        result.pathExists = true;

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        return result;
    }

    // BFS Algorithm
    std::queue<int> queue;
    std::unordered_set<int> visited;
    std::unordered_map<int, int> parent;

    // Initialize
    queue.push(startActorId);
    visited.insert(startActorId);
    parent[startActorId] = -1; // Start node has no parent

    bool found = false;

    // BFS traversal
    while (!queue.empty() && !found) {
        int currentActorId = queue.front();
        queue.pop();

        // Check if we reached the destination
        if (currentActorId == endActorId) {
            found = true;
            break;
        }

        // Explore all neighbors
        const std::vector<Edge>* neighbors = graph.getNeighbors(currentActorId);
        if (neighbors == nullptr) {
            continue;
        }

        for (const Edge& edge : *neighbors) {
            int neighborId = edge.targetActorId;

            // If not visited, add to queue
            if (visited.find(neighborId) == visited.end()) {
                visited.insert(neighborId);
                parent[neighborId] = currentActorId;
                queue.push(neighborId);

                // Early termination if we found the target
                if (neighborId == endActorId) {
                    found = true;
                    break;
                }
            }
        }
    }

    // Reconstruct path if found
    if (found) {
        result.path = reconstructPath(parent, startActorId, endActorId);
        result.pathExists = true;
        result.hopCount = static_cast<int>(result.path.size()) - 1;
        result.totalWeight = calculatePathWeight(graph, result.path);

        // Get actor names for the path
        for (int actorId : result.path) {
            const Actor* actor = graph.getActor(actorId);
            if (actor) {
                result.actorNames.push_back(actor->name);
            }
        }
    }
    else {
        std::cout << "No path found between the two actors.\n";
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return result;
}

//=====================================================================================
//                          Helper Methods
//=====================================================================================

std::vector<int> BFS::reconstructPath(
    const std::unordered_map<int, int>& parent,
    int startActorId,
    int endActorId
) {
    std::vector<int> path;
    int current = endActorId;

    // Trace back from end to start
    while (current != -1) {
        path.push_back(current);
        auto it = parent.find(current);
        if (it != parent.end()) {
            current = it->second;
        }
        else {
            break;
        }
    }

    // Reverse to get path from start to end
    std::reverse(path.begin(), path.end());

    return path;
}

int BFS::calculatePathWeight(const Graph& graph, const std::vector<int>& path) {
    int totalWeight = 0;

    for (size_t i = 0; i < path.size() - 1; i++) {
        int weight = graph.getEdgeWeight(path[i], path[i + 1]);
        totalWeight += weight;
    }

    return totalWeight;
}

void BFS::printPath(const PathResult& result) {
    std::cout << "\n=== BFS Path Result ===\n";

    if (!result.pathExists) {
        std::cout << "No path found.\n";
        std::cout << "=======================\n\n";
        return;
    }

    std::cout << std::format("Path Length: {} degrees of separation\n", result.hopCount);
    std::cout << std::format("Total Collaboration Weight: {}\n", result.totalWeight);
    std::cout << std::format("Execution Time: {:.3f} ms\n", result.executionTimeMs);
    std::cout << "\nPath:\n";

    for (size_t i = 0; i < result.actorNames.size(); i++) {
        std::cout << result.actorNames[i];

        if (i < result.actorNames.size() - 1) {
            std::cout << " → ";
        }
    }

    std::cout << "\n=======================\n\n";
}
