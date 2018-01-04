#ifndef TRANSLATE_C
#define TRANSLATE_C

#include <assert.h>
#include "array_predeclare.c"

#define trans( x ) void translate_T_##x
#define translate( x , ... ) translate_T_##x( T_##x * _, ##__VA_ARGS__ )
#define transdecl( x , ... ) void translate( x, ##__VA_ARGS__)
#define transdecl_sizet( x , ... ) size_t translate( x, ##__VA_ARGS__)
#define transcall( x , ... ) translate_T_##x( __VA_ARGS__ )
#define N( x ) T_##x

transdecl(Program);
transdecl(ExtDefList);
transdecl(ExtDef);
transdecl(ExtDecList);
size_t translate(Specifier, char*);
size_t translate(StructSpecifier, char*); // base name
transdecl(OptTag);
transdecl(Tag);
transdecl_sizet(VarDec, size_t);
transdecl_sizet(VarDimList, size_t);
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
RetType translate(Exp, int, int, int); // need return, truelabel falselabel
transdecl(Args);

int totTmp;
int totLab;

int totRange, currentRange; // 作用域全局编号

int currentFunctionReturnType; // 当前函数的返回值类型

///////////////////////

transdecl(Program)
{
	// 打印内置函数
	output("FUNCTION f%d:\n"
			"READ _tread\n"
			"RETURN _tread\n\n"
 		   "FUNCTION f%d:\n"
		    "PARAM _twrite\n"
			"WRITE _twrite\n"
			"RETURN #1\n\n"
		   "FUNCTION __memcpy:\n"
			"PARAM _cpy_t\n"
			"PARAM _cpy_s\n"
			"PARAM _cpy_l\n"
			"_cpyed := #0\n"
			"LABEL _before:\n"
			"IF _cpyed == _cpy_l GOTO _return\n"
			"*_cpy_t := *_cpy_s\n"
			"_cpy_t := _cpy_t + #4\n"
			"_cpy_s := _cpy_s + #4\n"
			"_cpyed := _cpyed + #4\n"
			"GOTO _before\n"
			"LABEL _return:\n"
			"RETURN #0\n\n",
			E_trie_find("read"),
			E_trie_find("write"));
	foreach( _->programBody, extdef, N(ExtDefList))
		transcall(ExtDef, extdef->code );
	// 打印运行时函数
	output("FUNCTION __init_global_array__:\n");
	foreach(E_gv_head, gv, E_gv)
	{
		int var_id = gv->item_id;
		int sz = E_symbol_table[var_id].len;
		output("DEC v%d [%d]\n", var_id, sz);
	}
	output("RETURN #0\n");
}

size_t translate(Specifier, char *baseName)
{
	size_t vari_type = 0;
	if (_->structName)
	{
		vari_type = transcall(StructSpecifier, _->structName, baseName);
	} else {
		vari_type = E_trie_find(_->typeName);
	}
	if (!vari_type)
	{
		ce("type %d in line %d:", 17, _->_start_line);
		ce("unknown type ``%s''", _->typeName);
		return 1;
		// 好像很多编译器都是拿int当默认类型
		// 此处参照这个设定
		// 应该是C标准里面的
	}
	return vari_type;
}

