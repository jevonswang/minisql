//
//  catalog.cpp
//  minisql-catalog-cli
//
//  Created by Duo Xu on 10/16/13.
//  Copyright (c) 2013 Duo Xu. All rights reserved.
//

#include <cstring>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "catalogman.h"
#include "global.h"

// 从文件读取数据，由构造函数调用
void CatalogMan::loadTableCatalogFromFile() {
	using namespace std;
	ifstream fin("catalog\\table.dat", ios::binary | ios::ate);
	long size = (long) fin.tellg();
	long num = size / (3 + NAME_LENGTH + sizeof(long) + 2 * sizeof(short));
	fin.seekg(0, ios::beg);

	TableCatalog tc;

	for (int i = 0; i < num; i++) {
		fin.read(&tc.flag, 1);
		fin.read(tc.tableName, NAME_LENGTH);
		fin.read(&tc.numberOfKeys, 1);
		fin.read(&tc.primaryKey, 1);
		fin.read((char *) &tc.indexFlags, sizeof(long));
		fin.read((char *) &tc.firstKey, sizeof(short));
		fin.read((char *) &tc.firstIndex, sizeof(short));
		tableCatalog.push_back(tc);
	}
	fin.close();


}

void CatalogMan::loadKeyCatalogFromFile() {
	using namespace std;
	ifstream fin("catalog\\key.dat", ios::binary | ios::ate);
	long size = (long) fin.tellg();
	long num = size / (3 + NAME_LENGTH + sizeof(short));
	fin.seekg(0, ios::beg);

	KeyCatalog kc;

	for (int i = 0; i < num; i++) {
		fin.read(&kc.flag, 1);
		fin.read(kc.keyName, NAME_LENGTH);
		fin.read(&kc.keyType, 1);
		fin.read(&kc.keyLength, 1);
		fin.read((char *) &kc.nextKey, sizeof(short));
		keyCatalog.push_back(kc);
	}
	fin.close();


}

void CatalogMan::loadIndexCatalogFromFile() {
	using namespace std;
	ifstream fin("catalog\\index.dat", ios::binary | ios::ate);
	long size = (long) fin.tellg();
	long num = size / (2 + NAME_LENGTH + 2 * sizeof(short));
	fin.seekg(0, ios::beg);

	IndexCatalog ic;

	for (int i = 0; i < num; i++) {
		fin.read(&ic.flag, 1);
		fin.read(ic.indexName, NAME_LENGTH);
		fin.read((char *) &ic.onTable, sizeof(short));
		fin.read(&ic.key, 1);
		fin.read((char *) &ic.nextIndex, sizeof(short));
		indexCatalog.push_back(ic);
	}
	fin.close();


}

// 将数据写入文件，由析构函数调用
void CatalogMan::saveTableCatalogToFile() {
	using namespace std;
	ofstream fout("catalog/table.dat", ios::binary);
	fout.seekp(0, ios::beg);
	for (size_t i = 0; i < tableCatalog.size(); i++) {
		fout.write(&tableCatalog[i].flag, 1);
		fout.write(tableCatalog[i].tableName, NAME_LENGTH);
		fout.write(&tableCatalog[i].numberOfKeys, 1);
		fout.write(&tableCatalog[i].primaryKey, 1);
		fout.write((char *) &tableCatalog[i].indexFlags, sizeof(long));
		fout.write((char *) &tableCatalog[i].firstKey, sizeof(short));
		fout.write((char *) &tableCatalog[i].firstIndex, sizeof(short));
	}
	fout.close();
}

void CatalogMan::saveKeyCatalogToFile() {
	using namespace std;
	ofstream fout("catalog/key.dat", ios::binary);
	fout.seekp(0, ios::beg);
	for (size_t i = 0; i < keyCatalog.size(); i++) {
		fout.write(&keyCatalog[i].flag, 1);
		fout.write(keyCatalog[i].keyName, NAME_LENGTH);
		fout.write(&keyCatalog[i].keyType, 1);
		fout.write((char *) &keyCatalog[i].keyLength, 1);
		fout.write((char *) &keyCatalog[i].nextKey, sizeof(short));
	}
	fout.close();
}

