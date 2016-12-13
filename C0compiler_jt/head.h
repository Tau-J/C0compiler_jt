#include<string>
#include<vector>
#include<sstream>
#include<fstream>
using namespace std;
/**
*	语法分析  函数头部声明
**/
void programProc();
void headProc();
void constProc();
void constDec();
void varProc();
void varDec();
void funcProc();
void returnFuncDec();
void voidFuncDec();
void mainProc();
void compoundStatement(bool ret);
void statementList(bool ret);
void statement(bool ret);
void condition();
void loop();
void valueParamList();
void assignment();
void scanfStatement();
void printfStatement();
void returnStatement(bool retv);
void switchStatement();
void caseStatement();
string caseTable(string,string);
void oneCase();
void callvoid();
void paramTable();
void expression();
void simpleExpression();
void term();
void factor();
void callReturn();
void testSemicolon();
void uInt();


/**
*	词法分析
**/

extern void getOneSym();
extern string token;
extern ofstream fout;

extern char srcin[1024];
extern char *ptr;
extern void getSym();
extern ifstream fin;
extern void programProc();

extern string resWord[];
extern string symName[];

enum symbols {
PSTART,
CONSTSY,
IFSY,
ELSESY,
WHILESY,
SWITCHSY,
CASESY,
DEFAULTSY,
SCANFSY,
PRINTFSY,
RETURNSY,
VOIDSY,
INTSY,
CHARSY,
MAINSY,
IDEN,
CHAR,
STRING,
INT,
PLUS,
MINUS,
STAR,
DIVID,
LPARENT,
RPARENT,
LBRACKET,
RBRACKET,
LBRACE,
RBRACE,
COMMA,
SEMICOLON,
BECOMES,
COLON,
LESSTHAN,
LESSEQU,
MORETHAN,
MOREEQU,
NOTEQU,
EQU
};
extern symbols symbol;

extern unsigned int num;

/**
*	符号表数据结构定义
**/
#define MAX_SYMTAB_LEN 1024
#define MAX_SUBSYMTAB_LEN 1024
#define MAX_IDXTAB_LEN 1024
enum symTabobj {
objCONST,
objVARIABLE,
objFUNCTION,
objPARAM
};
extern symTabobj t_obj;
extern string objName[];

enum symTabtyp {
typINT,
typCHAR,
typVOID,
typINTARRY,
typCHARARRY
};
extern symTabtyp t_typ;
extern string typName[];

// 符号表项定义
class symTab{
public:
	string name;//标识符名
	symTabobj obj;//标识符种类：0常量\1变量\2函数\3参数
	symTabtyp typ;//标识符类型：
			// 0-int\1-char\2-void\3-int[]\4-char[]
	int value;//存储常量值
	int addr;//地址
	int paramNum;//参数个数 对数组而言为元素个数
};

// // 分程序表定义
// class subSymbolTable{
// public:
// 	int last;//该分程序的最后一个标识符在符号表中的位置
// 	int lastparm;//该分程序的最后一个参数在符号表中的位置
// 	int psize;//参数及该分程序在运行栈中的内务信息区所占的存储单元数
// 	int ssize;//整个记录所占的总存储单元数
// };

class SymbolTable{
public:
	symTab element[MAX_SYMTAB_LEN];
	int level;//当前符号表层数
	int subTotal;//分程序总数
	// subSymbolTable subSymTab[MAX_SUBSYMTAB_LEN];
	int indexTab[MAX_IDXTAB_LEN];// 分程序索引表
};
extern SymbolTable symtable;

extern int mainIndex;

extern int loc(string,bool,bool);
extern int locInSubSymTab(string,int);

/**
*	出错处理
**/
extern void error(int errorNum);
extern void errhandler(int handleNum);
extern string errmsg[];
extern int lineNo;
extern bool compileOK;
extern int preErrNum;

/**
*	四元式  函数头部声明
**/

extern string nextVar();
extern string nextLabel();

#define MAX_QUAT_NUM 1024
// 四元式数据结构
class QUAT{
public:
	string opt; //操作符
	string f1; //操作数1
	string f2; //操作数2
	string f3; //操作数3
};
extern QUAT quats[MAX_QUAT_NUM];

extern void insMidCode(string op, string f1, string f2, string f3);
extern void insMidCode(string op, int f1, int f2, string f3);
extern void insMidCode(string op, string f1, int f2, string f3);

extern int varcnt;
extern bool isArry;
extern string res;
extern bool print_char;
extern string func_name;
extern int cur_q;

extern string t_name;
extern symTabobj t_obj;
extern symTabtyp t_typ;
extern int t_value;
extern int t_addr;
extern int t_paramNum;

#define MAX_SSTACK_SIZE 1024
extern string sstack[MAX_SSTACK_SIZE];
extern void midcode2MIPS();
extern stringstream ss;

extern void push_Const_Var(QUAT* q,int t);
extern void asm_func(QUAT* q);
extern void asm_param(QUAT * q);
extern void asm_end(QUAT* q);
extern void asm_call(QUAT* q);
extern void asm_plus(QUAT* q);
extern void asm_minus(QUAT* q);
extern void asm_mult(QUAT* q);
extern void asm_div(QUAT* q);
extern void asm_become(QUAT* q);
extern void asm_arrybecome(QUAT* q);
extern void asm_fupa(QUAT* q);
extern void asm_morethan(QUAT* q);
extern void asm_moreequ(QUAT* q);
extern void asm_lessthan(QUAT* q);
extern void asm_lessequ(QUAT* q);
extern void asm_equ(QUAT* q);
extern void asm_neq(QUAT* q);
extern void asm_jne(QUAT* q);
extern void asm_jmp(QUAT* q);
extern void asm_ret(QUAT* q);
extern void asm_scan(QUAT* q);
extern void asm_print(QUAT* q);
extern void asm_label(QUAT* q);
extern void asm_switch(QUAT* q);


