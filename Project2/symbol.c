#ifndef __EJQ_SYMBOL_C
#define __EJQ_SYMBOL_C

#define EJQ_SYMBOL_BASIC    (0x0)
#define EJQ_SYMBOL_ABSTRACT (0x1)
#define EJQ_SYMBOL_NORMAL   (0x2)
#define EJQ_SYMBOL_ARRAY    (0x4)
#define EJQ_SYMBOL_STRUCT   (0x8)
#define EJQ_SYMBOL_FUNCTION (0x10)

typedef struct E_symbol_item{
	int type_uid; // 类型UID
	char *name; // 全名
	size_t len; // 总长，对于数组，是每一项的长度
	int is_abstract;
	// 是否抽象，对于结构体中的变量，我们管它叫抽象变量，需要映射为具体实例+偏移量才能使用
	// is_abstract还能指这个东西的类别
	//
	// 特别的，0x000表示内置变量，如int、float
	// 0x001 抽象变量
	// 0x002 普通变量，由type_uid指定类别
	// 0x004 数组
	// 0x008 结构体
	// 0x010 函数
	size_t offset; // 对于抽象变量，需要指出它的偏移
	size_t son_cnt; // 子孙数目，这个是针对结构体的
	size_t* son; // 子节点，还是针对结构体的
	// 如果它是一个数组，则son_cnt表示的是这个数组的维数，接着son表示了每一维的长度
}E_symbol_item;

E_symbol_item* E_symbol_table = NULL;
size_t E_symbol_table_alloc = 0;
size_t E_symbol_table_cap = 0;

size_t E_symbol_table_new()
{
	if (E_symbol_table == NULL)
	{
		E_symbol_table = malloc(sizeof(E_symbol_item));
		E_symbol_table_alloc = 0;
		E_symbol_table_cap = 1;
	}
	if (E_symbol_table_alloc == E_symbol_table_cap)
	{
		E_symbol_item* newMem = realloc(E_symbol_table, sizeof(E_symbol_item) * E_symbol_table_cap * 2);
		assert(newMem);
		E_symbol_table = newMem;
		E_symbol_table_cap *= 2;
	}
	return E_symbol_table_alloc++;
}

void E_symbol_table_init()
{
	size_t intnode;
}

// 符号表，符号表已用条目，符号表总条目
// 符号表中第i项就是类型uid，预留2项(int,float)

// 类型字符串的表示
// int a[2][2];  => int/2
// int a[3][3];  => int/2
//
// struct A{};   => A
// struct A{struct B{int c;}a;}; => A/B
//
// 类型查询机制例
//
// struct A{
//	struct B {
//   int c, d;
//	}e, f;
//	int g;
// }h;
//
// e、f的类型是A/B
// h的类型是A
// 访问：h.f.d:
// 查询到h的类型是A，寻找抽象变量A/f
// 查询到抽象变量A/f的类型是A/B，偏移是8，长度是8，寻找抽象变量A/B/d
// 查询到抽象变量A/B/d的类型是int，偏移是4，长度是4
// 回退回去，得到h.f.d的地址是h[8 + 4]长度4
//
// struct A x[12][16];
// 访问x[5][3].f.d
//
// 查询到x的类型是A/2，获取查询的维数是{5,3}，计算出偏移量(5 * 16 + 3) * 20，基础类别是A
// 查询A/f的类别是A/B，偏移是8，长度是8
// 查询A/B/d的类别是int，偏移是4，长度4
// 计算出总偏移量1664
// 获取到x[5][3].f.d即x[1660]长度为4


#endif
