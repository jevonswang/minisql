#include "bplustreeV5.h"
#include "index.h"

#include <fstream>
#include <memory>
#include <map>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>

using namespace std;

void createIndex();
void insertIndex();
void removeIndex();
void dropIndex();

const int NUM = 100;
const int RANGE = 100;
const int LENGTH = 1300;

void query();
void print(list<int> list)
{
	for(auto it = list.begin(); it != list.end(); it ++)
	{
		cout << *it << " ";
	}
	cout << endl << endl;
}

int main()
{
	time_t begin = time(0);

	createIndex();
	removeIndex();
	dropIndex();
//	insertIndex();
//	query()
	time_t end = time(0);
	cout << "end - begin: " << end - begin << endl; 
	int x; cin >> x;
	return 0; 
}


void ascendingOrder( vector<pair<string, int>>& info , const int start, const int end)
{
	for(int i = start; i < end; i ++)
	{
		float fnum =(float)( (i * 1.) / RANGE);
		info.push_back( make_pair( floatToString(fnum), i ) );
	}
}
void descendingOrder( vector<pair<string, int>>& info, const int start, const int end)
{
	for(int i = start; i >= end; i --)
		info.push_back( make_pair( intToString(i), i ) );
}
void randomOrder( vector<pair<string, int>>& info)
{
	for(int i = 0; i < NUM; i ++)
	{
		int num = rand() % RANGE;
		float fnum =(float)( (num * 1.0) / RANGE);
		info.push_back( make_pair( floatToString( fnum ), num ) );
	}
}
void createIndex()
{
	cout << "here is createIndex() " << endl;
	ofstream out(".\\index\\test@");
	out.close();
	string tableName = "test";
	Attribute attr( FLOAT, LENGTH );
	vector<pair<string, int>> info;
	ascendingOrder(info, 0, RANGE);
	IndexMan::instance().createIndex(tableName, attr);
	IndexMan::instance().insertIndex(tableName, attr, info);
}

void print(const vector<int>& vt)
{
	for(auto it = vt.begin(); it != vt.end(); it ++)
	{
		cout << *it << " ";
	}
	cout << endl << endl;
}

/*void insertIndex()
{
	cout << "here is insertIndex() " << endl;
	string tableName = ".\\index\\test";
	Attribute attr( DATA_TYPE::FLOAT, LENGTH );	
	BPlusTree<float> tree(tableName, attr);
	vector<pair<string, int>> info;
	randomOrder(info);

	for(auto it = info.begin(); it != info.end(); it ++)
	{
		tree.insert( it -> first, it -> second );
	}
	tree.print();
}*/


void removeIndex()
{
	cout << "here is removeIndex()" << endl;
	string tableName = "test";
	Attribute attr( FLOAT, LENGTH );
	vector<string> info;
	BPlusTree<float> tree(tableName, attr);
	for(int i = RANGE/2; i <= RANGE; i ++)
	{
		float fnum =(float)( (i * 1.) / RANGE);
		info.push_back( floatToString(fnum) );
	}

	IndexMan::instance().deleteIndex(tableName, attr,info);
}

void dropIndex()
{
	cout << "here is dropIndex()" << endl;
	vector<Attribute> vtAttrName;
	vtAttrName.push_back( Attribute(FLOAT, LENGTH) );
	IndexMan::instance().dropIndex( string("test"),  vtAttrName);
}

void query()
{
	string tableName = ".\\index\\test";
	Attribute attr(FLOAT, LENGTH );
	BPlusTree<float> tree(tableName, attr);

	cout << "equal: " << floatToString(0.1f) << endl;
	Condition equal;
	equal.m_opType = EQUAL;
	equal.m_operand = floatToString(0.1f);
	auto result =tree.find( equal );
	print( result );
	cout << endl;

	cout << "No equal: " << floatToString(0.498f) << endl ;
	Condition noEqual;
	noEqual.m_opType = NO_EQUAL;
	noEqual.m_operand = floatToString(0.498f);
	result = tree.find( noEqual );
	print( result );

	cout << "Less: " << floatToString(0.050f) << endl;
	Condition less;
	less.m_opType = LESS;
	less.m_operand = floatToString( 0.050f );
	result = tree.find( less );
	print( result );

	cout << "no Greater: " << floatToString(0.050f) << endl;
	Condition noGreater;
	noGreater.m_opType = NO_GREATER;
	noGreater.m_operand = floatToString( 0.050f );
	result = tree.find( noGreater );
	print( result );

	cout << "Greater: " << floatToString(0.450f) << endl;
	Condition greater;
	greater.m_opType = GREATER;
	greater.m_operand = floatToString( 0.450f );
	result = tree.find( greater );
	print( result );

	cout << "no Less: " << floatToString(0.450f) << endl;
	Condition noLess;
	noLess.m_opType = NO_LESS;
	noLess.m_operand = floatToString(0.450f);
	result = tree.find(noLess);
	print( result );
}