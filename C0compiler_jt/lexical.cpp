//#define cDEBUG
 #define fDEBUG
 //#define DISPLAY
 #define sDEBUG
#include "head.h"
#include<iostream>
#include<fstream>
#include<sstream>
#include<cstring>
using namespace std;

#define RESWORDNUM 15

#ifdef fDEBUG
ofstream fout("result2.txt");
#endif


int lineNo = 0;

char chr = '\0';
string token;
unsigned int num;

symbols symbol;
char *ptr;
char srcin[1024];
int symCount = 0;
ifstream fin;

bool dontChange = 0;

string resWord[] =
{
	"","const","if","else","while","switch",
	"case","default","scanf","printf",
	"return","void","int","char","main"
};
string symName[] =
{
"",
"CONSTSY",
"IFSY",
"ELSESY",
"WHILESY",
"SWITCHSY",
"CASESY",
"DEFAULTSY",
"SCANFSY",
"PRINTFSY",
"RETURNSY",
"VOIDSY",
"INTSY",
"CHARSY",
"MAINSY",
"IDEN",
"CHAR",
"STRING",
"INT",
"PLUS",
"MINUS",
"STAR",
"DIVID",
"LPARENT",
"RPARENT",
"LBRACKET",
"RBRACKET",
"LBRACE",
"RBRACE",
"COMMA",
"SEMICOLON",
"BECOMES",
"COLON",
"LESSTHAN",
"LESSEQU",
"MORETHAN",
"MOREEQU",
"NOTEQU",
"EQU"
};

string objName[] ={
"CONST",
"VARIABLE",
"FUNCTION",
"PARAM"
};

string typName[] ={
"int",
"char",
"void",
"int[]",
"char[]"
};

void display();

bool isSpace(){
	return (chr == ' ') ? 1 : 0;
}

bool isNewline(){
	return (chr == '\n') ? 1 : 0;
}

bool isTab(){
	return (chr == '\t') ? 1 : 0;
}

// 判断是否为字母
bool isLetter(){
	if( ('A' <= chr && chr <= 'Z') ||
	    ('a' <= chr && chr <= 'z') ||
	    	chr == '_' )
			return 1;
	else return 0;
}

// 判断是否为数字
bool isDigit(){
	if('0' <= chr && chr <= '9')return 1;
	else return 0;
}

bool isColon(){
	return (chr == ':') ? 1 : 0;
}

bool isComma(){
	return (chr == ',') ? 1 : 0;
}

bool isSemi(){
	return (chr == ';') ? 1 : 0;
}

bool isEqu(){
	return (chr == '=') ? 1 : 0;
}

bool isPlus(){
	return (chr == '+') ? 1 : 0;
}

bool isMinus(){
	return (chr == '-') ? 1 : 0;
}

bool isStar(){
	return (chr == '*') ? 1 : 0;
}

bool isDivid(){
	return (chr == '/') ? 1 : 0;
}

bool isLpar(){
	return (chr == '(') ? 1 : 0;
}

bool isRpar(){
	return (chr == ')') ? 1 : 0;
}

bool isLbrkt(){
	return (chr == '[') ? 1 : 0;
}

bool isRbrkt(){
	return (chr == ']') ? 1 : 0;
}

bool isLbrce(){
	return (chr == '{') ? 1 : 0;
}

bool isRbrce(){
	return (chr == '}') ? 1 : 0;
}

bool isSQuote(){
	return (chr == '\'') ? 1 : 0;
}

bool isDQuote(){
	return (chr == '"' ) ? 1 : 0;
}

bool isLessThan(){
	return (chr == '<') ? 1 : 0;
}

bool isMoreThan(){
	return (chr == '>') ? 1 : 0;
}

bool isNot(){
	return (chr == '!') ? 1 : 0;
}

bool isValidStrChar(){
	if (chr == 32 ||
			chr == 33 ||
			(35 <= chr && chr <= 126))
		return 1;
	else return 0;
}
void catToken(){
	// 因为最后是在mars中运行，直接原样复制的话，
	// 遇到\n这种符号会被mars识别为转义后的符号
	// 因此还需要自行转义一次
	if(chr == '\\')token += "\\\\";
	else token += chr;
}

void retract(){
	ptr--;
}

void getChar(){

	chr = *ptr++;

	if( !dontChange ){
		// 所有大写都转为小写 A 65 a 97 相差32
		if('A' <= chr && chr <= 'Z')
			chr += 32;
	}
	
}

// 如果是保留字则返回对应的类编码，否则返回-1
int reserver(){
	for(int i=0;i<RESWORDNUM;i++){
		if(token == resWord[i])return i;
	}
	return -1;
}

void transNum(){

	// 检查数字是否超出范围

	// error:20 
	// 数字太大，最多10位数字，且绝对值不超过2147483647
	int len = token.length();
	if(len > 10)error(20);
	if(len == 10 && token[0] > '3')error(20);


	ss.str("");
	ss.clear();

	ss << token;
	ss >> num;

	if(symbol != MINUS && num > 2147483647)error(20);
	if(symbol == MINUS && num > 2147483648)error(20);
}

