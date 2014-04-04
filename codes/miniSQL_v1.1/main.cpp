#include <iostream>
#include "interpreter.h"
using namespace std;

char input[1000];

int main(){
	string query;
	bool isEnd = true;
	Interpreter itp;
	itp.clear();
	while(1){
		if(isEnd)
			cout<<"miniSQL>>";
		else
			cout<<"         ";

		gets(input);
		string line(input);
		isEnd = itp.getLine(line);
	}
}
