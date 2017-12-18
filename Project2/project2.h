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
#define pCast( x, y ) ((x *) y)
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
	N(ExtDefList) *programBody;
};
initGetNodeFunc(Program);

struct _(ExtDefList) {
	N(ExtDef) *code;
	N(ExtDefList) *next;
};
initGetNodeFunc(ExtDefList);

struct _(ExtDef) {
	N(Specifier) *spec;
	N(ExtDecList) *dec;
	N(FunDec) *function;
	N(CompSt) *functionBody;
};
initGetNodeFunc(ExtDef);

struct _(ExtDecList) {
	N(VarDec) *dec;
	N(ExtDecList) *next;
};
initGetNodeFunc(ExtDecList);

struct _(Specifier) {
	char *typeName;
	N(StructSpecifier) *structName;
};
initGetNodeFunc(Specifier);

struct _(StructSpecifier) {
	char *typeName;
	N(DefList) *structBody;
};
initGetNodeFunc(StructSpecifier);

struct _(VarDec) {
	char *varName;
	N(VarDimList) *dim;
};
initGetNodeFunc(VarDec);

struct _(VarDimList) {
	int *thisDim;
	N(VarDimList) *next;
};
initGetNodeFunc(VarDimList);

struct _(FunDec) {
	char *name;
	N(VarList) *varList;
};
initGetNodeFunc(FunDec);

struct _(VarList) {
	N(ParamDec) *thisParam;
	N(VarList) *next;
};
initGetNodeFunc(VarList);

struct _(ParamDec) {
	N(Specifier) *type;
	N(VarDec) *name;
};
initGetNodeFunc(ParamDec);

struct _(CompSt) {
	N(DefList) *defList;
	N(StmtList) *stmtList;
};
initGetNodeFunc(CompSt);

struct _(StmtList) {
	N(Stmt) *statement;
	N(StmtList) *next;
};
initGetNodeFunc(StmtList);

struct _(Stmt) {
	N(Exp) *expression;
	N(CompSt) *compStatement;
	int isReturn;
	int isWhile;
	N(Stmt) *ifTrue;
	N(Stmt) *ifFalse;
};
initGetNodeFunc(Stmt);

struct _(DefList) {
	N(Def) *def;
	N(DefList) *next;
};
initGetNodeFunc(DefList);

struct _(Def) {
	N(Specifier) *type;
	N(DecList) *decList;
};
initGetNodeFunc(Def);

struct _(DecList) {
	N(Dec) *dec;
	N(DecList) *next;
};
initGetNodeFunc(DecList);

struct _(Dec) {
	N(VarDec) *var;
	N(Exp) *value;
};
initGetNodeFunc(Dec);

struct _(Exp) {
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
	N(Exp) *exp;
	N(Args) *next;
};
initGetNodeFunc(Args);

#undef _
#undef N
#endif
