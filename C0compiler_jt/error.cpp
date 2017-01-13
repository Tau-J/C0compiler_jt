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
"字符串未一行输完或缺少另一个双引号",
"字符串中有非法字符",
"无法识别的标识符",
"标识符重复定义",
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
"只能对变量进行赋值",
"数字太大，最多10位数字，且绝对值不超过2147483647",
"int常量等号后面必须是整数",
"char常量的等号后面必须是字符",
"有返回的函数必须至少有一条返回语句",
"已取消的错误",
"有返回的函数返回语句必须有返回值",
"已取消的错误",
"已取消的错误",
"数组标识符必须跟随下标",
"实参个数与形参个数不等",
"已取消的错误",
"表达式中的函数类型只能是int或char",
"已取消的错误",
"已取消的错误",
"已取消的错误",
"声明顺序有误",
"已取消的错误",
"数组长度有前导零，或长度为0",
"应是标识符",
"常量只能定义为int或char",
"应是'='",
"应是';'或','",
"应是'{'",
"形参类型只能是int或char",
"数组定义的中括号内应为无符号正整数",
"无法识别语义的内容",
"因子中有不合法的内容",
"至少应该有一条case语句",
"返回语句后面只能是';'或者'('",
"符号表溢出",
"函数定义部分不能有语句",
"主函数的函数体之后不允许有内容",
"无返回的函数返回语句不能有返回值"

};

int handler[] =
{
0,
6,
6,
7,
8,
9,
9,
5,
3,
0,
0,
4,
1,
1,
1,
1,
11,
11,
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
3,
5,
3,
3,
1,
10,
1,
5,
1,
1,
1,
0,
4,
0,
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

	preErrNum = errorNum;
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
		case 5:
			// 跳到下一个分号
			while(symbol != SEMICOLON)
				getOneSym();
			break;
		case 6:
			// 跳到下一个可识别字符并读下一个单词
			getChar();
			while(chr != 32 && chr != 33 &&
				!(35 <= chr && chr <= 126) ){
				getChar();
				if(chr == '\0')getOneSym();
			}
            preErrNum = 0;
			getOneSym();
			preErrNum = 1;
			break;
		case 7:
			// 截掉多余的字符并跳到下一个分号或逗号
			while(chr != ';' && chr != ','){
				getChar();
				if(chr == '\0')getOneSym();
			}
			break;
		case 8:
			// 填充缺省字符'0'并跳到下一个分号或逗号
            token += '0';
            getChar();
            while(chr != ';' && chr != ','){
				getChar();
				if(chr == '\0')getOneSym();
			}
			break;
		case 9:
			// 跳到下一个以下集合的元素{IFSY WHILESY SWITCHSY LBRACKET IDEN RETURNSY SCANFSY PRINTFSY ELSESY RBRACE}
			// while(symbol != IFSY && symbol != WHILESY && symbol != SWITCHSY && symbol != LBRACKET && symbol != IDEN && symbol != RETURNSY && symbol != SCANFSY && symbol != PRINTFSY && symbol != ELSESY && symbol != RBRACE){
			//  while(symbol != RPARENT){
			// 	getOneSym();
			// }
			// chr = '"';
			while(chr != '"'){
				getChar();
				if(chr == '\0'){
					while(NULL != fin.getline(srcin,1024)){

						lineNo++;

						#ifdef fDEBUG
			        	fout << "-----------------" << endl;
					    #endif // DEBUG
					    #ifdef cDEBUG
					        cout << "-----------------" << endl;
					    #endif // fDEBUG

						ptr = srcin;

						for(int i=strlen(srcin)-1;
								srcin[i] == ' ' ||
								srcin[i] == '\t' ||
								srcin[i] == '\n';
								i--)
							srcin[i] = '\0';

						getChar();

						if(chr != '\0'){
							break;
						}
					}
				}
			}
			break;
		case 10:
			// 跳到下一个右小括号
			while(symbol != RPARENT)
				getOneSym();
			break;
		case 11:
			// 跳到下一个右大括号
			while(symbol != RBRACE)
				getOneSym();
			break;
	}
}