void CatalogMan::saveIndexCatalogToFile() {
	using namespace std;
	ofstream fout("catalog/index.dat", ios::binary);
	fout.seekp(0, ios::beg);
	for (size_t i = 0; i < indexCatalog.size(); i++) {
		fout.write(&indexCatalog[i].flag, 1);
		fout.write(indexCatalog[i].indexName, NAME_LENGTH);
		fout.write((char *) &indexCatalog[i].onTable, sizeof(short));
		fout.write(&indexCatalog[i].key, 1);
		fout.write((char *) &indexCatalog[i].nextIndex, sizeof(short));
	}
	fout.close();
}

CatalogMan::CatalogMan() {
	loadTableCatalogFromFile();
	loadKeyCatalogFromFile();
	loadIndexCatalogFromFile();
}
CatalogMan::~CatalogMan() {
	saveTableCatalogToFile();
	saveKeyCatalogToFile();
	saveIndexCatalogToFile();
}

Table &CatalogMan::getTable(const std::string &tableName) {
	Table *t = new Table;
	t->m_name = tableName;

	// 查找表
	short firstKeyIndex = 0;
	unsigned long indexFlags = 0;
	bool found = false;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			found = true;
			// 获得该表的索引标志
			indexFlags = tableCatalog[i].indexFlags;
			// 获得该表第一条属性的位置
			firstKeyIndex = tableCatalog[i].firstKey;
			break;
		}
	if (!found)
		throw "TableNotExist: The table \"" + tableName + "\" does not exist.";
	// 获取该表所有的属性信息
	short currentKeyIndex = firstKeyIndex;
	while (currentKeyIndex != -1) {
		// 获得单个属性的值
		Attribute attr;
		attr.m_name = keyCatalog[currentKeyIndex].keyName;
		switch (keyCatalog[currentKeyIndex].keyType) {
		case 0:
			attr.m_type = INT;
			break;
		case 1:
			attr.m_type = CHAR;
			break;
		case 2:
			attr.m_type = FLOAT;
			break;
		default:
			attr.m_type = CHAR;
			break;
		}
		attr.m_length = keyCatalog[currentKeyIndex].keyLength;
		attr.m_flag = 0;
		attr.m_flag |= (keyCatalog[currentKeyIndex].flag & CATALOG_IS_PRIMARY_KEY) ? 1 : 0;
		attr.m_flag |= (keyCatalog[currentKeyIndex].flag & CATALOG_IS_UNIQUE) ? 2 : 0;
		attr.m_flag |= (keyCatalog[currentKeyIndex].flag & CATALOG_IS_NOT_NULL) ? 4 : 0;
		attr.m_flag |= (keyCatalog[currentKeyIndex].flag & CATALOG_IS_INDEX) ? 8 : 0;

		t->m_attr.push_back(attr); // 存入 t 的属性表中
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}
	return *t;
}


