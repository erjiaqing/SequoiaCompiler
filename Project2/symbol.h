#ifndef __EJQ_SYMBOL_H
#define __EJQ_SYMBOL_H

typedef struct __ejq__symbol_item{
	int type; // 1 - int / 2 - float / 1024 - func-return void / 1025 - func-return int / 1026 - func-return float / negative: struct (in symbol table)
	int is_array; // is this an array?
	int width; // for varibles and struct, for array, this is the width of a single element
	int arg_cnt;
	int id;
	int father; // the symbol table is a tree!
	int* args; // for func / struct
}symbol_item;

symbol_item* ejq__symbol_table = NULL;
int ejq__symbol_table_length = 1, ejq__symbol_table_used = 1;
// symbol_item[0] is root!

int new_symbol_item(int, int, int, int, int, int, ...);

#include "symbol.c"
#endif