transdecl(ExtDef)
{
	if (_->function) // function
	{
		int son_cnt = 0;
		foreach ( _->function->varList , vlist , N(VarList) ) son_cnt++;
		// 计算函数参数个数
		size_t func_type = transcall(Specifier, _->spec, "");
		// 函数被定义了
		if (E_trie_find(_->function->name))
		{
			ce("type %d in line %d:", 4, _->_start_line);
			ce("duplicate identify ``%s'' in current namespace\n", _->function->name);
			return;
		}
		// TODO: 处理包含结构体定义的情况
		if (!func_type)
		{
			ce("type %d in line %d:", 17, _->_start_line);
			ce("unknown type ``%s''\n", _->spec->typeName);
			return;
		}
		int func_symbol_item = E_symbol_table_new();
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
		// 打印函数首部
		// output("FUNCTION %s:\n", _->function->name);
		if (strcmp(_->function->name, "main") == 0)
		{
			output("FUNCTION main:\n");
			output("_ := CALL __init_global_array__\n");
		} else {
			output("FUNCTION f%d:\n", func_symbol_item);
		}
		foreach ( _->function->varList, vlist, N(VarList) )
		{
			int func_arg_symbol = E_symbol_table_new();
			E_symbol_table[func_symbol_item].son[ti] = func_arg_symbol;
			E_symbol_table[func_arg_symbol].type_uid = E_trie_find(vlist->thisParam->type->typeName);
			E_symbol_table[func_arg_symbol].is_abstract = EJQ_SYMBOL_NORMAL;
			E_trie_insert(vlist->thisParam->name->varName, func_arg_symbol);
			if (!E_symbol_table[func_arg_symbol].type_uid)
			{
				ce("type %d in line %d:", 17, _->_start_line);
				ce("unknown type %s\n", vlist->thisParam->type->typeName);
				return;
			}
			ti++;
			// 获取参数
			output("PARAM v%d\n", func_arg_symbol);
		}
		currentFunctionReturnType = func_type;
		// 获取参数表
		// 假设参数类型全部为int
		// 翻译函数体
		transcall(CompSt, _->functionBody);
		currentFunctionReturnType = 0;
		// 恢复trie树版本
		E_trie_back_to_version(current_version);
		if (func_type == 1)
		{
			output("RETURN #0\n");
		} else if (func_type == 2)
		{
			output("RETURN #0.0\n");
		}
		output("\n");
	} else // variables
	{
		size_t vari_type = transcall(Specifier, _->spec, "");
		// 获取变量类型号
		foreach ( _->dec, decList, N(ExtDecList) )
		{
			// 单独处理每个变量
			if (E_trie_find(decList->dec->varName))
			{
				// 可持久化Trie树
				// 这是全局变量，所以直接查询当前的版本有没有定义就好了
				ce("type %d in line %d:", 3, _->_start_line);
				ce("redeclare of ``%s''", decList->dec->varName);
				continue;
			}
			// 计算这个变量
			size_t id = transcall(VarDec, decList->dec, vari_type);
			// 加入到trie树中
			E_trie_insert(decList->dec->varName, id);
			// 如果是数组的话，要记得申请内存
			int new_type = E_symbol_table[id].type_uid;
			int isArray = (E_symbol_table[new_type].is_abstract == EJQ_SYMBOL_ARRAY);
			// 全局变量里面的结构体，也要当作数组来初始化
			if (isArray || _->spec->structName)
				E_global_variable_list_append(id);
		}
	}
}

size_t translate(StructSpecifier, char *baseName)
{
	// 如果baseName为null，那么这不是在struct里面
	// 否则就在struct里面
	char *name = _->typeName;
	size_t structId = 0;
	if (_->structBody)
	{
		// 这是一个结构体的定义
		if (E_trie_find(name))
		{
			ce("type %d in line %d:", 16, _->_start_line);
			ce("identifier ``%s'' is already used", name);
		}
		structId = E_symbol_table_new();
		char buf[1024], buf2[1024];
		sprintf(buf, "%s%s%s", baseName, (baseName && baseName[0]) ? "." : "", name);
		E_trie_insert(buf, structId);
		int son_cnt = 0;
		foreach(_->structBody, def, N(DefList))
		{
			foreach(def->def->decList, dec, N(DecList))
			{
				son_cnt++;
				if (dec->dec->value)
				{
					ce("type %d in line %d:", 15, _->_start_line);
					ce("default value in struct is not allowed");
				}
			}
		}
		E_symbol_table[structId].son_cnt = son_cnt;
		E_symbol_table[structId].son = (size_t *)malloc(sizeof(size_t) * son_cnt);
		E_symbol_table[structId].name = (char *)malloc(strlen(buf) + 5);
		E_symbol_table[structId].is_abstract = EJQ_SYMBOL_STRUCT;
		strcpy(E_symbol_table[structId].name, buf);
		int current_son = 0;
		foreach(_->structBody, def, N(DefList))
		{
			size_t this_type = transcall(Specifier, def->def->type, buf);
			// 这里嵌套结构体的方式不是特别清楚，先这样写着
			foreach(def->def->decList, dec, N(DecList))
			{
				sprintf(buf2, "%s.%s", buf, dec->dec->var->varName);
				int tvar_id = E_trie_find(buf2);
				if (tvar_id && E_symbol_table[tvar_id].father == currentRange)
				{
					ce("type %d in line %d:", 15, _->_start_line);
					ce("duplicate define of variable ``%s''", buf2);
				}
				size_t var_id = transcall(VarDec, dec->dec->var, this_type);
				E_symbol_table[var_id].offset = E_symbol_table[structId].len;
				E_symbol_table[structId].len += E_symbol_table[var_id].len;
				E_symbol_table[structId].son[current_son++] = var_id;
				E_trie_insert(buf2, var_id);
			}
		}
	} else {
		structId = E_trie_find(name);
		if (!structId)
		{
			ce("type %d in line %d:", 17, _->_start_line);
			ce("struct ``%s'' is not declared", name);
			return 1;
		}
	}
	return structId;
}

