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
	foreach( _->programBody, extdef, N(ExtDefList))
		transcall(ExtDef, extdef->code );
}

transdecl(ExtDef)
{
	if (_->function) // function
	{
		fprintf(stderr, "function\n");
		int son_cnt = 0;
		foreach ( _->function->varList , vlist , N(VarList) ) son_cnt++;
		// 计算函数参数个数
		size_t func_type = E_trie_find(_->spec->typeName);
		// 函数被定义了
		if (E_trie_find(_->function->name))
		{
			fprintf(stderr, "Redeclare of function %s\n", _->function->name);
			return;
		}
		// TODO: 处理包含结构体定义的情况
		if (!func_type)
		{
			fprintf(stderr, "Unknown type %s (got func_type = %zu)\n", _->spec->typeName, func_type);
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
		foreach ( _->function->varList, vlist, N(VarList) )
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
		printf("FUNCTION %s\n", _->function->name);
		// 翻译函数体
		transcall(CompSt, _->functionBody);
		// 恢复trie树版本
		E_trie_back_to_version(current_version);
	} else // variables
	{
		size_t vari_type = E_trie_find(_->spec->typeName);
		if (!vari_type)
		{
			fprintf(stderr, "Unknown type %s (got vari_type = %zu)\n", _->spec->typeName, vari_type);
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
				fprintf(stderr, "Redeclare of %s\n", decList->dec->varName);
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
			fprintf(stderr, "Redeclare of %s\n", decList->dec->var->varName);
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
		printf("RETURN t_%06d\n", returnTmp);
		// 这是第一句输出的话啊!
	} else if (_->isWhile)
	{
		// while循环
		int while_begin = ++totLab;
		int while_end = ++totLab;
		printf("LABEL l_%06d:\n", while_begin);
		transcall(Exp, _->expression, False, 0, while_end);
		transcall(Stmt, _->ifTrue);
		printf("GOTO l_%06d\n", while_begin);
		printf("LABEL l_%06d:\n", while_end);
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
		printf("LABEL l_%06d:\n", true_branch);
		transcall(Stmt, _->ifTrue);
		if (_->ifFalse)
		{
			// 处理假分支
			// 这时候真分支刚刚执行完，需要跳走，避免执行假分支的内容
			printf("GOTO l_%06d\n", after_if);
			// 然后打出假分支的标号
			printf("LABEL l_%06d:\n", false_branch);
			transcall(Stmt, _->ifFalse);
		}
		// 最后打出if结束的标号
		printf("LABEL l_%06d:\n", after_if);
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

transdecl_sizet(Exp, int needReturn, int ifTrue, int ifFalse)
{
	int res = ++totTmp;
	if (_->isImm8 == EJQ_IMM8_INT)
	{
		printf("t_%06d := #%d\n", res, _->intVal);
		return res;
	} else if (_->isImm8 == EJQ_IMM8_FLOAT)
	{
		printf("t_%06d := #%.20f\n", res, _->floatVal);
		return res;
	} else if (_->isFunc)
	{
		// 这是一个函数调用
		foreach(_->args, arg, N(Args))
			printf("PUSH t_%06zu\n", transcall(Exp, arg->exp, True, 0, 0));
		// 先这样，后面再改成找符号表
		printf("t_%06d := CALL %s\n", res, _->funcName);
		return res;
	} else if (_->funcName)
	{
		// 这两个名字是一起的
		// 不是函数名就是变量名
		printf("t_%06d := %s\n", res, _->funcName);
		return res;
	} else {
		// 就是普通的表达式啦
		switch (_->op)
		{
			case EJQ_OP_ASSIGN:
				printf("%s := t_%06zu\n", _->lExp->funcName, transcall(Exp, _->rExp, True, 0, 0));
				break;
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
				printf("LABEL l_%06d\n", B1True);
				rval = transcall(Exp, _->rExp, False, trueLabel, falseLabel);
				// 如果需要返回值，那么一定是在计算的时候，不可能是在if或者while里面
				// 这样的话，ifTrue和ifFalse就不会给出
				// 换而言之，trueLabel和falseLabel一定是局部的标签
				if (needReturn)
				{
					int afterLabel = ++totLab;
					printf("LABEL l_%06d\n", trueLabel);
					printf("t_%06d := #1\n", res);
					printf("GOTO l_%06d\n", afterLabel);
					printf("LABEL l_%06d\n", falseLabel);
					printf("t_%06d := #0\n", res);
				}
				return res;
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
				printf("LABEL l_%06d\n", B1False);
				rval = transcall(Exp, _->rExp, False, trueLabel, falseLabel);
				if (needReturn)
				{
					int afterLabel = ++totLab;
					printf("LABEL l_%06d\n", trueLabel);
					printf("t_%06d := #1\n", res);
					printf("GOTO l_%06d\n", afterLabel);
					printf("LABEL l_%06d\n", falseLabel);
					printf("t_%06d := #0\n", res);
				}
				return res;
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
				printf("IF t_%06d %s t_%06d GOTO l_%06d\n", lval, op, rval, trueLabel);
				if (needReturn)
				{
					printf("t_%06d := #0", res);
					printf("GOTO l_%06d\n", falseLabel);
					printf("LABEL l_%06d\n", trueLabel);
					printf("t_%06d := #1", res);
					printf("LABEL l_%0d\n", falseLabel);
				} else {
					printf("GOTO l_%06d\n", falseLabel);
				}
				return res;
				break;
			}
			case EJQ_OP_PLUS:
			case EJQ_OP_MINUS:
			case EJQ_OP_STAR:
			case EJQ_OP_DIV:
			{
				int lval = transcall(Exp, _->lExp, True, 0, 0);
				int rval = transcall(Exp, _->rExp, True, 0, 0);
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
				printf("t_%06d := t_%06d %s t_%06d\n", res, lval, op, rval);
				break;
			}
			default:
			{
				fprintf(stderr, "未实现\n");
				assert(0);
			}
		}
	}
	if (ifTrue) printf("IF t_%06d != #1 GOTO l_%06d\n", res, ifTrue);
	if (ifFalse) printf("IF t_%06d == #0 GOTO l_%06d\n", res, ifFalse);
	return res;
}

transdecl(Args)
{
	fprintf(stderr, "未实现\n");
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
