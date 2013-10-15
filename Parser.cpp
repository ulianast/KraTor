#include "stdafx.h"
#include "Parser.h"
#include "MetaInfo.h"




	auto contents = [] (const std::string& fileName)->std::string {      // for reading from file. simple ifstream/wifstream  doesn't do this well 
																		 // because of failing in case FF
        std::ifstream in(fileName, std::ios::binary);
 
        return std::string(std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>());
    };


	MetaInfo MetainfoParser::Parse(string torfile_name)
	{

		string str=contents(std::string(torfile_name));
		map<string,var_types> dictionary;
		int pos=0;
		GetDictionary(&dictionary,&str,&pos);

		
		MetaInfo meta_info;
		if(dictionary.end()==dictionary.find("info"))
		{
			meta_info.correct=false;
			return meta_info;
		}
		
		map<string,var_types> info=boost::get<map<string,var_types>>(dictionary["info"]);

		string info_str=str.substr(infostart,infolength);
		hasher(info_str,meta_info.info_hash);
		
		

		if(info.end()!=info.find("length"))
		{
			meta_info.length=boost::get<long long int>(info["length"]);
			meta_info.multi_file=false;
		}
		else if(info.end()!=info.find("files"))
		{
			meta_info.multi_file=true;
			list<var_types> files=boost::get<list<var_types>>(info["files"]);
			for(list<var_types>::iterator it =files.begin(); it!=files.end();it++)
			{
				struct file _file;
				map<string,var_types> map_temp=boost::get<map<string,var_types>>(*it);
				if(map_temp.end()!=map_temp.find("path"))
				{
					list<var_types> l_s=boost::get<list<var_types>>(map_temp["path"]);
					for(list<var_types>::iterator it_l=l_s.begin();it_l!=l_s.end();it_l++)
						_file.path.push_back(boost::get<string>(*it_l));
				}
				else 
				{
					meta_info.correct=false;
					return meta_info;
				}
				if(map_temp.end()!=map_temp.find("length"))
					_file.file_length=boost::get<long long int>(map_temp["length"]);
				else {	meta_info.correct=false;	return meta_info;}
				
				meta_info.files.push_back(_file);
			}
		}
		else 
		{
			meta_info.correct=false;
			return meta_info;
		}

		if(info.end()!=info.find("name"))
		{
			meta_info.name=boost::get<string>(info["name"]);
		}

		if(info.end()!=info.find("piece length"))
			meta_info.piece_length=boost::get<long long int>(info["piece length"]);
		else {	meta_info.correct=false;	return meta_info;}

		if(info.end()!=info.find("pieces"))
			meta_info.pieces=boost::get<string>(info["pieces"]);
		else {	meta_info.correct=false;	return meta_info;}
		
		if(dictionary.end()!=dictionary.find("announce"))
			meta_info.announce=boost::get<string>(dictionary["announce"]);
		else meta_info.announce.clear();

		if(dictionary.end()!=dictionary.find("announce-list"))
		{
  			list<var_types> announ_l_item=boost::get<list<var_types>>(dictionary["announce-list"]);
			for(list<var_types>::iterator it =announ_l_item.begin();it!=announ_l_item.end();it++)
			{
				list<var_types> l_l=boost::get<list<var_types>>(*it);
				for(list<var_types>::iterator it_l=l_l.begin();it_l!=l_l.end();it_l++)
					meta_info.announce_list.push_back(boost::get<string>(*it_l));
			}

		}

		if(dictionary.end()!=dictionary.find("comment"))
			meta_info.comment=boost::get<string>(dictionary["comment"]);
		else meta_info.comment.clear();

		if(dictionary.end()!=dictionary.find("created by"))
			meta_info.created_by=boost::get<string>(dictionary["created by"]);

		if(dictionary.end()!=dictionary.find("creation date"))
			meta_info.creation_date=boost::get<long long int>(dictionary["creation date"]);
		else meta_info.creation_date=0;

		if(dictionary.end()!=dictionary.find("encoding"))
			meta_info.encoding=boost::get<string>(dictionary["encoding"]);
		else meta_info.encoding="UTF-8";

		if(meta_info.multi_file==false) meta_info.total_size=meta_info.length;
		else 
		{
			meta_info.total_size=0;
			for(list<struct file>::iterator it=meta_info.files.begin();it!=meta_info.files.end();it++)
				meta_info.total_size+=(*it).file_length;
		}

		return meta_info;
	}
	


	bool MetainfoParser::GetInteger(long long int *integer,string *str,int* pos)
	{
		(*integer)=-1;
		char c;
		bool negative=false;
		c=(*str)[*pos];
		if(c!='i') return false;
		else
		{
			(*pos)++;
	    for(;true;(*pos)++)
		{
			c=(*str)[*pos];
			if(c<'0'||c>'9')
			{
				if (c=='-'&&(*integer)==-1&&(!negative)) {negative=true;   (*integer)=0; continue;}
				if (c=='e') { (*pos)++;  (*integer)=negative?-(*integer):(*integer);return true;}
				return false;
			}
			else 
			{
				(*integer)=((*integer)==-1)?0:(*integer);
				(*integer)*=10;
				(*integer)+=c-'0';
				
			}
		}
		}
	}
	
	bool MetainfoParser::GetString(string *res_string,string* str,int *pos)
	{
		int string_length=0;
		char c;

		while(true)                  // get string length
		{
			c=(*str)[*pos];
			if(c>='0'&&c<='9')
			{
				string_length*=10;
				c-='0';
				string_length+=c;
				(*pos)++;
			}
			else if(c==':'&&string_length) {(*pos)++; break;}
			else return false;
		}

		if(string_length>0)          //get string
		{
			*res_string=((*str).substr((*pos),string_length));
			(*pos)+=string_length;
			return true;
		}
		else return false;
	}

	bool MetainfoParser::GetList(list<var_types>* res_list,string *str,int *pos)
	{
		char c=(*str)[*pos];
		if(c!='l') {return false;}
		else 
		{
			(*pos)++;
			list<var_types> tempor_list;
			while((*str)[(*pos)]!='e')
			{
				long long int try_integer;
				list<var_types> try_list;
				map<string,var_types> try_dictionary;
				string try_string;

				if(GetInteger(&try_integer,str,pos))
					tempor_list.push_back(try_integer);
				else if(GetString(&try_string,str,pos))
					tempor_list.push_back(try_string);
				else if (GetList(&try_list,str,pos))
					 tempor_list.push_back(try_list);
				else if(GetDictionary(&try_dictionary,str,pos))
					tempor_list.push_back(try_dictionary);
				else return false;
			}
			*res_list=tempor_list;
			(*pos)++;
			return true;
		}
	}

	bool MetainfoParser::GetDictionary(map<string,var_types>* res_dict,string* str,int *pos)
	{
		char c=(*str)[*pos];
		if(c!='d') {return false;}
		else 
		{
			(*pos)++;
			map<string,var_types> tempor_dict;
			while((*str)[(*pos)]!='e')
			{
				string key;
				GetString(&key,str,pos);
				if(key=="info")	infostart=*pos;
				
				long long int try_integer;
				list<var_types> try_list;
				map<string,var_types> try_dictionary;
				string try_string;

				if(GetInteger(&try_integer,str,pos))
					tempor_dict[key]=try_integer;
				else if(GetString(&try_string,str,pos))
					tempor_dict[key]=try_string;
				else if (GetList(&try_list,str,pos))
					tempor_dict[key]=try_list;
				else if(GetDictionary(&try_dictionary,str,pos))
					tempor_dict[key]=try_dictionary;
				else return false;

				if(key=="info")  infolength=(*pos)-infostart;
			}
			*res_dict=tempor_dict;
			(*pos)++;
			

			return true;
		}
	}



  std::string Utf8_to_1251(const char*str)
{			
	int size_str_unicode, size_ansi_str;
	
	/*const char* str =new const char[utf8_str.length];*/
	//str=utf8_str;

	size_str_unicode = MultiByteToWideChar(CP_UTF8,	0,	str,-1,	0,0); //convert from utf8 to unicode

					    //int MultiByteToWideChar(uCodePage, dwFlags, lpMultiByteStr, cchMultiByte, lpWideCharStr, cchWideChar)
						//UINT uCodePage;         /* codepage                        */ 
						//DWORD dwFlags;          /* character-type options           */ 
						//LPCSTR lpMultiByteStr;  /* address of string to map         */ 
						//int cchMultiByte;       /* number of characters in string   */  can be set -1 for null-terminated string
						//LPWSTR lpWideCharStr;   /* address of wide-character buffer */ 
						//int cchWideChar;        /* size of wide-character buffer    */ 
	
	if (!size_str_unicode) return 0;

	wchar_t *unicode_str = new wchar_t[size_str_unicode];

	if(!MultiByteToWideChar(CP_UTF8,0,str,-1,unicode_str,size_str_unicode))
	{
		delete[] unicode_str;
		return 0;
	}


	size_ansi_str = WideCharToMultiByte(1251,0,unicode_str,-1,0,0,0, 0);      //convertion from unicode to ansi

	if(!size_ansi_str)
	{
		delete [] unicode_str;
		return 0;
	}
	
	char *ansi_str = new char[size_ansi_str];

	if(!WideCharToMultiByte(1251,0,unicode_str,-1,ansi_str,size_ansi_str,0, 0))
	{
		delete[] unicode_str;
		delete [] ansi_str;
		return 0;
	}
	delete[] unicode_str;
	return ansi_str;
}

