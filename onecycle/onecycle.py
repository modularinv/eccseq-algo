import sys, os
from collections import deque

sys.setrecursionlimit(10 ** 6)
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
#inf = open(os.path.join(__location__, "input.in"), "r")
#ouf = open(os.path.join(__location__, "output.out"), "w")
inf = open("input.txt", "r")
ouf = open("output.out", "w")

def my_assert(cond, comment):
    if not cond:
        ouf.write(f"Assertion Failed: {comment}")
        sys.exit(0)

graph_size = int(inf.readline().rstrip())
adj = [[] for _ in range(graph_size)]
for i in range(graph_size):
    u, v = map(int, inf.readline().rstrip().split())
    u -= 1; v -= 1
    my_assert(u != v and 0 <= u < graph_size and 0 <= v < graph_size, "wrong input")
    adj[u].append(v)
    adj[v].append(u)
ecc = [0] * graph_size

cyc_visited = [False] * graph_size
parent = [0] * graph_size
backstart, backend = -1, -1
def cyc_dfs(cur, pre):
    global backstart, backend
    parent[cur] = pre
    cyc_visited[cur] = True
    for next in adj[cur]:
        if next != pre:
            if cyc_visited[next]:
                backstart, backend = cur, next
                return
            else:
                cyc_dfs(next, cur)
                if backstart != -1:
                    return
cyc_dfs(0, -1)
my_assert(backstart != -1, "no cycles")

cyc = []
backcur = backstart
while backcur != backend:
    cyc.append(backcur)
    backcur = parent[backcur]
cyc.append(backend)

cyc_size = len(cyc)
trees = [[] for _ in range(cyc_size)]
depth, max_depth = [-1] * graph_size, [-1] * graph_size
dist = [-1] * graph_size
for i in range(cyc_size):
    root = cyc[i]
    # ecc[root] = max(ecc[root], cyc_size // 2)
    def tree_dfs(cur, pre, d):
        trees[i].append(cur)
        for next in adj[cur]:
            if next not in [pre, cyc[i - 1], cyc[(i + 1) % cyc_size]]:
                d[next] = d[cur] + 1
                tree_dfs(next, cur, d)
    depth[root] = 0
    tree_dfs(root, -1, depth)

    deepest = -1
    for v in trees[i]:
        if max_depth[i] < depth[v]:
            max_depth[i] = depth[v]
            deepest = v
    my_assert(deepest != -1, "depth array is incorrect")

    dist[deepest] = 0
    tree_dfs(deepest, -1, dist)
    diam_len, farthest = -1, -1
    for v in trees[i]:
        if diam_len < dist[v]:
            diam_len = dist[v]
            farthest = v
    my_assert(farthest != -1, "dist array is incorrect")

    for v in trees[i]:
        ecc[v] = max(ecc[v], dist[v])
    dist[farthest] = 0
    tree_dfs(farthest, -1, dist)
    for v in trees[i]:
        ecc[v] = max(ecc[v], dist[v])

on_cyc = [0] * graph_size
def do_half():
    mod_max = [max_depth[i] + i for i in range(cyc_size)]
    ext_cyc = mod_max[:]
    for i in range(cyc_size):
        ext_cyc.append(mod_max[i] + cyc_size)
    dq = deque()
    interval = cyc_size // 2 + 1
    for i in range(cyc_size + interval - 1):
        while dq and dq[0][1] <= i - interval:
            dq.popleft()
        while dq and dq[-1][0] <= ext_cyc[i]:
            dq.pop()
        dq.append((ext_cyc[i], i))

        if i >= interval - 1:
            on_cyc[cyc[i - interval + 1]] = max(on_cyc[cyc[i - interval + 1]], dq[0][0] - (i - interval + 1))
do_half()
cyc.reverse()
max_depth.reverse()
do_half()
cyc.reverse()
max_depth.reverse()

for i in range(cyc_size):
    for v in trees[i]:
        ecc[v] = max(ecc[v], on_cyc[cyc[i]] + depth[v])

for i in range(graph_size):
    my_assert(0 <= ecc[i] < graph_size, "invalid eccentricity")
    ouf.write(f"{i} {ecc[i]}")