transdecl(Def)
{
	size_t vari_type = transcall(Specifier, _->type, "");
	foreach ( _->decList, decList, N(DecList))
	{
		int var_trie_node = E_trie_find(decList->dec->var->varName);
		if (var_trie_node && E_symbol_table[var_trie_node].father == currentRange)
		{
			ce("type %d in line %d:", 3, _->_start_line);
			ce("redeclare of ``%s''", decList->dec->var->varName);
			continue;
		}
		size_t id = transcall(VarDec, decList->dec->var, vari_type);
		// 如果是数组的话，要申请内存
		int new_type = E_symbol_table[id].type_uid;
		int isArray = (E_symbol_table[new_type].is_abstract == EJQ_SYMBOL_ARRAY);
		if (isArray || _->type->structName)
		{
			output("DEC v%zu [%zu]\n", id, E_symbol_table[id].len);
			if (decList->dec->value)
			{
				RetType rightval = transcall(Exp, decList->dec->value, False, 0, 0);
				ce("type %d in line %d:", 7, decList->dec->_start_line);
				ce("type mismatch, left val is ``%s'' but right val is ``%s''",
					E_symbol_table[new_type].name,
					E_symbol_table[rightval.type].name);
			}
		} else if (decList->dec->value)
		{
			RetType rightval = transcall(Exp, decList->dec->value, False, 0, 0);
			if (new_type != rightval.type)
			{
				ce("type %d in line %d:", 7, decList->dec->_start_line);
				ce("type mismatch, left val is ``%s'' but right val is ``%s''",
				E_symbol_table[new_type].name,
				E_symbol_table[rightval.type].name);
			} else {
				output("v%d := %s%d\n", (int)id, EJQ_LRTYPE(rightval), rightval.id);
			}
		}
		E_trie_insert(decList->dec->var->varName, id);
	}
}

transdecl_sizet(VarDec, size_t spec)
{
	size_t typeid = spec;
	if (_->dim)
	{
		typeid = transcall(VarDimList, _->dim, spec);
	}
	size_t thisid = E_symbol_table_new();
	E_symbol_table[thisid].type_uid = typeid;
	E_symbol_table[thisid].name = malloc(strlen(_->varName) + 5);
	strcpy(E_symbol_table[thisid].name, _->varName);
	E_symbol_table[thisid].len = E_symbol_table[typeid].len;
	E_symbol_table[thisid].is_abstract = 0x002;
	E_symbol_table[thisid].offset = 0;
	E_symbol_table[thisid].son_cnt = 0;
	E_symbol_table[thisid].son = NULL;
	E_symbol_table[thisid].father = currentRange;
	//E_trie_insert(_->varName, thisid);
	return thisid;
}

transdecl_sizet(VarDimList, size_t spec)
{
	size_t thisdim_id = spec;
	size_t thisid = E_symbol_table_new();
	char buf[1024];
	if (_->next)
	{
		thisdim_id = transcall(VarDimList, _->next, spec);
		E_symbol_table[thisid].dim = E_symbol_table[thisdim_id].dim + 1;
		sprintf(buf, "%s[]", E_symbol_table[thisdim_id].name);
	} else {
		E_symbol_table[thisid].dim = 1;
		sprintf(buf, "%s[]", E_symbol_table[spec].name);
	}
	E_symbol_table[thisid].base_type = spec;
	E_symbol_table[thisid].type_uid = thisdim_id;
	E_symbol_table[thisid].name = (char *)malloc(strlen(buf) + 5);
	strcpy(E_symbol_table[thisid].name, buf);
	E_symbol_table[thisid].len = E_symbol_table[thisdim_id].len * _->thisDim;
	E_symbol_table[thisid].is_abstract = 0x004;
	E_symbol_table[thisid].offset = 0;
	E_symbol_table[thisid].son_cnt = 0;
	E_symbol_table[thisid].son = NULL;
	return thisid;
}

transdecl(CompSt)
{
	int beforeRange = currentRange;
	currentRange = ++totRange;
	// 其实也只有这一个会影响到作用域
	// 声明区
	foreach(_->defList, def, N(DefList))
	{
		transcall(Def, def->def);
	}
	// 计算区
	foreach(_->stmtList, stmt, N(StmtList))
	{
		transcall(Stmt, stmt->statement);
	}
	currentRange = 0;
}

