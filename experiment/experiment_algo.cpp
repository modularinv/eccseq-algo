#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <queue>
#include <deque>
#include <utility>
using namespace std;

#include <chrono>
#include <map>
using namespace std::chrono;

void run(int v, int gamma, int test) {
    string file_name = to_string(v) + "_" + to_string(gamma) + "_" + to_string(test);
    ifstream in(".\\data\\" + file_name + ".txt");
	ofstream out(".\\result\\" + file_name + "-algoresult.txt");

	auto my_assert = [&](bool cond, const char* comment) {
		if (!cond) {
			out << "Assertion Failed: " << comment << '\n';
			out.close();
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

	auto start = high_resolution_clock::now();
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

	vector<vector<int>> cyc_idx(graph_size);
	for (int i = 0; i < cyc.size(); i++) {
		for (int v : cyc[i]) {
			cyc_idx[v].push_back(i);
		}
	}
	vector<vector<pair<int, bool>>> adj_copy(graph_size);
	for (int i = 0; i < graph_size; i++) {
		for (int v : adj[i]) {
			adj_copy[i].emplace_back(v, false);
		}
	}
	for (int i = 0; i < cyc.size(); i++) {
		int cyc_len = cyc[i].size();
		for (int j = 0; j < cyc_len; j++) {
			for (auto& [v, chk] : adj_copy[cyc[i][j]]) {
				if (v == cyc[i][(j - 1 + cyc_len) % cyc_len] || v == cyc[i][(j + 1) % cyc_len]) {
					chk = true;
				}
			}
		}
	}
	vector<vector<int>> nocyc_adj(graph_size);
	for (int i = 0; i < graph_size; i++) {
		for (auto [v, chk] : adj_copy[i]) {
			if (!chk) {
				nocyc_adj[i].push_back(v);
			}
		}
	}

	vector<bool> visited(graph_size), chk_cyc(cyc.size());
	function<void(int)> process_cyc = [&](int i) {
		vector<int> cur_cyc = cyc[i];
		chk_cyc[i] = true;
		int cyc_len = cur_cyc.size(), other_cyc = -1;
		for (int j = 0; j < cyc_len; j++) {
			if (visited[cur_cyc[j]]) {
				my_assert(other_cyc == -1, "invalid cycle traversal");
				other_cyc = j;
			}
		}
		my_assert(i == 0 || other_cyc != -1, "invalid cycle traversal");

		vector<vector<int>> trees(cyc_len);
		vector<int> depth(graph_size, -1), max_depth(cyc_len, -1);
		vector<int> dist(graph_size, -1);
		for (int j = 0; j < cyc_len; j++) {
			if (j == other_cyc) {
				max_depth[j] = ecc[cur_cyc[j]];
				trees[j].push_back(cur_cyc[j]);
				continue;
			}

			int root = cur_cyc[j];
			function<void(int, int, vector<int>&, bool)> tree_dfs = [&](int from, int pre, vector<int>& d, bool push) {
				if (push) {
					trees[j].push_back(from);
				}
				for (int to : nocyc_adj[from]) {
					if (to != pre) {
						d[to] = d[from] + 1;
						tree_dfs(to, from, d, push);
					}
				}
			};
			depth[root] = 0;
			tree_dfs(root, -1, depth, true);
			
			int deepest = -1;
			for (int v : trees[j]) {
				if (max_depth[j] < depth[v]) {
					max_depth[j] = depth[v];
					deepest = v;
				}
			}
			my_assert(deepest != -1, "depth array is incorrect");

			dist[deepest] = 0;
			tree_dfs(deepest, -1, dist, false);
			int diam_len = -1, farthest = -1;
			for (int v : trees[j]) {
				if (diam_len < dist[v]) {
					diam_len = dist[v];
					farthest = v;
				}
			}
			my_assert(farthest != -1, "dist array is incorrect");

			for (int v : trees[j]) {
				ecc[v] = max(ecc[v], dist[v]);
			}
			dist[farthest] = 0;
			tree_dfs(farthest, -1, dist, false);
			for (int v : trees[j]) {
				ecc[v] = max(ecc[v], dist[v]);
			}
		}
		my_assert(find(max_depth.begin(), max_depth.end(), -1) == max_depth.end(), "unvisited vertex exists");
		
		vector<int> on_cyc(graph_size);
		auto do_half = [&]() {
			vector<int> mod_max(cyc_len);
			for (int j = 0; j < cyc_len; j++) {
				mod_max[j] = max_depth[j] + j;
			}
			vector<int> ext_cyc = mod_max;
			for (int j = 0; j < cyc_len; j++) {
				ext_cyc.push_back(mod_max[j] + cyc_len);
			}
			deque<pair<int, int>> dq;
			int interval = cyc_len / 2;
			for (int j = 0; j <= cyc_len + interval - 1; j++) {
				while (!dq.empty() && dq.front().second <= j - interval) {
					dq.pop_front();
				}
				while (!dq.empty() && dq.back().first < ext_cyc[j]) {
					dq.pop_back();
				}
				dq.emplace_back(ext_cyc[j], j);

				if (j >= interval) {
					on_cyc[cur_cyc[j - interval]] = max(on_cyc[cur_cyc[j - interval]], dq.front().first - (j - interval));
				}
			}
		};
		do_half();
		reverse(cur_cyc.begin(), cur_cyc.end());
		reverse(max_depth.begin(), max_depth.end());
		do_half();
		reverse(cur_cyc.begin(), cur_cyc.end());
		reverse(max_depth.begin(), max_depth.end());
		
		vector<bool> upd_visited(graph_size);
		for (int j = 0; j < cyc_len; j++) {
			if (j != other_cyc) {
				for (int v : trees[j]) {
					ecc[v] = max(ecc[v], on_cyc[cur_cyc[j]] + depth[v]);
				}
			}
			else {
				queue<pair<int, int>> qu;
				qu.emplace(cur_cyc[j], 0);
				int stretch = 0;
				while (!qu.empty()) {
					auto [from, d] = qu.front(); qu.pop();
					stretch = max(stretch, d);
					for (int to : adj[from]) {
						if (!visited[to] && depth[to] != -1) {
							visited[to] = true;
							qu.emplace(to, d + 1);
						}
					}
				}
				
				qu.emplace(cur_cyc[j], 0);
				upd_visited[cur_cyc[j]] = true;
				while (!qu.empty()) {
					auto [from, d] = qu.front(); qu.pop();
					ecc[from] = max(ecc[from], d + stretch);
					for (int to : adj[from]) {
						if (!upd_visited[to] && to != cur_cyc[(j - 1 + cyc_len) % cyc_len] && to != cur_cyc[(j + 1) % cyc_len]) {
							upd_visited[to] = true;
							qu.emplace(to, d + 1);
						}
					}
				}
			}
		}
		

		if (other_cyc == -1) {
			for (int j = 0; j < cyc_len; j++) {
				for (int v : trees[j]) {
					visited[v] = true;
				}
			}
		}
		for (int j = 0; j < cyc_len; j++) {
			for (int v : trees[j]) {
				for (int next_cyc : cyc_idx[v]) {
					if (!chk_cyc[next_cyc]) {
						process_cyc(next_cyc);
					}
				}
			}
		}
 	};
	process_cyc(0);

	vector<int> cnt(graph_size);
	for (int i = 0; i < graph_size; i++) {
		my_assert(0 <= ecc[i] && ecc[i] < graph_size, "invalid eccentricity");
		cnt[ecc[i]]++;
	}
	vector<int> eccseq;
	for (int i = 0; i < graph_size; i++) {
		for (int j = 0; j < cnt[i]; j++) {
			eccseq.push_back(i);
		}
	}

	auto stop = high_resolution_clock::now();
	for (int i = 0; i < eccseq.size(); i++) {
		out << eccseq[i] << ' ';
	}
	out << '\n';
	out.close();
    
	auto duration = duration_cast<microseconds>(stop - start);
	cout << file_name << ": " << duration.count() << " microseconds\n";
}

int main() {
	map<int, vector<int>> data = {
		{ 10, vector<int>{ 1, 2, 4 } },
		{ 100, vector<int>{ 1, 2, 10, 49 } },
		{ 1000, vector<int>{ 1, 2, 10, 100, 499 } },
		{ 10000, vector<int>{ 1, 2, 10, 100, 1000, 4999 } },
		{ 100000, vector<int>{ 1, 2, 10, 100, 1000, 10000, 49999 } }
	};
    /*
    map<int, vector<int>> data = {
		{ 10, vector<int>{ 1, 2, 3, 4, 4 } },
		{ 100, vector<int>{ 10, 20, 30, 40, 49 } },
		{ 1000, vector<int>{ 100, 200, 300, 400, 499 } },
		{ 10000, vector<int>{ 1000, 2000, 3000, 4000, 4999 } },
	};*/
	for (auto [v, gamma_list] : data) {
		for (auto gamma : gamma_list) {
			for (int i = 1; i <= 5; i++) {
				run(v, gamma, i);
			}
			cout << "######\n";
		}
	}
	return 0;
}