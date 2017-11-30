#ifndef __EJQ_SYMBOL_C
#define __EJQ_SYMBOL_C

#include "symbol.h"
int new_symbol_item(int type, int is_array, int width, int id, int father, int arg_cnt, ...)
{
	if (ejq__symbol_table == NULL)
	{
		ejq__symbol_table = (symbol_item*) malloc(sizeof(symbol_item));
	}
	if (ejq__symbol_table_length == ejq__symbol_table_used)
	{
		// extend symbol table
		symbol_item* tmp = (symbol_item*) malloc(sizeof(symbol_item) * ejq__symbol_table_length * 2);
		memcpy(tmp, ejq__symbol_table, sizeof(symbol_item) * ejq__symbol_table_length);
		free(ejq__symbol_table);
		ejq__symbol_table = tmp;
		ejq__symbol_table_length *= 2;
		// just like vector in C++
	}
	ejq__symbol_table_used++;
	symbol_item* __this = ejq__symbol_table + ejq__symbol_table_used;
	__this->type = type;
	__this->is_array = is_array;
	__this->width = width;
	__this->arg_cnt = arg_cnt;
	__this->father = father;
	__this->args = (int*) malloc(sizeof(int) * arg_cnt);

	//----
	va_list vl;
	va_start(vl, arg_cnt);
	for (int i = 0; i < arg_cnt; i++)
		__this->args[i] = va_arg(vl, int);
	va_end(vl);
	//----

	return ejq__symbol_table_used;
}



#endif
