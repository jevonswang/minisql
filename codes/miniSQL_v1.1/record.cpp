#include "record.h"
#include "global.h"
#include <fstream>
#include <iostream>

bool RecordMan::createTable(const string& tableName, const vector<Attribute>& vtAttr){//创建表
	string filename = tableName + ".table";
	string empty_block(BLOCK_SIZE, 0);
	int size=0;//tuple_size
	for(int i=0; i<vtAttr.size(); i++){
		size+=vtAttr[i].m_length;
	}
	size++;
	int offset=0;
	while((offset+size)<BLOCK_SIZE){
		empty_block.replace(offset+size-1,1,"1");
		//string str=empty_block.substr(offset,size);		
		//string dirty = str.substr(size-1,1);
		//if(dirty=="1") return offset;
		offset+=size;
	}
	BufferMan::instance().write(filename, empty_block);/*2013-11-12*/
	return true;
}

bool RecordMan::dropTable(const std::string& tableName){//删除表
	std::string filename=tableName+".table";
	BufferMan::instance().remove(filename.c_str());//BufferMan::remove()改过到V2013-10-31就不会报错了
	return true;
}

vector<Tuple> RecordMan::selectTuple(const Table& table,const vector<Condition>& condition,int blockNum){//selectForeach
	string filename = table.m_name+".table";
	vector<Tuple> selected;//存放结果的vector
	size_t size = table.size()+1;

	for(int i=0;i<=blockNum;i++){//遍历所有块，取到等号吗？！
		string strOut;
		BufferMan::instance().read(filename, i, strOut);	//读出block的内容
		size_t pointer = 0;
		while((pointer+size) < strOut.size()){//解析strOut中的字符串内容
			string substring = strOut.substr(pointer,size);
			vector<string> vec = tupleToAttr(table.m_attr, substring);
			Tuple* tmp = new Tuple(vec);
			//bool RecordMan::isValid(int size, string str){
			if((isValid(size,substring)==true)&&(tmp->isSatisfied(table, condition)==true)){//该tuple符合要求/*20130-11-12*/
				selected.push_back(*tmp);
			}else{//不合要求，删除临时变量
				delete tmp;
			}
			pointer += size;
		}
	}
	return selected;
}

vector<Tuple> RecordMan::selectTuple(const Table& table,const vector<Condition>& condition, vector<int> m_offset){//selectViaIndex
	string filename = table.m_name+".table";
	vector<Tuple> selected;
	size_t size = table.size()+1;

	for(auto it=m_offset.begin(); it!=m_offset.end(); it++){//遍历所有offset
		string strOut;
		BufferMan::instance().read(filename, (*it)/BLOCK_SIZE, strOut);//取出block内容
		string substring;
		if(((*it)%BLOCK_SIZE+size)<BLOCK_SIZE){
			substring = strOut.substr((*it)%BLOCK_SIZE, size);
			vector<string> vec = tupleToAttr(table.m_attr, substring);
			Tuple* tmp = new Tuple(vec);
			if((isValid(size,substring))&&(tmp->isSatisfied(table, condition)==true)){//该tuple符合要求
				selected.push_back(*tmp);
			}else{//不合要求，删除临时变量
				delete tmp;
			}
		}
	}
	return selected;
}

int RecordMan::insertTuple(Table table, std::vector<std::string>& tuple){//插入记录
	string filename=table.m_name+".table";
	char dirty='0';
	int size = table.size()+1;
	string toBeInserted="";
	
	string str;
	int blockNum = BufferMan::instance().readLast(filename,str);

	for(int i=0;i<tuple.size();i++){
		if(table.m_attr[i].isUnique()==true) 
			if(hasExisted(table, tuple[i], i, blockNum)==true) 
				return -1;
		tuple[i].resize(table.m_attr[i].m_length,0);//应该已经保证了tuple每条attrbute记录长度符合
		toBeInserted+=tuple[i];
	}
	toBeInserted += dirty;
	//string str;
	//int blockNum = BufferMan::instance().readLast(filename,str);
	int i;
	for(i=0; i<=blockNum; i++){
		string strOut;
		BufferMan::instance().read(filename, i, strOut);
		int offset = findDirtyTuple(strOut,size);
		if(offset!=-1) {
			strOut.replace(offset,size, toBeInserted);
			//strOut.erase(offset,size) ;
			//strOut.insert(offset,toBeInserted);
			BufferMan::instance().write(filename, strOut, i);
			return (i*BLOCK_SIZE+offset);
		}else { //如果offset=-1，说明该块中不存在已经删除的记录，直接读取下一个block
			continue;
		}
	}
	if(i>blockNum){//之前的块中都满了
		toBeInserted.resize(BLOCK_SIZE,0);
		BufferMan::instance().write(filename, toBeInserted);
	}
	return blockNum*BLOCK_SIZE;//返回新block的偏移
}


