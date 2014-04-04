#include "global.h"
#include "interpreter.h"
#include "catalogman.h"
#include "cli.h"
#include "api.h"
#include <ctime>

using namespace std;

bool Interpreter::getLine(string line){
    bool isEnd = false;
    for(size_t i=0;i<line.length();i++){
		if(line[i]!=';'){
			query+=line[i];
        }
        else{
			parse();
			clear();
			isEnd = true;
			break;
        }
    }
    return isEnd;
}

void Interpreter::execfile(string filename){
    ifstream fp;
    fp.open(filename,ifstream::in);
    if (fp==NULL){
      cerr<<"Error to open the file."<<endl;
      return ;
    }
    else{
		char buf[1000];
		string temp;
		query = "";
		while(!fp.eof()){
			fp.getline(buf,1000);
			temp = buf;

			for(size_t i=0;i<temp.length();i++){
				if(temp[i]!=';'){
					query+=temp[i];
				}
				else{
					parse();
					clear();
					break;
				}
			}
		}
	}
}

void onCreateOp(strstream& str){
	string type;
	str>>type;
    if(type=="table"){
		string tablename,trash;
		str >> tablename >> trash;

		const char PRIMARY = 1;
		const char UNIQUE = 2;
		const char NOT_NULL = 4;
		const char INDEX = 8;
	
		std::vector<Attribute> newTable;

		while(1){
			string attri_name;
			DATA_TYPE	attri_type;
			int attri_flag = 0;
			int attri_length = 0;

			str >> attri_name;

			if(attri_name=="primary"){
				str >> trash >> trash;
				string pri_attri;

				str >> pri_attri >> trash;

				vector<Attribute>::iterator iter;

	            for(iter = newTable.begin();iter!=newTable.end();iter++){
					if(iter->m_name==pri_attri){
						iter->m_flag|=PRIMARY;
						break;
					}
				}
				break;
            }
			else{
				string type;
				str >> type;

				if(type=="char")
					attri_type = CHAR;
				else if(type=="int")
					attri_type = INT;
				else if(type=="float")
					attri_type = FLOAT;
				else{
					cerr<<"Unrecognized type: "<<type<<endl;
					return;
		     	}

	            if(type=="char")
					str >> trash >> attri_length >> trash;
				else attri_length = 4;

				string flag;
				str >> flag;
				if(flag=="primary"){
					attri_flag |= PRIMARY;
					str >> trash;
				}
				else if(flag=="unique"){
					attri_flag |= UNIQUE;
					str >> trash;
				}
				else if(flag=="not"){
					str >> flag;
					if(flag == "null"){
						attri_flag |= NOT_NULL;
						str >> trash;
					}
					else{
						cerr<<"Syntax error";
						return;
					}
				}
				else if((flag!=",")&&(flag!=")")){
					cerr<<"Syntax error"<<endl;
					return;
				}

				Attribute newAttri(attri_name,attri_type,attri_length,attri_flag);
				newTable.push_back(newAttri);
			}
		}

         try{
			 CatalogMan::instance().createTableCheck(tablename, newTable);
			 API::instance().createTable(tablename,newTable);
		 }catch (string errstr)
		 {
			 cerr << errstr << endl;
			 return;
		 }




	      cout<<"Table "<<tablename<<" has been created successfully."<<endl;
	}
    else if(type=="index"){
		string trash,indexname,tablename,attriname;
		str>>indexname>>trash>>tablename>>trash>>attriname;



		try{
		 	API::instance().createIndex(indexname,tablename,attriname);
		}catch(string err)
		{
			cerr << err <<endl;
			return;
		}



	    cout<<"Index "<<indexname<<" has been created successfully."<<endl;
	}
    else{
		cerr<<"Syntax error!"<<endl;
		return;
    }
}

void onDropOp(strstream& str){
	string type;
    str>>type;
    if(type=="table"){
		string tableName;
        str>>tableName;


		try{
			API::instance().dropTable(tableName);
		}catch(string err)
		{
			cerr << err <<endl;
			return;
		}

	    cout<<"Table "<<tableName<<" has been dropped successfully."<<endl;
    }
    else if(type=="index"){
        string indexName;
        str >> indexName;


	try{
		API::instance().dropIndex(indexName);
	}catch(string err)
	{
		cerr << err <<endl;
		return;
	}


	    cout<<"Index "<<indexName<<" has been dropped successfully."<<endl;
    }
}

