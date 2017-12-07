#ifndef _E_TRIE_CPP
#define _E_TRIE_CPP

#include <vector>
namespace ejq_compiler{
using std::vector;
class trie{
	class node{
		private:
			size_t next[256];
			int flg;
		public:
			node(){memset(next, 0, sizeof next);flag = -1;}
			node(const node &f)
			{
				// construct with fork
				memcpy(next, f.next, sizeof next);
				flg = f.flg;
			}
			size_t get_next(unsigned char n){return next[n];}
			size_t get_flg()const{return flg;}
			void set_flg(int nflg){flg = nflg;}
			void set_next(unsigned char n, int snext){next[n] = snext;}
	};
	vector<node> nodes;
	vector<size_t> roots;
	vector<size_t> versions;
	size_t current_version;
	public:
	trie(){
		current_version = 0;
		nodes.push_back(node());
		roots.push_back(0);
		versions.push_back(nodes.size());
	}
	size_t get_version(){return current_version;}
	void back_to_version(size_t version)
	{
		while (roots.size() > version) roots.pop_back();
		while (versions.size() > version) versions.pop_back();
		current_version = version;
	}
   	size_t insert(char *str, int type_id = -1)
	{
		size_t cur_version = ++current_version;
		size_t cur_root = nodes.size();
		nodes.push_back(nodes[cur_version - 1]);
		roots.push_back(cur_root);
		for (int i = 0; str[i]; i++)
		{
			size_t curnext = nodes[cur_root].get_next(str[i]);
			if (curnext)
			{
				nodes[cur_root].set_next(nodes.size());
				nodes.push_back(nodes[curnext]);
			} else {
				nodes[cur_root].set_next(nodes.size());
				nodes.push_back();
			}
			cur_root = cur_next;
		}
		nodes[cur_root].set_flg(type_id);
		versions.push_back(nodes.size());
	}
	int query(char *str)
	{
		size_t cur_root = roots[current_version];
		for (int i = 0; str[i]; i++)
		{
			if (nodes[cur_root].get_next(str[i]))
				cur_root = nodes[cur_root].get_next(str[i]);
			else
				return -1;
		}
		return nodes[cur_root].get_flg();
	}
	int query_in_version(char *str, size_t version)
	{
		size_t cur_root = roots[version];
		for (int i = 0; str[i]; i++)
		{
			if (nodes[cur_root].get_next(str[i]))
				cur_root = nodes[cur_root].get_next(str[i]);
			else
				return -1;
		}
		return nodes[cur_root].get_flg();
	}
};
}

#endif
