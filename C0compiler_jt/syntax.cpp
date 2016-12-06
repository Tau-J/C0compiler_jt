#include "head.h"
#include<iostream>
#include<string>
#include<fstream>
#define fDEBUG
#define sDEBUG
#define showsymtab

#ifdef fDEBUG
	#define cout fout
#endif

using namespace std;


//************************************************
// 以下为符号表管理
SymbolTable symtable;
bool isGlobalDec = 1;
int mainIndex = 0;

ofstream sfout("symtable.txt");

// 从符号表中查找非函数标识符 返回在符号表中的坐标，找不到则返回-1
int locInSubSymTab(string iden,int start = symtable.indexTab[symtable.subTotal]){

// 符号表越上面的离现在越近，越下面说明声明的时间越早
	// 此处 +1用于跳过函数头部，防止搞事的人局部变量和函数同名
	int i = start + 1;//当前运行的分程序开头
	int j = symtable.indexTab[1];//第一个函数的开头

//    const char* dis = iden.c_str();

	// 从该分程序的开头遍历到符号表的最顶部
	// 在局部变量区查找
	while(i < symtable.level){
		if(symtable.element[i].name == iden)
			return i;
        i++;
	}

	//运行到这里说明分程序符号表中没有定义，
	// 接下来查找最外层符号表

	// 从符号表最底部遍历到main函数的开头
	// 在全局变量区查找
	if(i == symtable.level){
		i = 0;
		while(i < j){
			if(symtable.element[i].name == iden)
				return i;
            i++;
		}
		if(i == j){
			error(7);//该标识符未定义
			return -1;
		}
	}
	return -1;
}

// 从符号表中查找标识符 返回标识符在符号表中的坐标，找不到则返回-1
// isFunc=1表示从分程序索引表中查找函数名标识符
// isFunc=0表示从当前分程序范围中查找常变量标识符
int loc(string iden, bool isFunc ,bool paramCheck =1){
	if(isFunc){
		for(int i = 1; i <= symtable.subTotal; i++){
			if(symtable.element[symtable.indexTab[i]].name == iden){
				// 找到此函数声明之后，还需要进一步判断

				if(paramCheck)
					if(symtable.element[symtable.indexTab[i]].paramNum != t_paramNum)
						return -29;//error(29)实参个数与形参个数不等

				return symtable.indexTab[i];
			}
		}
		return -1;
	}
	else{
		return locInSubSymTab(iden);
	}
}

// 将新定义的标识符登录到符号表
void enterIntoSymTab(string name, symTabobj obj,
	    			 symTabtyp typ, int value,
	    			 int addr, int paramNum){
//    const char* dis = name.c_str();

// 符号表爆栈
	if(symtable.level >= MAX_SYMTAB_LEN){
		error(49);//符号表溢出
		errhandler(0);//中止程序
	}

	if(obj == objFUNCTION){//先检查函数，因为函数可以利用分函数索引更快查找
		// 检查符号表中是否已经存在该函数标识符
		for(int i = 1; i <= symtable.subTotal; i++){
			if(symtable.element[symtable.indexTab[i]].name == name){
				error(8);//该标识符重复定义
				errhandler(0);
			}
		}

		// 检查无误,符号表中无此函数，添加到分程序索引表
		symtable.indexTab[++symtable.subTotal] = symtable.level;
	}else{//如果不是函数，则只能逐个遍历当前分程序
		// 检查局部变量有无重复定义
		// 此处 +1用于跳过函数头部，防止搞事的人局部变量和函数同名
		for(int i = symtable.indexTab[symtable.subTotal] + 1;
			i < symtable.level; i++){
			if(symtable.element[i].name == name){
				error(8);//该标识符重复定义
				errhandler(0);
			}
		}

		//检测无误，局部变量表和形参表中都无此标识符
		//判断当前代码运行是否处于全局变量声明阶段，如果是，则进行全局变量查重
		if(obj != objPARAM && isGlobalDec){
			for(int i = 0; i < symtable.indexTab[1];i++){
				if(symtable.element[i].name == name){
					error(8);//该标识符重复定义
					errhandler(0);
				}
			}
		}
	}

	//检测无误，说明此标识符可以登录到符号表了
		symtable.element[symtable.level].name = name;
		symtable.element[symtable.level].obj = obj;
		symtable.element[symtable.level].typ = typ;
		symtable.element[symtable.level].value = value;
		symtable.element[symtable.level].addr = addr;
		symtable.element[symtable.level].paramNum = paramNum;

		symtable.level++;
}

//***********************************************
// 辅助符号表登录变量,当前正在处理的内容

string t_name;
symTabobj t_obj;
symTabtyp t_typ;
int t_value;
int t_addr;
int t_paramNum;

bool isNeg = 0;
bool isArry = 0;

//***********************************************
// 以下为语法分析

void testSemicolon(){
	if(symbol != SEMICOLON)
		error(18);
	getOneSym();
}

// ＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
// 这里将主函数以前的部分视为headProc
void programProc(){

	// cout << "it is a programProc" <<endl;

	// #ifdef fDEBUG
	// fout << "it is a programProc" <<endl;
	// #endif

	headProc();
	mainProc();
}

// ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}
void headProc(){

	// cout << "it is a headProc" <<endl;

	t_addr = 0;
	constProc();
	varProc();
	t_addr = 0;

	// 结束全局变量定义，进入函数定义部分
	isGlobalDec = 0;
	funcProc();
}

// ＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
void constProc(){

	// cout << "it is a constProc" << endl;

	// #ifdef fDEBUG
	// fout << "it is a constProc" <<endl;
	// #endif

	while(symbol == CONSTSY){

		t_obj = objCONST;
		t_paramNum = 0;

		getOneSym();
		constDec();
		testSemicolon();
	}
}
// ＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}|
// 					   char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
void constDec(){

	cout << "it is a constDec" << endl;

	if(symbol == INTSY){

		t_typ = typINT;
		t_addr++;

		getOneSym();
		if(symbol == IDEN){

			t_name = token;

			getOneSym();
			if(symbol == BECOMES){
				// 默认是正整数
				isNeg = 0;

				getOneSym();
				// 正负号检测
				if(symbol == PLUS || symbol == MINUS){

                    isNeg = (symbol == MINUS)?1:0;

                    getOneSym();
				}

				if(symbol != INT) error(21);//int常量等号后面必须是整数
				else {

					t_value = (isNeg) ? (0-num) : num;

					// 登录符号表
					enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

					#ifdef sDEBUG
					cout << " name = " << t_name;
					cout << " obj = " << objName[t_obj];
					cout << " typ = " << typName[t_typ];
					cout << " value = " << t_value;
					cout << " addr = " << t_addr;
					cout << " paramNum = " << t_paramNum << endl;
					#endif


					insMidCode("const",typName[t_typ],t_value,t_name);

					getOneSym();
				}
				while(symbol == COMMA){
					getOneSym();
					if(symbol == IDEN){

						t_name = token;
						t_addr++;

						getOneSym();
						if(symbol == BECOMES){

							// 默认是正整数
							isNeg = 0;

							getOneSym();
							if(symbol == PLUS || symbol == MINUS){
                                isNeg = (symbol == MINUS)?1:0;
                                getOneSym();
                            }
							if(symbol != INT) error(21);//int常量等号后面必须是整数
							else {

								t_value = (isNeg) ? (0-num) : num;

								// 登录符号表
								enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

								#ifdef sDEBUG
								cout << " name = " << t_name;
								cout << " obj = " << objName[t_obj];
								cout << " typ = " << typName[t_typ];
								cout << " value = " << t_value;
								cout << " addr = " << t_addr;
								cout << " paramNum = " << t_paramNum << endl;
								#endif


								insMidCode("const",typName[t_typ],t_value,t_name);


								getOneSym();
							}
						}else error(40);//应是'='
					}else error(38);//应是标识符
				}
			}else error(40);//应是'='
		}else error(38);//应是标识符
	}
	else if(symbol == CHARSY){

		t_typ = typCHAR;

		getOneSym();
		if(symbol == IDEN){

			t_name = token;
			t_addr++;

			getOneSym();
			if(symbol == BECOMES){
				getOneSym();
				if(symbol != CHAR) error(22); //char常量的等号后面必须是字符
				else {

					t_value = token[0];

					// 登录符号表
					enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

					#ifdef sDEBUG
					cout << " name = " << t_name;
					cout << " obj = " << objName[t_obj];
					cout << " typ = " << typName[t_typ];
					cout << " value = " << (char)t_value;
					cout << " addr = " << t_addr;
					cout << " paramNum = " << t_paramNum << endl;
					#endif


					insMidCode("const",typName[t_typ],(char)t_value,t_name);


					getOneSym();
				}
				while(symbol == COMMA){
					getOneSym();
					if(symbol == IDEN){

						t_name = token;
						t_addr++;

						getOneSym();
						if(symbol == BECOMES){
							getOneSym();
							if(symbol != CHAR) error(41);//应是字符
							else {

								t_value = token[0];

								// 登录符号表
								enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

								#ifdef sDEBUG
								cout << " name = " << t_name;
								cout << " obj = " << objName[t_obj];
								cout << " typ = " << typName[t_typ];
								cout << " value = " << (char)t_value;
								cout << " addr = " << t_addr;
								cout << " paramNum = " << t_paramNum << endl;
								#endif


								insMidCode("const",typName[t_typ],(char)t_value,t_name);


								getOneSym();
							}

						}else error(40);//应是'='
					}else error(38);//应是标识符
				}
			}else error(40);//应是'='
		}else error(38);//应是标识符
	}
	else error(39);//常量只能定义为int或char
}

// ＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
void varProc(){

	// cout << "it is a varProc" << endl;

	// #ifdef fDEBUG
	// fout << "it is a varProc" <<endl;
	// #endif

	while(symbol == INTSY || symbol == CHARSY){

		// 先默认为是变量定义
		t_obj = objVARIABLE;
		t_typ = (symbol == INTSY)? typINT : typCHAR;

		getOneSym();
		if(symbol == IDEN){

			t_name = token;


			getOneSym();
			if(symbol == LPARENT){//如果标识符后面是(则表示已开始函数声明部分
				isGlobalDec = 0;
				returnFuncDec();
				return;
			}
			varDec();
			testSemicolon();
		}else error(38);//应是标识符
	}
}

// ＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ }
void varDec(){

	cout << "it is a varDec" << endl;

	if(symbol == LBRACKET){

		// 到达这里说明为数组定义，需要修改typ
		// 这里用了个小技巧，我定义的枚举类型中int是0，char是1
		t_typ = (t_typ) ? typCHARARRY : typINTARRY;

		getOneSym();
		uInt();
		if(symbol != RBRACKET) error(15);//应是']'
		else getOneSym();
	}

	t_addr++;

	// 到达这里则可以登录符号表了
	// 登录符号表
	enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

	#ifdef sDEBUG
	cout << " name = " << t_name;
	cout << " obj = " << objName[t_obj];
	cout << " typ = " << typName[t_typ];
	cout << " value = " << t_value;
	cout << " addr = " << t_addr;
	cout << " paramNum = " << t_paramNum << endl;
	#endif


	if(t_typ == typINT || t_typ == typCHAR){
		// 变量定义
		insMidCode(typName[t_typ],"","",t_name);
	}
	else {
		// 数组定义
		insMidCode(typName[t_typ],"",t_paramNum,t_name);
	}


	while(symbol == COMMA){
		getOneSym();
		if(symbol == IDEN){

			t_name = token;
			t_addr++;

			getOneSym();
			if(symbol == LBRACKET){

				// 到达这里说明为数组定义，需要修改typ
				// 这里用了个小技巧，我定义的枚举类型中int是0，char是1
				t_typ = (t_typ) ? typCHARARRY : typINT;

				getOneSym();
				uInt();
				if(symbol != RBRACKET)error(15);//应是']'
				else getOneSym();
			}

			// 登录符号表
			enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

			#ifdef sDEBUG
			cout << " name = " << t_name;
			cout << " obj = " << objName[t_obj];
			cout << " typ = " << typName[t_typ];
			cout << " value = " << t_value;
			cout << " addr = " << t_addr;
			cout << " paramNum = " << t_paramNum << endl;
			#endif


			if(t_typ == typINT || t_typ == typCHAR){
				// 变量定义
				insMidCode(typName[t_typ],"","",t_name);
			}
			else {
				// 数组定义
				insMidCode(typName[t_typ],"",t_paramNum,t_name);
			}



		}else error(38);//应是标识符
	}

}

// ＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝
// 只用于数组声明
void uInt(){

	// cout << "it is an unsigned int" << endl;

	 if (symbol == INT){
	 	if(token[0] == '0') error(37);//数组长度有前导零，或长度为0
	 	else {

	 		t_paramNum = num;

	 		getOneSym();
	 	}
	 }
}

// {＜有返回值函数定义＞|＜无返回值函数定义＞}
//1表示从变量定义转入函数定义 0表示正常进入函数定义
void funcProc(){

	// cout << "it is a funcProc" <<endl;

	// #ifdef fDEBUG
	// fout << "it is a funcProc" <<endl;
	// #endif

// 声明头部
	while(symbol == INTSY || symbol == CHARSY || symbol == VOIDSY){
		if(symbol == INTSY || symbol == CHARSY){

			t_typ = (symbol == INTSY)? typINT : typCHAR;

			getOneSym();
			if(symbol == IDEN){

				t_name = token;

				getOneSym();
				returnFuncDec();
			}else error(38);//应是标识符

		}
		else {//VOIDSY

			t_typ = typVOID;

			getOneSym();
			if(symbol == MAINSY){

				t_name = "main";

                return;//结束函数定义部分，准备进入主函数
			}else if(symbol == IDEN){

				t_name = token;

				getOneSym();
				voidFuncDec();
			}
			else error(38);//应是标识符
		}

		// // 当程序运行到这里说明函数已经定义完毕
		// insMidCode("end","","",t_name);
	}
	if(symbol == CONSTSY)
		error(35);//声明顺序有误
	if(symbol == SEMICOLON)
		error(50);//函数定义部分不能有';'
}

// ＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数＞‘)’ ‘{’＜复合语句＞‘}’
void returnFuncDec(){

	cout << "it is a returnFuncDec" << endl;

	if(symbol == LPARENT){

		insMidCode("func",typName[t_typ],"",t_name);// start of function t_name()

		t_obj = objFUNCTION;
		t_paramNum = 0;//默认没有参数
		t_addr = 0;
		func_name = t_name;// 记录当前函数名

		// 运行到这里说明函数头部定义已经结束，可以登录符号表了
		// 登录符号表
		enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

		#ifdef sDEBUG
		cout << " name = " << t_name;
		cout << " obj = " << objName[t_obj];
		cout << " typ = " << typName[t_typ];
		cout << " value = " << t_value;
		cout << " addr = " << t_addr;
		cout << " paramNum = " << t_paramNum << endl;
		#endif

		getOneSym();
		paramTable();

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();

		if(symbol == LBRACE){
			getOneSym();
			compoundStatement(1);
			if(symbol != RBRACE) error(16);//应是'}'
			else getOneSym();
		}else error(42);//应是'{'
	}else error(12);//应是'('
}

// ＜无返回值函数定义＞  ::= void＜标识符＞‘(’＜参数＞‘)’‘{’＜复合语句＞‘}’
void voidFuncDec(){

	cout << "it is a voidFuncDec" << endl;

	if(symbol == LPARENT){

		insMidCode("func",typName[t_typ],"",t_name);// start of function t_name()

		t_obj = objFUNCTION;
		t_paramNum = 0;//默认没有参数
		t_addr = 0;
		func_name = t_name;// 记录当前函数名

		// 运行到这里说明函数头部定义已经结束，可以登录符号表了
		// 登录符号表
		enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

		#ifdef sDEBUG
		cout << " name = " << t_name;
		cout << " obj = " << objName[t_obj];
		cout << " typ = " << typName[t_typ];
		cout << " value = " << t_value;
		cout << " addr = " << t_addr;
		cout << " paramNum = " << t_paramNum << endl;
		#endif

		getOneSym();
		paramTable();
		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();
		if(symbol == LBRACE){

            cout << "it is a statementList" << endl;

			getOneSym();
			compoundStatement(0);
			if(symbol != RBRACE) error(16);//应是'}'
			else getOneSym();

			insMidCode("end","","",func_name);// 记录当前函数名

		}else error(42);//应是'{'
	}else error(12);//应是'('
}

// ＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}|＜空＞
// paramNum用来统计参数个数
void paramTable(){
	if(symbol == RPARENT)return;
	else if(symbol == INTSY || symbol == CHARSY){

		t_obj = objPARAM;
		t_typ = (symbol == INTSY) ? typINT : typCHAR;
		t_paramNum = 1;

		getOneSym();
		if(symbol == IDEN){

			t_name = token;
			t_addr = t_paramNum;

			// 运行到这里说明一个参数定义已经结束，可以登录符号表了
			// 登录符号表
			enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

			#ifdef sDEBUG
			cout << " name = " << t_name;
			cout << " obj = " << objName[t_obj];
			cout << " typ = " << typName[t_typ];
			cout << " value = " << t_value;
			cout << " addr = " << t_addr;
			cout << " paramNum = " << t_paramNum << endl;
			#endif

			insMidCode("param",typName[t_typ],"",t_name);

			getOneSym();
			while(symbol == COMMA){
				getOneSym();
				if(symbol == INTSY || symbol == CHARSY){

					t_obj = objPARAM;
					t_typ = (symbol == INTSY) ? typINT : typCHAR;
					t_paramNum++; //统计参数个数

					getOneSym();
					if(symbol == IDEN){

						t_name = token;
						t_addr = t_paramNum;

						// 运行到这里说明一个参数定义已经结束，可以登录符号表了
						// 登录符号表
						enterIntoSymTab(t_name,t_obj,t_typ,t_value,t_addr,t_paramNum);

						#ifdef sDEBUG
						cout << " name = " << t_name;
						cout << " obj = " << objName[t_obj];
						cout << " typ = " << typName[t_typ];
						cout << " value = " << t_value;
						cout << " addr = " << t_addr;
						cout << " paramNum = " << t_paramNum << endl;
						#endif

						insMidCode("param",typName[t_typ],"",t_name);

						getOneSym();
					}else error(38);//应是标识符
				}else error(43);//形参类型只能是int或char
			}
			t_addr = 0;

			// 到达这里说明形参定义完毕，修改分程序参数个数
			symtable.element[symtable.indexTab[symtable.subTotal]].paramNum = t_paramNum;
			#ifdef sDEBUG
			cout << symtable.element[symtable.indexTab[symtable.subTotal]].name << " paramNum -> " << t_paramNum << endl;
			#endif

		}else error(38);//应是标识符
	}else error(43);//形参类型只能是int或char
}

// ＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
//ret=1 表示必须有返回语句 =0表示返回语句可有可无
void compoundStatement(bool ret){

	// cout << "it is a compoundStatement" << endl;

	// #ifdef fDEBUG
	// fout << "it is a compoundStatement" <<endl;
	// #endif

	constProc();
	varProc();
	statementList(ret);
}

