#include <bits/stdc++.h>
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
	
	int graph_size; in >> graph_size;
	vector<vector<int>> adj(graph_size);
	for (int i = 0; i < graph_size; i++) {
		int u, v;
		in >> u >> v;
		--u, --v;
		my_assert(u != v && 0 <= u && u < graph_size && 0 <= v && v < graph_size, "wrong input");
		adj[u].push_back(v);
		adj[v].push_back(u);
	}
	vector<int> ecc(graph_size);

	vector<bool> cyc_visited(graph_size);
	vector<int> parent(graph_size);
	int backstart = -1, backend = -1;
	function<void(int, int)> cyc_dfs = [&](int from, int pre) {
		parent[from] = pre;
		cyc_visited[from] = true;
		for (int to : adj[from]) {
			if (to != pre) {
				if (cyc_visited[to]) {
					backstart = from, backend = to;
					return;
				}
				else {
					cyc_dfs(to, from);
					if (backstart != -1) {
						return;
					}
				}
			}
		}
	};
	cyc_dfs(0, -1);
	my_assert(backstart != -1, "no cycles");
	
	vector<int> cyc;
	int backcur = backstart;
	while (backcur != backend) {
		cyc.push_back(backcur);
		backcur = parent[backcur];
	}
	cyc.push_back(backend);

	int cyc_size = cyc.size();
    vector<vector<int>> trees(cyc_size);
	vector<int> depth(graph_size, -1), max_depth(cyc_size, -1);
	vector<int> dist(graph_size, -1);
	for (int i = 0; i < cyc_size; i++) {
		int root = cyc[i];
		// ecc[root] = max(ecc[root], cyc_size / 2);
		function<void(int, int, vector<int>&)> tree_dfs = [&](int from, int pre, vector<int>& d) {
			trees[i].push_back(from);
			for (int to : adj[from]) {
				if (to != pre && to != cyc[(i - 1 + cyc_size) % cyc_size] && to != cyc[(i + 1) % cyc_size]) {
					d[to] = d[from] + 1;
					tree_dfs(to, from, d);
				}
			}
		};
		depth[root] = 0;
		tree_dfs(root, -1, depth);

		int deepest = -1;
		for (int v : trees[i]) {
			if (max_depth[i] < depth[v]) {
				max_depth[i] = depth[v];
				deepest = v;
			}
		}
		my_assert(deepest != -1, "depth array is incorrect");

		dist[deepest] = 0;
		tree_dfs(deepest, -1, dist);
		int diam_len = -1, farthest = -1;
		for (int v : trees[i]) {
			if (diam_len < dist[v]) {
				diam_len = dist[v];
				farthest = v;
			}
		}
		my_assert(farthest != -1, "dist array is incorrect");

		for (int v : trees[i]) {
			ecc[v] = max(ecc[v], dist[v]);
		}
		dist[farthest] = 0;
		tree_dfs(farthest, -1, dist);
		for (int v : trees[i]) {
			ecc[v] = max(ecc[v], dist[v]);
		}
	}
	
    vector<int> on_cyc(graph_size);
    auto do_half = [&]() {
        vector<int> mod_max(cyc_size);
        for (int i = 0; i < cyc_size; i++) {
            mod_max[i] = max_depth[i] + i;
        }
        vector<int> ext_cyc = mod_max;
        for (int i = 0; i < cyc_size; i++) {
            ext_cyc.push_back(mod_max[i] + cyc_size);
        }
        deque<pair<int, int>> dq;
        int interval = cyc_size / 2 + 1;
        for (int i = 0; i <= cyc_size + interval - 2; i++) {
            while (!dq.empty() && dq.front().second <= i - interval) {
                dq.pop_front();
            }
            while (!dq.empty() && dq.back().first < ext_cyc[i]) {
                dq.pop_back();
            }
            dq.emplace_back(ext_cyc[i], i);

            if (i >= interval - 1) {
                on_cyc[cyc[i - interval + 1]] = max(on_cyc[cyc[i - interval + 1]], dq.front().first - (i - interval + 1));
            }
        }
    };
    do_half();
    reverse(cyc.begin(), cyc.end());
    reverse(max_depth.begin(), max_depth.end());
    do_half();
    reverse(cyc.begin(), cyc.end());
    reverse(max_depth.begin(), max_depth.end());

    for (int i = 0; i < cyc_size; i++) {
        for (int v : trees[i]) {
            ecc[v] = max(ecc[v], on_cyc[cyc[i]] + depth[v]);
        }
    }

	for (int i = 0; i < graph_size; i++) {
        my_assert(0 <= ecc[i] && ecc[i] < graph_size, "invalid eccentricity");
		out << i << ' ' << ecc[i] << '\n';
	}
	return 0;
}