transdecl(Stmt)
{
	if (_->isReturn)
	{
		// 有返回值，要把返回值return掉
		// TODO: 类型检查
		RetType returnTmp = transcall(Exp, _->expression, True, 0, 0);
		// 需要返回值，没有跳转
		char buf[30];
		RetStringify(buf, &returnTmp);
		output("RETURN %s\n", buf);
		if (currentFunctionReturnType != returnTmp.type)
		{
			ce("type %d in line %d:", 8, _->_start_line);
			ce("return value mismatch, ``%s'' expected, got ``%s''",
					E_symbol_table[currentFunctionReturnType].name,
					E_symbol_table[returnTmp.type].name
				);
		}
		// 这是第一句输出的话啊!
	} else if (_->isWhile)
	{
		// while循环
		int while_begin = ++totLab;
		int while_end = ++totLab;
		output("LABEL l%d:\n", while_begin);
		transcall(Exp, _->expression, False, 0, while_end);
		transcall(Stmt, _->ifTrue);
		output("GOTO l%d\n", while_begin);
		output("LABEL l%d:\n", while_end);
	} else if (_->ifTrue)
	{
		// 有true，不是while，那就是if了
		int true_branch = ++totLab;
		int after_if = ++totLab;
		int false_branch = after_if;
		if (_->ifFalse)
		{
			false_branch = 0;
		}
		// 注意到如果没有假分支，那么假就跳到if结束
		RetType r = transcall(Exp, _->expression, False, true_branch, false_branch);
		if (r.type != 1)
		{
			ce("type %d in line %d:", 7, _->_start_line);
			ce("type of conditional expression is not ``int'' (get ``%s'')", E_symbol_table[r.type].name);
		}
		// 肯定有真分支
		if (_->ifFalse)
		{
			transcall(Stmt, _->ifFalse);
			output("GOTO l%d\n", after_if);
		}
		output("LABEL l%d:\n", true_branch);
		transcall(Stmt, _->ifTrue);
		// upd 先翻译假分支，再翻译真分支，节省一次goto
		/*
		if (_->ifFalse)
		{
			// 处理假分支
			// 这时候真分支刚刚执行完，需要跳走，避免执行假分支的内容
			output("GOTO l%d\n", after_if);
			// 然后打出假分支的标号
			output("LABEL l%d:\n", false_branch);
			transcall(Stmt, _->ifFalse);
		}
		*/
		// 最后打出if结束的标号
		output("LABEL l%d:\n", after_if);
	} else if (_->compStatement)
	{
		// 谁这么无聊把一个普通的statement中间还加上大括号
		// 好吧，我干过\_(O. O)~
		transcall(CompSt, _->compStatement);
	} else if (_->expression)
	{
		// 最后是普通的一条语句
		// 终于到了普通的语句了
		// 不要返回值，不跳
		transcall(Exp, _->expression, False, 0, 0);
		// 啊，这么简单的么？
		// 没办法啊，Exp里面的信息量挺大的
		// 突然发现控制流也不难啊
	}
}

