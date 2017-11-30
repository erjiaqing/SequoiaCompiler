#ifndef __EJQ_TRIE_H
#define __EJQ_TRIE_H
typedef struct ejq__trienode {
	struct ejq__trienode *nxt[256];
	int flag;
}trienode;

trienode* new_node();
int trie_insert(char *iden, int item_id);
#endif
