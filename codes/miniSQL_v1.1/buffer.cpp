#include "buffer.h"
#include <iostream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <fstream>

using namespace std;

BufferMan::BufferMan()
{
	try
	{
		init();
	} catch(...)
	{
		cout << "init failed!" << endl;
		abort();
	}
}

BufferMan::~BufferMan()
{
	for(auto it = m_used.begin(); it != m_used.end(); it ++)
	{
		if((*it) -> m_dirty)
		{
			File::write( *it );
		}
		delete *it;
	}
}

bool BufferMan::read(const string& fileName, const int blockNum, string& strOut)               // 返回可能用于判断的bool
{
	if( isInBuffer(fileName, blockNum) )
		return readBlock(fileName, blockNum, strOut);     // 在buffer中，从块中读
	else
		return fileToBlock(fileName, blockNum, strOut);   // 不在buffer中，从文件中读
}

// new added in 13-10-19 19:20
int BufferMan::readLast(const std::string& fileName, std::string& strOut)           // 返回块号
{
	return File::readLast(fileName, strOut);
}

// fixed at 13-10-18, ret changed!
int BufferMan::write(const string& fileName, const string& content, const int blockNum)     // 返回写入的block, 用于b+tree
{
	string noUse;
	if(blockNum != -1)                                                            // 指向性地写
	{
		if( isInBuffer(fileName, blockNum) == false )
			fileToBlock(fileName, blockNum, noUse);           // 不在buffer中，将文件中对应的块读入buffer (传入的noUse是为与read接口兼容)
		writeBlock(fileName, blockNum, content);             // 将content写入buffer中的那块
		return blockNum;
	} else
	{
		int num;
		File::write(fileName, content, num);                         // 非指向性地写，写追加到文件尾，传出参数num标识块号
		fileToBlock(fileName, num, noUse);                         // 不在buffer中，将文件中对应的块读入buffer (传入的noUse是为与read接口兼容)
		return num;
	}
}

void BufferMan::remove(const string& fileName)
{
	std::string cmd( string("del ") + fileName );
	system( cmd.c_str() );
	removeInMap( fileName );
	removeInList( fileName ); // 由removeInList delete!
}

/*
// new added in 13-10-19 19:00
void BufferMan::remove(const string& fileName, const int blockNum)
{
	Block* target = m_map[ getKey(fileName, blockNum) ];
	assert( target != NULL );
	removeInMap(fileName, blockNum);
	removeInList(target);
	delete target;
}*/


// public接口 以上。//////////////////////////////////////////////////////////////////////////
void BufferMan::removeInMap(const string& fileName)
{
	for(auto it = m_map.begin() ; it != m_map.end(); )
	{
		if( it -> first.find( fileName + string("@")) == 0 )
		{
			auto toBeErase = it ++;
			m_map.erase( toBeErase );
		} else
		{
			it ++;
		}
	}	
}

void BufferMan::removeInList(const string& fileName) // 调用removeInList前必须确定已经从map中移走
{
	for(auto it = m_used.begin(); it != m_used.end(); )
	{
		if( (*it) -> m_fileName == fileName )
		{
			delete *it;
			auto toBeErase = it ++;
			m_used.erase( toBeErase );
		} else
		{
			it ++;
		}
	}
}

void BufferMan::init()
{
}

bool BufferMan::isInBuffer(const string& fileName, const int blockNum)
{
	const string key = getKey(fileName, blockNum);     // 获得文件名+块号对应的键值
	return m_map.count(key) == 1;                   // m_map中存有该键则返回true
}

bool BufferMan::readBlock(const string& fileName, const int blockNum, string& strOut) // 被read接口调用
{
	const string key = getKey(fileName, blockNum);
	if( m_map[key] -> isPin() == false )                              // 在该块非常驻块时，进行LRU策略维护
	{
		m_used.remove(m_map[key]);                                 
		m_used.push_back(m_map[key]);
	}
	strOut = m_map[key] -> m_content;
	return true;
}

bool BufferMan::fileToBlock(const string& fileName, const int blockNum, string& strOut)
{
	char* dst = new char[BLOCK_SIZE];                                           // 读文件的传出参数
	if( File::read(fileName, blockNum, dst) == false)       // 读失败(blockNum大于文件的块总数时)
	{
		delete[] dst;
		return false;
	} else
	{
		strOut = string(dst, BLOCK_SIZE);
		if( isFull() ) 
		{
			replace(fileName, blockNum, strOut);                       // 块数达到上限，进行LRU替换
	  	                                                               // 用dst怎么出错了!
		} else         
		{ 
			newBlock(fileName, blockNum, strOut);                   // 块数并未达到上限，创建新的used节点 
		}
		delete[] dst;
		return true;
	}
}

bool BufferMan::writeBlock(const string& fileName, const int blockNum, const string& content)
{
	assert( isInBuffer(fileName, blockNum) == true );                 // for dbg
	string key = getKey(fileName, blockNum);
	if(m_map[key] -> m_content != content)                                // optimized in 13-10-20
	{
		m_map[key] -> m_content = content;									// 修改该块内容
		m_map[key] -> m_dirty = true;												// 设置dirty bit
	}
	return true;
}

string BufferMan::getKey(const string& fileName, const int blockNum) const
{
	stringstream tmp;
	string num;  
	tmp << blockNum;
	tmp >> num;
	return (fileName + string("@") + num);                                               // 返回在m_map中对应的键值
}

