struct edge{
    int t, v, n;
}e[100005];
int te;
int h[100005], S, T, n, m;
int que[100005], qh, qt;
int lay[100005];

int min(int x, int y)
{
    if (x < y) return x;
    return y;
}

int add_edge(int s, int t, int v)
{
    e[te].t = t;e[te].v = v;e[te].n = h[s];h[s] = te;te = te + 1;
    e[te].t = s;e[te].v = 0;e[te].n = h[t];h[t] = te;te = te + 1;
}

int bfs(int S, int T)
{
	int i = 0;
	while (i < n)
	{
		lay[i] = 0;
		i = i + 1;
	}
//    fill(lay, -1, n);
    que[qh = qt = 0] = S;
    while (qh <= qt)
    {
        int u = que[qh];
        int ed = h[u];
        qh = qh + 1;
        ///
        while (ed != -1)
        {
            if (e[ed].v && lay[e[ed].t] == -1) {
                lay[e[ed].t] = lay[u] + 1;
                qt = qt + 1;
                que[qt] = e[ed].t;
            }
            ed = e[ed].n;
        }
    }
    if (lay[T] == -1) return 0;
    return 1;
}

int dinic(int u, int T, int f)
{
    int tans, ans = 0, p = h[u], k;
    if (u == T) return f;
    while (p != -1 && f)
    {
        if (lay[e[p].t] == lay[u] + 1 && (tans = dinic(e[p].t, T, min(f, e[p].v))))
        {
            e[p].v = e[p].v - tans;
            e[p / 2 + 1 - (p - p / 2 * 2)].v = e[p / 2 + 1 - (p - p / 2 * 2)].v + tans;
            ans = ans + tans;
            f = f - tans;
        }
        p = e[p].n;
    }
    return ans;
}

int max_flow()
{
    int ans = 0, tans;
    while (bfs(S, T))
        while (tans = dinic(S, T, 0x3f3f3f3f))
            ans = ans + tans;
    return ans;
}

int main()
{
    int i;
	while (i < 100005)
	{
		h[i] = -1;
		i = i + 1;
	}
	n = read();
	m = read();
    S = 0;
    T = n + 1;
    i = 1;
    while (i <= n)
    {
        int ta, tb;
        ta = read();
		tb = read();
        add_edge(S, i, ta);
        add_edge(i, T, tb);
        i = i + 1;
    }
    i = 1;
    while (i <= m)
    {
        int ta, tb, tc;
		ta = read();
		tb = read();
		tc = read();
        add_edge(ta, tb, tc);
        add_edge(tb, ta, tc);
    }
    write(max_flow());
    return 0;
}
