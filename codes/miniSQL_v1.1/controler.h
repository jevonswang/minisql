#ifndef _CONTROLER_H_
#define _CONTROLER_H_

//#include "record.h"
#include "global.h"
#include <list>
#include <string>
#include <vector>


class Controler {
public:
    Controler() : m_tuple(emptyList) {
    }
	void querySwitch();
	std::list<std::string> m_cmd;
	std::string m_tableName;
	std::vector<Attribute> m_vtAttr;
	std::string m_indexName;
	std::string m_attrName;
    std::vector<std::string> emptyList;
	Tuple m_tuple;
	Query m_query;
	QUERY_TYPE m_queryType;
    
    // Table m_table;
	std::vector<Tuple> m_table;
	friend class CLI;
	void run();

    static Controler &instance() {
        static Controler inst;
        return inst;
    }
};

#endif
