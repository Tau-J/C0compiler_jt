#include<iostream>
#include<fstream>
#include<sstream>
#include<cstring>
#include "head.h"
using namespace std;



int main(){

	char path[1024];


	cout << "input path: "<<endl;
	cin >> path;



	fin.open(path);

// 词法分析运行的代码 v1
	// while(NULL != fin.getline(srcin,1024)){
	// 	ptr = srcin;

	// 	getSym();
	// }

// 词法分析运行的代码 v2
//	 getSym();


// 语法分析的代码 - 四元式生成
	getOneSym();
	programProc();

	midcode2MIPS();

	fin.close();

	return 0;
}
