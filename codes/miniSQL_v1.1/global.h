#pragma once

#include <vector>
#include <string>
#include <map>
#include <cassert>
#include <sstream>

const size_t BLOCK_SIZE = 4096;
const size_t BLOCK_TOT = 10000;
const double EPS = 0.0000001;

enum DATA_TYPE
{
	INT, 
	CHAR, 
	FLOAT
};

enum OP_TYPE 
{
	EQUAL, 
	NO_EQUAL, 
	LESS, 
	GREATER, 
	NO_GREATER, 
	NO_LESS
};

enum QUERY_TYPE
{ 
	CREATE_TABLE,
	DROP_TABLE,
	CREATE_INDEX,
	DROP_INDEX,
	INSERT_TUPLE,
	DELETE_TUPLE,
	SELECT_TUPLE,
	EXECFILE,
	QUIT
};

class Exception
{
public:
	int m_excepNum;
	std::string m_tableName;
	std::string m_indexName;
	std::string m_attrName;
	std::string m_input;
};

class Attribute;
class Table
{
public:
	Table(const std::string& n, const std::vector<Attribute> vt):
		m_name(n), m_attr(vt) {}
	Table() {}
	Attribute getAttribute(const std::string& attrName) const;
	std::vector<std::string> getAttrNameIsIndex() const;
	size_t size() const;
	size_t getAttrNum(const std::string& attrName) const; // acquire the locate of the 'attrName' in Table
	size_t getAttrBegin(const std::string& attrName) const;
	size_t getAttrEnd(const std::string& attrName) const;
	DATA_TYPE getAttrType(const std::string& attrName) const;

	std::string m_name;
	std::vector<Attribute> m_attr;
private:
	void setIndex(const std::string& attrName);
	void unsetIndex(const std::string& attrName);
	friend std::istream& operator>>(std::istream& is, Table& table);
	friend std::ostream& operator<<(std::ostream& os, const Table& table);
	friend class CatalogMan;
};

class Attribute
{
public:
	Attribute(const std::string& n, DATA_TYPE t, size_t l, size_t f):
		m_name(n), m_type(t), m_length(l), m_flag(f) 
	{
		assert( isIndex() == false );
	}
	Attribute(DATA_TYPE t, size_t length): // for test convenient
		m_type(t), m_length(length) {}
	Attribute(const std::string& n, DATA_TYPE t, size_t l): // for test convenient
		m_name(n), m_type(t), m_length(l) {}
	Attribute() {}
	bool isPrimary() const { return (m_flag & PRIMARY) != 0; }
	bool isUnique() const { return (m_flag & UNIQUE)  != 0; }
	bool isNotNull() const { return (m_flag & NOT_NULL)  != 0; }
	bool isIndex() const { return (m_flag & INDEX)  != 0; }
	void setIndex() { m_flag |= 0x08;}
	void unsetIndex() { m_flag &= 0x07;}
	friend std::istream& operator>>(std::istream& is, Attribute& attr);
	friend std::ostream& operator<<(std::ostream& os, const Attribute& attr);

	std::string m_name;
	DATA_TYPE m_type;
	size_t m_length;
	size_t m_flag;
	static const char PRIMARY = 1;
	static const char UNIQUE = 2;
	static const char NOT_NULL = 4;
	static const char INDEX = 8;
};

class Condition;
class Query
{
public:
//	std::vector<std::string> m_select;
	std::string m_tableName;
	std::vector<Condition> m_condition;
};
class Condition
{
public:
	Condition (){};
	Condition(const std::string& attrName, OP_TYPE opType, const std::string& operand):
		m_attrName(attrName), m_opType(opType), m_operand(operand) {}
	std::string m_attrName;
	OP_TYPE m_opType;
	std::string m_operand;
};

class Tuple
{
public:
	Tuple() {}
	Tuple(const Table& table, const std::string& content, int begin);
	Tuple(const std::vector<std::string>& strTuple);
	bool isSatisfied(const Table& table, const std::vector<Condition>& condition) const;

	std::vector<std::string> m_content;
private:
	bool judge(const DATA_TYPE valueType, const std::string& value, const Condition& condition) const;
	bool intJudge(const OP_TYPE opType, const std::string& opA, const std::string& opB) const;
	bool charJudge(const OP_TYPE opType, const std::string& opA, const std::string& opB) const;	
	bool floatJudge(const OP_TYPE opType, const std::string& opA, const std::string& opB) const;
};

inline void intTo4Bytes(int arg, std::string& ret, int start, int end)
{
	unsigned char cover = 255;
	for(int i = end - 1; i >= start; i --)
	{
		ret[i] = (char)(arg & cover);
		arg >>= 8;
	}
}

inline int fourBytesToInt(const std::string& content, int start, int end)
{
	int ret = 0;
	unsigned char cover = 255;
	for(int i = start; i < end; i ++)
	{
		ret <<= 8;
		ret += (content[i] & cover);
	}
	return ret;
}

inline std::string intToString(int num)
{
	std::stringstream ss;
	std::string ret;
	ss << num;
	ss >> ret;
	return ret;
}

inline std::string floatToString(float num)
{
	std::stringstream ss;
	std::string ret;
	ss << num;
	ss >> ret;
	return ret;	
}

inline int stringToInt(const std::string& str)
{
	std::stringstream ss;
	int ret;
	ss << str;
	ss >> ret;
	return ret;
}

inline float stringToFloat(const std::string& str)
{
	std::stringstream ss;
	float ret;
	ss << str;
	ss >> ret;
	return ret;
}

