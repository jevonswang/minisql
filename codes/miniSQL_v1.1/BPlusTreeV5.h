#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <map>
#include <queue>
#include <cmath>
#include "buffer.h"
#include "global.h"

// V1, 13-10-04
// V2, 13-10-06, Ð´³Étemplate BPlusTree
// V3, 13-10-18, ½«InnerNode, LeafNodeÐÞ¸ÄÎªNode, ÒÔcombination(¶ø·Çinherit)µÄÐÎÊ½±íÊ¾½Úµã£»ÒýÈëshared_ptr¹ÜÀí½Úµã
// V4, 13-10-25, ÆúÓÃshared_ptr
// V5, 13-11-01, Ôö¼ÓinsertInValueBlock, removeInValueBlock·½·¨£¬ÊµÏÖ¶Ôµ¥ÊôÐÔ¶àÖµµÄÖ§³Ö

template<typename KEY>
class BPlusTree
{
public:
	class Node;
	BPlusTree(const std::string& tableName, const Attribute& attr);     // insert & delete Index
	~BPlusTree();

	void insert(const std::string& key, const int pointer) const;                                          // insert
	void remove(const std::string& key, const int pointer) const;          
	std::vector<int> find(const Condition& condition) const;

	void print() const;
private:
	bool alreadyExistBPlusTree() const;
	void createRootOnEmptyFile() const;

	Node findMinimalLeaf(const Node& node) const;
	std::vector<int> findEqual(const Condition& condition) const;
	std::vector<int> findNoequal(const Condition& condition) const;
	std::vector<int> findLess(const Condition& condition) const;
	std::vector<int> findGreater(const Condition& condition) const;
	std::vector<int> onFindNoLess(const Condition& condition) const;
	std::vector<int> onFindNoGreater(const Condition& condition) const;
	std::vector<int> findInValueBlock(int pointer) const;
	Node findNoLess(const Condition& condition, std::vector<int>& vtOut) const;
	Node findNoGreater(const Condition& condition, std::vector<int>& vtOut) const;
	Node find(const Node& node, const KEY& key) const;
	Node findInInner(const Node& node, const KEY& key) const;

	void onCreateIndex(const std::vector< std::pair<std::string, int> >& info) const; // called by ctor
	Node fetchRootFromFile() const;

	template<typename T> bool isEqual(const T& a, const T& b) const { return a == b; }
	template<> bool isEqual<float>(const float& a, const float& b) const { return fabs(a-b)<M_EPS; } // template specialization
	template<> bool isEqual<std::string>(const std::string& a, const std::string& b) const { return (strcmp(a.c_str(), b.c_str())==0); }
		bool isLastLeaf(const Node& node) const { return (node.m_last == M_ROOT_NUM); }
	bool hasKey(const Node& node, const KEY& key) const;

	void insertInParent(Node& node, const KEY& key, const int pointer) const;
	void insertInLeafWithNewKey(Node& leaf, const KEY& key, const int pointer) const;
	void insertInLeafWithoutNewKey(Node& leaf, const KEY& key, const int pointer) const;
	void insertInInner(Node& node, const KEY& key, const int pointer) const;
	int createLeftSibling(Node& root, const KEY& key, const int pointer) const;
	int createRightSibling(Node& left, KEY& keyOut) const;

	void removeEntry(Node& node, const KEY& key, const int pointer) const;
	void removeInNode(Node& node, const KEY& key, const int pointer) const;
	void removeInLeaf(Node& node, const KEY& key, const int pointer) const;
	void removeInInner(Node& node, const KEY& key, const int pointer) const;
	void makeChildTheNewRoot(Node& root) const;
	Node getSibling(const Node& a, const KEY& myKey, KEY& keyOut) const;	
	bool canMerge(const Node& a, const Node& b) const;
	void swap(Node& a, Node& b) const;
	void merge(Node& left, Node& right, const KEY& midKey) const;
	void mergeInLeaf(Node& left, Node& right) const;
	void mergeInInner(Node& left,Node& right, const KEY& key) const; 
	KEY borrowFromLeft(Node& left, Node& right, const KEY& oldMidKey) const;
	KEY borrowFromRight(Node& left, Node& right, const KEY& oldMidKey) const;	
	KEY getLeastKey(Node& node) const; // new added in 13-11-06
	void borrowFromLeft_inLeaf(Node& left, Node& right, const KEY& oldMidKey) const;
	void borrowFromLeft_inInner(Node& left, Node& right, const KEY& oldMidKey) const;
	void borrowFromRight_inLeaf(Node& left, Node& right, const KEY& oldMidKey) const;
	void borrowFromRight_inInner(Node& left, Node& right, const KEY& oldMidKey) const;
	void replaceKey(Node& node, const KEY& oldKey, const KEY& newKey) const;

	void insertInValueBlock(int &oldChildPtr, int pointer) const;                 // new added in V5
	void removeInValueBlock(int &oldChildPtr, int toBeRemoved) const; // new added in V5

	void updateChildren(const Node& parent) const;
	void updateChild(const Node& parent, Node& child) const;
	std::string nodeToString(const Node& node) const;
	Node blockToNode(const int blockNum) const;
	void nodeToBuffer(const Node& node) const;
	std::string keyToString( const KEY& key ) const;
	KEY stringToKey( const std::string& str ) const;