bool BufferMan::isFull()
{
	return m_used.size() == BLOCK_TOT;
}

bool BufferMan::replace(const string& fileName, const int blockNum, const string& content)
{
	cout << "here is BufferMan::replace()" << endl;
	Block* cur;
	while( (cur = m_used.front()) -> isPin() )                        // 找到非常驻块cur
	{
		m_used.pop_front();
		m_used.push_back(cur);
	}
	if(cur -> isDirty())
	{
		cur -> writeToFile();
	}
	string oldKey = cur -> getKey();
	string newKey = getKey(fileName, blockNum); 
	cur -> update(fileName, blockNum, content);              // 更新内容
	m_map.erase( m_map.find(oldKey) );                            // 维护map
	m_map.insert( make_pair(newKey, cur) );
	return true;
}

bool BufferMan::newBlock(const string& fileName, const int blockNum, const string& source)
{
	Block* block = new Block();
	block -> update(fileName, blockNum, source);
	m_used.push_back(block);                                              // 将新块指针插入链表
	m_map.insert( make_pair(block->getKey(), block) ); // 维护map
	return true;
}

// 嵌套类。/////////////////////////////////////////////////////////////////////////////////////
BufferMan::Block::Block(): m_content(BLOCK_SIZE, 0)  // 
{
	m_dirty = false;
	m_blockNum = -1;
}

bool BufferMan::Block::isDirty() const
{
	return m_dirty;
}

bool BufferMan::Block::isPin() const
{
	return m_pin;
}

bool BufferMan::Block::writeToFile() const
{
	return File::write(this);
}

string BufferMan::Block::getKey() const                           // 返回在BuuferMan::m_map中对应的键值
{
	stringstream tmp;
	string num;  
	tmp << m_blockNum;
	tmp >> num;
	return (m_fileName + string("@") + num);
}

void BufferMan::Block::update(const string& fileName, const int blockNum, const string& source)
{
	m_dirty = false;
	m_pin = false;
	this -> m_fileName = fileName;
	this -> m_blockNum = blockNum;
	this -> m_content = source;
}

bool BufferMan::File::read(const string& fileName, const int blockNum, char*& dst)
{
	ifstream in(fileName, ios::binary | fstream::ate);

	if(in == 0)
	{
		File::createFile(fileName);                                            // new added in 13-10-18, for bplustree
		in.open(fileName, ios::binary | fstream::ate);
	}
	if((size_t)in.tellg() <= blockNum * BLOCK_SIZE)          // 文件中不存在该块(可能出现的错误读操作) ==, <=?
	{
		in.close();
		return false;
	}
	in.seekg(blockNum * BLOCK_SIZE);
	in.read(dst, BLOCK_SIZE);
	in.close();

	return true;
}

bool BufferMan::File::write(const Block* cur)        // 被Block::writeToFile调用，将dirty的块内容写回文件
{
	ofstream inOut(cur -> m_fileName, ios::in | ios::out | ios::binary) ; // 写覆盖
/*	if( inOut == 0)
	{
		File::createFile(cur -> m_fileName);                           // 创建个新文件 : P for bplustree
		inOut.open(cur -> m_fileName, ios::in | ios::out | ios::binary);
	}*/
	inOut.seekp(BLOCK_SIZE * cur -> m_blockNum);
	inOut.write( cur -> m_content.c_str(), BLOCK_SIZE);  // sth wrong here!!!
	inOut.close();

	return true;
}

bool BufferMan::File::write(const string& fileName, const string& content, int& num)
{
	ofstream out(fileName, ios::in | ios::out | ios::binary );
	                                                                                                                // 写追加到文件尾
	if( out == 0)
	{
		File::createFile(fileName);                                                               // 创建个新文件 : P for bplustree
		out.open(fileName, ios::in | ios::out | ios::binary );
	}

	assert( out != 0 );
	out.seekp(0, ios_base::end);
	long end = out.tellp();
	num = (end / BLOCK_SIZE);                                                                // 将(块)尾号作为传出参数
	string toBeWrite(BLOCK_SIZE, 0);                                                     // ensure the rest of the 'toBeWrite' is 0
	toBeWrite = content;
	out.seekp( num * BLOCK_SIZE );
	out.write( toBeWrite.c_str(), BLOCK_SIZE);
	out.close();

	return true;
}

int BufferMan::File::readLast(const string& fileName, string& strOut)
{
	ifstream in(fileName, ios::binary);
	assert( in != 0 );
	in.seekg(-4096, ios_base::end);                                          // BLOCK_SIZE为无符号数，不可使用-BLOCK_SIZE
	const long target = in.tellg();
	char dst[BLOCK_SIZE];
	in.read(dst, BLOCK_SIZE);
	strOut = string(dst, BLOCK_SIZE);
	return (target / BLOCK_SIZE);                                              // 将(块)尾号作为传出参数
}

void BufferMan::File::createFile(const string& fileName)
{
	fstream out(fileName, ios::app);
	out.close();
}

/*
string Block::getContent() const
{
	return content;
}

string Block::getFileName() const
{
	return fileName;
}

int Block::getBlockNum() const
{
	return blockNum;
}

void Block::setContent(const string& content)
{
	this.content = content;
}*/