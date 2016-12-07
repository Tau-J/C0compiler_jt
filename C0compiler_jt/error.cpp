#include "head.h"
#include<cstdlib>
#include<iostream>
using namespace std;

#define fDEBUG


string errmsg[]={
"缺少源文件",
"无法识别的字符",
"单独的一个感叹号",
"单引号中出现多个字符",
"单引号中放了非法字符",
"字符串未一行输完",
"字符串中有非法字符",
"该标识符未定义",
"该标识符重复定义",
"找不到main函数",
"main函数必须是void类型",
"缺少类型声明",
"应是'('",
"应是'['",
"应是')'",
"应是']'",
"应是'}'",
"应是':'",
"应是';'",
"条件中出现非法的关系表达式",
"数字太大，最多10位数字，且绝对值不超过2147483647",
"int常量等号后面必须是整数",
"char常量的等号后面必须是字符",
"变量定义的'['后面必须是无符号整数",
"表达式格式不合法",
"有返回的函数返回语句必须有返回值",
"实参和对应形参类型应该相同，除非形参类型为int，实参为char",
"应是变量",
"数组标识符必须跟随下标",
"实参个数与形参个数不等",
"scanf或printf的参数类型不正确",
"表达式中的函数类型只能是int或char",
"赋值语句中被赋值的变量必须与表达式类型相同，除非表达式是char",
"内存溢出",
"应是常量",
"声明顺序有误",
"复合语句声明顺序有误",
"数组长度有前导零，或长度为0",
"应是标识符",
"常量只能定义为int或char",
"应是'='",
"应是';'或','",
"应是'{'",
"形参类型只能是int或char",
"有返回的函数必须至少有一条不在分支里的返回语句",
"语句中变量不能单独出现而没有其他符号",
"因子中有不合法的内容",
"至少应该有一条case语句",
"返回语句后面只能是';'或者'('",
"符号表溢出",
"函数定义部分不能有语句",
"主函数的函数体之后不允许有内容"
};

int handler[] =
{
0,
1,
1,
1,
1,
1,
1,
1,
1,
0,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
2,
2,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
3,
1,
1,
1,
1,
3,
3,
1,
1,
1,
1,
1,
1,
1,
0,
4,
1
};

unsigned int errCnt = 0;
bool compileOK = 1;

void error(int errorNum){
    compileOK = 0; // 编译失败标识，在生成的目标代码中加入无法运行的提示
	cout << "errCnt: " << ++errCnt << endl;

	// cout << "!!!!!!!!!!!!!!error : " << errorNum << "!!!!!!!!!!!!!!!" <<endl;
	// cout << "line: " << lineNo << endl;
	// cout << "code: " << srcin << endl;
	// cout << "reason: " << errmsg[errorNum] << endl;
	// #ifdef sDEBUG
	// cout << lineNo << endl;
	// #endif
	#ifdef fDEBUG
	fout << "!!!!!!!!!!!!!!error : " << errorNum << "!!!!!!!!!!!!!!!" <<endl;
	fout << "line: " << lineNo << endl;
	fout << "code: " << srcin << endl;
	fout << "reason: " << errmsg[errorNum] << endl;
	#endif // fDEBUG

	errhandler(handler[errorNum]);
}

void errhandler(int handleNum){
	switch(handleNum){
		case 0:
			// 中止程序
			cout << "## fatal error. stop compiling. ##" <<endl;
			exit(0);
		case 1:
			// 不做任何处理
			break;
		case 2:
			// 读下一个单词
			getOneSym();
			break;
		case 3:
			// 跳到下一个分号或逗号
			while(symbol != SEMICOLON && symbol != COMMA)
				getOneSym();
			break;
		case 4:
			// 跳到下一个类型声明符号int\char\void 
			while(symbol != INTSY && symbol != CHARSY && symbol != VOIDSY)
				getOneSym();
			break;
	}
}
