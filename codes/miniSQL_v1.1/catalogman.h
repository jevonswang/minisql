//
//  catalogman.h
//  minisql-catalog-cli
//
//  Created by Duo Xu on 10/16/13.
//  Copyright (c) 2013 Duo Xu. All rights reserved.
//

#ifndef _CATALOG_H_
#define _CATALOG_H_

#include <string>
#include <vector>
#include "global.h"

#define NAME_LENGTH 20

struct TableCatalog {
	char flag;
	char tableName[NAME_LENGTH];
	char numberOfKeys; // <= 32
	char primaryKey; // 如果 > 32 表示无主键
	unsigned long indexFlags; // 对每一位，0 表示该键无索引，1 表示该键有索引
	unsigned short firstKey; // key 目录文件中，该表第一条键信息的编号
	short firstIndex; // index 目录文件中，该表第一条索引信息的编号
};
struct KeyCatalog {
	char flag;
	char keyName[NAME_LENGTH];
	char keyType; // 键类型，0 表示 int，1 表示 char(n)，2 表示 float
	char keyLength; // 键长度
	short nextKey; // 该表下一条键信息的编号，若无则置 -1
};
struct IndexCatalog {
	char flag;
	char indexName[NAME_LENGTH];
	unsigned short onTable; // 该索引所属表在 table 目录文件中的编号
	char key; // 该索引针对所属表的键编号
	short nextIndex; // 该表下一条索引信息的编号，若无则置 -1
};

class CatalogMan {
private:
	// 标志位相关
	static const char CATALOG_SPACE_USED = 0x80;
	static const char CATALOG_HAS_PRIMARY_KEY = 0x40;
	static const char CATALOG_HAS_INDEX = 0x20;
	static const char CATALOG_IS_PRIMARY_KEY = 0x10;
	static const char CATALOG_IS_UNIQUE = 0x08;
	static const char CATALOG_IS_NOT_NULL = 0x04;
	static const char CATALOG_IS_INDEX = 0x02;
	static const char CATALOG_HAS_NEXT = 0x01;
	// 把磁盘读取的目录文件存入此处的 vector 量中，对目录的修改操作直接针对此处变量
	std::vector<TableCatalog> tableCatalog;
	std::vector<KeyCatalog> keyCatalog;
	std::vector<IndexCatalog> indexCatalog;
	// 从文件读取数据，由构造函数调用
	void loadTableCatalogFromFile();
	void loadKeyCatalogFromFile();
	void loadIndexCatalogFromFile();
	// 将数据写入文件，由析构函数调用
	void saveTableCatalogToFile();
	void saveKeyCatalogToFile();
	void saveIndexCatalogToFile();
public:
	CatalogMan();
	virtual ~CatalogMan();
	static CatalogMan &instance() {
		static CatalogMan inst;
		return inst;
	}
	Table &getTable(const std::string &tableName);
	std::string getAttrName(const std::string &indexName);
	bool alreadyHasBPlusTree(const std::string &tableName,
			const std::string &keyName);
	// bool gonnaRemoveBPlusTree(const std::string &indexName);

	std::string getTableName(const std::string &indexName);
	void createTableCheck(const std::string &tableName,
			const std::vector<Attribute> &attributes);
	void createTable(const std::string &tableName,
			const std::vector<Attribute> &attributes);
	void dropTableCheck(const std::string &tableName);
	void dropTable(const std::string &tableName);
	void createIndexCheck(const std::string &indexName,
			const std::string &tableName, const std::string &keyName);
	void createIndex(const std::string &indexName, const std::string &tableName,
			const std::string &keyName);
	void dropIndexCheck(const std::string &indexName);
	void dropIndex(const std::string &indexName);
	void selectCheck(const Query &query);
	void insertTupleCheck(const std::string &tableName,
			const std::vector<std::string> &tuple);
	void deleteTupleCheck(const Query &query);
};

#endif