// ＜语句列＞   ::= ｛＜语句＞｝
//ret=1 表示必须有返回语句 =0表示返回语句可有可无
void statementList(bool ret){

	cout << "it is a statementList" << endl;

	bool retd = 0;

	while(symbol == IFSY || symbol == WHILESY ||
		symbol == LBRACE || symbol == IDEN ||
		symbol == SCANFSY || symbol == PRINTFSY ||
		symbol == SEMICOLON || symbol == SWITCHSY ||
		symbol == RETURNSY){

    // #ifdef sDEBUG
    // if(lineNo == 70)
    //     cout << "";
    // #endif // sDEBUG

		if(symbol == RETURNSY) retd = 1;
		statement(ret);
	}

	if(ret && !retd){
		error(44);//有返回的函数必须至少有一条不在分支里的返回语句
	}
}

// ＜语句＞    ::= ＜条件语句＞｜＜循环语句＞|
// ‘{’＜语句列＞‘}’｜＜有返回值函数调用语句＞; |
// ＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜
// ＜写语句＞;｜＜空＞;|＜情况语句＞｜＜返回语句＞;
void statement(bool ret=0){

	// cout << "it is a statement" << endl;

	// #ifdef fDEBUG
	// fout << "it is a statement" <<endl;
	// #endif

	switch(symbol){
		case IFSY:
			getOneSym();
			condition();
			break;
		case WHILESY:
			getOneSym();
			loop();
			break;
		case LBRACE:
			getOneSym();
			statementList(0);
			if(symbol != RBRACE) error(16);//应是'}'
			else getOneSym();
			break;
		case IDEN:

			t_name = token;

			if(token == "a")
                cout <<t_name;

			getOneSym();
			if(symbol == LPARENT){
				// 可能是有返回的函数调用语句
				// 可能是无返回的函数调用语句

				cout << "it is a function call" << endl;

				getOneSym();
				valueParamList();
				if(symbol != RPARENT) error(14);//应是')'
				else getOneSym();

				int idx = loc(t_name,1);
				if(idx == -1)error(7);// 该标识符未定义
				else if(idx == -29)error(29);// 实参数目与形参不一致

				insMidCode("call",t_name,"","");

				testSemicolon();
			}
			else if(symbol == BECOMES || symbol == LBRACKET){
				// 可能是变量赋值语句
				assignment();
				testSemicolon();
			}
			else error(45);//语句中变量不能单独出现而没有其他符号
			break;
		case SCANFSY:
			getOneSym();
			scanfStatement();
			testSemicolon();
			break;
		case PRINTFSY:
			getOneSym();
			printfStatement();
			testSemicolon();
			break;
		case SEMICOLON:
			testSemicolon();
			break;
		case SWITCHSY:
			getOneSym();
			switchStatement();
			break;
		case RETURNSY:
			getOneSym();
			returnStatement(ret);
			testSemicolon();
			break;
		default:break;
	}
}

// ＜条件语句＞  ::=  if ‘(’＜条件＞‘)’＜语句＞［else＜语句＞］
// ＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜
	// ＜表达式＞ //表达式为0条件为假，否则为真
// ＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==
void condition(){

	cout << "it is a if" << endl;

	string t_p,p3;
	string label1,label2;

	label1 = nextLabel();//如果不满足if，则跳转到label1，位于if语句块的结束
	label2 = nextLabel();//如果有else，则label2位于else语句块的结束

	if(symbol == LPARENT){
		getOneSym();
		expression();

		p3 = res;


		if(symbol == LESSTHAN || symbol == LESSEQU ||
			symbol == MORETHAN || symbol == MOREEQU ||
			symbol == EQU || symbol == NOTEQU){

			string op;

			switch(symbol){
				case LESSTHAN:
					op = "<";break;
				case LESSEQU:
					op = "<=";break;
				case MORETHAN:
					op = ">";break;
				case MOREEQU:
					op = ">=";break;
				case EQU:
					op = "==";break;
				case NOTEQU:
					op = "!=";break;
                default:break;
			}

			getOneSym();
			expression();

			t_p = p3;
			p3 = nextVar();
			insMidCode(op,t_p,res,p3);

		}

		// 运行到这里说明条件表达式已经获取并计算出布尔值
		insMidCode("jne",p3,"",label1);

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();

		statement();
		insMidCode("jmp","","",label2);

		if(symbol == ELSESY){

			cout << "it is a else" << endl;

			getOneSym();

			insMidCode("label","","",label1);

			statement();
		}else
			insMidCode("label","","",label1);

		insMidCode("label","","",label2);

	}else error(12);//应是'('
}

