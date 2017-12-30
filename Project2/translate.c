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
transdecl(Specifier);
transdecl(StructSpecifier);
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
			"RETURN #1\n\n",
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

transdecl(ExtDef)
{
	if (_->function) // function
	{
		int son_cnt = 0;
		foreach ( _->function->varList , vlist , N(VarList) ) son_cnt++;
		// 计算函数参数个数
		size_t func_type = E_trie_find(_->spec->typeName);
		// 函数被定义了
		if (E_trie_find(_->function->name))
		{
			ce("duplicate identify ``%s'' in current namespace\n", _->function->name);
			return;
		}
		// TODO: 处理包含结构体定义的情况
		if (!func_type)
		{
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
				ce("unknown type %s\n", vlist->thisParam->type->typeName);
				return;
			}
			ti++;
			// 获取参数
			output("PARAM v%d\n", func_arg_symbol);
		}
		// 获取参数表
		// 假设参数类型全部为int
		// 翻译函数体
		transcall(CompSt, _->functionBody);
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
		size_t vari_type = E_trie_find(_->spec->typeName);
		if (!vari_type)
		{
			ce("unknown type ``%s''", _->spec->typeName);
			return;
		}
		// TODO: 处理包含结构体定义的情况
		// 获取变量类型号
		foreach ( _->dec, decList, N(ExtDecList) )
		{
			// 单独处理每个变量
			if (E_trie_find(decList->dec->varName))
			{
				// 可持久化Trie树
				// 这是全局变量，所以直接查询当前的版本有没有定义就好了
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
			if (isArray)
				E_global_variable_list_append(id);
		}
	}
}

transdecl(Def)
{
	foreach ( _->decList, decList, N(DecList))
	{
		size_t vari_type = E_trie_find(_->type->typeName);
		if (E_trie_find(decList->dec->var->varName))
		{
			ce("redeclare of ``%s''", decList->dec->var->varName);
			continue;
		}
		size_t id = transcall(VarDec, decList->dec->var, vari_type);
		// 如果是数组的话，要申请内存
		int new_type = E_symbol_table[id].type_uid;
		int isArray = (E_symbol_table[new_type].is_abstract == EJQ_SYMBOL_ARRAY);
		if (isArray)
			output("DEC v%zu [%zu]\n", id, E_symbol_table[id].len);

		//E_trie_insert(decList->dec->var->varName, id);
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
	E_trie_insert(_->varName, thisid);
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
		sprintf(buf, "%s[]", E_symbol_table[thisdim_id].name);
	} else {
		sprintf(buf, "%s[]", E_symbol_table[spec].name);
	}
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
}