	void packHead(const Node& node, std::string& strOut) const;
	void packBody(const Node& node, std::string& strOut) const;
	void packKey(const Node& node, std::string& strOut, int& cur) const;
	void packChild(const Node& node, std::string& strOut, int& cur) const;
	void unpackHead(Node& node, const std::string& content) const;
	void unpackBody(Node& node, const std::string& content) const;
	void unpackKey(Node& node, const std::string& content, const int listSize) const;
	void unpackChild(Node& node, const std::string& content, const int listSize) const;
	void print(const Node& ) const;
	void printInLeaf(int child) const;

	std::string m_fileName;
	DATA_TYPE m_type;
	size_t m_length;
	size_t m_maxSize;
	mutable size_t m_valuePtr;

	class Node
	{
	public:
		Node(): m_valid(M_VALID) {}
		Node(const Node& node): 
			m_valid(node.m_valid), m_leaf(node.m_leaf), m_parent(node.m_parent),
			m_key(node.m_key), m_child(node.m_child), m_last(node.m_last), m_blockNum(node.m_blockNum){}
		bool isLeaf() const { return m_leaf == M_LEAF; }
		bool isRoot() const { return m_blockNum == 0; }
		bool isRootAndHasOneChild() const { return isRoot() && m_key.size() == 0; }
		bool hasKey(const KEY& key) const;
		bool hasEnoughSpace(size_t judge) const { return m_key.size() < judge; }
		bool hasTooFewKey(size_t judge) const { return m_key.size() < judge; }
		bool isOnRightOf(const Node& node) const { return m_key.front() > node.m_key.front(); }
		bool isOnRightOf(const Node& node, const KEY& myKey) const { return myKey > node.m_key.front(); }
		void setLeaf() { m_leaf = M_LEAF; }
		void unsetLeaf() { m_leaf = 0; }
		void setUnvalid() { m_valid = 0; }

		char m_valid;
		char m_leaf;
		int m_parent;
		std::list<KEY> m_key;
		std::list<int> m_child;
		int m_last;
		int m_blockNum;
	};
};
const static std::string M_PREFIX = ".\\index\\";
const static int M_NODE_HEAD_SIZE = 16;
const static double M_EPS = 0.0000001;
const static char M_VALID = 1;
const static char M_LEAF = 2;
const static int M_HEAD_SIZE = 16;
const static int M_ROOT_NUM = 0;
const static int M_VALUE_END = -1;

/*
template<typename KEY>
const double BPlusTree<KEY>::M_EPS = 0.0000001;
template<typename KEY>
const int BPlusTree<KEY>::M_NODE_HEAD_SIZE = 16;
template <typename KEY>
const std::string BPlusTree<KEY>::M_PREFIX = ".\\index\\";

template <typename KEY>
const int BPlusTree<KEY>::Node::M_ROOT_NUM = 0;
template <typename KEY>
const int BPlusTree<KEY>::Node::M_HEAD_SIZE = 16;
template <typename KEY>
const char BPlusTree<KEY>::Node::M_VALID = 1;
template <typename KEY>
const char BPlusTree<KEY>::Node::M_LEAF = 2;
*/

template<typename KEY>
BPlusTree<KEY>::BPlusTree(const std::string& tableName, const Attribute& attr):          // insert & delete Index
	m_fileName( tableName ), m_type( attr.m_type ), m_length( attr.m_length ),
	m_maxSize( (BLOCK_SIZE - M_HEAD_SIZE - 4) / (attr.m_length + 4) + 1 )
{
//	cout << "m_maxSize: " << m_maxSize << endl;
	if ( alreadyExistBPlusTree() )
	{
		string content;
		BufferMan::instance().read(m_fileName,M_ROOT_NUM, content);
		m_valuePtr = fourBytesToInt(content, 9, 13);
	} else
	{
		createRootOnEmptyFile();
	}
}

template<typename KEY>
bool BPlusTree<KEY>::alreadyExistBPlusTree() const
{
	ofstream out(m_fileName, ios::in | ios::out | ios::binary );
	if (out != 0)
	{
		out.close();
		return true;
	} else
	{
		out.close();
		return false;
	}
}

template<typename KEY>
void BPlusTree<KEY>::createRootOnEmptyFile() const
{
	Node root;
	root.m_valid = M_VALID;
	root.m_leaf = M_LEAF;
	root.m_parent = 0;
	root.m_last = 0;
	root.m_blockNum = 0;
	BufferMan::instance().write( m_fileName, nodeToString(root) );
	string content(BLOCK_SIZE, 0);
	BufferMan::instance().write( m_fileName, content );
	m_valuePtr = BLOCK_SIZE * 1 + M_NODE_HEAD_SIZE;
}


template<typename T> void vtSplice(std::vector<T> &dest,
		std::vector<T> &src) {
	typename std::vector<T>::iterator iter;
	for (iter = src.begin(); iter != src.end(); iter++)
		dest.push_back(*iter);
}





template<typename KEY>
BPlusTree<KEY>::~BPlusTree() // write m_valuePtr back to the file
{
	string rootContent;
	BufferMan::instance().read(m_fileName, M_ROOT_NUM, rootContent);
	intTo4Bytes(m_valuePtr, rootContent, 9, 13);
	BufferMan::instance().write(m_fileName, rootContent, M_ROOT_NUM);
}


template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::fetchRootFromFile() const
{
	return blockToNode(0);
}


template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::find(const Node& node, const KEY& key) const
{
	if(node.isLeaf())
	{
		return node;
	} else
	{
		return findInInner(node, key);
	}
}

template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::findInInner(const Node& node, const KEY& key) const
{
	auto itKey = node.m_key.begin();
	auto itChild = node.m_child.begin();
	for( ; itKey != node.m_key.end(); itKey ++, itChild++ )
	{
		if( key < *itKey )
		{
			return find( blockToNode(*itChild), key );
		}
	}
	return find( blockToNode(node.m_last), key );
}