bool RecordMan::hasExisted(Table table, string content, int num, int blockNum){
	Condition* con = new Condition(table.m_attr[num].m_name, EQUAL, content);
	vector<Condition> cons ;
	cons.push_back(*con);
	vector<Tuple> vt = selectTuple(table, cons, blockNum);//遍历取得
	if(vt.size()>0) return true;
	else return false;
}


vector<pair<Tuple,int>> RecordMan::deleteTuple(const Table& table,const vector<Condition>& condition, vector<int> m_offset){//删除记录,返回offset,需要index
	string filename = table.m_name+".table";
	char dirty='1';
	vector<pair<Tuple,int>> deleted;
	size_t size = 1 + table.size();//记录每个tuple大小,table.size()在新版中有定义
	
	for(auto it=m_offset.begin(); it!=m_offset.end(); it++){//遍历所有offset
		string strOut;
		BufferMan::instance().read(filename, (*it)/BLOCK_SIZE, strOut);
		int offset = (*it) % BLOCK_SIZE;//block内偏移
		if((offset+size)<BLOCK_SIZE){
			string substring = strOut.substr(offset,size);//取得一个tuple的string
			vector<string> vec = tupleToAttr(table.m_attr, substring);
			Tuple* tmp = new Tuple(vec);
			if((isValid(size,substring))&&(tmp->isSatisfied(table, condition))){//满足删除条件
				substring.replace(size-1,1,1,dirty);
				strOut.replace(offset,size,substring);
				deleted.push_back( pair<Tuple,int>((*tmp), offset) );
			}else{
				delete tmp;
			}
		}
		BufferMan::instance().write(filename, strOut, (*it)/BLOCK_SIZE);
	}
	return deleted;
}

vector<pair<Tuple,int>> RecordMan::deleteTuple(const Table& table, const vector<Condition>& condition){//删除记录,返回offset,foreach
	string filename = table.m_name+".table";
	char dirty = '1';
	size_t size = 1 + table.size();//tuple_size
	string str;//其实readLast读出最后一块的内容没有用，只需要最后一块的blockNum
	vector<pair<Tuple,int>> deleted;

	int blockNum = BufferMan::instance().readLast(filename, str);
	for(int i=0;i<=blockNum;i++){//遍历所有block
		string strOut;
		BufferMan::instance().read(filename, i, strOut);	//读出block的内容
		size_t pointer=0;

		while((pointer+size)<strOut.size()){//解析strOut中的字符串内容
			string substring = strOut.substr(pointer,size);
			vector<string> vec = tupleToAttr(table.m_attr, substring);
			Tuple* tmp = new Tuple(vec);
			if((isValid(size,substring))&&(tmp->isSatisfied(table, condition))){//满足删除条件
				substring.replace(size-1,1,1,dirty);
				strOut.replace(pointer,size,substring);
				int offset = i*BLOCK_SIZE+pointer;
				deleted.push_back( pair<Tuple,int>((*tmp), offset) );
			}else{
				delete tmp;
			}

			pointer+=size;
		}
		BufferMan::instance().write(filename, strOut, i);
	}
	return deleted;

}

////////////////////////////////////////////////////////////////
int RecordMan::findDirtyTuple(string strOut,int size){
	int offset = 0;
	while((offset+size) < strOut.size()){
		string str=strOut.substr(offset,size);		
		string dirty = str.substr(size-1,1);
		if(dirty=="1") return offset;
		offset += size;
	}
	return -1;
}
////////////////////////////////////////////////////////////
vector<string> RecordMan::tupleToAttr(vector<Attribute> m_attr,string tuple_str){
	vector<Attribute>::iterator it;
	vector<string> result;
	int ptr=0;
	for(it=m_attr.begin(); it!=m_attr.end(); it++){
		string tmp = tuple_str.substr(ptr,(*it).m_length);
		ptr += (*it).m_length;
		result.push_back(tmp);
	}
	return result;
}
////////////////////////////////////////////////////////////
vector<pair<string, int>> RecordMan::getOffsetInfo(const Table& table, const string& attrName,const int blockID){
	//vector<pair<string, int>> info = RecordMan::instance().getOffsetInfo(table, attrName, cnt);
	string fileName = table.m_name+".table";
	string content;
	if(BufferMan::instance().read(fileName, blockID, content)==false)
		return vector<pair<string, int>>();
	vector<pair<string, int>> info;
	//int cnt = getCnt( content );
	size_t size = 1 + table.size();
	size_t attrBegin = table.getAttrBegin(attrName);
	size_t attrEnd = table.getAttrEnd(attrName);
	
	size_t offset=0;//定义block内的一个偏移
	while((offset+size)<content.size()){
		string tuple = content.substr(offset,size);
		string tupleStr = content.substr(offset+attrBegin, attrEnd-attrBegin);
		if(isValid(size, tuple)){
			offset += BLOCK_SIZE*blockID;
			info.push_back( pair<string, int>( tupleStr, offset ) );
		}
		offset += size;
	}
	return info;
}

bool RecordMan::isValid(int size, string str){
	if(str.substr(size-1,1)=="0")//dirty char=="0",没有被删掉
		return true;
	else return false;
}
