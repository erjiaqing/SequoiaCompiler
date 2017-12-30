#ifndef TRANSLATE_C
#define TRANSLATE_C

#include <assert.h>

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
transdecl_sizet(Exp, int, int, int); // need return, truelabel falselabel
transdecl(Args);

int totTmp;
int totLab;

///////////////////////

transdecl(Program)
{
	output("FUNCTION read:\n"
			"READ _tread\n"
			"RETURN _tread\n\n"
		   "FUNCTION write:\n"
		    "PARAM _twrite\n"
			"WRITE _twrite\n"
			"RETURN #1\n\n");
	foreach( _->programBody, extdef, N(ExtDefList))
		transcall(ExtDef, extdef->code );
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
		// 打印函数首部
		output("FUNCTION %s:\n", _->function->name);
		foreach ( _->function->varList, vlist, N(VarList) )
		{
			int func_arg_symbol = E_symbol_table_new();
			E_symbol_table[func_symbol_item].son[ti] = func_arg_symbol;
			E_symbol_table[func_arg_symbol].type_uid = E_trie_find(vlist->thisParam->type->typeName);
			E_trie_insert(vlist->thisParam->name->varName, func_arg_symbol);
			if (!E_symbol_table[func_arg_symbol].type_uid)
			{
				ce("unknown type %s\n", vlist->thisParam->type->typeName);
				return;
			}
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
			ce("unknown type %s\n", _->spec->typeName);
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
				ce("redeclare of %s\n", decList->dec->varName);
				continue;
			}
			// 计算这个变量
			size_t id = transcall(VarDec, decList->dec, vari_type);
			// 加入到trie树中
			E_trie_insert(decList->dec->varName, id);
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
			ce("redeclare of %s\n", decList->dec->var->varName);
			continue;
		}
		size_t id = transcall(VarDec, decList->dec->var, vari_type);
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
	E_symbol_table[thisid].name = _->varName;
	E_symbol_table[thisid].len = E_symbol_table[typeid].len;
	E_symbol_table[thisid].is_abstract = 0x002;
	E_symbol_table[thisid].offset = 0;
	E_symbol_table[thisid].son_cnt = 0;
	E_symbol_table[thisid].son = NULL;
	return thisid;
}

transdecl_sizet(VarDimList, size_t spec)
{
	size_t thisdim_id = spec;
	size_t thisid = E_symbol_table_new();
	char buf[512];
	if (_->next)
	{
		thisdim_id = transcall(VarDimList, _->next, spec);
		sprintf(buf, "%s/a", E_symbol_table[thisdim_id].name);
	} else {
		sprintf(buf, "%d/a", (int)spec);
	}
	E_symbol_table[thisid].type_uid = thisdim_id;
	E_symbol_table[thisid].name = (char *)malloc(strlen(buf) + 5);
	for (size_t i = 0; buf[i]; i++)
		E_symbol_table[thisid].name[i] = buf[i];
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
		int returnTmp = transcall(Exp, _->expression, True, 0, 0);
		// 需要返回值，没有跳转
		output("RETURN t%d\n", returnTmp);
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
		transcall(Exp, _->expression, False, true_branch, false_branch);
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
			output("PUSH t%zu\n", transcall(Exp, arg->exp, True, 0, 0));
		}
		// 先这样，后面再改成找符号表
		output("t%d := CALL %s\n", res, _->funcName);
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
			ce("undefined variable ``%s''\n", _->funcName);
			return ret;
		}
		ret.type = E_symbol_table[varName].type_uid;
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
				RetType leftval = transcall(_->lExp);
				RetType rightval = transcall(_->rExp);
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
						"%s%c%d := %s%c%zu\n",
					   	(leftval.lrtype & EJQ_RET_PTR) ? "*" : "",
					   	"vc"[(leftval.lrtype >> 1) & 1], 
						leftval.id,
						(rightval.lrtype & EJQ_RET_PTR) ? "*" : "",
						"vc"[(rightval.lrtype >> 1) & 1],
						rightval.id);
				break;
			}
			case EJQ_OP_AND:
			{
				int falseLabel = ifFalse ? ifFalse : ++totLab;
				int trueLabel = ifTrue ? ifTrue : ++totLab;
				int B1True = ++totLab;
				int B1False = falseLabel;
				int lval, rval;
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
					output("t%d := #1\n", res);
					output("GOTO l%d\n", afterLabel);
					output("LABEL l%d:\n", falseLabel);
					output("t_%d := #0\n", res);
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
				int lval, rval;
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
					output("t%d := #1\n", res);
					output("GOTO l%d\n", afterLabel);
					output("LABEL l%d:\n", falseLabel);
					output("t%d := #0\n", res);
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
				int lval, rval;
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
				output("IF t%d %s t%d GOTO l%d\n", lval, op, rval, trueLabel);
				if (needReturn)
				{
					output("t%d := #0", res);
					output("GOTO l%d\n", falseLabel);
					output("LABEL l%d\n", trueLabel);
					output("t_%06d := #1", res);
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
				RetType lval = transcall(Exp, _->lExp, True, 0, 0);
				RetType rval = transcall(Exp, _->rExp, True, 0, 0);
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
						"v%d := %s%c%d %s %s%c%d\n",
						res,
					   	(leftval.lrtype & EJQ_RET_PTR) ? "*" : "",
					   	"vc"[(leftval.lrtype >> 1) & 1], 
						leftval.id,
						op,
						(rightval.lrtype & EJQ_RET_PTR) ? "*" : "",
						"vc"[(rightval.lrtype >> 1) & 1],
						rightval.id);
				ret.type = leftval.type;
				break;
			}
			case EJQ_OP_UNARY_MINUS:
			{
				int lval = transcall(Exp, _->lExp, True, 0, 0);
				if (leftval.type > 2)
				{
					ce("operator %s on ``%s'' is not defined", op, E_symbol_table[leftval.type].name);
				}
				output("t%d := #0 - %s%c%d\n", res, 
						(leftval.lrtype & EJQ_RET_PTR) ? "*" : "",
					   	"vc"[(leftval.lrtype >> 1) & 1], 
						leftval.id);
				break;
			}
			default:
			{
				error("未实现\n");
				assert(0);
			}
		}
	}
	if (ifTrue) output("IF %s%c%d != #1 GOTO l%d\n",
						(ret.lrtype & EJQ_RET_PTR) ? "*" : "",
					   	"vc"[(ret.lrtype >> 1) & 1], 
						ret.id, ifTrue);
	if (ifFalse) output("IF %s%c%d == #0 GOTO l%d\n",
						(rer.lrtype & EJQ_RET_PTR) ? "*" : "",
					   	"vc"[(ret.lrtype >> 1) & 1], 
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