template<typename KEY>
bool BPlusTree<KEY>::hasKey(const Node& node, const KEY& key ) const
{
	for( auto it = node.m_key.begin(); it != node.m_key.end(); it ++ )
	{
		//		if( key == *it )
		if( isEqual(key, *it) )
		{
			return true;
		}
	}
	return false;
}

template<typename KEY>
void BPlusTree<KEY>::insert(const std::string& keyStr, const int pointer) const
{
	KEY key = stringToKey( keyStr );

	Node& leaf = find( fetchRootFromFile() , key);
	if ( hasKey(leaf, key) )
	{
		insertInLeafWithoutNewKey(leaf, key, pointer);
		nodeToBuffer( leaf );
		return ;
	}

	if ( leaf.hasEnoughSpace(m_maxSize - 1) )
	{
		insertInLeafWithNewKey(leaf, key, pointer);
		nodeToBuffer( leaf );
	} else
	{
		insertInLeafWithNewKey(leaf, key, pointer);
		KEY midKey;
		int ptrToRight = createRightSibling( leaf,  midKey );
		insertInParent(leaf, midKey, ptrToRight);
	}
}

template<typename KEY>
void BPlusTree<KEY>::insertInParent(Node& node, const KEY& key, const int pointer) const
{
	if( node.isRoot() )
	{
		createLeftSibling( node, key, pointer);
	} else
	{
		Node parent = blockToNode( node.m_parent );
		if( parent.hasEnoughSpace( m_maxSize - 1 ) )
		{
			insertInInner(parent, key, pointer);
			nodeToBuffer(parent);
		} else
		{
			insertInInner(parent, key, pointer);
			KEY midKey;
			int ptrToRight = createRightSibling(parent, midKey);
			insertInParent( parent, midKey, ptrToRight );
		}
	}
}

template<typename KEY>
void BPlusTree<KEY>::insertInLeafWithNewKey(Node& leaf, const KEY&key, const int pointer) const
{
	auto itKey = leaf.m_key.begin();
	auto itChild = leaf.m_child.begin();
	for( ; itKey != leaf.m_key.end(); itKey ++, itChild ++ )
	{
		if( key < *itKey )
		{
			leaf.m_key.insert( itKey, key );
			int child = -1;
			insertInValueBlock( child, pointer );
			leaf.m_child.insert( itChild, child );
			return ;
		}
	}

	int child = -1;
	insertInValueBlock( child, pointer );
	leaf.m_key.push_back( key );
	leaf.m_child.push_back( child );
}

template<typename KEY>
void BPlusTree<KEY>::insertInLeafWithoutNewKey(Node& leaf, const KEY& key, const int pointer) const
{
	auto itKey = leaf.m_key.begin();
	auto itChild = leaf.m_child.begin();
	for( ; itKey != leaf.m_key.end(); itKey ++, itChild ++)
	{
		if ( isEqual(key, *itKey) )
		{
			insertInValueBlock( *itChild, pointer );
			return ;
		}
	}
	assert( false );
}

// ugly it may be, but it works : P... new added on 1st, Nov
template<typename KEY>
void BPlusTree<KEY>::insertInValueBlock(int& oldChildPtr, const int pointer) const
{
//	cout << m_valuePtr << endl;
//	cout << pointer << endl;
	string content(BLOCK_SIZE, 0);
	if ( (m_valuePtr % BLOCK_SIZE == 0) || (BLOCK_SIZE - (m_valuePtr % BLOCK_SIZE)) < 8 ) // insert operation takes 8 bytes
	{
		int blockNum = BufferMan::instance().write(m_fileName, content);
		m_valuePtr = BLOCK_SIZE * blockNum + M_NODE_HEAD_SIZE;
	}
	BufferMan::instance().read(m_fileName, m_valuePtr / BLOCK_SIZE, content); // ¶ÁÈë
	intTo4Bytes(pointer, content, m_valuePtr%BLOCK_SIZE, m_valuePtr%BLOCK_SIZE+4);
	intTo4Bytes(oldChildPtr, content, (m_valuePtr + 4)%BLOCK_SIZE, (m_valuePtr + 4)%BLOCK_SIZE+4);
	BufferMan::instance().write(m_fileName, content, m_valuePtr / BLOCK_SIZE); // Ð´»Ø
	oldChildPtr = m_valuePtr;
	m_valuePtr += 8;
}

template<typename KEY>
void BPlusTree<KEY>::insertInInner(Node& parent, const KEY&key, const int pointer) const
{
	auto itKey = parent.m_key.begin();
	auto itChild = parent.m_child.begin();
	for( ; itKey != parent.m_key.end(); itKey ++, itChild ++)
	{
		if( key < *itKey )
		{
			if( blockToNode(*itChild).isOnRightOf(blockToNode(pointer)) )
			{
				parent.m_key.insert( itKey, key );
				parent.m_child.insert( itChild, pointer );
			} else
			{
				parent.m_key.insert( itKey, key );
				parent.m_child.insert( itChild, *itChild );
				*itChild = pointer;
			}
			//			nodeToBuffer( parent );
			return ;
		}
	}

	if( blockToNode(parent.m_last).isOnRightOf(blockToNode(pointer)) )
	{
		parent.m_key.push_back( key );
		parent.m_child.push_back( pointer );
	} else
	{
		parent.m_key.push_back( key );
		parent.m_child.push_back( parent.m_last );
		parent.m_last = pointer;
	}
	//	nodeToBuffer( parent );
}

