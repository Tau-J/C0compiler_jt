#include "head.h"
#include<iostream>
#include<fstream>

#define fDEBUG
#ifdef fDEBUG
	#define cout mfout
#endif

using namespace std;

ofstream mfout("midcode.txt");


int varcnt = 0;//实参计数
int labcnt = 0;//标签计数

string res;//用于传递表达式与项与因子之间的返回值
bool print_char;// 用于标记该表达式是否只由一个存放字符的常变量构成
string func_name;// 用于记录当前正在执行的函数名

QUAT quats[MAX_QUAT_NUM];

int cur_q = 0; // 四元式表指针



void insMidCode(string op, string f1, string f2, string f3){

	cout <<"quats:"<<cur_q<<"  " << op <<','<< f1 <<','<< f2 <<',' << f3 <<endl;

	quats[cur_q].opt = op;
	quats[cur_q].f1 = f1;
	quats[cur_q].f2 = f2;
	quats[cur_q].f3 = f3;
	cur_q++;
}

void insMidCode(string op, int f1, int f2, string f3){
	cout <<"quats:"<<cur_q<<"  " << op <<','<< f1 <<','<< f2 <<',' << f3 <<endl;

	quats[cur_q].opt = op;

	ss.str("");
	ss.clear();
	ss << f1;
	quats[cur_q].f1 = ss.str();

	ss.str("");
	ss.clear();
	ss << f2;
	quats[cur_q].f2 = ss.str();

	quats[cur_q].f3 = f3;
	cur_q++;

}
void insMidCode(string op, string f1, int f2, string f3){
	cout <<"quats:"<<cur_q<<"  " << op <<','<< f1 <<','<< f2 <<',' << f3 <<endl;

	quats[cur_q].opt = op;
	quats[cur_q].f1 = f1;

	ss.str("");
	ss.clear();
	ss << f2;
	quats[cur_q].f2 = ss.str();

	quats[cur_q].f3 = f3;
	cur_q++;

}

string nextVar(){

    ss.str("");
    ss.clear();
    ss << varcnt++;
	return "$temp_var"+ss.str();
}

string nextLabel(){

	ss.str("");
	ss.clear();
	ss << labcnt++;

	return "temp_label"+ss.str();
}
