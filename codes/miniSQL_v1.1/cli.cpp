//
//  cli.cpp
//  minisql-catalog-cli
//
//  Created by Duo Xu on 11/4/13.
//  Copyright (c) 2013 Duo Xu. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <cstdio>
#include "api.h"
#include "controler.h"
#include "cli.h"
#include "catalogman.h"

std::list<std::string> &CLI::read() {
    using namespace std;
    string line;
    getline(cin, line);
    list<string> *result = new list<string>;
    // 我们假设数据库涉及到的字符串均不包含单引号
    bool isBetweenQuotationMarks = false;
    string currentInstruction;
    for (size_t i = 0; i < line.length(); i++) {
        switch (line[i]) {
            case '\'':
                currentInstruction += line[i];
                isBetweenQuotationMarks = !isBetweenQuotationMarks;
                break;
            case ';':
                currentInstruction += line[i];
                if (!isBetweenQuotationMarks) {
                    result->push_back(currentInstruction); // 一条命令结束
                    currentInstruction.clear(); // 清空代表当前读入命令的字符串
                }
                break;
            default:
                currentInstruction += line[i];
                break;
        }
    }
    return *result;
}

void CLI::printResult() {
    using namespace std;
    Controler &ctrl = Controler::instance();
    switch (ctrl.m_queryType) {
        case CREATE_TABLE:
            cout << "Table " << ctrl.m_tableName << " created successfully.\n";
            break;
        case DROP_TABLE:
            cout << "Table " << ctrl.m_tableName << " dropped successfully.\n";
            break;
        case CREATE_INDEX:
            cout << "Index " << ctrl.m_indexName << " created successfully.\n";
            break;
        case DROP_INDEX:
            cout << "Index " << ctrl.m_indexName << " dropped successfully.\n";
            break;
        case INSERT_TUPLE:
            cout << "Tuple inserted successfully.\n";
            break;
        case DELETE_TUPLE:
            cout << "Tuple deleted successfully.\n";
            break;
        case QUERY_TYPE::SELECT_TUPLE:
            printSelectResult();
            break;
        case EXECFILE:
            cout << "File execution completed.\n";
            break;
        case QUIT:
            cout << "Bye.\n";
            break;
    }

    // for debugging only
    /*
    QUERY_TYPE qt = QUERY_TYPE::SELECT_TUPLE;
    switch (qt) {
        case QUERY_TYPE::CREATE_TABLE:
            cout << "Table " << " created successfully.\n";
            break;
        case QUERY_TYPE::DROP_TABLE:
            cout << "Table " << " dropped successfully.\n";
            break;
        case QUERY_TYPE::CREATE_INDEX:
            cout << "Index " << " created successfully.\n";
            break;
        case QUERY_TYPE::DROP_INDEX:
            cout << "Index " << " dropped successfully.\n";
            break;
        case QUERY_TYPE::INSERT_TUPLE:
            cout << "Tuple inserted successfully.\n";
            break;
        case QUERY_TYPE::DELETE_TUPLE:
            cout << "Tuple deleted successfully.\n";
            break;
        case QUERY_TYPE::SELECT_TUPLE:
            printSelectResult();
            break;
        case QUERY_TYPE::EXECFILE:
            cout << "File execution completed.\n";
            break;
        case QUERY_TYPE::QUIT:
            cout << "Bye.\n";
            break;
    }
    */
}

void CLI::printException(std::string e) {
    using namespace std;
    cout << e << endl;
}

void CLI::printSelectResult() {
    using namespace std;

    // 获取表头信息以供输出
    Table t = CatalogMan::instance().getTable(Controler::instance().m_tableName);
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

	while(API::instance().hasNext()) {
		vector<Tuple> tuples = API::instance().next();
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

    // for debugging only
    /*
    // 获取表头信息以供输出
    vector<int> cellWidth = {11, 20, 8, 6};
    // 输出表头上边界
    printLine(cellWidth);
    // 输出表头内容
    cout << "|";
    for (int i = 0; i < cellWidth.size(); i++) {
        ostringstream oss;
        oss << cellWidth[i] - 2;
        string format = " %-";
        format += oss.str(); // "i to s"
        format += "s |";
        printf(format.c_str(), "test");
    }
    cout << endl;
    // 输出表头下边界（也就是第一行记录的上边界）
    printLine(cellWidth);
    vector<string> v_s1 = {"001", "Michael AAA", "M", "1234"};
    vector<string> v_s2 = {"002", "Jane BBB", "F", "9382"};
    vector<string> v_s3 = {"003", "Tom CCC", "M", "0567"};
    Tuple t1(v_s1), t2(v_s2), t3(v_s3);
	for (int i = 0; i < 3; i++) {
        vector<Tuple> tuples = {t1, t2, t3};
        for (int i = 0; i < tuples.size(); i++) {
            // 输出当前行内容
            cout << "|";
            for (int j = 0; j < cellWidth.size(); j++) {
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
    */
}