#ifndef __EJQ_YACC_EXTEND_H_
#define __EJQ_YACC_EXTEND_H_

const char* debug_color_begin[] = {
	"\033[31m", // color error
	"\033[33m", // color warning
	"\033[34m", // color info
	"\033[32m", // color ok
	"", // nothing
};

#define _E_COLOR_ERR 0
#define _E_COLOR_WARN 1
#define _E_COLOR_INFO 2
#define _E_COLOR_OK 3

const char* write_color(const int i) {
	if (i >= 0 && i < 4) return debug_color_begin[i];
	return debug_color_begin[4];
}

#endif
