// network_cli.cpp
// CSCI 325 - Network routing CLI
// Data structures: Graph (adjacency list), Min-Heap (priority_queue), Stack
//
// Build: g++ -std=c++17 -Wall -o network_cli network_cli.cpp
// Run:   ./network_cli

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <limits>
#include <algorithm>
#include <cctype>

using namespace std;

// ---------------- Graph ----------------

struct Link {
    string to;
    int cost;      // latency / weight
    bool active;
};

class Network {
public:
    void addRouter(const string& name) {
        if (!adj.count(name)) {
            adj[name] = {};
            routerUp[name] = true;
        }
    }

    // Undirected link: stored in both routers' adjacency lists
    void addLink(const string& a, const string& b, int cost) {
        addRouter(a);
        addRouter(b);
        adj[a].push_back({b, cost, true});
        adj[b].push_back({a, cost, true});
    }

    bool hasRouter(const string& name) const { return adj.count(name) > 0; }

    void toggleRouter(const string& name) {
        routerUp[name] = !routerUp[name];
    }

    bool toggleLink(const string& a, const string& b) {
        bool found = false;
        for (auto& l : adj[a]) if (l.to == b) { l.active = !l.active; found = true; }
        for (auto& l : adj[b]) if (l.to == a) { l.active = !l.active; }
        return found;
    }

    // Print every router and its links, with status
    void print() const {
        cout << "\n================ NETWORK GRAPH ================\n";
        int activeRouters = 0, activeLinks = 0;
        for (const auto& [name, links] : adj) {
            bool up = routerUp.at(name);
            if (up) activeRouters++;
            cout << " [" << (up ? "UP  " : "DOWN") << "] Router " << name << "\n";
            if (links.empty()) cout << "        (no links)\n";
            for (const auto& l : links) {
                bool usable = l.active && up && routerUp.at(l.to);
                if (usable && name < l.to) activeLinks++;   // count each undirected link once
                cout << "        " << (usable ? "--" : "xx") << "> " << left << setw(4) << l.to
                     << " cost " << setw(3) << l.cost
                     << (usable ? "" : "  (inactive)") << "\n";
            }
        }
        cout << "-----------------------------------------------\n";
        cout << " Active routers: " << activeRouters << "/" << adj.size()
             << "   Active links: " << activeLinks << "\n";
        cout << "===============================================\n";
    }

    // Dijkstra's algorithm using a min-heap; returns path (empty if unreachable)
    vector<string> shortestPath(const string& src, const string& dst, int& totalCost) const {
        const int INF = numeric_limits<int>::max();
        map<string, int> dist;
        map<string, string> prev;
        for (const auto& [name, _] : adj) dist[name] = INF;

        totalCost = -1;
        if (!routerUp.at(src) || !routerUp.at(dst)) return {};

        using Item = pair<int, string>;                        // (distance, router)
        priority_queue<Item, vector<Item>, greater<Item>> pq;  // min-heap
        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (d > dist[u]) continue;     // stale entry
            if (u == dst) break;

            for (const auto& l : adj.at(u)) {
                if (!l.active || !routerUp.at(l.to)) continue;
                int nd = d + l.cost;
                if (nd < dist[l.to]) {
                    dist[l.to] = nd;
                    prev[l.to] = u;
                    pq.push({nd, l.to});
                }
            }
        }

        if (dist[dst] == INF) return {};

        // Walk back from dst using a stack so the path comes out in order
        stack<string> st;
        for (string at = dst; ; at = prev.at(at)) {
            st.push(at);
            if (at == src) break;
        }
        vector<string> path;
        while (!st.empty()) { path.push_back(st.top()); st.pop(); }

        totalCost = dist[dst];
        return path;
    }

private:
    map<string, vector<Link>> adj;   // adjacency list
    map<string, bool> routerUp;
};

// ---------------- CLI helpers ----------------

string upper(string s) {
    for (auto& c : s) c = toupper(static_cast<unsigned char>(c));
    return s;
}

string readRouter(const Network& net, const string& prompt) {
    string name;
    while (true) {
        cout << prompt;
        if (!(cin >> name)) return "";
        name = upper(name);
        if (net.hasRouter(name)) return name;
        cout << "  No router named '" << name << "'. Try again.\n";
    }
}

void sendTestPacket(const Network& net) {
    cout << "\n--- Send Test Packet ---\n";
    string src = readRouter(net, "Source router: ");
    string dst = readRouter(net, "Destination router: ");
    if (src.empty() || dst.empty()) return;

    int cost;
    vector<string> path = net.shortestPath(src, dst, cost);

    if (path.empty()) {
        cout << "\n  Packet DROPPED: no active route from Router " << src
             << " to Router " << dst << ".\n";
        return;
    }

    cout << "\n  Packet delivered!\n  Path: ";
    for (size_t i = 0; i < path.size(); i++) {
        cout << "Router " << path[i];
        if (i + 1 < path.size()) cout << " -> ";
    }
    cout << "\n  Hops: " << path.size() - 1 << "   Total cost: " << cost << "\n";
}

void buildSampleNetwork(Network& net) {
    net.addLink("A", "B", 4);
    net.addLink("A", "C", 2);
    net.addLink("B", "C", 1);
    net.addLink("B", "D", 5);
    net.addLink("C", "D", 8);
    net.addLink("C", "E", 10);
    net.addLink("D", "E", 2);
    net.addLink("D", "F", 6);
    net.addLink("E", "F", 3);
}

int main() {
    Network net;
    buildSampleNetwork(net);

    while (true) {
        cout << "\n========== NETWORK ROUTER CLI ==========\n"
             << " 1) Show network graph\n"
             << " 2) Send test packet\n"
             << " 3) Toggle router up/down\n"
             << " 4) Toggle link up/down\n"
             << " 5) Exit\n"
             << "Choose an option: ";

        int choice;
        if (!(cin >> choice)) {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Please enter a number 1-5.\n";
            continue;
        }

        switch (choice) {
            case 1: net.print(); break;
            case 2: sendTestPacket(net); break;
            case 3: {
                string r = readRouter(net, "Router to toggle: ");
                if (!r.empty()) { net.toggleRouter(r); cout << "  Router " << r << " toggled.\n"; }
                break;
            }
            case 4: {
                string a = readRouter(net, "Link endpoint 1: ");
                string b = readRouter(net, "Link endpoint 2: ");
                if (a.empty() || b.empty()) break;
                if (net.toggleLink(a, b)) cout << "  Link " << a << " <-> " << b << " toggled.\n";
                else cout << "  No direct link between " << a << " and " << b << ".\n";
                break;
            }
            case 5: cout << "Goodbye!\n"; return 0;
            default: cout << "  Please enter a number 1-5.\n";
        }
    }
    return 0;
}