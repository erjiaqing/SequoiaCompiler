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

#ifdef _DEBUG
	#define debug(...) fprintf(stderr, "%s", debug_color_begin[2]),fprintf(stderr, ##__VA_ARGS__),fprintf(stderr, "%s", "\033[0m")
#else
	#define debug(...) {/* nothing */}
#endif

#define error(...) fprintf(stderr, "%s", debug_color_begin[0]), fprintf(stderr, ##__VA_ARGS__), fprintf(stderr, "%s\n", "\033[0m")

#define ce(...) fprintf(stderr, "%s[ CE ]\033[0m", debug_color_begin[0]), fprintf(stderr, ##__VA_ARGS__), fprintf(stderr, "\n")
#define warn(...) fprintf(stderr, "%s[WARN]\033[0m", debug_color_begin[1]), fprintf(stderr, ##__VA_ARGS__), fprintf(stderr, "\n")
#define output(...) fprintf(result_writer, ##__VA_ARGS__)

#endif
