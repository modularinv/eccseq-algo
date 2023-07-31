#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <deque>
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

	vector<pair<int, int>> cyc_memo;
	vector<bool> cyc_visited(graph_size);
	vector<int> parent(graph_size), dfs_order(graph_size);
	int counter = 0;
	function<void(int, int)> cyc_dfs = [&](int from, int pre) {
		parent[from] = pre;
		cyc_visited[from] = true;
		dfs_order[from] = counter++;
		for (int to : adj[from]) {
			if (to != pre) {
				if (cyc_visited[to]) {
					if (dfs_order[to] < dfs_order[from]) {
						cyc_memo.emplace_back(from, to);
					}
				}
				else {
					cyc_dfs(to, from);
				}
			}
		}
	};
	cyc_dfs(0, -1);
	my_assert(!cyc_memo.empty(), "no cycles");
	
	vector<vector<int>> cyc;
	for (auto [backstart, backend] : cyc_memo) {
		cyc.push_back(vector<int>());
		int backcur = backstart;
		while (backcur != backend) {
			cyc.back().push_back(backcur);
			backcur = parent[backcur];
		}
		cyc.back().push_back(backend);
	}

	vector<int> cyc_idx(graph_size, -1);
	for (int i = 0; i < cyc.size(); i++) {
		for (int v : cyc[i]) {
			cyc_idx[v] = i;
		}
	}
	
}