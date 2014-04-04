#include "api.h"
#include "catalogman.h"
#include <algorithm>
#include <vector>
#include "record.h"
#include "catalogman.h"
#include "index.h"
#include "cli.h"
#include  <string>

using namespace std;

void printLine(std::vector<int>& cellWidth) {
        using namespace std;
        cout << "+";
        for (int i = 0; i < (int)cellWidth.size(); i++) {
            for (int j = 1; j <= cellWidth[i]; j++)
                cout << "-";
            cout << "+";
        }
        cout << endl;
}

void printHeader(std::vector<int>& cellWidth,Table& t){
	
    // 输出表头上边界
    printLine(cellWidth);
    // 输出表头内容
    cout << "|";
    for (size_t i = 0; i < cellWidth.size(); i++) {
        ostringstream oss;
        oss << cellWidth[i] - 2;
        string format = " %-";
        format += oss.str(); // "i to s"
        format += "s |";
        printf(format.c_str(), t.m_attr[i].m_name.c_str());
    }
    cout << endl;
    // 输出表头下边界（也就是第一行记录的上边界）
    printLine(cellWidth);
}

void API::selectTuple(const Query& query)
{


	CatalogMan::instance().selectCheck(query);
	m_table = CatalogMan::instance().getTable(query.m_tableName);
	m_query = query;
	m_cnt = 0;
	m_flag = false;
	m_blockBound = RecordMan::instance().getBound(query.m_tableName);
	initOffset();
	

	// 获取表头信息以供输出
    Table t = CatalogMan::instance().getTable(query.m_tableName);
    vector<int> cellWidth;
    int width;
    // 确定每一列的宽度（不含边界字符，宽度为字段大小、字段名称这二者的较大值 + 2）
    for (size_t i = 0; i < t.m_attr.size(); i++) {
        if (t.m_attr[i].m_name.length() > t.m_attr[i].m_length)
            width = t.m_attr[i].m_name.length() + 2;
        else
            width = t.m_attr[i].m_length + 2;
        cellWidth.push_back(width);
    }

	if(m_flag && m_offset.size()==0)
		cout<<"There is no such tuple in this table."<<endl;
	else
	while(API::instance().hasNext()) {

		vector<Tuple> tuples = API::instance().next();

		if(tuples.size()!=0)
		  	printHeader(cellWidth,t);
		else{
			cout<<"There is no such tuple in this table."<<endl;
			return;
		}


        for (size_t i = 0; i < tuples.size(); i++) {
            // 输出当前行内容
            cout << "|";
            for (size_t j = 0; j < cellWidth.size(); j++) {
                ostringstream oss;
                oss << cellWidth[j] - 2;
                string format = " %-";
                format += oss.str(); // "i to s"
                format += "s |";
                printf(format.c_str(), tuples[i].m_content[j].c_str());
            }
            cout << endl;
            // 输出当前行下边界
            printLine(cellWidth);
        }
	}

}

void API::initOffset()
{
	Table table = CatalogMan::instance().getTable(m_query.m_tableName);

	bool first = true;
	std::vector<Condition> m_con = m_query.m_condition;
	std::vector<Condition>::iterator it; 
	for (it = m_con.begin(); it != m_con.end();)
	{
		if( table.getAttribute(it -> m_attrName).isIndex() )
		{	
			m_flag = true;		
			vector<int> vtOffset = IndexMan::instance().find(table, *it);
			it = m_con.erase( it); // 剔除已在B+树中检索过的约束条件

			if( first )
			{
				m_offset = vtOffset;
				if(m_offset.size()>1)m_offset.pop_back();
				first = false;
			} else
			{
				m_offset = setIntersection(m_offset, vtOffset);
			}
		}
		else{
			m_flag = false;
			it++;
		}
	}
	m_query.m_condition = m_con;
}

vector<int> API::setIntersection(vector<int>& a, vector<int>& b)
{
	vector<int> ret(a.size());
	sort(a.begin(), a.end());
	sort(b.begin(), b.end());
	set_intersection(a.begin(), a.end(), b.begin(), b.end(), ret.begin());
	ret.resize( ret.size() );
	return ret;
}

bool API::hasNext()
{
	if( m_flag )
	{
		return (m_cnt < (int)m_offset.size());
	} else
	{
		return (m_cnt <= m_blockBound);
	}
}

vector<Tuple> API::next()
{
	if( m_flag )
	{
		return selectViaIndex();
	} else
	{
		return selectViaForeach();
	}
}