char* _1251_to_uth8(const char* str)
{
	int size_str_unicode, size_ansi_str;
	size_str_unicode = MultiByteToWideChar(1251,	0,	str,-1,	0,0); //convert from utf8 to unicode

					    //int MultiByteToWideChar(uCodePage, dwFlags, lpMultiByteStr, cchMultiByte, lpWideCharStr, cchWideChar)
						//UINT uCodePage;         /* codepage                        */ 
						//DWORD dwFlags;          /* character-type options           */ 
						//LPCSTR lpMultiByteStr;  /* address of string to map         */ 
						//int cchMultiByte;       /* number of characters in string   */  can be set -1 for null-terminated string
						//LPWSTR lpWideCharStr;   /* address of wide-character buffer */ 
						//int cchWideChar;        /* size of wide-character buffer    */ 
	
	if (!size_str_unicode) return 0;

	wchar_t *unicode_str = new wchar_t[size_str_unicode];

	if(!MultiByteToWideChar(1251,0,str,-1,unicode_str,size_str_unicode))
	{
		delete[] unicode_str;
		return 0;
	}


	size_ansi_str = WideCharToMultiByte(CP_UTF8,0,unicode_str,-1,0,0,0, 0);      //convertion from unicode to ansi

	if(!size_ansi_str)
	{
		delete [] unicode_str;
		return 0;
	}
	
	char *ansi_str = new char[size_ansi_str];

	if(!WideCharToMultiByte(CP_UTF8,0,unicode_str,-1,ansi_str,size_ansi_str,0, 0))
	{
		delete[] unicode_str;
		delete [] ansi_str;
		return 0;
	}
	delete[] unicode_str;
	return ansi_str;

}  

void display(char* hash)
{
	std::cout << "SHA1: " << std::hex;
    for(int i = 0; i < 20; ++i)
    {
        std::cout << ((hash[i] & 0x000000F0) >> 4) 
                  <<  (hash[i] & 0x0000000F);
    } 
    std::cout << std::endl; // Das wars  
}


void hasher(string a, char hash[20])
{
	boost::uuids::detail::sha1 s;
	//char hash[20];
	//std::string a = "The quick brown fox jumps over the lazy dog";
	s.process_bytes(a.c_str(), a.size());
	unsigned int digest[5];
	s.get_digest(digest);
	for(int i = 0; i < 5; ++i)
	{
		const char* tmp = reinterpret_cast<char*>(digest);
		hash[i*4] = tmp[i*4+3];
		hash[i*4+1] = tmp[i*4+2];
		hash[i*4+2] = tmp[i*4+1];
		hash[i*4+3] = tmp[i*4];
	}
	display(hash);

}

