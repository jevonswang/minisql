#include <list>
#include <string>
#include "controler.h"
#include "api.h"
#include "cli.h"
// #include "interpreter.h"

using namespace std;


void Controler::run()
{
	while( true )
	{
		list<string> tmpCmd = CLI::read();
		m_cmd.splice( m_cmd.end(),
                     tmpCmd, tmpCmd.begin(), tmpCmd.end() );
		while( m_cmd.empty() == false )
		{
			try
			{
				//m_result = Interpreter::parse( m_cmd.front() );
				querySwitch();
				CLI::printResult();
			} catch(string e) // 此处许多修改
			{
				CLI::printException(e);
			}
		}
	}
}

void Controler::querySwitch()
{
	switch( m_queryType )
	{
		case CREATE_TABLE:
			API::instance().createTable(m_tableName, m_vtAttr);
			break;
		case DROP_TABLE:
			API::instance().dropTable(m_tableName);
			break;
		case CREATE_INDEX:
			API::instance().createIndex(m_indexName, m_tableName, m_attrName);
			break;
		case DROP_INDEX:
			//API::instance().dropIndex(m_indexName, m_tableName, m_attrName);
			break;
		case INSERT_TUPLE:
			//API::instance().insertTuple(m_tableName, m_tuple);
			break;
		case DELETE_TUPLE:
			API::instance().deleteTuple(m_query);
			break;
		case SELECT_TUPLE:
			API::instance().selectTuple(m_query);
			break;
		case EXECFILE:
		{
			//list<string> tmpCmd = API::onExecfile();
			//m_cmd.splice( m_cmd.end(),
            //             tmpCmd, tmpCmd.begin(), tmpCmd.end() );
			break;
		}
		case QUIT:
			exit(0);
			break; // no use
	}
}