std::string CatalogMan::getTableName(const std::string &indexName) {
	using namespace std;
	// 查找索引
	unsigned short onTable = 0;
	char key = 0;
	bool found = false;
	for (size_t i = 0; i < indexCatalog.size(); i++)
		if ((indexCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(indexCatalog[i].indexName, indexName.c_str())) {
			found = true;
			// 获得该索引所属表、字段号
			onTable = indexCatalog[i].onTable;
			key = indexCatalog[i].key;
			break;
		}
	if (!found)
		throw "IndexNotExist: The index \"" + indexName + "\" does not exist.";
	// 返回表名
	return tableCatalog[onTable].tableName;
}



std::string CatalogMan::getAttrName(const std::string &indexName) {
	using namespace std;
	// 查找索引
	unsigned short onTable = 0;
	char key = 0;
	bool found = false;
	for (size_t i = 0; i < indexCatalog.size(); i++)
		if ((indexCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(indexCatalog[i].indexName, indexName.c_str())) {
			found = true;
			// 获得该索引所属表、字段号
			onTable = indexCatalog[i].onTable;
			key = indexCatalog[i].key;
			break;
		}
	if (!found)
		throw "IndexNotExist: The index \"" + indexName + "\" does not exist.";
	// 从表目录文件中找到该表，获得它的第一条属性在属性目录文件中的位置
	unsigned short firstKey = tableCatalog[onTable].firstKey;
	// 根据 key 编号，在 keyCatalog 中找到这条属性
	unsigned short currentKeyIndex = firstKey;
	for (char i = 0; i < key; i++)
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	return keyCatalog[currentKeyIndex].keyName;
}

bool CatalogMan::alreadyHasBPlusTree(const std::string &tableName,const std::string &keyName) {
	using namespace std;
	// 表是否存在
	short firstKeyIndex = 0, firstIndexIndex = 0;
	unsigned long indexFlags = 0;
	bool found = false, hasIndex = false;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			found = true;
			// 获得该表的索引标志
			indexFlags = tableCatalog[i].indexFlags;
			// 获得该表第一条属性、索引（如果有）的位置
			firstKeyIndex = tableCatalog[i].firstKey;
			if (tableCatalog[i].flag & CATALOG_HAS_INDEX) {
				hasIndex = true;
				firstIndexIndex = tableCatalog[i].firstIndex;
			}
			break;
		}
	if (!found)
		throw "TableNotExist: The table \"" + tableName + "\" does not exist.";
	// 属性是否存在
	found = false;
	short currentKeyIndex = firstKeyIndex, keyNumber = -1;
	while (currentKeyIndex != -1 && !found) {
		keyNumber++;
		if (!strcmp(keyCatalog[currentKeyIndex].keyName, keyName.c_str())) {
			found = true;
			break;
		}
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}
	if (!found)
		throw "KeyNotExist: The key \"" + keyName + "\" does not exist.";
	// 返回索引是否存在
	return (indexFlags >> keyNumber) & 1;
}

/*
 bool CatalogMan::gonnaRemoveBPlusTree(const std::string &indexName) {
 }
 */

void CatalogMan::createTableCheck(const std::string &tableName,
		const std::vector<Attribute> &attributes) {
	using namespace std;
	// 表名是否冲突
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			throw "TableAlreadyExists: The table \"" + tableName
					+ "\" already exists.";
			break;
		}
	// 属性是否合法
	if (attributes.size() < 1 || attributes.size() > 32)
		throw string("KeyIllegal: Too few or too many keys.");

	for (size_t i = 0; i < attributes.size(); i++)
		if (attributes[i].m_length < 1 || attributes[i].m_length > 255)
			throw string("KeyIllegal: Illegal length of the keys.");
}


void CatalogMan::createTable(const std::string &tableName,
		const std::vector<Attribute> &attributes) {
	// tableCatalog 中查找未使用空间，或新开空间
	TableCatalog tc;
	// 设置好要写入的 table 信息（CATALOG_HAS_PRIMARY_KEY 标志在稍后创建属性时添加，CATALOG_HAS_INDEX 在创建索引时添加）
	tc.flag = CATALOG_SPACE_USED;
	tc.flag &= ~CATALOG_HAS_INDEX;
	memset(tc.tableName, 0, NAME_LENGTH);
	strcpy(tc.tableName, tableName.c_str());
	tc.numberOfKeys = attributes.size();
	tc.primaryKey = 0;
	tc.indexFlags = 0;
	// 写入 tableCatalog
	short newTableIndex = -1;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if (!(tableCatalog[i].flag & CATALOG_SPACE_USED)) {
			newTableIndex = i;
			tableCatalog[i] = tc;
			break;
		}
	if (newTableIndex == -1) {
		tableCatalog.push_back(tc);
		newTableIndex = tableCatalog.size() - 1;
	}

	// keyCatalog 中查找未使用空间，或新开空间
	KeyCatalog kc;
	short currentKeyIndex = -1, previousKeyIndex = 0;
	for (size_t i = 0; i < attributes.size(); i++) {
		// 对当前属性，设置好要写入的 key 信息
		kc.flag = CATALOG_SPACE_USED;
		if (attributes[i].isPrimary()) {
			kc.flag |= CATALOG_IS_PRIMARY_KEY;
			kc.flag |= CATALOG_IS_UNIQUE;
			kc.flag |= CATALOG_IS_NOT_NULL;
		}
		if (attributes[i].isUnique())
			kc.flag |= CATALOG_IS_UNIQUE;
		if (attributes[i].isNotNull())
			kc.flag |= CATALOG_IS_NOT_NULL;
		memset(kc.keyName, 0, NAME_LENGTH);
		strcpy(kc.keyName, attributes[i].m_name.c_str());
		switch (attributes[i].m_type) {
		case INT:
			kc.keyType = 0;
			kc.keyLength = 4;
			break;
		case CHAR:
			kc.keyType = 1;
			kc.keyLength = attributes[i].m_length;
			break;
		case FLOAT:
			kc.keyType = 2;
			kc.keyLength = 4;
			break;
		default: // 默认看作长度为 255 的字符串
			kc.keyType = 1;
			kc.keyLength = 255;
			break;
		}
		kc.nextKey = -1;
		// 写入 keyCatalog
		currentKeyIndex++;
		while (currentKeyIndex < (int)keyCatalog.size()
				&& (keyCatalog[currentKeyIndex].flag & CATALOG_SPACE_USED))
			currentKeyIndex++;
		if (currentKeyIndex >= (int)keyCatalog.size())
			// 新开空间
			keyCatalog.push_back(kc);
		else
			// 占用当前未使用空间
			keyCatalog[currentKeyIndex] = kc;

		if (i == 0)
			// 如果当前是第一条属性，修改 tableCatalog 的 firstKey
			tableCatalog[newTableIndex].firstKey = currentKeyIndex;
		else
			// 否则修改此表上一条属性的 nextKey（即当前不是第一条属性）
			keyCatalog[previousKeyIndex].nextKey = currentKeyIndex;

		previousKeyIndex = currentKeyIndex;

		// 如果当前属性是主键，修改 tableCatalog 的标志和 primaryKey
		if (kc.flag & CATALOG_IS_PRIMARY_KEY) {
			tableCatalog[newTableIndex].flag |= CATALOG_HAS_PRIMARY_KEY;
			tableCatalog[newTableIndex].primaryKey = i;
		}
	}
}

