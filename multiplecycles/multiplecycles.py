import sys, os
from collections import deque

sys.setrecursionlimit(10 ** 6)
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
inf = open(os.path.join(__location__, "input.in"), "r")
ouf = open(os.path.join(__location__, "output.out"), "w")

def my_assert(cond, comment):
    if not cond:
        ouf.write(f"Assertion Failed: {comment}")
        ouf.close()
        sys.exit(0)

graph_size, edge_num = map(int, inf.readline().rstrip().split())
adj = [[] for _ in range(graph_size)]
for i in range(edge_num):
    u, v = map(int, inf.readline().rstrip().split())
    u -= 1; v -= 1
    my_assert(u != v and 0 <= u < graph_size and 0 <= v < graph_size, "wrong input")
    adj[u].append(v)
    adj[v].append(u)
inf.close()
ecc = [0] * graph_size

cyc_memo = []
cyc_visited = [False] * graph_size
parent, dfs_order = [0] * graph_size, [0] * graph_size
counter = 0
def cyc_dfs(cur, pre):
    global counter
    parent[cur] = pre
    cyc_visited[cur] = True
    dfs_order[cur] = counter
    counter += 1
    for next in adj[cur]:
        if next != pre:
            if cyc_visited[next]:
                if dfs_order[next] < dfs_order[cur]:
                    cyc_memo.append((cur, next))
            else:
                cyc_dfs(next, cur)
cyc_dfs(0, -1)
my_assert(cyc_memo, "no cycles")

cyc = []
for backstart, backend in cyc_memo:
    cyc.append([])
    backcur = backstart
    while backcur != backend:
        cyc[-1].append(backcur)
        backcur = parent[backcur]
    cyc[-1].append(backend)

cyc_idx = [[] for _ in range(graph_size)]
for i in range(len(cyc)):
    for v in cyc[i]:
        cyc_idx[v].append(i)

adj_copy = [[] for _ in range(graph_size)]
for i in range(graph_size):
    for v in adj[i]:
        adj_copy[i].append((v, False))
for i in range(len(cyc)):
    cyc_len = len(cyc[i])
    for j in range(cyc_len):
        for k in range(len(adj_copy[cyc[i][j]])):
            v = adj_copy[cyc[i][j]][k][0]
            if v in [cyc[i][(j - 1) % cyc_len], cyc[i][(j + 1) % cyc_len]]:
                adj_copy[cyc[i][j]][k] = (v, True)
nocyc_adj = [[] for _ in range(graph_size)]
for i in range(graph_size):
    for v, chk in adj_copy[i]:
        if not chk:
            nocyc_adj[i].append(v)

visited = [False] * graph_size
def process_cyc(i, prev_cyc):
    cur_cyc = cyc[i][:]
    cyc_len, other_cyc = len(cur_cyc), -1
    for j in range(cyc_len):
        if visited[cur_cyc[j]]:
            my_assert(other_cyc == -1, "invalid cycle traversal")
            other_cyc = j
    my_assert(i == 0 or other_cyc != -1, "invalid cycle traversal")
    
    trees = [[] for _ in range(cyc_len)]
    depth, max_depth = [-1] * graph_size, [-1] * cyc_len
    dist = [-1] * graph_size
    for j in range(cyc_len):
        if j == other_cyc:
            max_depth[j] = ecc[cur_cyc[j]]
            trees[j].append(cur_cyc[j])
            continue

        root = cur_cyc[j]
        def tree_dfs(cur, pre, d, push):
            if push:
                trees[j].append(cur)
            for next in nocyc_adj[cur]:
                if next != pre:
                    d[next] = d[cur] + 1
                    tree_dfs(next, cur, d, push)
        depth[root] = 0
        tree_dfs(root, -1, depth, True)
        
        deepest = -1
        for v in trees[j]:
            if max_depth[j] < depth[v]:
                max_depth[j] = depth[v]
                deepest = v
        my_assert(deepest != -1, "depth array is incorrect")

        dist[deepest] = 0
        tree_dfs(deepest, -1, dist, False)
        diam_len, farthest = -1, -1
        for v in trees[j]:
            if diam_len < dist[v]:
                diam_len = dist[v]
                farthest = v
        my_assert(farthest != -1, "dist array is incorrect")

        for v in trees[j]:
            ecc[v] = max(ecc[v], dist[v])
        dist[farthest] = 0
        tree_dfs(farthest, -1, dist, False)
        for v in trees[j]:
            ecc[v] = max(ecc[v], dist[v])
    my_assert(-1 not in max_depth, "unvisited vertex exists")
    
    on_cyc = [0] * graph_size
    def do_half():
        mod_max = [max_depth[j] + j for j in range(cyc_len)]
        ext_cyc = mod_max[:]
        for j in range(cyc_len):
            ext_cyc.append(mod_max[j] + cyc_len)
        dq = deque()
        interval = cyc_len // 2
        for j in range(cyc_len + interval):
            while dq and dq[0][1] <= j - interval:
                dq.popleft()
            while dq and dq[-1][0] < ext_cyc[j]:
                dq.pop()
            dq.append((ext_cyc[j], j))

            if j >= interval:
                on_cyc[cur_cyc[j - interval]] = max(on_cyc[cur_cyc[j - interval]], dq[0][0] - (j - interval))
    do_half()
    cur_cyc.reverse()
    max_depth.reverse()
    do_half()
    cur_cyc.reverse()
    max_depth.reverse()
    
    upd_visited = [False] * graph_size
    for j in range(cyc_len):
        if j != other_cyc:
            for v in trees[j]:
                ecc[v] = max(ecc[v], on_cyc[cur_cyc[j]] + depth[v])
        else:
            qu = deque()
            qu.append((cur_cyc[j], 0))
            stretch = 0
            while qu:
                cur, d = qu[0]
                qu.popleft()
                stretch = max(stretch, d)
                for next in adj[cur]:
                    if not visited[next] and depth[next] != -1:
                        visited[next] = True
                        qu.append((next, d + 1))
            
            qu.append((cur_cyc[j], 0))
            upd_visited[cur_cyc[j]] = True
            while qu:
                cur, d = qu[0]
                qu.popleft()
                ecc[cur] = max(ecc[cur], d + stretch)
                for next in adj[cur]:
                    if not upd_visited[next] and next not in [cur_cyc[(j - 1) % cyc_len], cur_cyc[(j + 1) % cyc_len]]:
                        upd_visited[next] = True
                        qu.append((next, d + 1))
    
    if other_cyc == -1:
        for j in range(cyc_len):
            for v in trees[j]:
                visited[v] = True
    for j in range(cyc_len):
        for v in trees[j]:
            for next_cyc in cyc_idx[v]:
                if next_cyc not in [prev_cyc, i]:
                    process_cyc(next_cyc, i)
process_cyc(0, -1)

cnt = [0] * graph_size
for i in range(graph_size):
    my_assert(0 <= ecc[i] < graph_size, "invalid eccentricity")
    cnt[ecc[i]] += 1
eccseq = []
for i in range(graph_size):
    for j in range(cnt[i]):
        eccseq.append(i)
for i in range(len(eccseq)):
    ouf.write(f"{eccseq[i]} ")
ouf.write("\n")
ouf.close()