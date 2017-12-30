#ifndef _E_PROJECT_2_2_H
#define _E_PROJECT_2_2_H

#define EJQ_OP_ASSIGN   (1)
#define EJQ_OP_AND      (2)
#define EJQ_OP_OR       (3)
#define EJQ_OP_RELOP_LT (4)
#define EJQ_OP_RELOP_LE (5)
#define EJQ_OP_RELOP_EQ (6)
#define EJQ_OP_RELOP_GE (7)
#define EJQ_OP_RELOP_GT (8)
#define EJQ_OP_RELOP_NE (9)
#define EJQ_OP_PLUS     (10)
#define EJQ_OP_MINUS    (11)
#define EJQ_OP_STAR     (12)
#define EJQ_OP_DIV      (13)

#define EJQ_OP_UNARY_MINUS    (-1)
#define EJQ_OP_UNARY_NOT      (-2)

#define EJQ_OP_ARRAY   (-3)
#define EJQ_OP_STRUCT  (-4)

/////// imm8 val

#define EJQ_IMM8_INT   (1)
#define EJQ_IMM8_FLOAT (2)

#define EJQ_RET_LVAL (1)
#define EJQ_RET_RVAL (3)
#define EJQ_RET_LPTR (5)
#define EJQ_RET_RPTR (7)

const char* EJQ_V_OUTPUT_TYPE[] = {
	"", "v", "", "t", "", "*v", "", "*t"
};

#define EJQ_LRTYPE( x ) (EJQ_V_OUTPUT_TYPE[ x.lrtype ])
#define EJQ_F_LR "%s%d"
#define EJQ_ARG_LR( x ) EJQ_LRTYPE(x),(x).id

#define EJQ_RET_PTR (4)
#define EJQ_RET_L (1)
#define EJQ_RET_R (3)

#endif
