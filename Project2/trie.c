#ifndef __EJQ_TRIE_H
#define __EJQ_TRIE_H

// 没有依赖
//
// 这里实现一个可持久化字典树用来处理作用域的问题
//
// 可持久化字典树支持下面三个操作
// 1. 查询当前版本中是否有某个节点
// 2. 新建一个版本，并向该版本中插入一个单词
// 3. 回退到某个版本（即删除某个版本之后创建的全部内容
//
// 可持久化字典树的历史版本构成了一个栈式结构，这与变量作用域的形式天然相容
//
// 对于结构体里的变量，因为太懒不想创建节点，所以用
// <结构体名>.<子结构体名>...<变量名>
// 好像并没有什么不妥
//
// Flag表示了这个节点对应的编号
//
// C没有类，没有命名空间，没有成员函数，好累……

#include <stdlib.h>
#include <string.h>

typedef struct ejq__trienode {
	size_t nxt[256];
	// 指向下一个节点的指针
	// 因为0号是总根，没有节点的子节点会是0号
	// 所以0号也有不存在的含义
	int flag;
} E_trie;

E_trie* E_trie_nodes = NULL; // 用于存储节点的内存池，相当于C++中的vector
size_t* E_trie_roots = NULL; // 用于存储各版本根节点编号
size_t* E_trie_version_state = NULL; // 用于存储各版本E_trie_node_used值
size_t E_trie_current_version = 0; // 当前版本号(的下一个版本号)，也是字典树中的条目数
size_t E_trie_version_cap = 0; // 版本容量
size_t E_trie_node_used = 0;
size_t E_trie_tot_node = 0;

// 获取一个新的节点
int __E_trie_new_node()
{
	if (E_trie_tot_node == 0)
	{
		// 初始化
		E_trie_tot_node++;
		E_trie_nodes = (E_trie*)malloc(sizeof(E_trie) * (E_trie_tot_node));
	}
	if (E_trie_node_used == E_trie_tot_node)
	{
		E_trie* E_new_trie_nodes = (E_trie*)malloc(sizeof(E_trie) * (E_trie_tot_node * 2));
		// 要一块新的
		memcpy(E_new_trie_nodes, E_trie_nodes, sizeof(E_trie) * (E_trie_tot_node));
		// 把旧的搞到新的里面去
		free(E_trie_nodes);
		// 丢掉旧的
		E_trie_nodes = E_new_trie_nodes;
		// 把新的当旧的
		E_trie_tot_node *= 2;
		// 但是容量变成新的
	}
	memset(E_trie_nodes + E_trie_node_used, 0, sizeof(E_trie));
	return E_trie_node_used++;
}

int __E_trie_fork_node(int orig)
{
	size_t T = __E_trie_new_node();
	memcpy(E_trie_nodes[T].nxt, E_trie_nodes[orig].nxt, sizeof(E_trie_nodes[0].nxt));
	E_trie_nodes[T].flag = E_trie_nodes[orig].flag;
	return T;
}

// 创建一个新版本，并设置根节点
int __E_trie_new_version()
{
	if (E_trie_version_cap == 0)
	{
		E_trie_version_cap++;
		E_trie_roots = (size_t*)malloc(sizeof(size_t) * (E_trie_version_cap));
		E_trie_version_state = (size_t*)malloc(sizeof(size_t) * (E_trie_version_cap));
	}
	// 版本池满了，扩大两倍
	if (E_trie_current_version == E_trie_version_cap)
	{
		size_t* E_new_version_roots = (size_t*)malloc(sizeof(size_t) * (E_trie_version_cap * 2));
		memcpy(E_new_version_roots, E_trie_roots, sizeof(size_t) * (E_trie_version_cap));
		free(E_trie_roots);
		//--
		size_t* E_new_version_state = (size_t*)malloc(sizeof(size_t) * (E_trie_version_cap * 2));
		memcpy(E_new_version_state, E_trie_version_state, sizeof(size_t) * (E_trie_version_cap));
		free(E_trie_version_state);
		//--
		E_trie_roots = E_new_version_roots;
		E_trie_version_cap *= 2;
	}
	size_t E_trie_last_version = E_trie_current_version;
	if (E_trie_last_version) {

		E_trie_last_version--;
		E_trie_roots[E_trie_current_version] = __E_trie_fork_node(E_trie_roots[E_trie_last_version]);
	} else {
		E_trie_roots[E_trie_current_version] = __E_trie_new_node();
	}
	// 复制上一个节点的根
	return E_trie_current_version++;
}

// 获取当前版本
size_t E_trie_get_current_version() {return E_trie_current_version - 1;}

// 封闭当前的版本，不允许新的new_node()
void __E_trie_finalize_new_version()
{
	E_trie_version_state[E_trie_get_current_version()] = E_trie_tot_node;
}

// 往字典树中插入一个单词，对应的语法结构编号是item_id
int __E_trie_insert(char *s, int item_id)
{
	size_t rt = __E_trie_new_version();
	for (int i = 0; s[i]; i++)
	{
		if (E_trie_nodes[rt].nxt[s[i]])
		{
			E_trie_nodes[rt].nxt[s[i]] = __E_trie_fork_node(E_trie_nodes[rt].nxt[s[i]]);
			// 如果已经有了，就复制一个
		}
		else
		{
			E_trie_nodes[rt].nxt[s[i]] = __E_trie_new_node();
			// 如果没有，就创建一个新的
		}
		rt = E_trie_nodes[rt].nxt[s[i]];
	}
	E_trie_nodes[rt].flag = item_id;
	__E_trie_finalize_new_version();
	return 0;
	// 没有问题
}

// 在字典树的当前版本中查询一个单词，并返回其Flag
int E_trie_find(char *s)
{
	if (!E_trie_current_version) return 0;
	// 如果没有创建过版本，所以就直接返回不存在好了
	size_t rt = E_trie_roots[E_trie_get_current_version()];
	for (int i = 0; s[i]; i++)
	{
		if (E_trie_nodes[rt].nxt[s[i]])
			rt = E_trie_nodes[rt].nxt[s[i]];
		else
			return 0;
	}
	return E_trie_nodes[rt].flag;
}

// 但是要先检查一个
int E_trie_insert(char *s, int item_id)
{
	if (E_trie_find(s)) return 1;
	return __E_trie_insert(s, item_id);
}

// 回退到某个版本创建之后的状态
void E_trie_back_to_version(int version_id)
{
	E_trie_current_version = version_id + 1;
	E_trie_node_used = E_trie_version_state[version_id];
}
#endif
