//
//  cli.h
//  minisql-catalog-cli
//
//  Created by Duo Xu on 11/4/13.
//  Copyright (c) 2013 Duo Xu. All rights reserved.
//

#ifndef _CLI_H_
#define _CLI_H_

#include <list>
#include <string>
#include <vector>
#include <iostream>

class CLI {
public:
    static std::list<std::string> &read();
    static void printResult();
    static void printException(std::string e);
	static void printSelectResult();
private:
    static void printLine(std::vector<int> cellWidth) {
        using namespace std;
        cout << "+";
        for (int i = 0; i < (int)cellWidth.size(); i++) {
            for (int j = 1; j <= cellWidth[i]; j++)
                cout << "-";
            cout << "+";
        }
        cout << endl;
    }
};

#endif