void CatalogMan::dropTableCheck(const std::string &tableName){
	// 表名是否冲突
	bool found = false;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			found = true;
			break;
		}
	if (!found)
		throw "TableNotExist: The table \"" + tableName + "\" does not exist.";
}

void CatalogMan::dropTable(const std::string &tableName) {
	// 删除 tableCatalog 中对应的元素
	short firstKeyIndex = 0, firstIndexIndex = 0;
	bool hasIndex = false;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) && 
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			// 获得该表第一条属性、索引（如果有）的位置
			firstKeyIndex = tableCatalog[i].firstKey;
			if (tableCatalog[i].flag & CATALOG_HAS_INDEX) {
				hasIndex = true;
				firstIndexIndex = tableCatalog[i].firstIndex;
			}
			// 设置该空间为未使用
			tableCatalog[i].flag &= ~CATALOG_SPACE_USED;
			// 如果刚好在最后一个位置，直接从 vector 中删除该元素以节省空间
			if (i == tableCatalog.size() - 1)
				tableCatalog.pop_back();
		}

	// 删除 keyCatalog 中对应的元素
	short currentKeyIndex = firstKeyIndex;
	while (currentKeyIndex != -1) {
		// 注：该表的最后一条属性的 nextKey 是 -1，故有此循环条件
		keyCatalog[currentKeyIndex].flag &= ~CATALOG_SPACE_USED;
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}

	// 删除 indexCatalog 中对应的元素（如果有索引）
	if (hasIndex) {
		short currentIndexIndex = firstIndexIndex;
		while (currentIndexIndex != -1) {
			// 注：该表的最后一条索引的 nextIndex 是 -1，故有此循环条件
			indexCatalog[currentIndexIndex].flag &= ~CATALOG_SPACE_USED;
			currentIndexIndex = indexCatalog[currentIndexIndex].nextIndex;
		}
	}
}

void CatalogMan::createIndexCheck(const std::string &indexName,
		const std::string &tableName, const std::string &keyName){
	using namespace std;
	// 表是否存在
	short firstKeyIndex = 0, firstIndexIndex = 0;
	unsigned long indexFlags = 0;
	bool found = false, hasIndex = false;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			found = true;
			// 获得该表的索引标志
			indexFlags = tableCatalog[i].indexFlags;
			// 获得该表第一条属性、索引（如果有）的位置
			firstKeyIndex = tableCatalog[i].firstKey;
			if (tableCatalog[i].flag & CATALOG_HAS_INDEX) {
				hasIndex = true;
				firstIndexIndex = tableCatalog[i].firstIndex;
			}
			break;
		}
	if (!found)
		throw "TableNotExist: The table \"" + tableName + "\" does not exist.";
	// 属性是否存在、属性是否是唯一值（unique）
	found = false;
	short currentKeyIndex = firstKeyIndex, keyNumber = -1;
	while (currentKeyIndex != -1 && !found) {
		keyNumber++;
		if (!strcmp(keyCatalog[currentKeyIndex].keyName, keyName.c_str())) {
			found = true;
			if (!(keyCatalog[currentKeyIndex].flag & CATALOG_IS_UNIQUE))
				throw "KeyNotUnique: The key \"" + keyName
						+ "\" is not unique.";
			break;
		}
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}
	if (!found)
		throw "KeyNotExist: The key \"" + keyName + "\" does not exist.";
	// 索引是否冲突
	if ((indexFlags >> keyNumber) & 1)
		throw "IndexAlreadyExists: An index already exists on the key \""
				+ keyName + "\".";
	for (size_t i = 0; i < indexCatalog.size(); i++)
		if ((indexCatalog[i].flag & CATALOG_SPACE_USED) &&
			!strcmp(indexCatalog[i].indexName, indexName.c_str()))
			throw string(
					"IndexAlreadyExists: An index of the same name already exists.");
}