template<typename KEY>
int BPlusTree<KEY>::createLeftSibling(Node &root, const KEY& key, const int pointerToRight) const
{
	Node left;
	left.m_leaf = root.m_leaf;
	left.m_parent = root.m_blockNum;
	left.m_key.splice( left.m_key.begin(),
		root.m_key, root.m_key.begin(), root.m_key.end() );
	left.m_child.splice( left.m_child.begin(),
		root.m_child, root.m_child.begin(), root.m_child.end() );
	left.m_last = root.m_last;
	int blockNum = BufferMan::instance().write( m_fileName, nodeToString(left) );
	left.m_blockNum = blockNum;
	//	nodeToBuffer( left );
	updateChildren( left );
	root.m_key.push_back( key );
	root.m_child.push_back( left.m_blockNum );
	root.m_last = pointerToRight; // right of the rv(left)
	root.unsetLeaf();
	nodeToBuffer( root );
	return blockNum;
}

template<typename KEY>
int BPlusTree<KEY>::createRightSibling(Node &left, KEY& keyOut) const
{
	auto itKey = left.m_key.begin();
	auto itChild = left.m_child.begin();
	for(size_t i = 1; i <= m_maxSize/2; i ++)
	{
		itKey ++;
		itChild ++;
	}
	Node right;
	right.m_leaf = left.m_leaf;
	right.m_parent = left.m_parent;
	right.m_key.splice( right.m_key.begin(),
		left.m_key, itKey, left.m_key.end());
	right.m_child.splice( right.m_child.begin(),
		left.m_child, itChild, left.m_child.end());
	right.m_last = left.m_last;
	int blockNum = BufferMan::instance().write( m_fileName, nodeToString(right) );
	right.m_blockNum = blockNum;
	if( right.isLeaf() )
	{
		left.m_last = right.m_blockNum;		
		keyOut = right.m_key.front();
	} else
	{
		left.m_last = right.m_child.front();
		keyOut = right.m_key.front();
		right.m_child.pop_front();
		right.m_key.pop_front();
		nodeToBuffer(right);
	}
	nodeToBuffer( left );
	updateChildren( right );
	return blockNum;
}


template<typename KEY>
void BPlusTree<KEY>::updateChildren(typename const BPlusTree<KEY>::Node& parent) const
{
	if(parent.isLeaf())
	{
		return ;
	}
	for(auto it = parent.m_child.begin(); it != parent.m_child.end(); it ++)
	{
		Node child = blockToNode(*it);
		updateChild(parent, child);
	}
	Node child = blockToNode(parent.m_last);
	updateChild( parent, child );
}

template<typename KEY>
void BPlusTree<KEY>::updateChild(typename const BPlusTree<KEY>::Node& parent, Node& child) const
{
	child.m_parent = parent.m_blockNum;
	nodeToBuffer( child );
	//	BufferMan::instance().write( m_fileName, nodeToBlock(child), child.m_blockNum);
}

template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::blockToNode(int blockNum) const
{
	std::string content(BLOCK_SIZE, 0);
	BufferMan::instance().read( m_fileName, blockNum, content );
	Node ret;
	unpackHead( ret, content );
	unpackBody( ret, content );
	ret.m_blockNum = blockNum;
	return ret;
}

template<typename KEY>
std::string BPlusTree<KEY>::nodeToString( const Node& node ) const
{
	std::string ret(BLOCK_SIZE, 0);
	packHead( node, ret );
	packBody( node, ret );
	return ret;
}

template<typename KEY>
void BPlusTree<KEY>::nodeToBuffer(const Node& node) const
{
	BufferMan::instance().write( m_fileName, nodeToString(node), node.m_blockNum );
}

template<typename KEY>
void BPlusTree<KEY>::unpackHead(typename BPlusTree<KEY>::Node& node, const std::string& content) const
{
	assert( (content[0] & M_VALID) != 0 );
	node.m_valid = content[0] & M_VALID;
	node.m_leaf = content[0] & M_LEAF;
	node.m_parent = fourBytesToInt(content, 1, 5);		
}

template<typename KEY>
void BPlusTree<KEY>::unpackBody(typename BPlusTree<KEY>::Node& node, const std::string& content) const
{
	int listSize = fourBytesToInt(content, 5, 9);
	unpackKey(node, content, listSize);
	unpackChild(node, content, listSize);
	node.m_last = fourBytesToInt( content, M_HEAD_SIZE + listSize * (m_length + 4), M_HEAD_SIZE + listSize * (m_length + 4) + 4 );
}

template<typename KEY>
void BPlusTree<KEY>::unpackKey(typename BPlusTree<KEY>::Node& node, const std::string& content, const int listSize) const
{
	int cur = M_HEAD_SIZE;
	for(int i = 0; i < listSize; i ++)
	{
		node.m_key.push_back( stringToKey( content.substr(cur,  m_length).c_str() ) ); 
		cur += m_length;
	}
}

template<typename KEY>
void BPlusTree<KEY>::unpackChild(typename BPlusTree<KEY>::Node& node, const std::string& content, const int listSize) const
{
	int cur = M_HEAD_SIZE + listSize * m_length;
	for(int i = 0; i < listSize; i++)
	{
		node.m_child.push_back( fourBytesToInt(content, cur, cur + 4) );
		cur += 4;
	}
}

