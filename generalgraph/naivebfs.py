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

for i in range(graph_size):
    qu = deque()
    visited = [False] * graph_size
    qu.append((i, 0))
    visited[i] = True
    while qu:
        cur, dist = qu[0]
        qu.popleft()
        ecc[cur] = max(ecc[cur], dist)
        for next in adj[cur]:
            if not visited[next]:
                qu.append((next, dist + 1))
                visited[next] = True

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