void CatalogMan::createIndex(const std::string &indexName,
		const std::string &tableName, const std::string &keyName) {
	// 在 tableCatalog 中找到该表
	short tableIndex = 0, firstKeyIndex = 0, firstIndexIndex = 0;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			tableIndex = i;
			firstKeyIndex = tableCatalog[i].firstKey;
			firstIndexIndex = tableCatalog[i].firstIndex;
			break;
		}
	// 在 keyCatalog 中找到该属性，记录下来它是第几个属性，并设置标志 CATALOG_IS_INDEX
	short currentKeyIndex = firstKeyIndex, keyNumber = -1;
	while (currentKeyIndex != -1) {
		keyNumber++;
		if (!strcmp(keyCatalog[currentKeyIndex].keyName, keyName.c_str())) {
			keyCatalog[currentKeyIndex].flag |= CATALOG_IS_INDEX;
			break;
		}
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}
	// 设置 tableCatalog 的索引标志
	tableCatalog[tableIndex].indexFlags |= 1 << keyNumber;
	// indexCatalog 中查找未使用空间，或新开空间
	IndexCatalog ic;
	// 设置好要写入的 index 信息
	ic.flag = CATALOG_SPACE_USED;
	memset(ic.indexName, 0, NAME_LENGTH);
	strcpy(ic.indexName, indexName.c_str());
	ic.onTable = tableIndex;
	ic.key = keyNumber;
	ic.nextIndex = -1;
	// 写入 indexCatalog
	int newIndexIndex = -1;
	for (size_t i = 0; i < indexCatalog.size(); i++)
		if (!(indexCatalog[i].flag & CATALOG_SPACE_USED)) {
			newIndexIndex = i;
			indexCatalog[i] = ic;
			break;
		}
	if (newIndexIndex == -1) {
		indexCatalog.push_back(ic);
		newIndexIndex = indexCatalog.size() - 1;
	}
	// 如果之前该表已有索引，就把新索引和该表的其他索引连接起来，否则它就是该表的第一条索引
	if (!(tableCatalog[tableIndex].flag & CATALOG_HAS_INDEX)) {
		tableCatalog[tableIndex].flag |= CATALOG_HAS_INDEX;
		tableCatalog[tableIndex].firstIndex = newIndexIndex;
	} else {
		int currentIndexIndex = firstIndexIndex;
		while (indexCatalog[currentIndexIndex].nextIndex != -1)
			currentIndexIndex = indexCatalog[currentIndexIndex].nextIndex;
		indexCatalog[currentIndexIndex].nextIndex = newIndexIndex;
	}
}