// ＜循环语句＞   ::=  while ‘(’＜条件＞‘)’＜语句＞
void loop(){

	cout << "it is a while" << endl;

	string label1,label2;
	string p3;

	label1 = nextLabel();// 位于逻辑运算的前一条
	label2 = nextLabel();// 位于循环语句块的跳转语句的下一条

	if(symbol == LPARENT){
		getOneSym();

		insMidCode("label","","",label1);

		expression();

		p3 = res;

		if(symbol == LESSTHAN || symbol == LESSEQU ||
			symbol == MORETHAN || symbol == MOREEQU ||
			symbol == EQU || symbol == NOTEQU){

			string op;

			switch(symbol){
				case LESSTHAN:
					op = "<";break;
				case LESSEQU:
					op = "<=";break;
				case MORETHAN:
					op = ">";break;
				case MOREEQU:
					op = ">=";break;
				case EQU:
					op = "==";break;
				case NOTEQU:
					op = "!=";break;
                default:break;
			}

			getOneSym();
			expression();

			string t_p = p3;
			p3 = nextVar();
			insMidCode(op,t_p,res,p3);

		}

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();

		insMidCode("jne",p3,"",label2);

		statement();

		insMidCode("jmp","","",label1);
		insMidCode("label","","",label2);

	}else error(12);//应是'('
}

// ＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞
void valueParamList(){

	// cout << "it is a valueParamList" << endl;

	if(symbol == RPARENT){
		t_paramNum = 0; // 没有参数
		return;
	}

	vector<string> v; // 用于临时存放实参表

	expression();

	v.push_back(res);

	while(symbol == COMMA){
		getOneSym();
		expression();
		v.push_back(res);
	}

	// 当运行到这里说明实参表已读取完毕
	int size = v.size();
	for(int i=0;i<size;i++){
		insMidCode("fupa","","",v[i]);
	}

	t_paramNum = size;
}

// ＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|
// 			＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞
void assignment(){

	cout << "it is an assignment" << endl;

	int t = loc(t_name,0);
	if(t < 0 ) error(7);//该标识符未定义

	string p3 = t_name; //记录被赋值对象

	if(symbol == BECOMES){

		isArry = 0;

		getOneSym();
		expression();

		insMidCode("=",res,"",p3);
	}
	else if(symbol == LBRACKET){

		isArry = 1;

		getOneSym();
		expression();

		string idx = res;

		if(symbol != RBRACKET) error(15);//应是']'
		else getOneSym();
		if(symbol != BECOMES) error(40);//应是'='
		else getOneSym();

		expression();

		insMidCode("[]=",res,idx,p3);
	}
}

// ＜读语句＞    ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
void scanfStatement(){

	cout << "it is a scanfStatement" << endl;

	string p3;
	int idx;

	if(symbol == LPARENT){
		getOneSym();
		if(symbol == IDEN){

			p3 = token;
			idx = loc(p3,0);

			if(idx == -1)error(7);// 该标识符未定义

			insMidCode("scan","","",p3);

			getOneSym();

			while(symbol == COMMA){
				getOneSym();
				if(symbol == IDEN){

					p3 = token;
					idx = loc(p3,0);

					if(idx == -1) error(7);// 该标识符未定义

					insMidCode("scan","","",p3);

					getOneSym();
				}else error(38);//应是标识符
			}
		}else error(38);//应是标识符

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();
	}else error(12);//应是'('
}

// ＜写语句＞    ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’|
// printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’
void printfStatement(){

	cout << "it is a printfStatement" << endl;

	string p1,p2,p3;

	if(symbol == LPARENT){
		getOneSym();
		if(symbol == STRING){

			p1 = token;
			p2 = "";
			p3 = "";

			getOneSym();
			if(symbol == COMMA){
				getOneSym();
				expression();

				p2 = res;
				p3 = (print_char)?"char":"int";
			}
		}
		else {
			expression();

			p1 = "";
			p2 = res;
			p3 = (print_char)?"char":"int";
		}

		// 运行到这里说明写语句已读取完毕
		insMidCode("print",p1,p2,p3);

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();
	}else error(12);//应是'('
}

// ＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞［＜缺省＞］‘}’
void switchStatement(){

	cout << "it is a switchStatement" << endl;

    string switch_key;
	string end_lab = nextLabel();
	string dft_lab;

	if(symbol == LPARENT){
		getOneSym();
		expression();

		switch_key = res;
		insMidCode("switch",switch_key,"","start");

        if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();

		if(symbol == LBRACE){
			getOneSym();
			dft_lab = caseTable(switch_key,end_lab);

			if(symbol == DEFAULTSY){
				getOneSym();
				if(symbol == COLON){
					getOneSym();

					
					insMidCode("label","","",dft_lab);

					statement();
				}else error(17);//应是':'
			}else insMidCode("label","","",dft_lab);

			if(symbol != RBRACE) error(16);//应是'}'
			else getOneSym();


			insMidCode("label","","",end_lab);

		}else error(42);//应是'{'

	insMidCode("switch","","","end");

	}else error(12);//应是'('
}

// ＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’]
void returnStatement(bool retv){

	cout << "it is a returnStatement" << endl;

	if(symbol == SEMICOLON){

        if(retv)error(25);//有返回的函数返回语句必须有返回值

		insMidCode("ret","","","");
		insMidCode("end","","",func_name);

		return;

	}
	else if(symbol == LPARENT){
		getOneSym();
		expression();

		string p3 = res;

		insMidCode("ret","","",p3);
		insMidCode("end","","",func_name);

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();
	}else error(48);//返回语句后面只能是';'或者'('
}

// ＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
void expression(){

	// cout << "it is an expression" << endl;

	bool neg = 0;
	string p1,p3;
	print_char = 1;

	if(symbol == PLUS || symbol == MINUS){
		print_char = 0;
		neg = (symbol == PLUS)? 0:1;
		getOneSym();
	}

	term();
	if(neg){
		p1 = res;
		p3 = nextVar();
		insMidCode("-","0",p1,p3);
	}else{
		p3 = res;
	}

	while(symbol == PLUS || symbol == MINUS){

		print_char = 0;

		if(symbol == PLUS || symbol == MINUS){
				neg = (symbol == PLUS)? 0:1;
		}

		getOneSym();
		term();

		string t_p = p3;// 把上一项的计算结果保留下来
		p3 = nextVar(); // 为p3申请临时变量用于存放上一项与该项的运算结果
		p1 = res; // p1用于存放该项的结果

		if(neg){
			insMidCode("-",t_p,p1,p3); // p3 = t_p - p1;
		}else{
			insMidCode("+",t_p,p1,p3); // p3 = t_p + p1;
		}
	}

	// 当运行到这里说明表达式已计算完毕，且最终结果保存在p3中了

	res = p3; // 将表达式的计算结果用res传递出去
}

// ＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
void term(){

	// cout << "it is a term" << endl;

	string p1,p2,p3;

	factor();

	p3 = res;

	while(symbol == STAR || symbol == DIVID){

		print_char = 0;

		p1 = p3;
		string t_op = (symbol == STAR)? "*" : "/";

		getOneSym();
		factor();

		p2 = res;
		p3 = nextVar(); // 申请临时变量存储项

		insMidCode(t_op,p1,p2,p3);
	}

	res = p3; // 将项的计算结果用res传递回去
}

// ＜因子＞    ::= ＜标识符＞｜
// ＜标识符＞‘[’＜表达式＞‘]’｜
// ＜整数＞|＜字符＞｜＜有返回值函数调用语句＞|
// ‘(’＜表达式＞‘)’
void factor(){

	// cout << "it is a factor" << endl;

	int t = -1;
	string id,p3;
	bool t_pc = print_char;
	bool neg = 0;

	if(symbol == IDEN){
		// 可能是标识符
		// 可能是＜标识符＞‘[’＜表达式＞‘]
		// 可能是有返回值函数调用语句

		id = token;

		getOneSym();
		if(symbol == LBRACKET){
			getOneSym();

			expression();
			print_char = t_pc;

			if(symbol != RBRACKET) error(15);//应是']'
			else getOneSym();

			// 到达这里说明该因子为数组元素
			string idx;// 用于接收expression返回的数组下标
			idx = res;

			t = loc(id,0);

			if(t == -1 )error(7);// 该标识符未定义

			p3 = nextVar(); // 生成一个临时变量用于寄存数组元素
			insMidCode("=",id,idx,p3); // p3 = id[idx]
			res = p3; //将该临时变量返回给上一级

			if( symtable.element[t].typ != typCHARARRY)
				print_char = 0;



			return;
		}
		else if(symbol == LPARENT){//有返回值函数调用语句

			getOneSym();
			valueParamList();
			print_char = t_pc;

			if(symbol != RPARENT) error(14);//应是')'
			else getOneSym();

			t = loc(id,1); // 在分程序索引表中查找该函数

			if(t == -1)error(7);//该标识符未定义
			else if(t == -29)error(29);//实参个数与形参个数不等
			else if(symtable.element[t].typ != typINT &&
			 		symtable.element[t].typ != typCHAR)error(31);//表达式中的函数类型只能是int或char

			// 到达这里说明该有返回函数检测无误
			p3 = nextVar(); // 生成一个临时变量用于寄存函数返回值
			insMidCode("call",id,"",p3);
			res = p3; //将该临时变量返回给上一级

			// if(symtable.element[t].typ != typCHAR) print_char = 0;
			print_char = 0;//遇到函数调用一律视为打印数字

			return;
		}

		//到达这里说明该因子只由一个常变量标识符构成
		t = loc(id,0);

		if(t < 0)error(7);//该标识符未定义
		else if(symtable.element[t].typ == typCHARARRY ||
				symtable.element[t].typ == typINTARRY)
			error(28); // 数组标识符必须跟随下标

		res = id;//将该变量返回给上一级

		if(symtable.element[t].typ != typCHAR) print_char = 0;

		return;

	}else if(symbol == PLUS || symbol == MINUS ||
		symbol == INT || symbol == CHAR){

		print_char = 0;

		if(symbol == PLUS){
			neg = 0;
			getOneSym();
		}
		else if(symbol == MINUS){
			neg = 1;
			getOneSym();
		}

		int n;
		if(symbol == INT){
			n = (neg)? 0-num:num;
		}else{
			n = token[0];
		}

		ss.str("");
		ss.clear();
		ss << n;

		res = ss.str();

		getOneSym();
		return;
	}else if(symbol == LPARENT){

		getOneSym();
		expression();
		print_char = t_pc;

		if(symbol != RPARENT) error(14);//应是')'
		else getOneSym();

		print_char = 0;
	}else error(46);//因子中有不合法的内容
}

// ＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}
string caseTable(string switch_key,string end_lab){

	int oneCase;
	string t_lab1,t_lab2;
	string t_cmp;
	bool neg = 0;

	if(symbol == CASESY){
		getOneSym();

		cout << "it is a oneCase" << endl;

		t_lab1 = nextLabel();
		t_lab2 = nextLabel();
		
		insMidCode("label","","",t_lab1);

		if(symbol == PLUS || symbol == MINUS ||
		symbol == INT || symbol == CHAR){//常量
//  ********** 这里等待添加 int|char 判断 *************

			neg = 0;

			if(symbol == PLUS)
				getOneSym();
			else if(symbol == MINUS){
				neg = 1;
				getOneSym();
			}

			if(symbol == CHAR)
				oneCase = token[0];
			else{
				oneCase = (neg)? 0-num:num;
			}

			t_cmp = nextVar();
			insMidCode("==",switch_key,oneCase,t_cmp);

			getOneSym();
			if(symbol == COLON){
				getOneSym();

				insMidCode("jne",t_cmp,"",t_lab2);

				statement();

				insMidCode("jmp","","",end_lab);

				while(symbol == CASESY){
					getOneSym();

					cout << "it is a oneCase" << endl;

					t_lab1 = t_lab2;
					t_lab2 = nextLabel();

					
					insMidCode("label","","",t_lab1);

					if(symbol == PLUS || symbol == MINUS ||
					symbol == INT || symbol == CHAR){//常量
						//  ********** 这里等待添加 int|char 判断 *************

						neg = 0;

						if(symbol == PLUS)
							getOneSym();
						else if(symbol == MINUS){
							neg = 1;
							getOneSym();
						}

						if(symbol == CHAR)
							oneCase = token[0];
						else{
							oneCase = (neg)? 0-num:num;
						}

						t_cmp = nextVar();
						insMidCode("==",switch_key,oneCase,t_cmp);

						getOneSym();
						if(symbol == COLON){
							getOneSym();

							insMidCode("jne",t_cmp,"",t_lab2);

							statement();

							insMidCode("jmp","","",end_lab);
						}else error(17);//应是':'
					}else error(39);//常量只能定义为int或char
				}
			}else error(17);//应是':'
		}else error(39);//常量只能定义为int或char
	}else error(47);//至少应该有一条case语句

	return t_lab2;
}

// ＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
void mainProc(){

	cout << "it is a mainProc" << endl;

	mainIndex = cur_q;// 记录main函数在四元式表中的位置

	insMidCode("func",typName[t_typ],"",t_name);// start of function t_name()


	if(symbol == MAINSY){//正常必然是从funcProc进入main
		getOneSym();

		func_name = "main";

		if(symbol == LPARENT){
			getOneSym();
			if(symbol != RPARENT) error(14);//应是')'
			else getOneSym();
		}else error(12);//应是'('

		// 登录符号表
		enterIntoSymTab("main",objFUNCTION,typVOID,t_value,t_addr,t_paramNum);


		if(symbol == LBRACE){
			getOneSym();
			compoundStatement(0);
			if(symbol == CONSTSY) error(35);//声明顺序有误
			if(symbol != RBRACE) error(16);//应是'}'
			else {
				symbol = PSTART;
				getOneSym();
				if(symbol != PSTART)
					error(51);//主函数的函数体之后不允许有内容
			}

			insMidCode("end","","",func_name);// start of function t_name()


		}else error(42);//应是'{'
	}
	else{//非正常进入则说明没有main函数
		error(9);//找不到main函数
	}

	#ifdef showsymtab

	for(int i=1;i<=symtable.subTotal;i++)
		sfout << " " << symtable.indexTab[i] ;

	sfout << endl;

	for(int i=0;i<symtable.level;i++){
		sfout << i <<':' <<endl;
		sfout << "name:"<< symtable.element[i].name
			<< " obj = " << objName[symtable.element[i].obj]
			<< " typ = " << typName[symtable.element[i].typ]
			<< " value = " << (symtable.element[i].value)
			<< " addr = " << symtable.element[i].addr
			<< " paramNum = " << symtable.element[i].paramNum<<'\n' << endl;
    }

	#endif

}


