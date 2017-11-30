#ifndef __EJQ_TRIE_C
#define __EJQ_TRIE_C

#include "trie.h"

trienode* trie_root = NULL;

trienode* new_node()
{
	trienode* ret = (trienode*)malloc(sizeof(trienode));
	memset(ret, 0, sizeof(trienode));
	return ret;
}

int trie_insert(char *iden, int item_id)
{
#ifdef _DEBUG
	fprintf(stderr, "[%sINFO\033[0m] trie insert %s%s\033[0m\n", write_color(_E_COLOR_INFO), wrtie_color(_E_COLOR_OK), iden);
#endif
	trienode* cur;
	if (trie_root == NULL) trie_root = new_node();
	cur = trie_root;
	for (int i = 0; iden[i]; i++) // C99 feature
	{
		if (cur->nxt[iden[i]] == NULL)
			cur->next[iden[i]] = new_node();
		cur = cur->next[iden[i]];
	}
	if (cur->vartable)
	{
		// already declared identifier
		return 0;
	}
	cur->vartable = item_id;
	return 1;
}

int trie_find(char *iden)
{
	trienode* cur = trie_root;
	for (int i = 0; iden[i] && cur; i++)
		cur = cur->next[iden[i]];
	if (!cur) return 0;
	if (!cur->vartable) return 0;
	return cur->vartable;
}

#endif
