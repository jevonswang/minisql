#include "global.h"
#include <iostream>
#include <cmath>

using namespace std;

Attribute Table::getAttribute(const string& attrName) const
{
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if(it -> m_name == attrName)
		{
			return *it;
		}
	}
}

vector<string> Table::getAttrNameIsIndex() const
{
	vector<string> ret;
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if (it->isUnique() || it->isIndex() || it->isPrimary())
		{
			ret.push_back(it -> m_name);
		}
	}
	return ret;
}

size_t Table::size() const
{
	size_t size = 0;
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		size += it -> m_length;
	}
	return size;
}

size_t Table::getAttrNum(const string& attrName) const 
{
	size_t num = 0;
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if( attrName == it -> m_name )
		{
			return num;
		}
		num ++;
	}
	assert( false );
	return 0;
}

size_t Table::getAttrBegin(const string& attrName) const
{
	size_t begin = 0;
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if( attrName == it -> m_name )
		{
			return begin;
		} 
		begin += it -> m_length;
	}
	assert( false );
	return 0;
}

size_t Table::getAttrEnd(const string& attrName) const
{
	size_t end = 0;
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		end += it -> m_length;
		if( attrName == it -> m_name )
		{
			return end;
		} 
	}
	assert( false );
	return 0;
}

DATA_TYPE Table::getAttrType(const string& attrName) const
{
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if( attrName == it -> m_name )
		{
			return it -> m_type;
		} 
	}
	assert( false );	
	return INT;
}

void Table::setIndex(const string& attrName)
{
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if(it -> m_name == attrName)
		{
			return it -> setIndex();
		}
	}
}

void Table::unsetIndex(const string& attrName)
{
	for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
	{
		if(it -> m_name == attrName)
		{
			return it -> unsetIndex();
		}
	}
}

Tuple::Tuple(const Table& table, const string& content, int begin)
{
	for(auto it = table.m_attr.begin(); it != table.m_attr.end(); it ++)
	{
		m_content.push_back( content.substr(begin, it -> m_length).c_str() ); // substr�Ĳ�����pos, len!!!
		begin += it -> m_length;
	}
}

Tuple::Tuple(const vector<string>& strTuple)
{
	for(auto it = strTuple.begin(); it != strTuple.end(); it ++)
	{
		m_content.push_back(*it);
	}
}

bool Tuple::isSatisfied(const Table& table, const vector<Condition>& condition) const
{
	for(auto itCon = condition.begin(); itCon != condition.end(); itCon ++)
	{
		int attrNum = table.getAttrNum( itCon -> m_attrName );
		DATA_TYPE attrType = table.getAttrType( itCon -> m_attrName );
		if( judge(attrType, m_content[attrNum], *itCon) == false )
		{
			return false;
		}
	}
	return true; // û����ֵʱ��Ȼ����true...
}

bool Tuple::judge(const DATA_TYPE operandType, const string& operandA, const Condition& condition) const
{
	switch( operandType )
	{
		case INT:
			return intJudge(condition.m_opType, operandA, condition.m_operand);
		case CHAR:
			return charJudge(condition.m_opType, operandA, condition.m_operand);
		case FLOAT:
			return floatJudge(condition.m_opType, operandA, condition.m_operand);
		default:
			return false; // soft complior
	}	
}


bool Tuple::intJudge(const OP_TYPE opType, const string& operandA, const string& operandB) const
{
	switch(opType)
	{
		case EQUAL:
			return stringToInt(operandA) == stringToInt(operandB);
		case NO_EQUAL:
			return stringToInt(operandA) != stringToInt(operandB);
		case LESS:
			return stringToInt(operandA) < stringToInt(operandB);
		case GREATER:
			return stringToInt(operandA) > stringToInt(operandB);
		case NO_GREATER:
			return stringToInt(operandA) <= stringToInt(operandB);
		case NO_LESS:
			return stringToInt(operandA) >= stringToInt(operandB);
		default:
			return false; // soft complior
	}
}

bool Tuple::charJudge(const OP_TYPE opType, const string& operandA, const string& operandB) const
{
	string opA(operandA.c_str()); // left out the 0s in the end
	switch(opType)
	{
		case EQUAL:
			return opA == operandB;
		case NO_EQUAL:
			return opA != operandB;
		case LESS:
			return opA < operandB;
		case GREATER:
			return opA > operandB;
		case NO_GREATER:
			return opA <= operandB;
		case NO_LESS:
			return opA >= operandB;
		default:
			return false; // soft complior
	}
}


bool Tuple::floatJudge(const OP_TYPE opType, const string& operandA, const string& operandB) const
{
	switch(opType)
	{
		case EQUAL:
			return fabs( stringToFloat(operandA) - stringToFloat(operandB) ) < EPS;
		case NO_EQUAL:
			return fabs( stringToFloat(operandA) - stringToFloat(operandB) ) >= EPS;
		case LESS:
			return stringToFloat(operandA) < stringToFloat(operandB);
		case GREATER:
			return stringToFloat(operandA) > stringToFloat(operandB);
		case NO_GREATER:
			return stringToFloat(operandA) <= stringToFloat(operandB);
		case NO_LESS:
			return stringToFloat(operandA) >= stringToFloat(operandB);
		default:
			return false; // soft complior
	}
}
