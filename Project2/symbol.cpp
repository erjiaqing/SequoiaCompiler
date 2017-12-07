#ifndef _E_SYMBOL_CPP
#define _E_SYMBOL_CPP

#include <vector>
#include <string>

namespace ejq_compiler{
	using std::vector;
	using std::string;
	struct symbol_table_item{
		private:
		int type_uid;
		string name;
		size_t size;
		bool is_abstract, is_struct, is_array, is_base_type;
		vector<size_t> son;
		public:
		symbol_table_item(int type_uid, string name, size_t size, bool is_abstract, bool is_struct, bool is_array, bool is_base_type, vector<size_t> son):
			type_uid(type_uid), name(name), size(size), is_abstract(is_abstract),
			is_struct(is_struct), is_array(is_array), is_base_type(is_base_type), son(son)
		{}
	};
	vector<symbol_table_item> symbol_table;
	void init_symbol_table()
	{
		// 初始化int和float两个类型
		symbol_table.push_back(symbol_table_item(0, "int", 4, false, false, false, true, vector<size_t>()));
		symbol_table.push_back(symbol_table_item(1, "float", 4, false, false, false, true, vector<size_t>()));
		// 它们不加入符号表
	}
};

#endif
