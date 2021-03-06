#ifndef __EJQ_YACC_NODE_C_
#define __EJQ_YACC_NODE_C_

typedef struct node{
	char *label;
	int soncnt;
	int _start_line, _start_char, _end_line, _end_char;
	struct node** son;
} node;

typedef struct node* node_star;

int isLabel(node_star a, const char *b)
{
	char *aa = a->label;
	return strcmp(aa, b) == 0;
}

char* getLabel(node_star a)
{
	return a->label;
}

node_star _newnode(char *name, int start_lineno, int start_pos, int end_lineno, int end_pos, int n, ...)
{
#ifdef _DEBUG
	fprintf(stderr, "[%sINFO\033[0m] new node %s%s\033[0m\n", write_color(_E_COLOR_INFO), write_color(_E_COLOR_OK), name);
#endif
	node_star ret = (node_star) malloc(sizeof(node));
	ret->label = (char*) malloc((strlen(name) + 5) * (sizeof(char)));
	strcpy(ret->label, name);
	//fprintf(stderr, "Newnode: %s\n", ret->label);
	va_list vl;
	va_start(vl, n);
	ret->soncnt = n;
	if (start_lineno)
	{
		ret->_start_line = start_lineno;
		ret->_start_char = start_pos;
		ret->_end_line = end_lineno;
		ret->_end_char = end_pos;
	} else {
		ret->_start_line = ret->_start_char = 0x7fffffff;
		ret->_end_line = ret->_end_char = 0;
	}
	int soncnt = n;
	if (soncnt)
	{
		ret->son = (node**) malloc((sizeof(node*)) * soncnt);
		for (int i = 0; i < n; i++)
		{
			ret->son[i] = va_arg(vl, node*);
			if (ret->son[i])
			{
				calcPosition(ret, ret->son[i]);/*
				if (ret->son[i]->start_lineno == ret->start_lineno && ret->son[i]->start_pos < ret->start_pos)
					ret->_start_pos = ret->son[i]->start_pos;
				else if (ret->son[i]->start_lineno < ret->start_lineno)
					ret->_start_pos = ret->son[i]->start_pos, ret->start_lineno = ret->son[i]->start_lineno;
				if (ret->son[i]->end_lineno == ret->end_lineno && ret->son[i]->end_pos > ret->end_pos)
					ret->end_pos = ret->son[i]->end_pos;
				else if (ret->son[i]->end_lineno > ret->end_lineno)
					ret->end_pos = ret->son[i]->end_pos, ret->end_lineno = ret->son[i]->end_lineno;*/
			}
		}
	} else {
	}
	va_end(vl);
	return ret;
}


#endif
