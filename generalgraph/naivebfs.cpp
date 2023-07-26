#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <queue>
#include <utility>
using namespace std;

int main() {
	ifstream in("input.in");
	ofstream out("output.out");

	auto my_assert = [&](bool cond, const char* comment) {
		if (!cond) {
			out << "Assertion Failed: " << comment << '\n';
			exit(0);
		}
	};
	
	int graph_size, edge_num;
    in >> graph_size >> edge_num;
	vector<vector<int>> adj(graph_size);
	for (int i = 0; i < edge_num; i++) {
		int u, v;
		in >> u >> v;
		--u, --v;
		my_assert(u != v && 0 <= u && u < graph_size && 0 <= v && v < graph_size, "wrong input");
		adj[u].push_back(v);
		adj[v].push_back(u);
	}
	vector<int> ecc(graph_size);

	for (int i = 0; i < graph_size; i++) {
        queue<pair<int, int>> qu;
        vector<bool> visited(graph_size);
        qu.emplace(i, 0);
        visited[i] = true;
        while (!qu.empty()) {
            auto [from, dist] = qu.front(); qu.pop();
            ecc[i] = max(ecc[i], dist);
            for (int to : adj[from]) {
                if (!visited[to]) {
                    qu.emplace(to, dist + 1);
                    visited[to] = true;
                }
            }
        }
    }

	for (int i = 0; i < graph_size; i++) {
		my_assert(0 <= ecc[i] && ecc[i] < graph_size, "invalid eccentricity");
		out << i << ' ' << ecc[i] << '\n';
	}
	return 0;
}