template<typename KEY>
void BPlusTree<KEY>::packHead(typename const BPlusTree<KEY>::Node& node, std::string& strOut) const
{
	strOut[0] = node.m_valid | node.m_leaf;
	intTo4Bytes(node.m_parent, strOut, 1, 5);
	intTo4Bytes(node.m_key.size(), strOut, 5, 9);
}

template<typename KEY>
void BPlusTree<KEY>::packBody(typename const BPlusTree<KEY>::Node& node, std::string& strOut) const
{
	int cur = M_HEAD_SIZE;
	packKey(node, strOut, cur);
	packChild(node, strOut, cur);
	intTo4Bytes(node.m_last, strOut, cur, cur + 4);
}

template<typename KEY>
void BPlusTree<KEY>::packKey(typename const BPlusTree<KEY>::Node& node, std::string& strOut, int &cur) const
{
	std::string key;
	for(auto it = node.m_key.begin(); it != node.m_key.end(); it ++)
	{
		key = keyToString(*it);
		size_t j = 0;
		assert( key.length() <= m_length );  // for dbg
		for(; j < key.length(); j ++)
		{
			strOut[ cur ++ ] = key[j];
		}
		while( (j++) < m_length )
		{
			strOut[ cur ++ ] = 0;
		}
	}
}

template<typename KEY>
void BPlusTree<KEY>::packChild(typename const BPlusTree<KEY>::Node& node, std::string& ret, int &cur) const
{
	for(auto itChild = node.m_child.begin(); itChild != node.m_child.end(); itChild ++)
	{
		intTo4Bytes(*itChild, ret, cur, cur + 4);
		cur += 4;
	}
}

template<typename KEY>
KEY BPlusTree<KEY>::stringToKey(const std::string& content) const
{
	KEY ret;
	std::stringstream ss;
	ss << content;
	ss >> ret;
	return ret;
}

template<typename KEY>
std::string BPlusTree<KEY>::keyToString(const KEY& key) const
{
	std::string ret;
	std::stringstream ss;
	ss << key;
	ss >> ret;
	return ret;
}

template<typename KEY>
void BPlusTree<KEY>::remove(const std::string& keyStr, const int pointer) const
{
//	std::cout << "gonna remove: " << keyStr << endl;
	KEY key =  stringToKey( keyStr );
	auto leaf = find( fetchRootFromFile(), key );
	if( hasKey(leaf, key) == false )
	{
		return;
	}
	removeEntry(leaf, key, pointer);
//	print();
//	cout << endl;
}

template<typename KEY>
void BPlusTree<KEY>::removeEntry(typename BPlusTree<KEY>::Node& node, const KEY& key, const int pointer ) const
{
	removeInNode(node, key, pointer);

	//	std::cout << key << ", " << pointer << endl;
	//	print();

	if( node.isRootAndHasOneChild() )
	{
		makeChildTheNewRoot( node );
	} else if( node.isRoot() == false && node.hasTooFewKey( (m_maxSize-1)/2 ) )
	{
		KEY midKey;
		Node sibling = getSibling(node, key, midKey); // 默认返回一个右兄弟
		Node parent = blockToNode(node.m_parent);
		if( canMerge(node, sibling) ) 
		{
			if( node.isOnRightOf(sibling, key) )
			{
				swap(sibling, node);
				int blockNum = node.m_blockNum;
				node.m_blockNum = sibling.m_blockNum;
				sibling.m_blockNum = blockNum;
			}
			merge( node, sibling, midKey );
			removeEntry( parent, midKey, sibling.m_blockNum );			
		} else // borrow
		{
			KEY newMidKey;
			if( node.isOnRightOf(sibling, key) )
			{
				newMidKey = borrowFromLeft(sibling, node,midKey);
			} else
			{
				newMidKey = borrowFromRight(node, sibling, midKey);
			}
			replaceKey( parent, midKey, newMidKey );
//			print();
		}
	}
}

template<typename KEY>
void BPlusTree<KEY>::removeInNode(typename BPlusTree<KEY>::Node& node, const KEY& key, const int pointer) const
{
	if( node.isLeaf() )
	{
		removeInLeaf( node, key, pointer );
	} else
	{
		removeInInner( node, key, pointer );
	}
}

template<typename KEY>
void BPlusTree<KEY>::removeInLeaf(typename BPlusTree<KEY>::Node& leaf, const KEY& key, const int toBeRemoved) const
{
	auto itKey = leaf.m_key.begin();
	auto itChild = leaf.m_child.begin();
	for( ; itKey != leaf.m_key.end(); itKey ++, itChild ++)
	{
		if( isEqual(key, *itKey) )
		{
			removeInValueBlock(*itChild, toBeRemoved);
			if (*itChild == M_VALUE_END)
			{
				leaf.m_key.erase( itKey );
				leaf.m_child.erase( itChild );
			}
			nodeToBuffer( leaf );
			return ;
		}
	}
	assert(false);
}

// ugly it may be, but it works : P... new added on 1st, Nov
template<typename KEY>
void BPlusTree<KEY>::removeInValueBlock(int &oldChildPtr, const int toBeRemoved) const
{
	if (oldChildPtr == M_VALUE_END)
	{
		return ;
	}

	string content;
	BufferMan::instance().read(m_fileName, oldChildPtr / BLOCK_SIZE, content);
	if ( fourBytesToInt(content, oldChildPtr%BLOCK_SIZE, oldChildPtr%BLOCK_SIZE + 4) != toBeRemoved )
	{
		int next = fourBytesToInt(content, oldChildPtr%BLOCK_SIZE+4, oldChildPtr%BLOCK_SIZE+8);
		removeInValueBlock(next, toBeRemoved);
		intTo4Bytes(next, content, oldChildPtr%BLOCK_SIZE+4, oldChildPtr%BLOCK_SIZE+8);
		BufferMan::instance().write(m_fileName, content, oldChildPtr / BLOCK_SIZE);
	} else // find it!
	{
		oldChildPtr = fourBytesToInt(content, oldChildPtr%BLOCK_SIZE + 4, oldChildPtr%BLOCK_SIZE + 8);
	}
}