vector<Tuple> API::selectViaIndex()
{
	vector<int> target;
	int start = m_offset.front();
	/*
	vector<int> temp;
	temp = m_offset;
	for(auto it = m_offset.begin(); (it != m_offset.end()) && (*it - start) < 100 * BLOCK_SIZE;it++  ) // 100这个参数可以改的
	{
		target.push_back( *it );
		//m_offset.pop_front();
		//m_offset.erase(m_offset.begin());
		temp.pop_back();
	}
	m_offset = temp;
	*/
	m_cnt++;
	return RecordMan::instance().selectTuple(m_table, m_query.m_condition, m_offset);
}

vector<Tuple> API::selectViaForeach()
{
	return RecordMan::instance().selectTuple(m_table, m_query.m_condition, m_cnt ++);
}

void API::createTable(const string& tableName, const vector<Attribute>& vtAttr) const
{

	try{
	CatalogMan::instance().createTable(tableName, vtAttr);
	}catch (string errstr){throw;}

	RecordMan::instance().createTable(tableName,vtAttr);

}

void API::dropTable(const string& tableName) const
{
	CatalogMan::instance().dropTableCheck(tableName);
	Table table = CatalogMan::instance().getTable(tableName);
	vector<string> vtAttrName = table.getAttrNameIsIndex();
	CatalogMan::instance().dropTable(tableName); // dropIndex at the same time
	RecordMan::instance().dropTable(tableName); 
	IndexMan::instance().dropIndex(tableName, vtAttrName);
}


/******************* drop index indexName on tableName **********************************/
void API::dropIndex(const string& indexName) const
{
	CatalogMan::instance().dropIndexCheck(indexName);
	string tableName = CatalogMan::instance().getTableName(indexName);
	string attrName = CatalogMan::instance().getAttrName(indexName);

	/*if ( CatalogMan::instance().gonnaRemoveBPlusTree(indexName) == false )
	{
		CatalogMan::instance().dropIndex(indexName);
	} else */
	{
		CatalogMan::instance().dropIndex(indexName);		
		IndexMan::instance().dropIndex(tableName, attrName);
	}
}

/******************* create index indexName on tableName(attrName)  **********************/
void API::createIndex(const string& indexName, const string& tableName, const string& attrName) const
{
	CatalogMan::instance().createIndexCheck(indexName, tableName, attrName);

	if ( CatalogMan::instance().alreadyHasBPlusTree(tableName, attrName) )
	{
		return ;
	}

	Table table = CatalogMan::instance().getTable( tableName );
	Attribute attr = table.getAttribute(attrName);
	CatalogMan::instance().createIndex(indexName, tableName, attrName);

	size_t blockNum = 0;
	vector<pair<string, int> > info;
	while(true)
	{
		info.clear();
		info = RecordMan::instance().getOffsetInfo(table, attrName, blockNum);
		if(info.empty())
		{
			break;
		}
		IndexMan::instance().insertIndex(tableName, attr, info);
		blockNum ++;
	}
}

void API::insertTuple(const string& tableName, vector<string>& strTuple)
{
	CatalogMan::instance().insertTupleCheck(tableName, strTuple); // auto check hasTable
	Table table = CatalogMan::instance().getTable(tableName);
	int offset = RecordMan::instance().insertTuple(table, strTuple);

	if(offset == -1) 
		throw string("Insert failed: violation of the constraint 'unique'.");

	vector<pair<Tuple, int> > result;
	result.push_back( make_pair(Tuple(strTuple), offset ));
	fixIndex(table, result, true);
}

void API::deleteTuple(const Query& query)
{
	CatalogMan::instance().deleteTupleCheck(query);
	Table table = CatalogMan::instance().getTable(query.m_tableName);
	m_query = query;
	initOffset();
	vector<pair<Tuple, int> > result;
	if( m_flag == true )
	{
		result = RecordMan::instance().deleteTuple(table, query.m_condition, m_offset);
	} else
	{
		result = RecordMan::instance().deleteTuple(table, query.m_condition);
	}
	fixIndex(table, result, false);

}

void API::fixIndex(const Table& table, const vector<pair<Tuple, int> >& result,bool isInsert)
{
	auto it = table.m_attr.begin();
	for( ; it != table.m_attr.end(); it ++)
	{
		if(it -> isIndex() || it -> isPrimary() || it -> isUnique())
		{
			Attribute attr = table.getAttribute(it -> m_name);
			auto info = getInfo( table.getAttrNum(it->m_name), result );
			if ( isInsert )
			{
				IndexMan::instance().insertIndex( table.m_name, attr, info );
			} else
			{
				IndexMan::instance().deleteIndex( table.m_name, attr, info );
			}
		}
	}
}

vector<pair<string, int> > API::getInfo(const int locate, const vector<pair<Tuple, int> >& tuple) const
{
	vector<pair<string, int> > ret;
	for(auto it = tuple.begin(); it != tuple.end(); it ++)
	{
		ret.push_back( make_pair( it -> first.m_content[locate], it -> second) );
	}
	return ret;
}
