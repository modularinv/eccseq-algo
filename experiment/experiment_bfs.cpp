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
	ofstream out(".\\result\\" + file_name + "-bfsresult.txt");

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
	};
    */
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