template<typename KEY>
void BPlusTree<KEY>::removeInInner(typename BPlusTree<KEY>::Node& node, const KEY& key,  const int pointer) const
{
	for(auto itKey = node.m_key.begin() ; itKey != node.m_key.end(); itKey ++ )
	{
		//		if( key == *itKey )  // ½«constÊµ²Î·ÅÔÚ×ó±ß£¬¾Í²»»á³öÏÖ¸³Öµ¶ø·Ç±È½ÏµÄÐÐÎªÁË
		if( isEqual(key, *itKey) )
		{
			node.m_key.erase( itKey );
			break;
		}
	}
	for(auto itChild = node.m_child.begin(); itChild != node.m_child.end(); itChild ++)
	{
		if( pointer == *itChild )
		{
			node.m_child.erase( itChild );
			break;
		}
	}
	if( pointer == node.m_last )
	{
		node.m_last = node.m_child.back();
		node.m_child.pop_back();
	}
	nodeToBuffer(node);
}



template<typename KEY>
void BPlusTree<KEY>::makeChildTheNewRoot(typename BPlusTree<KEY>::Node& root) const
{
	int childBlockNum = (root.m_child.empty())? root.m_last: root.m_child.front();
	Node child = blockToNode( childBlockNum );
	root.m_child.clear();
	root.m_leaf = child.m_leaf;
	root.m_key.splice( root.m_key.begin(),
		child.m_key, child.m_key.begin(), child.m_key.end() );
	root.m_child.splice( root.m_child.begin(),
		child.m_child, child.m_child.begin(), child.m_child.end() );
	root.m_last = child.m_last;
	child.setUnvalid();
	updateChildren( root );
	nodeToBuffer( root );
	nodeToBuffer( child );
}

template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::getSibling(const Node& node, const KEY& myKey, KEY& keyOut) const
{
	Node parent = blockToNode( node.m_parent );
	auto itKey = parent.m_key.begin();                              
	auto itChild = parent.m_child.begin();
	for( ; itKey != parent.m_key.end(); itKey ++, itChild ++) // Ä¬ÈÏ·µ»ØÓÒÐÖµÜ
	{
		if( myKey <  *itKey )
		{
			keyOut = *itKey;
			if( itKey != (-- parent.m_key.end()) )
			{
				return blockToNode( * (++itChild) );
			} else
			{
				return blockToNode( parent.m_last );
			}
		}
	}
	keyOut = parent.m_key.back();
	return blockToNode( *(-- parent.m_child.end()) ); // ·µ»Ø×óÐÖµÜ
}

template<typename KEY>
bool BPlusTree<KEY>::canMerge(typename const BPlusTree<KEY>::Node& a, const Node& b) const
{
	if( a.isLeaf() )
	{
		return (a.m_key.size() + b.m_key.size()) <= (m_maxSize-1);
	} else
	{
		return (a.m_key.size() + b.m_key.size()) < (m_maxSize-1);
	}
} 

template<typename KEY>
void BPlusTree<KEY>::swap(typename BPlusTree<KEY>:: Node& left, Node& right) const
{
	auto tmpKey = left.m_key;
	auto tmpChild = left.m_child;
	int tmpLast = left.m_last;

	left.m_key = right.m_key;
	left.m_child = right.m_child;
	left.m_last = right.m_last;

	right.m_key = tmpKey;
	right.m_child = tmpChild;
	right.m_last = tmpLast;
}

template<typename KEY>
void BPlusTree<KEY>::merge(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& midKey) const
{
	if( left.isLeaf() )
	{
		mergeInLeaf(left, right);
	} else
	{
		mergeInInner(left, right, midKey);
	}
	right.setUnvalid();
	updateChildren( left );
	nodeToBuffer( left );
	nodeToBuffer( right );
}

template<typename KEY>
void BPlusTree<KEY>::mergeInLeaf(typename BPlusTree<KEY>::Node& left, Node& right) const
{
	left.m_key.splice( left.m_key.end(),
		right.m_key, right.m_key.begin(), right.m_key.end() );
	left.m_child.splice( left.m_child.end(),
		right.m_child, right.m_child.begin(), right.m_child.end() );
	left.m_last = right.m_last;
}

template<typename KEY>
void BPlusTree<KEY>::mergeInInner(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& midKey) const
{
	left.m_key.push_back( midKey );
	left.m_child.push_back( left.m_last );
	left.m_key.splice( left.m_key.end(),
		right.m_key, right.m_key.begin(), right.m_key.end() );
	left.m_child.splice( left.m_child.end(),
		right.m_child, right.m_child.begin(), right.m_child.end() );
	left.m_last = right.m_last;
}

template<typename KEY>
KEY BPlusTree<KEY>::borrowFromLeft(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& oldMidKey) const
{
//	KEY newMidKey = left.m_key.back();
	if( left.isLeaf() )
	{
		borrowFromLeft_inLeaf(left, right, oldMidKey);
	} else
	{
		borrowFromLeft_inInner(left, right, oldMidKey);
	}
	KEY newMidKey = getLeastKey( right );
	nodeToBuffer( left );
	nodeToBuffer( right );
	return newMidKey;
}