void clearToken(){
	token = "";
}

void getOneSym(){

	if(chr == '\0'){
		bool r = 1;
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
				r = 0;
				break;
			}

		}
		if(r)return;
	}

	clearToken();

	while( isSpace() || isNewline() || isTab())
		getChar();

	// if(chr == '\0')return;

	if(isLetter()){
		while(isLetter() || isDigit()){
			catToken();
			getChar();
		}
		retract();

		int resultValue = reserver();
		if(resultValue == -1)symbol = IDEN;
		else symbol = symbols(resultValue);
	}
	else if(isDigit()){
		while(isDigit()){
			catToken();
			getChar();
		}
		retract();

		transNum();
		symbol = INT;
	}
	else if(isPlus()){
		symbol = PLUS;
	}
	else if(isMinus()){
		symbol = MINUS;
	}
	else if(isStar()){
		symbol = STAR;
	}
	else if(isDivid()){
		symbol = DIVID;
	}
	else if(isLpar()){
		symbol = LPARENT;
	}
	else if(isRpar()){
		symbol = RPARENT;
	}
	else if(isLbrkt()){
		symbol = LBRACKET;
	}
	else if(isRbrkt()){
		symbol = RBRACKET;
	}
	else if(isLbrce()){
		symbol = LBRACE;
	}
	else if(isRbrce()){
		symbol = RBRACE;
	}
	else if(isComma()){
		symbol = COMMA;
	}
	else if(isSemi()){
		symbol = SEMICOLON;
	}
	else if(isEqu()){
		getChar();
		if(isEqu()){
			token = "==";
			symbol = EQU;
		}
		else {
			retract();
			token = "=";
			symbol = BECOMES;
		}
	}
	else if(isSQuote()){
		dontChange = 1;
		getChar();
		dontChange = 0;
		if(isPlus() || isMinus() ||
		   isStar() || isDivid() ||
		   isLetter() || isDigit()){
			token += chr;
			getChar();
			if(isSQuote()) symbol = CHAR;
			else error(3);
		}else error(4);
	}
	else if(isDQuote()){
		dontChange = 1;
		getChar();
		

		bool errorFlag = 0;

		while( ! isDQuote()){

			if( ! isValidStrChar()){
				errorFlag = 1;
				break;
			}

			catToken();
			getChar();

			if(chr == '\0'){
				error(5);
				break;
			}
		}
		dontChange = 0;

		if(errorFlag) error(6);

		// if(token == "") error(28); // 字符串可以是空串，此条错误已移除

		symbol = STRING;
	}
	else if(isColon()){
		symbol = COLON;
	}
	else if(isLessThan()){
		getChar();
		if(isEqu()) {
			token = "<=";
			symbol = LESSEQU;
		}
		else {
			retract();
			token = "<";
			symbol = LESSTHAN;
		}
	}
	else if(isMoreThan()){
		getChar();
		if(isEqu()){
			token = ">=";
			symbol = MOREEQU;
		}
		else{
			retract();
			token = ">";
			symbol = MORETHAN;
		}
	}
	else if(isNot()){
		getChar();
		if(isEqu()){
			token = "!=";
			symbol = NOTEQU;
		}
		else error(2);
	}
	else error(1);

    #ifdef DISPLAY
	display();
	#endif

	getChar();
}
void display(){

    #ifndef fDEBUG
     cout << ++symCount << '\t';
	 cout << symName[symbol] << "\t\t";
	 if (token == ""){
	 	if(chr == '"')return;
	  	cout << chr <<endl;
	 }
	 else cout << token << endl;

    #endif

    #ifdef fDEBUG
        fout << ++symCount << '\t';
        fout << symName[symbol] << "\t\t";
        if (token == ""){
        	if(chr == '"')return;
        	fout << chr <<endl;
        }
        else fout << token << endl;
    #endif

    // if(symCount == 5)
    //     cout <<"";

}

void getSym(){

// 去掉行尾的空格\tab\换行符
	// for(int i=strlen(srcin)-1;srcin[i] == ' ' ||
	// 						 srcin[i] == '\t' ||
	// 						 srcin[i] == '\n';
	// 	i--)
	// 	srcin[i] = '\0';

	// getChar();


	// if(NULL != fin.getline(srcin,1024)){
	// 		ptr = srcin;

	// 		for(int i=strlen(srcin)-1;
	// 				srcin[i] == ' ' ||
	// 				srcin[i] == '\t' ||
	// 				srcin[i] == '\n';
	// 				i--)
	// 			srcin[i] = '\0';

	// 		getChar();

	// 	}
	while(NULL != fin.getline(srcin,1024)){

		ptr = srcin;

		for(int i=strlen(srcin)-1;
				srcin[i] == ' ' ||
				srcin[i] == '\t' ||
				srcin[i] == '\n';
				i--)
			srcin[i] = '\0';

		getChar();

		while(chr != '\0'){
			getOneSym();

			// getChar();
		}

		#ifdef fDEBUG
        fout << "-----------------" << endl;
	    #endif // DEBUG
	    #ifdef cDEBUG
	        cout << "-----------------" << endl;
	    #endif // fDEBUG

	}

}
