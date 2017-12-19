#ifndef TRANSLATE_C
#define TRANSLATE_C

#define trans( x ) void translate_T_##x
#define translate( x , ... ) translate_T_##x( T_##x * _, ##__VA_ARGS__ )
#define transdecl( x , ... ) void translate( x, ##__VA_ARGS__)
#define transcall( x , ... ) translate_T_##x( __VA_ARGS__ )
#define N( x ) T_##x

transdecl(Program);
transdecl(ExtDefList);
transdecl(ExtDef);
transdecl(ExtDecList);
transdecl(Specifier);
transdecl(StructSpecifier);
transdecl(OptTag);
transdecl(Tag);
transdecl(VarDec);
transdecl(VarDimList);
transdecl(FunDec);
transdecl(VarList);
transdecl(ParamDec);
transdecl(CompSt);
transdecl(StmtList, int); // after exec
transdecl(Stmt);
transdecl(DefList);
transdecl(Def);
transdecl(DecList);
transdecl(Dec);
transdecl(Exp, int, int); // truelabel falselabel
transdecl(Args);

///////////////////////

transdecl(Program) {transcall(ExtDefList, _->programBody );}

transdecl(ExtDefList)
{
	foreach(_, extdef, N(ExtDef))
	{
		transcall(ExtDef, extdef);
	}
}

transdecl(ExtDef)
{
	if (_->function) // function
	{
		sprintf(stderr, "function\n");
		int son_cnt = 0;
		foreach ( _->function->varList , vlist , VarList ) son_cnt++;
		// 计算函数参数个数
		size_t func_type = E_trie_find(_->spec->typeName);
		if (!func_type)
		{
			fprintf(stderr, "Unknown type %s\n", _->spec->typeName);
			return;
		}
		size_t func_symbol_item = E_symbol_table_new();
		E_symbol_table[func_symbol_item].type_uid = func_type;
		E_symbol_table[func_symbol_item].is_abstract = EJQ_SYMBOL_FUNCTION;
		E_symbol_table[func_symbol_item].name = _->function->name;
		E_trie_insert(_->function->name, func_symbol_item);
		// 创建函数语法树节点
		size_t current_version = E_trie_get_current_version();
		// 备份当前trie树版本号
		E_symbol_table[func_symbol_item].son_cnt = son_cnt;
		E_symbol_table[func_symbol_item].son = (size_t*)malloc(sizeof(size_t) * son_cnt);
		int ti = 0;
		foreach ( _->function->varList, vlist, VarList )
		{
			size_t func_arg_symbol = E_symbol_table_new();
			E_symbol_table[func_symbol_item].son[ti] = func_arg_symbol;
			E_symbol_table[func_arg_symbol].type_uid = E_trie_find(vlist->thisParam->type->typeName);
			if (!E_symbol_table[func_arg_symbol].type_uid)
			{
				fprintf(stderr, "Unknown type %s\n", vlist->thisParam->type->typeName);
				return;
			}
		}
		// 翻译函数体
		transcall(CompSt, _->functionBody);
		// 恢复trie树版本
		E_trie_back_to_version(current_version);
	} else // variables
	{
	}
}

#undef trans
#undef transdecl
#undef translate
#undef N

#endif