transdecl(Stmt)
{
	if (_->isReturn)
	{
		// 有返回值，要把返回值return掉
		// TODO: 类型检查
		RetType returnTmp = transcall(Exp, _->expression, True, 0, 0);
		// 需要返回值，没有跳转
		output("RETURN t%d\n", returnTmp.id);
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
			false_branch = ++totLab;
		}
		// 注意到如果没有假分支，那么假就跳到if结束
		RetType r = transcall(Exp, _->expression, False, true_branch, false_branch);
		if (r.type != 1)
		{
			ce("type of conditional expression is not ``int''");
		}
		// 肯定有真分支
		output("LABEL l%d:\n", true_branch);
		transcall(Stmt, _->ifTrue);
		if (_->ifFalse)
		{
			// 处理假分支
			// 这时候真分支刚刚执行完，需要跳走，避免执行假分支的内容
			output("GOTO l%d\n", after_if);
			// 然后打出假分支的标号
			output("LABEL l%d:\n", false_branch);
			transcall(Stmt, _->ifFalse);
		}
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
	RetType ret;
	int res = ++totTmp;
	ret.lrtype = EJQ_RET_RVAL;
	ret.id = res;
	ret.type = 1;
	ret.isImm8 = 0;
	if (_->isImm8 == EJQ_IMM8_INT)
	{
		output("t%d := #%d\n", res, _->intVal);
		ret.type = 1; // int
		return ret;
	} else if (_->isImm8 == EJQ_IMM8_FLOAT)
	{
		output("t%d := #%.20f\n", res, _->floatVal);
		ret.type = 2; // float
		return ret;
	} else if (_->isFunc)
	{
		// 检查函数是否存在
		int func_type = E_trie_find(_->funcName);
		int func_args = E_symbol_table[func_type].son_cnt;
		int func_rettype = E_symbol_table[func_type].type_uid;
		if (!func_type || E_symbol_table[func_type].is_abstract != EJQ_SYMBOL_FUNCTION)
		{
			ce("function ``%s'' not found", _->funcName);
			ret.type = 1;
			return ret;
		}
		debug("translate function call ``%s'' --> return with [%s](%d)\n", _->funcName, E_symbol_table[E_symbol_table[func_type].type_uid].name, E_symbol_table[func_type].type_uid);
		// 这是一个函数调用
		int needArgs = func_args;
		int realArgs = 0;
		foreach(_->args, arg, N(Args)) realArgs++;
		if (realArgs < needArgs)
			ce("call function ``%s'' with %d arguments, %d needed", _->funcName, realArgs, needArgs);
		else if (realArgs > needArgs)
			ce("call function ``%s'' with %d arguments, requires exactly %d", _->funcName, realArgs, needArgs);
		if (realArgs != needArgs)
			warn("behavior is unknown");
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
					ce("type of argument %d mismatch: requires ``%s'', got ``%s''", needArgs + 1, E_symbol_table[son_node_type].name, E_symbol_table[t.type].name);
				}
			}
			output("PUSH t%d\n", t.id);
		}
		// 先这样，后面再改成找符号表
		output("t%d := CALL f%d\n", res, func_type);
		ret.type = func_rettype;
		return ret;
	} else if (_->funcName)
	{
		// 这两个名字是一起的
		// 不是函数名就是变量名
		debug("find ``%s'' in trie\n", _->funcName);
		int varName = E_trie_find(_->funcName);
		if (!varName)
		{
			ce("undefined variable ``%s''", _->funcName);
			return ret;
		}
		if (E_symbol_table[varName].is_abstract != EJQ_SYMBOL_NORMAL)
		{
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
		return ret;
	} else {
		// 就是普通的表达式啦
		switch (_->op)
		{
			case EJQ_OP_ASSIGN:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				RetType rightval = transcall(Exp, _->rExp, True, 0, 0);
				if (leftval.type != rightval.type)
				{
					ce("type mismatch, left val is ``%s'' but right val is ``%s''",
							E_symbol_table[leftval.type].name,
							E_symbol_table[rightval.type].name);
				}
				if ((leftval.lrtype != EJQ_RET_LVAL) && !(leftval.lrtype & EJQ_RET_PTR))
				{
					ce("result of left hand expression should be left value");
				}
				output(
						"%s%d := %s%d\n",
					   	EJQ_LRTYPE(leftval), 
						leftval.id,
						EJQ_LRTYPE(rightval),
						rightval.id);
				break;
			}
			case EJQ_OP_AND:
			{
				int falseLabel = ifFalse ? ifFalse : ++totLab;
				int trueLabel = ifTrue ? ifTrue : ++totLab;
				int B1True = ++totLab;
				int B1False = falseLabel;
				RetType lval, rval;
				if (needReturn)
				{
					assert(ifTrue == 0 && ifFalse == 0);
					// 这里加个断言
				}
				lval = transcall(Exp, _->lExp, False, B1True, falseLabel);
				output("LABEL l%d\n", B1True);
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
				output("IF %s%d %s %s%d GOTO l%d\n",
						EJQ_LRTYPE(lval),
						lval.id, op,
						EJQ_LRTYPE(rval),
						rval.id, trueLabel);
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
					ce("type mismatch, left val is ``%s'' but right val is ``%s''",
							E_symbol_table[leftval.type].name,
							E_symbol_table[rightval.type].name);
				}
				if (leftval.type > 2)
				{
					ce("operator %s on ``%s'' and ``%s'' is not defined", op, E_symbol_table[leftval.type].name, E_symbol_table[rightval.type].name);
				}
				output(
						"%s%d := %s%d %s %s%d\n",
						EJQ_LRTYPE(ret),
						ret.id,
						EJQ_LRTYPE(leftval),
						leftval.id,
						op,
						EJQ_LRTYPE(rightval),
						rightval.id);
				ret.type = leftval.type;
				break;
			}
			case EJQ_OP_UNARY_MINUS:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				if (leftval.type > 2)
				{
					ce("operator ``-'' on ``%s'' is not defined", E_symbol_table[leftval.type].name);
				}
				output("t%d := #0 - %s%d\n", ret.id, 
						EJQ_LRTYPE(leftval),
						leftval.id
					  );
				break;
			}
			case EJQ_OP_ARRAY:
			{
				RetType leftval = transcall(Exp, _->lExp, True, 0, 0);
				if (!(E_symbol_table[leftval.type].is_abstract & EJQ_SYMBOL_ARRAY))
				{
					ce("operator ``[]'' applied to non-array variable");
					return ret;
				}
				RetType rightval = transcall(Exp, _->rExp, True, 0, 0);
				if (rightval.type != 1)
				{
					ce("expression in ``[]'' returns non-integer value");
					return ret;
				}
				// 如果是数组的话，这里写的就是这一维的类型
				ret.type = E_symbol_table[leftval.type].type_uid;
				ret.lrtype = EJQ_RET_RPTR;
				output("_offset := %s%d * #%zu\n", EJQ_LRTYPE(rightval), rightval.id, E_symbol_table[ret.type].len);
				output("t%d := %c%d + _offset\n", ret.id, "vt"[(leftval.lrtype >> 1) & 1], leftval.id);
				break;
			}
			default:
			{
				error("未实现\n");
				assert(0);
			}
		}
	}
	if (ifTrue) output("IF %s%d != #1 GOTO l%d\n",
						EJQ_LRTYPE(ret),
						ret.id, ifTrue);
	if (ifFalse) output("IF %s%d == #0 GOTO l%d\n",
						EJQ_LRTYPE(ret),
						ret.id, ifFalse);
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
