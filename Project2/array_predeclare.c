#ifndef E_ARRAY_PREDECLARE_C
#define E_ARRAY_PREDECLARE_C

struct E_global_variable_list;
typedef struct E_global_variable_list E_gv;

struct E_global_variable_list{
	E_gv *next;
	size_t item_id;
};

#define head E_gv_head

E_gv *head;

void E_global_variable_list_append(size_t which)
{
	E_gv *thi = (E_gv*) malloc(sizeof(E_gv));
	memset(thi, 0, sizeof(E_gv));
	thi->item_id = which;
	thi->next = head;
	head = thi;
}

#undef head

#endif