void CatalogMan::dropIndexCheck(const std::string &indexName){
	// 检查内容：索引是否存在
	bool found = false;
	for (size_t i = 0; i < indexCatalog.size(); i++)
		if ((indexCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(indexCatalog[i].indexName, indexName.c_str())) {
			found = true;
			break;
		}
	if (!found)
		throw "IndexNotExist: The index \"" + indexName + "\" does not exist.";
}

void CatalogMan::dropIndex(const std::string &indexName) {
	// 找到这条索引，并记录该索引所针对的表编号、字段编号，以及它的下一条索引（同表），将空间标记为未使用
	short tableIndex = 0, keyNumber = 0, indexIndex = 0, nextIndexIndex = 0;
	for (size_t i = 0; i < indexCatalog.size(); i++)
		if ((indexCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(indexCatalog[i].indexName, indexName.c_str())) {
			tableIndex = indexCatalog[i].onTable;
			keyNumber = indexCatalog[i].key;
			nextIndexIndex = indexCatalog[i].nextIndex;
			indexCatalog[i].flag &= ~CATALOG_SPACE_USED;
			indexIndex = i;
			break;
		}
	
	// 在 keyCatalog 中找到该属性，并设置标志位 CATALOG_IS_INDEX 为 0
	short firstKeyIndex = tableCatalog[tableIndex].firstKey;
	short currentKeyIndex = firstKeyIndex;
	short currentKeyNumber = -1;


	while (currentKeyIndex != -1) {
		currentKeyNumber++;
		if (currentKeyNumber == keyNumber) {
			keyCatalog[currentKeyIndex].flag &= ~CATALOG_IS_INDEX;
			break;
		}
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}
	
	// 根据 tableCatalog 中的第一条索引编号，找到要删除索引的上一条索引并进行重连接操作；还有一种情况是，要删除的就是第一条索引
	if (tableCatalog[tableIndex].firstIndex != indexIndex) {
		int currentIndexIndex = tableCatalog[tableIndex].firstIndex;
		while (indexCatalog[currentIndexIndex].nextIndex != indexIndex)
			currentIndexIndex = indexCatalog[currentIndexIndex].nextIndex;
		indexCatalog[currentIndexIndex].nextIndex = nextIndexIndex;
	} else
		tableCatalog[tableIndex].firstIndex = nextIndexIndex;
	// 设置 tableCatalog 中的标志，如果索引全部删除还需设置 CATALOG_HAS_INDEX = 0
	tableCatalog[tableIndex].indexFlags &= ~(unsigned long) (1 << keyNumber);
	if (tableCatalog[tableIndex].firstIndex == -1)
		tableCatalog[tableIndex].flag &= ~CATALOG_HAS_INDEX;
}

void CatalogMan::selectCheck(const Query &query){
	using namespace std;
	// 表是否存在
	bool found = false;
	short firstKeyIndex = 0;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, query.m_tableName.c_str())) {
			found = true;
			firstKeyIndex = tableCatalog[i].firstKey;
			break;
		}
	if (!found)
		throw "TableNotExist: The table \"" + query.m_tableName
				+ "\" does not exist.";
	// 条件出现的所有属性是否均存在
	for (size_t i = 0; i < query.m_condition.size(); i++) {
		found = false;
		short currentKeyIndex = firstKeyIndex;
		while (currentKeyIndex != -1 && !found) {
			if (!strcmp(keyCatalog[currentKeyIndex].keyName,
					query.m_condition[i].m_attrName.c_str())) {
				found = true;
				break;
			}
			currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
		}
		if (!found)
			throw string(
					"KeyNotExist: One of the keys in the query does not exist.");
	}
}

void CatalogMan::insertTupleCheck(const std::string &tableName,
		const std::vector<std::string> &tuple) {
	using namespace std;
	// 表是否存在
	bool found = false;
	short numberOfKeys = 0, firstKeyIndex = 0;
	for (size_t i = 0; i < tableCatalog.size(); i++)
		if ((tableCatalog[i].flag & CATALOG_SPACE_USED) &&
	        !strcmp(tableCatalog[i].tableName, tableName.c_str())) {
			found = true;
			numberOfKeys = tableCatalog[i].numberOfKeys;
			firstKeyIndex = tableCatalog[i].firstKey;
			break;
		}
	if (!found)
		throw "TableNotExist: The table \"" + tableName + "\" does not exist.";
	// 值是否合法
	if (tuple.size() > numberOfKeys)
		throw string("ValueIllegal: Illegal number of columns.");
	short currentKeyIndex = firstKeyIndex;
	for (size_t i = 0; i < tuple.size(); i++) {
		switch (keyCatalog[currentKeyIndex].keyType) {
		case 0: // int
			// 可扩充
			break;
		case 1: // char(n)
			if ((int)tuple[i].length() > keyCatalog[currentKeyIndex].keyLength)
				throw string(
						"ValueIllegal: The value to insert is longer than the length of the key.");
			break;
		case 2: // float
			// 可扩充
			break;
		default: // 视作 char(n)
			break;
		}
		currentKeyIndex = keyCatalog[currentKeyIndex].nextKey;
	}
}

void CatalogMan::deleteTupleCheck(const Query &query) throw (std::string) {
	selectCheck(query);
}