void onInsertOp(strstream& str){
	string trash,tableName;
    str >> trash ;
	if(trash!="into"){
		cerr << "Syntax error!" <<endl;
		return;
	}

	str >> tableName >> trash ;

	if(trash!="values"){
		cerr << "Syntax error!" <<endl;
		return;
	}
		
	str >> trash;

    vector<string> newTuple;

    string s = "";
	string t;
    while(str >> t)s=s+t;

    string item;
    size_t i,j;
    i=s.find_first_of(",");
	item=s.substr(0,i);
	if(item[0]=='\'')
		item=s.substr(1,i-2);
	newTuple.push_back(item);

    while(1){
		j=s.find_first_of(",",i+1);

		if(j==string::npos){
			item=s.substr(i+1,s.size()-i-2);
			if(item[0]=='\'')
				item=s.substr(i+2,s.size()-i-4);
			newTuple.push_back(item);
			break;
		}

        item=s.substr(i+1,j-i-1);
		if(item[0]=='\'')
			item=s.substr(i+2,j-i-3);
        newTuple.push_back(item);
        i=j;
	}
	
	

	try{
    API::instance().insertTuple( tableName , newTuple );
	}catch(string err)
	{
		cerr << err <<endl;
		return;
	}


	cout<<"This tuple has been inserted successfully."<<endl;
}

void onSelectOp(strstream& str){
		string trash,tableName;
    str >> trash >> trash >> tableName ;
    
	Query newQuery;
	newQuery.m_tableName = tableName;

	while(str >> trash){
		Condition con;
		
		str >> con.m_attrName ;
		
		string op_type;
		str >> op_type;

		if(op_type=="=")
			con.m_opType = EQUAL;
		else if(op_type=="<>")
			con.m_opType = NO_EQUAL;
		else if(op_type=="<")
			con.m_opType = LESS;
		else if(op_type==">")
			con.m_opType = GREATER;
		else if(op_type=="<=")
			con.m_opType = NO_GREATER;
		else if(op_type==">=")
			con.m_opType = NO_LESS;
		else{
			cerr<<"Unrecognized op_type: "<<op_type<<endl;
            return;
		}

		string operand;
		str >> operand;
		if(operand[0]=='\'')
			operand = operand.substr(1,operand.length()-2);
		con.m_operand = operand;

		newQuery.m_condition.push_back(con);

	}

	clock_t start = clock();
	
	try{
    	API::instance().selectTuple(newQuery);
	}catch(string err)
	{
		cerr << err <<endl;
		return;
	}

	clock_t stop = clock();
	double duration = ((double)(stop-start))/CLK_TCK;
	cout<<"The time duration is: "<<duration<<"s"<<endl;
   
}

void onDeleteOp(strstream& str){
	string trash,tableName;
    str >> trash >> tableName;

	Query query;
	query.m_tableName = tableName;

	if(str>>trash){

        string attrName;
        string opType;
        string operand;

		std::vector<Condition> m_condition;


        Condition con;
        str >> con.m_attrName;
		string op_type;
		str >> op_type;

		if(op_type=="=")
			con.m_opType = EQUAL;
		else if(op_type=="<>")
			con.m_opType = NO_EQUAL;
		else if(op_type=="<")
			con.m_opType = LESS;
		else if(op_type==">")
			con.m_opType = GREATER;
		else if(op_type=="<=")
			con.m_opType = NO_GREATER;
		else if(op_type==">=")
			con.m_opType = NO_LESS;
		else{
			cerr<<"Unrecognized op_type: "<<op_type<<endl;
            return;
		}
		

		str >> con.m_operand;

		if(con.m_operand[0]=='\'')
			con.m_operand = con.m_operand.substr(1,con.m_operand.length()-2);


        m_condition.push_back(con);
		query.m_condition = m_condition;
	}
	

   	try{
		API::instance().deleteTuple(query);
	}catch(string err)
	{
		cerr << err <<endl;
		return;
	}

	cout<<"This tuple has been deleted successfully."<<endl;
}

void insert_space(string& query){
	for(size_t i=0;i<query.length();i++){
		if(query[i]=='(' || query[i]==')' || query[i]==','){
			query.insert(i," ");
			query.insert(i+2," ");
			i+=2;
		}
	}
}

void Interpreter::parse(){

	if(query=="quit") exit(0);
	if(query=="")return;

    insert_space(query);//insert space to separate the query

	strstream str;
	str << query;
	
	string trash,option;
	str>>option;

    if(option=="create"){
        onCreateOp(str);
    }
    else if(option=="drop"){
		onDropOp(str);
    }
    else if(option=="insert"){
		onInsertOp(str);
    }
    else if(option=="select"){
		onSelectOp(str);
    }
    else if(option=="delete"){
		onDeleteOp(str);
    }
    else if(option=="execfile"){
    	string filename;
    	str>>filename;
    	execfile(filename);
    }
    else{
    	cerr<<"Syntax error!"<<endl;
    }
}
