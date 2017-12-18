#ifndef _E_PROJECT_1_H
#define _E_PROJECT_1_H

// 这里放Project1相关的代码，因为发现Project2引入的东西可能会很大

// error type B
void yyerror(const char *msg)
{
	legal = 0;
	fprintf(stderr, "error type B on line %d: %s\n", lineno, msg);
}

// error type A
void llerror(const char *msg)
{
	legal = 0;
	fprintf(stderr, "error type A on line %d: %s\n", lineno, msg);
}

void move_to_next_line()
{
	lineno++;charno = 1;
	fgets(line_buffer, 65535, line_reader);
}

void _raise_line_error(int from, int to, int level, char *note)
{
#ifdef _DEBUG
// 输出一些好看一点的错误信息，如果想看效果的话，make debug即可
	for (int i = 0; line_buffer[i]; i++)
	{
		if (i + 1 == from) fprintf(stderr, "%s", write_color(level));
		if (i + 1 == to) fprintf(stderr, "\033[0m");
		fprintf(stderr, "%c", line_buffer[i]);
	}
	for (int i = 0; i + 1 < to; i++)
	{
		if (i + 1 == from) fprintf(stderr, "%s^", write_color(level));
		else if (i + 1 > from) fprintf(stderr, "~");
		else fprintf(stderr, line_buffer[i] == '\t' ? "\t" : " ");
	}
	fprintf(stderr, "\033[0m\n");
	if (note)
	{
		for (int i = 0; i + 1 <= from; i++)
			fprintf(stderr, line_buffer[i] == '\t' ? "\t" : " ");
		fprintf(stderr, "%s\n", note);
	}
#endif
}

#define raise_line_error_4(a, b, c, d) _raise_line_error(a, b, c, d)
#define raise_line_error(a, b, c) _raise_line_error(a, b, c, NULL)

typedef void* void_star;
#define YYSTYPE void_star

/* 下面这一段是用来统计可变参数的个数的，来源是stackoverflow */

#define PP_NARG(...) \
		PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
		PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
		_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
		_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
		_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
		_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
		_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
		_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
		_61,_62,_63,N,...) N
#define PP_RSEQ_N() \
		63,62,61,60,                   \
		59,58,57,56,55,54,53,52,51,50, \
		49,48,47,46,45,44,43,42,41,40, \
		39,38,37,36,35,34,33,32,31,30, \
		29,28,27,26,25,24,23,22,21,20, \
		19,18,17,16,15,14,13,12,11,10, \
		9,8,7,6,5,4,3,2,1,0

// 重写新建节点的伪函数，方便自己调用

#define nnewnode(_000, a, b, c, d) _newnode(_000, a, b, c, d, 0)
#define newnode(_000, ...) _newnode(_000, 0, 0, 0, 0, PP_NARG(__VA_ARGS__), ##__VA_ARGS__)



#endif
