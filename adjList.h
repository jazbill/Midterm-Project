#ifndef ADJLIST_H
#define ADJLIST_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct Edge {
    string dest;
    int weight;
};

class Graph {
private:
    unordered_map<string, vector<Edge>> adjList;
    unordered_set<string> offlineNodes;

public:
    void addVertex(string v) {
        if (adjList.find(v) == adjList.end()) {
            adjList[v] = vector<Edge>();
        }
    }

    void addEdge(string src, string dest, int weight, bool directed = true) {
        addVertex(src);
        addVertex(dest);

        adjList[src].push_back({dest, weight});
        if (!directed) {
            adjList[dest].push_back({src, weight});
        }
    }

    void disableNode(const string& node) {
        if (adjList.count(node)) {
            offlineNodes.insert(node);
        }
    }

    void enableNode(const string& node) {
        offlineNodes.erase(node);
    }

    bool isNodeOnline(const string& node) const {
        return adjList.count(node) && !offlineNodes.count(node);
    }

    // Returns active topology so Dijkstra automatically routes around offline nodes
    unordered_map<string, vector<Edge>> getAdjList() const {
        unordered_map<string, vector<Edge>> activeGraph;

        for (const auto& pair : adjList) {
            const string& node = pair.first;
            if (offlineNodes.count(node)) {
                continue;
            }

            vector<Edge> activeEdges;
            for (const Edge& edge : pair.second) {
                if (!offlineNodes.count(edge.dest)) {
                    activeEdges.push_back(edge);
                }
            }
            activeGraph[node] = activeEdges;
        }

        return activeGraph;
    }

    void printGraph() const {
        for (auto it = adjList.begin(); it != adjList.end(); it++) {
            cout << it->first;
            if (offlineNodes.count(it->first)) {
                cout << " [OFFLINE]";
            }
            cout << " -> ";

            for (int i = 0; i < it->second.size(); i++) {
                cout << "[" << it->second[i].dest << " (" << it->second[i].weight << "ms)] ";
            }
            cout << endl;
        }
    }
};

#endif
