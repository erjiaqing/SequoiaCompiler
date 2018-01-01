#ifndef _E_PROJECT_2_H
#define _E_PROJECT_2_H

#include <stdlib.h>
#include <string.h>

#define declare_struct( x ) \
	struct ECS##x ; \
	typedef struct ECS##x T_##x; \
	T_##x *getNew##x##Node()
#define _( x ) ECS##x
#define N( x ) T_##x
#define getNewNode( x ) getNew##x##Node()
#define dollarNode( x ) T_##x *ret_kjfwhbjkwbm = getNewNode( x )
#define _r ret_kjfwhbjkwbm 
#define initGetNodeFunc( x ) \
	T_##x *getNew##x##Node() {\
		T_##x *ret = (T_##x *) malloc(sizeof( T_##x ) );\
		memset(ret, 0, sizeof( T_##x ) );\
		return ret;\
	}
#define calcPosition( x , y )\
	{\
		if (y) {\
		if (x->_start_line == 0) {\
			x->_start_line = y->_start_line, \
			x->_start_char = y->_start_char, \
			x->_end_line = y->_end_line,\
			x->_end_char = y->_end_char;\
		} else {\
			if (x->_start_line > y->_start_line) { \
				x->_start_line = y->_start_line;\
				x->_start_char = y->_start_char;\
			} else if (x->_start_line == y->_start_line && x->_start_char > y->_start_char) {\
				x->_start_char = y->_start_char;\
			}\
			if (x->_end_line < y->_end_line) { \
				x->_end_line = y->_end_line;\
				x->_end_char = y->_end_char;\
			} else if (x->_end_line == y->_end_line && x->_end_char < y->_end_char) {\
				x->_end_char = y->_end_char;\
			}\
		}\
		}\
	}
#define pCast( x, y ) ((x *) y)

#define Pj2Type( x ) T_##x

#define POSITION int _start_line, _start_char, _end_line, _end_char

#define foreach( x , y , type) for (type * y = x; y; y = y->next) 
#define False (0)
#define True (1)
typedef int Bool;

typedef struct __ret_type{
	int lrtype; // 左值还是右值
	int isImm8; // 是不是立即数
	union {
		int i;
		float f;
	} imm8val; // 备用，立即数的值
	int id; // 对应的变量id (v###)，或者是 (t###)
	int type; // 变量类型，其实主要是针对临时变量，它们不在符号表里面，所以得这样查类型
} RetType;

void RetStringify(char *buf, const RetType *r)
{
	if (r->isImm8 == EJQ_IMM8_INT)
		sprintf(buf, "#%d", r->imm8val.i);
	else if (r->isImm8 == EJQ_IMM8_FLOAT)
		sprintf(buf, "#%.20f", r->imm8val.f);
	else
		sprintf(buf, "%s%d", EJQ_LRTYPE((*r)), r->id);
}

// Extend C grammar
// make it like C++

// 和Project2相关的声明，包括细化节点类型

declare_struct(Program);
declare_struct(ExtDefList);
declare_struct(ExtDef);
declare_struct(ExtDecList);
declare_struct(Specifier);
declare_struct(StructSpecifier);
declare_struct(OptTag);
declare_struct(Tag);
declare_struct(VarDec);
declare_struct(VarDimList);
declare_struct(FunDec);
declare_struct(VarList);
declare_struct(ParamDec);
declare_struct(CompSt);
declare_struct(StmtList);
declare_struct(Stmt);
declare_struct(DefList);
declare_struct(Def);
declare_struct(DecList);
declare_struct(Dec);
declare_struct(Exp);
declare_struct(Args);

struct _(Program){
	POSITION;
	N(ExtDefList) *programBody;
};
initGetNodeFunc(Program);

struct _(ExtDefList) {
	POSITION;
	N(ExtDef) *code;
	N(ExtDefList) *next;
};
initGetNodeFunc(ExtDefList);

struct _(ExtDef) {
	POSITION;
	N(Specifier) *spec;
	N(ExtDecList) *dec;
	N(FunDec) *function;
	N(CompSt) *functionBody;
};
initGetNodeFunc(ExtDef);

struct _(ExtDecList) {
	POSITION;
	N(VarDec) *dec;
	N(ExtDecList) *next;
};
initGetNodeFunc(ExtDecList);

struct _(Specifier) {
	POSITION;
	char *typeName;
	N(StructSpecifier) *structName;
};
initGetNodeFunc(Specifier);

struct _(StructSpecifier) {
	POSITION;
	char *typeName;
	N(DefList) *structBody;
};
initGetNodeFunc(StructSpecifier);

struct _(VarDec) {
	POSITION;
	char *varName;
	N(VarDimList) *dim;
};
initGetNodeFunc(VarDec);

struct _(VarDimList) {
	POSITION;
	int thisDim;
	N(VarDimList) *next;
};
initGetNodeFunc(VarDimList);

struct _(FunDec) {
	POSITION;
	char *name;
	N(VarList) *varList;
};
initGetNodeFunc(FunDec);

struct _(VarList) {
	POSITION;
	N(ParamDec) *thisParam;
	N(VarList) *next;
};
initGetNodeFunc(VarList);

struct _(ParamDec) {
	POSITION;
	N(Specifier) *type;
	N(VarDec) *name;
};
initGetNodeFunc(ParamDec);

struct _(CompSt) {
	POSITION;
	N(DefList) *defList;
	N(StmtList) *stmtList;
};
initGetNodeFunc(CompSt);

struct _(StmtList) {
	POSITION;
	N(Stmt) *statement;
	N(StmtList) *next;
};
initGetNodeFunc(StmtList);

struct _(Stmt) {
	POSITION;
	N(Exp) *expression;
	N(CompSt) *compStatement;
	int isReturn;
	int isWhile;
	N(Stmt) *ifTrue;
	N(Stmt) *ifFalse;
};
initGetNodeFunc(Stmt);

struct _(DefList) {
	POSITION;
	N(Def) *def;
	N(DefList) *next;
};
initGetNodeFunc(DefList);

struct _(Def) {
	POSITION;
	N(Specifier) *type;
	N(DecList) *decList;
};
initGetNodeFunc(Def);

struct _(DecList) {
	POSITION;
	N(Dec) *dec;
	N(DecList) *next;
};
initGetNodeFunc(DecList);

struct _(Dec) {
	POSITION;
	N(VarDec) *var;
	N(Exp) *value;
};
initGetNodeFunc(Dec);

struct _(Exp) {
	POSITION;
	N(Exp) *lExp;
	N(Exp) *rExp;
	int op;
	int isImm8;
	int intVal;
	float floatVal;
	//
	int isFunc;
	char *funcName;
	N(Args) *args;
	//
};
initGetNodeFunc(Exp);

struct _(Args) {
	POSITION;
	N(Exp) *exp;
	N(Args) *next;
};
initGetNodeFunc(Args);

#undef _
#undef N
#endif
