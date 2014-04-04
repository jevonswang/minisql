#ifndef _API_H_
#define _API_H_

#pragma once

#include <string>
#include <vector>
//#include "../miniSQL/global.h"
//#include "../Buffer/buffer.h"
//#include "record.h"
#include "catalogman.h"
// #include "index.h"
#include "global.h"
// #include "buffer.h"

// author: Zhefeng WANG, Duo XU, Xiaoshuang ZHANG, Qi Wang
// V1: 13-10-12
// V2: 13-10-25, 基本是针对recordMan与indexMan修改的。引入m_query等成员变量，修改selectVia*
//               添加public方法hasNext, next; 以及private的initOffset
// V3: 13-10-30, beyond word :-(

class API
{
public:
	static API& instance();
	void createTable(const std::string& tableName, const std::vector<Attribute>& vtAttr) const;
	void dropTable(const std::string& tableName) const;
	void createIndex(const std::string& indexName, const std::string& tableName, const std::string& attrName) const;
	void dropIndex(const std::string& indexName) const;
	void selectTuple(const Query& query);
	bool hasNext();
	std::vector<Tuple> next();
	void insertTuple(const std::string& tableName, std::vector<std::string>& tuple);
	void deleteTuple(const Query& query);
private:
	API(){};
	~API(){};
	API(const API& );
	API& operator=(const API&);
	void fixIndex(const Table& table, const std::vector<std::pair<Tuple, int> >& result,bool isInsert);
	
	void initOffset();
	std::vector<Tuple> selectViaIndex();
	std::vector<Tuple> selectViaForeach();
	std::vector<int> setIntersection(std::vector<int>& a, std::vector<int>& b);
	std::vector<std::pair<std::string, int> > getInfo(const int locate, const std::vector<std::pair<Tuple, int> > & tuple) const;
    
	Query m_query;
	Table m_table;
    std::vector<int> m_offset;
	bool m_flag;
	int m_cnt;
	int m_blockBound;
};

inline API& API::instance()
{
	static API obj;
	return obj;
}


//	bool updateTuple(const std::string& tableName, const std::vector<std::string>& tuple);
//	bool addAttribute(const std::string& tableName, const Attribute& attr) {}  // additional
//	bool dropAttribute(const std::string& tableName, const std::string& attrName) {} // additional
//	bool selectViaForeach(const Query& query, vector<string>& vtOut);
//	int getBlockNum(int offset) { return offset / BLOCK_SIZE; }
//	int getOffset(int offset) { return offset % BLOCK_SIZE; }

#endif