template<typename KEY>
void BPlusTree<KEY>::borrowFromLeft_inLeaf(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& noUse) const
{
	// ... false here!
	right.m_key.push_front( left.m_key.back() );	
	right.m_child.push_front( left.m_child.back() );
	left.m_key.pop_back();
	left.m_child.pop_back();
}

template<typename KEY>
void BPlusTree<KEY>::borrowFromLeft_inInner(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& oldMidKey) const
{
	right.m_key.push_front( oldMidKey );
	right.m_child.push_front( left.m_last );
	left.m_last = left.m_child.back();
	left.m_key.pop_back();
	left.m_child.pop_back();
	updateChild( right, blockToNode(right.m_child.front()) );
}

template<typename KEY>
KEY BPlusTree<KEY>::borrowFromRight(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& midKey) const
{
	if ( left.isLeaf() )
	{
		borrowFromRight_inLeaf(left, right, midKey);
	} else
	{
		borrowFromRight_inInner(left, right, midKey);
	}
//	KEY newMidKey = right.m_key.front();
	KEY newMidKey = getLeastKey( right ); // 要用新的key!!!!! 13-11-06
	nodeToBuffer( left );
	nodeToBuffer( right );
	return newMidKey;
}

template<typename KEY>
KEY BPlusTree<KEY>::getLeastKey(typename BPlusTree<KEY>::Node& node) const
{
	if ( node.isLeaf() )
	{
		return node.m_key.front();
	} else
	{
		return getLeastKey( (node.m_child.empty()==false)? blockToNode(node.m_child.front()): blockToNode(node.m_last) );
	}
}


// div, must clear dx?
template<typename KEY>
void BPlusTree<KEY>::borrowFromRight_inLeaf(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& oldMidKey) const
{
	left.m_key.push_back( oldMidKey );
	left.m_child.push_back( right.m_child.front() );
	right.m_key.pop_front();
	right.m_child.pop_front();
}

template<typename KEY>
void BPlusTree<KEY>::borrowFromRight_inInner(typename BPlusTree<KEY>::Node& left, Node& right, const KEY& oldMidKey) const
{
	left.m_key.push_back( oldMidKey );
	left.m_child.push_back( left.m_last );
	left.m_last = right.m_child.front();
	right.m_key.pop_front();
	right.m_child.pop_front();
	updateChild( left, blockToNode(left.m_last) );
}

