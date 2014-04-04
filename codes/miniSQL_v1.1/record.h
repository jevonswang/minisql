#pragma once

#include <string>
#include <vector>
#include "global.h"
#include "buffer.h"

using namespace std;



class RecordMan{
private:
	
public:
	static RecordMan& instance(); 
	int getBound(const std::string& tableName) const {
		std::string noUse;
		return BufferMan::instance().readLast(tableName+".table", noUse); 
	}
	bool createTable(const string& tableName, const vector<Attribute>& vtAttr);//������
	bool dropTable(const std::string& tableName);//ɾ����
	//std::vector<std::string> selectTuple(const Table& table);//�Ƿ���Ҫû��������ѡ��select * from table
	vector<Tuple> selectTuple(const Table& table,const vector<Condition>& condition, int blockNum);//selectForeach
	vector<Tuple> selectTuple(const Table& table,const vector<Condition>& condition, vector<int> m_offset);//selectviaIndex
	int insertTuple(Table table, std::vector<std::string>& tuple);//�����¼
	vector<pair<Tuple,int> > deleteTuple(const Table& table, const vector<Condition>& condition, vector<int> m_offset);//ͨ��Indexɾ����¼
	vector<pair<Tuple,int> > deleteTuple(const Table& table, const vector<Condition>& condition);//ͨ������ɾ����¼
	vector<pair<string, int> > getOffsetInfo(const Table& table, const string& attrName,const int cnt);//ȡ��ĳ���Ե�����ƫ����
private:
	//int getTupleSize(const Table& table);//�ú����ȼ���table.size()
	//bool comparator(Table table, std::map<Attribute,std::string> tuple, std::vector<Condition> condition);
	int findDirtyTuple(string strOut,int size);//return offset
	vector<string> tupleToAttr(vector<Attribute> m_attr,string tuple_str);
	bool isValid(int size, string str);
	bool hasExisted(Table table, string content, int num, int blockNum);

};


inline RecordMan& RecordMan::instance(){
	static RecordMan obj;
	return obj;
}