RetType translate(Exp, int needReturn, int ifTrue, int ifFalse)
{
	char buf[30];
	RetType ret;
	int res = ++totTmp;
	ret.lrtype = EJQ_RET_RVAL;
	ret.id = res;
	ret.type = 1;
	ret.isImm8 = 0;
	debug("op = %d\n", _->op);
	if (_->isImm8 == EJQ_IMM8_INT)
	{
		ret.isImm8 = EJQ_IMM8_INT;
		ret.imm8val.i = _->intVal;
//		output("t%d := #%d\n", ret.id, _->intVal);
		ret.type = 1; // int
//		return ret;
	} else if (_->isImm8 == EJQ_IMM8_FLOAT)
	{
		ret.isImm8 = EJQ_IMM8_FLOAT;
		ret.imm8val.i = _->floatVal;
//		output("t%d := #%.20f\n", ret.id, _->floatVal);
		ret.type = 2; // float
//		return ret;
	} else if (_->isFunc)
	{
		// 检查函数是否存在
		int func_type = E_trie_find(_->funcName);
		int func_args = E_symbol_table[func_type].son_cnt;
		int func_rettype = E_symbol_table[func_type].type_uid;
		if (!func_type)
		{
			ce("type %d in line %d:", 2, _->_start_line);
			ce("function ``%s'' not found", _->funcName);
			ret.type = 1;
			return ret;
		}
		if (E_symbol_table[func_type].is_abstract != EJQ_SYMBOL_FUNCTION)
		{
			ce("type %d in line %d:", 11, _->_start_line);
			ce("``%s'' is not a function", _->funcName);
			ret.type = 1;
			return ret;
		}
		debug("translate function call ``%s'' --> return with [%s](%d)\n", _->funcName, E_symbol_table[E_symbol_table[func_type].type_uid].name, E_symbol_table[func_type].type_uid);
		// 这是一个函数调用
		int needArgs = func_args;
		int realArgs = 0;
		foreach(_->args, arg, N(Args)) realArgs++;
		if (realArgs < needArgs)
		{
			ce("type %d in line %d:", 9, _->_start_line);
			ce("call function ``%s'' with %d arguments, %d needed", _->funcName, realArgs, needArgs);
		}
		else if (realArgs > needArgs)
		{
			ce("type %d in line %d:", 9, _->_start_line);
			ce("call function ``%s'' with %d arguments, requires exactly %d", _->funcName, realArgs, needArgs);
		}
		//if (realArgs != needArgs)
		//	warn("behavior is unknown");
		foreach(_->args, arg, N(Args))
		{
			RetType t = transcall(Exp, arg->exp, True, 0, 0);
			needArgs--;
			if (needArgs >= 0)
			{
				int son_node = E_symbol_table[func_type].son[needArgs];
				int son_node_type = E_symbol_table[son_node].type_uid;
				if (t.type != son_node_type)
				{
					ce("type %d in line %d:", 9, _->_start_line);
					ce("type of argument %d mismatch: requires ``%s'', got ``%s''", needArgs + 1, E_symbol_table[son_node_type].name, E_symbol_table[t.type].name);
				}
			}
			output("PUSH t%d\n", t.id);
		}
		// 先这样，后面再改成找符号表
		output("t%d := CALL f%d\n", res, func_type);
		ret.type = func_rettype;
		return ret;
	} else if (_->funcName && !_->lExp)
	{
		// 这两个名字是一起的
		// 如果没有左侧的表达式
		// 不是函数名就是变量名
		// 结构体：有funcName, 没有isFunc, 有表达式
		// 函数：有isFunc
		// 变量：有funcName, 没有isFunc, 没有表达式
		debug("find ``%s'' in trie\n", _->funcName);
		int varName = E_trie_find(_->funcName);
		if (!varName)
		{
			ce("type %d in line %d:", 1, _->_start_line);
			ce("undefined variable ``%s''", _->funcName);
			return ret;
		}
		if (E_symbol_table[varName].is_abstract != EJQ_SYMBOL_NORMAL)
		{
			ce("type %d in line %d:", 1, _->_start_line);
			ce("``%s'' is not a variable", _->funcName);
			return ret;
		}
		ret.type = E_symbol_table[varName].type_uid;
		if (E_symbol_table[ret.type].is_abstract & EJQ_SYMBOL_ARRAY)
			ret.lrtype = EJQ_RET_LPTR;
		else
			ret.lrtype = EJQ_RET_LVAL;
		ret.id = varName;
		// output("t%d := v%d\n", res, varName);
		// return ret;
	} else {
		// 就是普通的表达式啦
		switch (_->op)
		{
			case EJQ_OP_ASSIGN:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				RetType rightval = transcall(Exp, _->rExp, True, 0, 0);
				ret = leftval;
				if ((leftval.lrtype != EJQ_RET_LVAL) && !(leftval.lrtype & EJQ_RET_PTR))
				{
					ce("type %d in line %d:", 6, _->_start_line);
					ce("result of left hand expression should be left value");
				}
				if ((E_symbol_table[leftval.type].is_abstract & EJQ_SYMBOL_ARRAY) &&
					(E_symbol_table[rightval.type].is_abstract & EJQ_SYMBOL_ARRAY))
				{
					// 对于数组
					if (E_symbol_table[leftval.type].base_type == E_symbol_table[rightval.type].base_type &&
						E_symbol_table[leftval.type].dim == E_symbol_table[rightval.type].dim)
					{
						output("PUSH #%zu\n", E_symbol_table[rightval.type].len);
						output("PUSH %c%d\n",
								"vt"[(rightval.lrtype / 2) & 1], rightval.id);
						output("PUSH %c%d\n",
								"vt"[(leftval.lrtype / 2) & 1], leftval.id);
						output("_ := CALL __memcpy");
						// 对于数组，写了一个类似于memcpy的函数
					} else {
						ce("type %d in line %d:", 7, _->_start_line);
						ce("type mismatch, left val is ``%s'' but right val is ``%s''",
							E_symbol_table[leftval.type].name,
							E_symbol_table[rightval.type].name);

					}
					break;
				} else if (leftval.type != rightval.type)
				{
					ce("type %d in line %d:", 7, _->_start_line);
					ce("type mismatch, left val is ``%s'' but right val is ``%s''",
							E_symbol_table[leftval.type].name,
							E_symbol_table[rightval.type].name);
				}
				if ((E_symbol_table[leftval.type].is_abstract & EJQ_SYMBOL_STRUCT) &&
					(E_symbol_table[rightval.type].is_abstract & EJQ_SYMBOL_STRUCT))
				{
					output("PUSH #%zu\n", E_symbol_table[rightval.type].len);
					output("PUSH %c%d\n",
							"vt"[(rightval.lrtype / 2) & 1], rightval.id);
					output("PUSH %c%d\n",
							"vt"[(leftval.lrtype / 2) & 1], leftval.id);
					output("_ := CALL __memcpy");
					break;
				}
				RetStringify(buf, &rightval);
				output("%s%d := %s\n",
					   	EJQ_LRTYPE(leftval), 
						leftval.id,
						buf);
				break;
			}
			case EJQ_OP_AND:
			{
				int falseLabel = ifFalse ? ifFalse : ++totLab;
				int trueLabel = ifTrue ? ifTrue : ++totLab;
				int B1True = ++totLab;
				RetType lval, rval;
				if (needReturn)
				{
					assert(ifTrue == 0 && ifFalse == 0);
					trueLabel = ++totLab;
					falseLabel = ++totLab;
					// 这里加个断言
				}
				lval = transcall(Exp, _->lExp, False, B1True, falseLabel);
				output("LABEL l%d:\n", B1True);
				rval = transcall(Exp, _->rExp, False, trueLabel, falseLabel);
				// 如果需要返回值，那么一定是在计算的时候，不可能是在if或者while里面
				// 这样的话，ifTrue和ifFalse就不会给出
				// 换而言之，trueLabel和falseLabel一定是局部的标签
				if (needReturn)
				{
					int afterLabel = ++totLab;
					output("LABEL l%d:\n", trueLabel);
					output("t%d := #1\n", ret.id);
					output("GOTO l%d\n", afterLabel);
					output("LABEL l%d:\n", falseLabel);
					output("t%d := #0\n", ret.id);
					output("LABEL l%d:\n", afterLabel);
				}
				ret.type = 1;
				if (ifTrue == 0) output("LABEL l%d:\n", trueLabel);
				if (ifFalse == 0) output("LABEL l%d:\n", falseLabel);
				// 布尔表达式的返回值一定是int
				return ret;
				break;
			}
			case EJQ_OP_OR:
			{
				int falseLabel = ifFalse ? ifFalse : ++totLab;
				int trueLabel = ifTrue ? ifTrue : ++totLab;
				int B1True = trueLabel;
				int B1False = ++totLab;
				RetType lval, rval;
				if (needReturn)
				{
					assert(ifTrue == 0 && ifFalse == 0);
					// 和上面一样的断言
				}
				lval = transcall(Exp, _->lExp, False, B1True, B1False);
				output("LABEL l%d:\n", B1False);
				rval = transcall(Exp, _->rExp, False, trueLabel, falseLabel);
				if (needReturn)
				{
					int afterLabel = ++totLab;
					output("LABEL l%d:\n", trueLabel);
					output("t%d := #1\n", ret.id);
					output("GOTO l%d\n", afterLabel);
					output("LABEL l%d:\n", falseLabel);
					output("t%d := #0\n", ret.id);
					output("LABEL l%d:\n", afterLabel);
				}
				if (ifTrue == 0) output("LABEL l%d:\n", trueLabel);
				if (ifFalse == 0) output("LABEL l%d:\n", falseLabel);
				ret.type = 1;
				return ret;
				break;
			}
			case EJQ_OP_RELOP_LT:
			case EJQ_OP_RELOP_LE:
			case EJQ_OP_RELOP_EQ:
			case EJQ_OP_RELOP_GE:
			case EJQ_OP_RELOP_GT:
			case EJQ_OP_RELOP_NE:
			{
				RetType lval, rval;
				int trueLabel = ifTrue ? ifTrue : ++totLab;
				int falseLabel = ifFalse ? ifFalse : ++totLab;
				char op[3];
				if (needReturn)
				{
					assert(ifTrue == 0 && ifFalse == 0);
					// 同上
				}
				switch (_->op)
				{
					case EJQ_OP_RELOP_LT:
						strcpy(op, "<");break;
					case EJQ_OP_RELOP_LE:
						strcpy(op, "<=");break;
					case EJQ_OP_RELOP_EQ:
						strcpy(op, "==");break;
					case EJQ_OP_RELOP_GE:
						strcpy(op, ">=");break;
					case EJQ_OP_RELOP_GT:
						strcpy(op, ">");break;
					case EJQ_OP_RELOP_NE:
						strcpy(op, "!=");break;
					default:
						assert(0);
						//夭寿啦，代码出错啦
				}
				lval = transcall(Exp, _->lExp, True, 0, 0);
				rval = transcall(Exp, _->rExp, True, 0, 0);
				char buf1[30];
				char buf2[30];
				RetStringify(buf1, &lval);
				RetStringify(buf2, &rval);
				output("IF %s %s %s GOTO l%d\n",
						buf1, op,
						buf2, trueLabel);
				if (needReturn)
				{
					output("t%d := #0", ret.id);
					output("GOTO l%d\n", falseLabel);
					output("LABEL l%d\n", trueLabel);
					output("t%d := #1", ret.id);
					output("LABEL l%d\n", falseLabel);
				} else {
					output("GOTO l%d\n", falseLabel);
				}
				ret.type = 1;
				return ret;
				break;
			}
			case EJQ_OP_PLUS:
			case EJQ_OP_MINUS:
			case EJQ_OP_STAR:
			case EJQ_OP_DIV:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				RetType rightval = transcall(Exp, _->rExp, True, 0, 0);
				char op[3];
				switch (_->op)
				{
					case EJQ_OP_PLUS:
						strcpy(op, "+");break;
					case EJQ_OP_MINUS:
						strcpy(op, "-");break;
					case EJQ_OP_STAR:
						strcpy(op, "*");break;
					case EJQ_OP_DIV:
						strcpy(op, "/");break;
					default:
						assert(0);
						// 夭寿啦，代码又出错啦
				}
				if (leftval.type != rightval.type)
				{
					ce("type %d in line %d:", 7, _->_start_line);
					ce("type mismatch, left val is ``%s'' but right val is ``%s''",
							E_symbol_table[leftval.type].name,
							E_symbol_table[rightval.type].name);
				}
				if (leftval.type > 2)
				{
					ce("type %d in line %d:", 7, _->_start_line);
					ce("operator %s on ``%s'' and ``%s'' is not defined", op, E_symbol_table[leftval.type].name, E_symbol_table[rightval.type].name);
				}
				ret.type = leftval.type;
				if (leftval.isImm8 == rightval.isImm8 && leftval.isImm8 == EJQ_IMM8_INT)
				{
					int value = 0;
					switch(_->op)
					{
						case EJQ_OP_PLUS:
							value = leftval.imm8val.i + rightval.imm8val.i;break;
						case EJQ_OP_MINUS:
							value = leftval.imm8val.i - rightval.imm8val.i;break;
						case EJQ_OP_STAR:
							value = leftval.imm8val.i * rightval.imm8val.i;break;
						case EJQ_OP_DIV:
							value = leftval.imm8val.i / rightval.imm8val.i;break;
					}
					if (ret.lrtype == 3)
					{
						ret.isImm8 = EJQ_IMM8_INT;
						ret.imm8val.i = value;
					} else {
						output("%s%d := #%d\n", EJQ_LRTYPE(ret), ret.id, value);
					}
				} else if (leftval.isImm8 == rightval.isImm8 && leftval.isImm8 == EJQ_IMM8_FLOAT)
				{
					float value = 0;
					switch(_->op)
					{
						case EJQ_OP_PLUS:
							value = leftval.imm8val.f + rightval.imm8val.f;break;
						case EJQ_OP_MINUS:
							value = leftval.imm8val.f - rightval.imm8val.f;break;
						case EJQ_OP_STAR:
							value = leftval.imm8val.f * rightval.imm8val.f;break;
						case EJQ_OP_DIV:
							value = leftval.imm8val.f / rightval.imm8val.f;break;
					}
					if (ret.lrtype == 3)
					{
						ret.isImm8 = EJQ_IMM8_FLOAT;
						ret.imm8val.i = value;
					} else {
						output("%s%d := #%.20f\n", EJQ_LRTYPE(ret), ret.id, value);
					}
				} else {
					char buf1[30];
					char buf2[30];
					RetStringify(buf1, &leftval);
					RetStringify(buf2, &rightval);
					output("%s%d := %s %s %s\n",
						EJQ_LRTYPE(ret),
						ret.id, buf1, op, buf2);
				}
				break;
			}
			case EJQ_OP_UNARY_MINUS:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				if (leftval.type > 2)
				{
					ce("type %d in line %d:", 7, _->_start_line);
					ce("operator ``-'' on ``%s'' is not defined", E_symbol_table[leftval.type].name);
				}
				RetStringify(buf, &leftval);
				output("t%d := #0 - %s\n", ret.id, buf);
				break;
			}
			case EJQ_OP_ARRAY:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				if (!(E_symbol_table[leftval.type].is_abstract & EJQ_SYMBOL_ARRAY))
				{
					ce("type %d in line %d:", 10, _->_start_line);
					ce("operator ``[]'' applied to non-array variable");
					return ret;
				}
				RetType rightval = transcall(Exp, _->rExp, True, 0, 0);
				if (rightval.type != 1)
				{
					ce("type %d in line %d:", 12, _->_start_line);
					ce("expression in ``[]'' returns non-integer value");
					return ret;
				}
				// 如果是数组的话，这里写的就是这一维的类型
				ret.type = E_symbol_table[leftval.type].type_uid;
				ret.lrtype = EJQ_RET_RPTR;
				if (rightval.isImm8)
				{
					int offset = rightval.imm8val.i * E_symbol_table[ret.type].len;
					output("t%d := %c%d + #%d\n", ret.id, "vt"[(leftval.lrtype >> 1) & 1], leftval.id, offset);
				} else {
					output("_offset := %s%d * #%zu\n", EJQ_LRTYPE(rightval), rightval.id, E_symbol_table[ret.type].len);
					output("t%d := %c%d + _offset\n", ret.id, "vt"[(leftval.lrtype >> 1) & 1], leftval.id);
				}
				break;
			}
			case EJQ_OP_STRUCT:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				if (!(E_symbol_table[leftval.type].is_abstract & EJQ_SYMBOL_STRUCT))
				{
					ce("type %d in line %d:", 13, _->_start_line);
					ce("operator ``.'' applied to non-struct variable");
					return ret;
				}
				char buf[1024];
				sprintf(buf, "%s.%s", E_symbol_table[leftval.type].name, _->funcName);
				debug("real access to :%s\n", buf);
				int target_val = E_trie_find(buf);
				if (!target_val)
				{
					ce("type %d in line %d:", 14, _->_start_line);
					ce("unknown field ``%s'' in struct ``%s''", _->funcName, E_symbol_table[leftval.type].name);
					return ret;
				}
				ret.lrtype = EJQ_RET_RPTR;
				ret.type = E_symbol_table[target_val].type_uid;
				output("t%d := %c%d + #%zu\n", ret.id, "vt"[(leftval.lrtype >> 1) & 1], leftval.id, E_symbol_table[target_val].offset);
				break;
			}
			case EJQ_OP_UNARY_NOT:
			{
				RetType leftval = transcall(Exp, _->lExp, needReturn, ifFalse, ifTrue);
				if (needReturn)
				{
					int trueLabel = ++totLab;
					int falseLabel = ++totLab;
					RetStringify(buf, &leftval);
					output("IF %s == #0 GOTO l%d\n",
						buf,
						trueLabel);
					output("t%d := #0", ret.id);
					output("GOTO l%d\n", falseLabel);
					output("LABEL l%d\n", trueLabel);
					output("t%d := #1", ret.id);
					output("LABEL l%d\n", falseLabel);
				}
				return ret;
			}
			default:
			{
				error("未实现\n");
				assert(0);
			}
		}
	}
	RetStringify(buf, &ret);
	if (ifTrue) output("IF %s != #0 GOTO l%d\n", buf, ifTrue);
	if (ifFalse)
	{
		// upd 如果有真，那么这句一定是假，不用想，直接跳
		if (ifTrue)
			output("GOTO l%d\n", ifFalse);
		else
			output("IF %s == #0 GOTO l%d\n", buf, ifFalse);
	}
	return ret;
}

transdecl(Args)
{
	error("This function (%s) should never be called\n", __func__);
	assert(0);
}

void Translate(N(Program) *P)
{
	transcall(Program, P);
}

#undef trans
#undef transdecl
#undef translate
#undef N

#endif