template<typename KEY>
void BPlusTree<KEY>::replaceKey(typename BPlusTree<KEY>::Node& node, const KEY& oldKey, const KEY& newKey) const
{
	for(auto it = node.m_key.begin(); it != node.m_key.end(); it ++)
	{
		//		if( *it == oldKey )
		if( isEqual(*it, oldKey) )
		{
			*it = newKey;
			nodeToBuffer( node );
			return ;
		}
	}
	assert( false );
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::find(const Condition& condition) const
{
	switch(condition.m_opType)
	{
		case EQUAL:
			return findEqual(condition);
		case NO_EQUAL:
			return findNoequal(condition);
		case LESS:
			return findLess(condition);
		case GREATER:
			return findGreater(condition);
		case NO_LESS:
			return onFindNoLess(condition);
		case NO_GREATER:
			return onFindNoGreater(condition);
		default:
			return vector<int>();
	}
}

template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::findMinimalLeaf(const Node& node) const
{
	if( node.isLeaf() )
	{
		return node;
	} else
	{
		return findMinimalLeaf( blockToNode(node.m_child.front()) );
	}
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::findInValueBlock(int pointer) const
{
	vector<int> ret;
	string content;
	int former = pointer;
	BufferMan::instance().read(m_fileName, pointer/BLOCK_SIZE, content);
	while( pointer != M_VALUE_END )
	{
		if ( pointer / BLOCK_SIZE != former / BLOCK_SIZE ) // block number changed!
		{
			BufferMan::instance().read(m_fileName, pointer/BLOCK_SIZE, content);
		}
		ret.push_back( fourBytesToInt(content, pointer%BLOCK_SIZE, pointer%BLOCK_SIZE+4) );
		former = pointer;
		pointer = fourBytesToInt(content, pointer%BLOCK_SIZE+4, pointer%BLOCK_SIZE+8);
	}
	return ret;
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::findEqual(const Condition& condition) const
{
	KEY key = stringToKey(condition.m_operand);
	Node node = find( fetchRootFromFile(), key );

	auto itKey = node.m_key.begin();
	auto itChild = node.m_child.begin();
	for( ; itKey != node.m_key.end(); itKey ++, itChild ++ )
	{
//		cout << key << endl;
//		cout << "itKey: " << *itKey << endl;
		if( isEqual(key, *itKey) )
		{
			return findInValueBlock(*itChild);
		}
	}

	return vector<int>();
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::findNoequal(const Condition& condition) const 
    {
	KEY key = stringToKey( condition.m_operand );
//	std::cout << "key: " << key << endl;
	Node cur = findMinimalLeaf( fetchRootFromFile() );
	std::vector<int> vtOffset;

	while( true )
	{
		auto itKey = cur.m_key.begin();
		auto itChild = cur.m_child.begin();
		for( ; itKey != cur.m_key.end(); itKey ++, itChild ++ )
		{
			if( isEqual(key, *itKey) == false )
			{
				vector<int> tmp = findInValueBlock( *itChild ); // new added in V5
				vtSplice(vtOffset, tmp);
			}
		}

		if( isLastLeaf(cur) )
		{
			break;
		} else
		{
			cur = blockToNode( cur.m_last );
		}
	}

	return vtOffset;
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::findLess(const Condition& condition) const
{
	vector<int> vtOffset;
	Node cur = findNoGreater(condition, vtOffset);
	KEY key = stringToKey( condition.m_operand );

	auto itKey = cur.m_key.begin();
	auto itChild = cur.m_child.begin();
	for( ; itKey != cur.m_key.end(); itKey ++, itChild ++)
	{
		if( isEqual(key, *itKey) )
		{
			vtOffset.erase( -- vtOffset.end() );
			break;
		}
	}

	return vtOffset;
}

template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::findNoGreater(const Condition& condition, std::vector<int>& vtOut) const
{
	KEY key = stringToKey( condition.m_operand );
	Node cur = findMinimalLeaf( fetchRootFromFile() );

	while( true )
	{
		bool end = false;
		auto itKey = cur.m_key.begin();
		auto itChild = cur.m_child.begin();
		for( ; itKey != cur.m_key.end(); itKey ++, itChild ++ )
		{
			if ( *itKey < key || isEqual(*itKey, key) )
			{
				vector<int> tmp = findInValueBlock( *itChild ); // new added in V5
				vtSplice(vtOut, tmp);
			} else
			{
				end = true;
				break;
			}
		}

		if ( end == true || isLastLeaf(cur) )
		{
			break;
		} else
		{
			cur = blockToNode( cur.m_last );
		}
	}

	return find( fetchRootFromFile(), key );
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::findGreater(const Condition& condition) const
{
	vector<int> vtOffset;
	Node cur = findNoLess(condition, vtOffset);
	KEY key = stringToKey( condition.m_operand );

	auto itKey = cur.m_key.begin();
	auto itChild = cur.m_child.begin();
	for( ; itKey != cur.m_key.end(); itKey ++, itChild ++)
	{
		if( isEqual(key, *itKey) )
		{
			vtOffset.erase( vtOffset.begin() );
			break;
		}
	}

	return vtOffset;
}

template<typename KEY>
typename BPlusTree<KEY>::Node BPlusTree<KEY>::findNoLess(const Condition& condition, std::vector<int>& vtOut) const 
{
	KEY key = stringToKey(condition.m_operand);
	Node cur = find( fetchRootFromFile(), key );
	Node ret = cur;

	auto itKey = cur.m_key.begin();
	auto itChild = cur.m_child.begin();
	for( ; itKey != cur.m_key.end(); itKey ++, itChild ++ )
	{
		if ( *itKey > key || isEqual(*itKey, key) )
		{
			vector<int> tmp = findInValueBlock( *itChild ); // new added in V5
			vtSplice(vtOut, tmp);
		}
	}

	while( isLastLeaf(cur) == false )
	{
		cur = blockToNode( cur.m_last );
		for(itChild = cur.m_child.begin(); itChild != cur.m_child.end(); itChild ++)
		{
			vector<int> tmp = findInValueBlock( *itChild ); // new added in V5
			vtSplice(vtOut, tmp);
		}
	}

	return ret;
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::onFindNoLess(const Condition& condition) const
{
	vector<int> ret;
	findNoLess( condition, ret );
	return ret;
}

template<typename KEY>
std::vector<int> BPlusTree<KEY>::onFindNoGreater(const Condition& condition) const
{
	vector<int> ret;
	findNoGreater(condition, ret);
	return ret;
}

template<typename KEY>
void BPlusTree<KEY>::print() const
{
	print( fetchRootFromFile() );
}

template<typename KEY>
void BPlusTree<KEY>::print(const Node& node) const
{
	std::string descritor = (node.isLeaf())? "Leaf, ": "Inner, ";
	std::cout << descritor << "bNum is " << node.m_blockNum << ", \nkeys: " ;
	auto itKey = node.m_key.begin();
	auto itChild = node.m_child.begin();
	for( ; itKey != node.m_key.end(); itKey ++)
	{
		cout << *itKey << ", ";
	}
	std::cout << "\npointers: ";
	bool first = true;
	for( ; itChild != node.m_child.end(); itChild ++)
	{
		if( node.isLeaf() )
		{
			if ( first == false )
			{
				std::cout << ", ";
			}
			first = false;
			printInLeaf( *itChild );
		} else
		{
			if (first == false)
			{
				std::cout << ", ";
			}
			first = false;
			cout << *itChild;
		}
	}
	if(first == false)
		std::cout <<", ";
	std::cout << node.m_last << endl;

	if( node.isLeaf() == false )
	{
		for(auto itChild = node.m_child.begin(); itChild != node.m_child.end(); itChild ++)
		{
			print( blockToNode(*itChild) ); 
		}
		print( blockToNode( node.m_last) );
	}
}

template<typename KEY>
void BPlusTree<KEY>::printInLeaf(int child) const
{
	std::cout << "(";
	bool first = true;
	while( child != M_VALUE_END )
	{
		std::string content;
		BufferMan::instance().read(m_fileName, child/BLOCK_SIZE, content);
		if(first == false)
		{
			std::cout << ", ";
		}
		first = false;
		std::cout << fourBytesToInt(content, child%BLOCK_SIZE, (child)%BLOCK_SIZE+4);
		child = fourBytesToInt(content, (child+4)%BLOCK_SIZE, (child+4)%BLOCK_SIZE+4);
	}
	std::cout << ")";
}

// combination vs inherit
// readable(modulize) vs abstract(efficience)