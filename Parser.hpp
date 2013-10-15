#include "stdafx.h"
//#include <boost/locale.hpp>



std::string Utf8_to_1251(const char*str);
char* _1251_to_uth8(const char* str);
void display(char* hash);
void hasher(string a, char hash[20]);


typedef boost::make_recursive_variant < 
		long long int, 
		string, 
		list<boost::recursive_variant_>,
		map<string,boost::recursive_variant_> 
	>::type var_types;


class MetainfoParser
{
public:	
	struct MetaInfo Parse(string torfile_name);
private:
	int infostart;
	int infolength;
	bool GetDictionary(map<string,var_types>* res_dict,string* str,int *pos);
	bool GetInteger(long long int *integer,string *str,int* pos);
	bool GetString(string *res_string,string* str,int *pos);
	bool GetList(list<var_types>* res_list,string *str,int *pos);
	
};



