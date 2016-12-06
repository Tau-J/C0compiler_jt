#include<iostream>
#include<fstream>
#include "head.h"
#define fDEBUG

#ifdef fDEBUG
	#define cout afout
#endif

#define showcomment

using namespace std;

ofstream afout("asm.txt");

int sp = 0; // 栈顶指针
int ap = 0; // 临时记录函数四元式位置
int qp = 0; // 四元式读取指针

stringstream ss;

#define MAX_TMP_VAR_TABLE 1024
class TempVar{
public:
	string name;
	int index;
};

TempVar tempvar_table[MAX_TMP_VAR_TABLE];
int tvp = 0; //temp var table pointer

class REG_POOL{
public:
	int _t[8];
	bool u_t[8];

	int _s[8];
	bool u_s[8];

	REG_POOL(){
		for(int i=0;i<8;i++)
			_t[i] = u_t[i] = 0;
	}

	void clean_t(){
		for(int i=0;i<8;i++)
			_t[i] = u_t[i] = 0;
	}

	void clean_s(){
		for(int i=0;i<8;i++)
			_s[i] = u_s[i] = 0;
	}

	int get_t(){
		for(int i=0;i<8;i++)
			if(!u_t[i]){
				u_t[i] = 1;
				return i;
			}
        return -1;
	}

	int get_s(){
		for(int i=0;i<8;i++)
			if(!u_s[i]){
				u_s[i] = 1;
				return i;
			}
        return -1;
	}

};

REG_POOL reg_pool;

// 字符串表
#define MAX_STRING_TABLE 1024
string strtable[MAX_STRING_TABLE];
int strindex[MAX_STRING_TABLE] = {0};
int strp = 0;

// switch迭代栈，用于恢复sp
#define MAX_SWITCH_STACK 1024
class SwitchStack{
public:
	string name;
	string key;
	int index;
};
SwitchStack switchstack[MAX_SWITCH_STACK];
int swp = 0;

void addtempvar(string name){
	tempvar_table[tvp].name = name;
	tempvar_table[tvp].index = sp -1;
	tvp++;
}
void sinktempvar(){
	// 将栈顶数据下沉一格S
	// 此时$sp正好指向栈顶数据,用t8作为中转完成下沉操作
	#ifdef showcomment
	cout <<"# sink"<<endl;
	#endif
	// lw  $t8,0($sp)
	cout <<"lw\t$t8,0($sp)"<<endl;
	// sw  $t8,4($sp)
	cout <<"sw\t$t8,4($sp)\n"<<endl;

	// 下沉完成后$sp位置正好合适

	// 临时寄存表中的记录也需要更新
	// 栈顶数据一定在表的末尾，可用tvp获取
	tempvar_table[tvp-1].index--;
}

void pushFuncIntoSStack(){


// preabp 将上一级头部fp压栈
	#ifdef showcomment
	cout <<"# preabp = $fp"<<endl;
	#endif
	// sw  $fp,0($sp)
	cout <<"sw\t$fp,0($sp)"<<endl;

// 更新fp
	#ifdef showcomment
	cout <<"# $fp = $sp"<<endl;
	#endif
	// add  $fp,$sp,0
	cout <<"add  $fp,$sp,0"<<endl;

	// add  $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4\n"<<endl;
	sp++;

// ret_addr 本次返回的代码位置
	#ifdef showcomment
	cout <<"# ret_addr = $ra"<<endl;
	#endif
	// sw  $ra,0($sp)
	cout <<"sw\t$ra,0($sp)"<<endl;
	// add  $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4\n"<<endl;
	sp++;
}

// t是临时寄存器编号
void push_Const_Var(QUAT * q){

	int t = reg_pool.get_t();

	if(q->opt == "const"){
			// 常量

			// li  $t0,num
			cout <<"li\t$t"<<t<<","<<q->f2<<endl;

			// sw  $t0,0($sp)
			cout <<"sw\t$t"<<t<<",0($sp)"<<endl;
			// add  $sp,$sp,-4
			cout <<"add\t$sp,$sp,-4\n"<<endl;
			sp++;

			//在符号表里记录相对地址

		}
		else if(q->opt == "int" || q->opt == "char"){
			// 非数组变量

			//sw  $0,0($sp)
			cout <<"sw\t$0,0($sp)"<<endl;
			// add  $sp,$sp,-4
			cout <<"add\t$sp,$sp,-4\n"<<endl;
			sp++;
		}
		else if(q->opt == "int[]" || q->opt == "char[]"){
			// 数组变量

			int len =0;

			ss.str("");
			ss.clear();
			ss << q->f2;
			ss >> len;

			// for(int j=0;j<len;j++){
//
				//sw $0,0($sp)
				// cout <<"sw\t$0,0($sp)"<<endl;
				// //add  $sp,$sp,-4
				// cout <<"add\t$sp,$sp,-4\n"<<endl;
			// 	sp++;
			// }

			int t0 = reg_pool.get_t();
			string label1 = nextLabel();
			string label2 = nextLabel();

			sp += len;
			// li  $t0,len
			cout <<"li\t$t"<<t0<<","<<len<<endl;

			// lab1:
			cout <<label1<<":"<<endl;
			// beq $t0,$0,lab2
			cout <<"beq\t$t"<<t0<<",$0,"<<label2<<endl;
			// sw $0,0($sp)
			cout <<"sw\t$0,0($sp)"<<endl;
			// add  $sp,$sp,-4
			cout <<"add\t$sp,$sp,-4\n"<<endl;
			// add  $t0,$t0,-1
			cout <<"add\t$t"<<t0<<",$t"<<t0<<",-1"<<endl;
			// j  lab1
			cout <<"j\t"<<label1<<endl;
			// lab2:
			cout <<label2<<":"<<endl;

		}
    reg_pool.clean_t();
}

void midcode2MIPS(){

	// 初始化
	sp = 0;
	ap = 0;
	qp = cur_q; // 获取四元式表的长度,cur_q之前用于生成四元式表，此时指向表尾部

	// .data 字符串声明
	cout <<".data\n"<<endl;
	for(cur_q=0;cur_q < qp ; cur_q++)
		if(quats[cur_q].opt == "print" && quats[cur_q].f1 != ""){
			bool flag = 0;
			int s = strp;
			// 优化data区字符串的长度，将重复的字符串整合掉
			for(int i=0;i<strp;i++)
				if(strtable[i] == quats[cur_q].f1){
					s = i;
					flag = 1;
					break;
				}
			if(flag){
				// 如果字符串表里已经有重复的字符串
				// 将该字符串的索引改为已有字符串的索引

			}else{
				// 没有重复字符串
				// 将该字符串加入表中，并记录索引

				strtable[strp++] = quats[cur_q].f1;

				ss.str("");
				ss.clear();
				ss << "str" << s;

				cout << ss.str() << ":\t.asciiz\t\"" << quats[cur_q].f1 <<"\\n\"\n"<<endl;

			}
			// 此处记录索引
			strindex[cur_q] = s;
		}


	cout <<".text"<<endl;

	// 在t9中保存栈底坐标
	// add  $t9,$sp,0
	cout <<"add\t$t9,$sp,0\n"<<endl;

	// fp初始化为栈底
	// add  $fp,$sp,0
	cout <<"add\t$fp,$sp,0\n"<<endl;


	// 全局常变量入栈

	int glen = symtable.indexTab[1];
	for(cur_q=0;cur_q<glen;cur_q++){
		push_Const_Var(&quats[cur_q]);
	}



	// 当运行到这里说明全局常变量已经全部载入栈中,跳转到主函数

	cout <<"j\tmain\n"<<endl;

	// 函数定义区
	for(cur_q = glen;cur_q < mainIndex ; cur_q++){

		//局部常变量声明
		push_Const_Var(&quats[cur_q]);

		if(quats[cur_q].opt == "func"){
			asm_func(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "param"){
			asm_param(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "end"){
			asm_end(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "call"){
			asm_call(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "label"){
			asm_label(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "+"){
			asm_plus(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "-"){
			asm_minus(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "*"){
			asm_mult(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "/"){
			asm_div(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "="){
			asm_become(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "[]="){
			asm_arrybecome(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "fupa"){
			asm_fupa(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == ">"){
			asm_morethan(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == ">="){
			asm_moreequ(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "<"){
			asm_lessthan(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "<="){
			asm_lessequ(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "=="){
			asm_equ(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "!="){
			asm_neq(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "jne"){
			asm_jne(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "jmp"){
			asm_jmp(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "ret"){
			asm_ret(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "scan"){
			asm_scan(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "print"){
			asm_print(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "switch"){
			asm_switch(&quats[cur_q]);
		}
	}

	cur_q = mainIndex; // 获取主函数头部


	for(; cur_q < qp ; cur_q++){
		//局部常变量声明
		push_Const_Var(&quats[cur_q]);

		if(quats[cur_q].opt == "func"){
			asm_func(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "param"){
			asm_param(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "end"){
			asm_end(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "call"){
			asm_call(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "label"){
			asm_label(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "+"){
			asm_plus(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "-"){
			asm_minus(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "*"){
			asm_mult(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "/"){
			asm_div(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "="){
			asm_become(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "[]="){
			asm_arrybecome(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "fupa"){
			asm_fupa(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == ">"){
			asm_morethan(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == ">="){
			asm_moreequ(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "<"){
			asm_lessthan(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "<="){
			asm_lessequ(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "=="){
			asm_equ(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "!="){
			asm_neq(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "jne"){
			asm_jne(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "jmp"){
			asm_jmp(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "ret"){
			asm_ret(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "scan"){
			asm_scan(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "print"){
			asm_print(&quats[cur_q]);
		}
		else if(quats[cur_q].opt == "switch"){
			asm_switch(&quats[cur_q]);
		}
	}




}

/*
* 将name变量的值存入寄存器t中
* 		name可以是整数、临时寄存值
* 当get_addr=1时 返回name在运行栈中的绝对地址
* 当t_offset不为-1时，代表是name[t_offset]，需要加上偏移量
*/
void get_from_stack(int t, string name, bool get_addr = 0, int t_offset = -1){
	#ifdef showcomment
	cout <<"# get " <<name <<endl;
	#endif

//	const char* dis = name.c_str();


	if(name[0] == '-' || ('0' <= name[0] && name[0] <= '9') )
	{// 如果是数字
		// addi  $ti,$0,name
		cout <<"addi\t$t"<<t<<",$0,"<<name<<'\n'<<endl;
		return;
	}
	else if(name[0] == '$'){
	// 如果是临时寄存值


		for(int i=tvp-1;i>=0;i--){
			// 查询临时寄存表，获取运行栈相对地址
			if(tempvar_table[i].name == name){

				// addi  $t,$0,temp_index
				cout <<"addi\t$t"<<t<<",$0,"<<(tempvar_table[i].index * 4)<<endl;
				// 获取绝对地址
				// sub  $t,$fp,$t
				cout <<"sub\t$t"<<t<<",$fp,$t"<<t<<endl;

				if(get_addr)
					return;

				// 按地址取值
				// lw  $t,0($t)
				cout <<"lw\t$t"<<t<<",0($t"<<t<<")\n"<<endl;

				// 取值结束后sp回退一格

				// 如果该临时寄存值为switch key，则不回退
				if(swp == 0){
					// add  $sp,$sp,4
					cout <<"add\t$sp,$sp,4\n"<<endl;
					sp--;

					if(i != tvp-1)
						sinktempvar();
				}else{
					if(name != switchstack[swp-1].key && name != switchstack[swp-1].name){
						// add  $sp,$sp,4
						cout <<"add\t$sp,$sp,4\n"<<endl;
						sp--;

						// 如果取的寄存值不在栈顶，
						// 且不是switch key，
						// 则需要把栈顶数据下沉
						if(i != tvp-1 && name != switchstack[swp-1].name)
							sinktempvar();
					}
				}
				return;
			}
		}

		cout <<"!!!!!!!!!!临时表中未登录的数据!!!!!!!!!!"<<endl;


	}
	// 如果是常变量
	int func_loc = loc(quats[ap].f3,1,0);//符号表中该函数的位置
	int var_loc = locInSubSymTab(name,func_loc); // 符号表中该常变量的位置
	int s_addr;

	// 运行栈中该常变量相对于函数头部的位置
	if(var_loc > func_loc){
        s_addr =   var_loc - func_loc + 1 ;
		for(int i = func_loc+1;i<var_loc;i++)
			if(symtable.element[i].typ == 3 || symtable.element[i].typ == 4)
				s_addr += symtable.element[i].paramNum - 1;
	}
    else if(var_loc < func_loc){
        s_addr = var_loc;
        for(int i = 0;i<var_loc;i++)
        	if(symtable.element[i].typ == 3 || symtable.element[i].typ == 4)
				s_addr += symtable.element[i].paramNum - 1;
    }

    // 先用t存放该值在运行栈中相对于头部的偏移量，t初始化为0
    // li  $t,0
    cout <<"li\t$t"<<t<<",0\n"<<endl;

	if(t_offset > -1){
		// 如果是数组，则在t中加上相应的偏移量
		// t = offset
		// add  $t,$t,$t_offset
		cout <<"add\t$t"<<t<<",$t"<<t<<",$t"<<t_offset<<endl;
	}

	// 加上偏移量
	// t += s_addr
	// addi  $t,$t,s_addr
	cout <<"addi\t$t"<<t<<",$t"<<t<<","<<s_addr<<endl;
	// sll   $t,$t,2
	cout <<"sll\t$t"<<t<<",$t"<<t<<",2"<<endl;

	if(var_loc > func_loc){
		// 局部变量头部为fp
		// sub  $t,$fp,$t
		cout <<"sub\t$t"<<t<<",$fp,$t"<<t<<endl;
	}
	else if(var_loc < func_loc){
		// 全局变量头部为t9
		// sub  $t,$t9,$t
		cout <<"sub\t$t"<<t<<",$t9,$t"<<t<<endl;
	}
	else cout <<"!!!!!!!!!!常变量相对地址!!!!!!!!!!!"<<endl;

	// 此时t中存放的是该变量的运行栈绝对地址

// 获取运行栈中的绝对地址，提前返回，不执行寄存器加载
	if(get_addr)
		return;

	//根据绝对地址取值，存入t中
	// lw    $t,0($t)
	cout <<"lw\t$t"<<t<<",0($t"<<t<<")\n"<<endl;

	return;
}

void asm_become(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif


//    const char* dp = q->opt.c_str();
//	const char* d1 = q->f1.c_str();
//	const char* d2 = q->f2.c_str();
//	const char* d3 = q->f3.c_str();


	int t1,t2;
	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	if( (q->f3)[0] == '$' ){
		if(q->f2 != ""){
			// a = b[i] 形式
			int t3 = reg_pool.get_t();

			get_from_stack(t2,q->f2);
			get_from_stack(t1,q->f1,0,t2);

			sp++;
			addtempvar(q->f3);

			get_from_stack(t3,q->f3,1);

			// sw  $t1,0($t3)
			cout <<"sw\t$t"<<t1<<",0($t"<<t3<<")\n"<<endl;

			// add  $sp,$sp,-4
			cout <<"add\t$sp,$sp,-4\n"<<endl;

		}
		else cout <<"!!!!!!!!!!!!!!!"<<endl;


	}else{
		if(q->f2 != ""){
			// a = b[i] 形式
			int t3 = reg_pool.get_t();

			get_from_stack(t2,q->f2);
			get_from_stack(t1,q->f1,0,t2);
			get_from_stack(t3,q->f3,1);

			// sw  $t1,0($t3)
			cout <<"sw\t$t"<<t1<<",0($t"<<t3<<")\n"<<endl;

		}else{
			// a = b 形式
			get_from_stack(t1,q->f1);
			get_from_stack(t2,q->f3,1);

			if(q->f1 == "temp" && q->f3 == "t")
                cout <<"";

			// sw  $t1,0($t2)
			cout <<"sw\t$t"<<t1<<",0($t"<<t2<<")\n"<<endl;
		}
	}





	reg_pool.clean_t();
}

void asm_func(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	cout << q->f3 << ":" <<endl;
	sp = 0;
	pushFuncIntoSStack();
	ap = cur_q;

}
void asm_param(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	// 从参数寄存器中取出参数，压入栈中
	int s = cur_q - ap - 1;
	// sw  $si,0($sp)
	cout <<"sw\t$s"<<s<<",0($sp)"<<endl;
	//add  $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4\n"<<endl;
	sp++;
}
void asm_end(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	// 还原sp 回到本级的头部
	// add  $sp,$fp,0
	cout <<"add\t$sp,$fp,0"<<endl;

	// 还原ra 以保证正确返回
	// lw  $ra,-4($fp)
	cout <<"lw\t$ra,-4($fp)"<<endl;

	// 还原fp 返回上一级的fp
	// lw  $fp,0($fp)
	cout <<"lw\t$fp,0($fp)"<<endl;


	if(q->f3 != "main"){
		// jr	$ra
		cout << "jr\t$ra\n" << endl;
	}

}
void asm_call(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f3 != ""){
		sp++;
		addtempvar(q->f3);
	}


	reg_pool.clean_s();// 清空s寄存器池

	// jal  target
	cout << "jal\t" << q->f1 <<'\n'<< endl;
}
void asm_plus(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f1 == "0"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f2);

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;

	}
	else if(q->f2 == "0"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f1);

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;

	}
	else{

		int t0,t1;
		t0 = reg_pool.get_t();
		t1 = reg_pool.get_t();

		get_from_stack(t0,q->f1);
		get_from_stack(t1,q->f2);

		// add  $t0,$t0,$t1
		cout << "add\t$t"<<t0<<",$t"<<t0<<",$t"<<t1<<endl;
		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;


	}

	// add $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4"<<endl;
	sp++;

	addtempvar(q->f3);

	reg_pool.clean_t();
}
void asm_minus(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f1 == "0"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f2);

		// sub  $t0,$0,$t0
		cout << "sub\t$t"<<t0<<",$0,$t"<<t0<<endl;

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}
	else if(q->f2 == "0"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f1);

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}
	else {
		int t0,t1;
		t0 = reg_pool.get_t();
		t1 = reg_pool.get_t();

		get_from_stack(t0,q->f1);
		get_from_stack(t1,q->f2);

		// sub  $t0,$t0,$t1
		cout << "sub\t$t"<<t0<<",$t"<<t0<<",$t"<<t1<<endl;
		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}

	//add  $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4\n"<<endl;
	sp++;

	if(q->f3 == "$temp_var11")
        cout<<"";

	addtempvar(q->f3);

	reg_pool.clean_t();
}
void asm_mult(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f1 == "1"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f2);

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;

	}
	else if(q->f2 == "1"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f1);

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}
	else{
		int t0,t1;
		t0 = reg_pool.get_t();
		t1 = reg_pool.get_t();

		get_from_stack(t0,q->f1);
		get_from_stack(t1,q->f2);

		// mult  $t0,$t1
		cout << "mult\t$t"<<t0<<",$t"<<t1<<endl;
		// mflo $t0
		cout << "mflo\t$t"<<t0<<endl;
		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}

	//add  $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4\n"<<endl;
	sp++;

	addtempvar(q->f3);

	reg_pool.clean_t();
}
void asm_div(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f2 == "0"){
		error(52); // 除数不能为0
	}
	else if(q->f2 == "1"){
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f1);

		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}
	else {
		int t0,t1;
		t0 = reg_pool.get_t();
		t1 = reg_pool.get_t();

		get_from_stack(t0,q->f1);
		get_from_stack(t1,q->f2);

		// div  $t0,$t0,$t1
		cout << "div\t$t"<<t0<<",$t"<<t0<<",$t"<<t1<<endl;
		// sw  $t0,0($sp)
		cout << "sw\t$t"<<t0<<",0($sp)"<<endl;
	}



	//add  $sp,$sp,-4
	cout <<"add\t$sp,$sp,-4\n"<<endl;
	sp++;

	addtempvar(q->f3);

	reg_pool.clean_t();
}
void asm_label(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	cout << q->f3 << ":" <<endl;
}
void asm_arrybecome(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2,t3;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();
	t3 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);
	get_from_stack(t3,q->f3,1,t2);

	// sw  $t1,0($t3)
	cout <<"sw\t$t"<<t1<<",0($t"<<t3<<")\n"<<endl;

	reg_pool.clean_t();
}
void asm_morethan(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);

	// sub  $t2,$t2,$t1
	cout <<"sub\t$t"<<t2<<",$t"<<t2<<",$t"<<t1<<endl;

	QUAT* tq = &quats[++cur_q];
	// bgez  $t2,label
	cout <<"bgez\t$t"<<t2<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean_t();
}
void asm_moreequ(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);

	// sub  $t2,$t2,$t1
	cout <<"sub\t$t"<<t2<<",$t"<<t2<<",$t"<<t1<<endl;

	QUAT* tq = &quats[++cur_q];
	// bgtz  $t2,label
	cout <<"bgtz\t$t"<<t2<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean_t();
}
void asm_lessthan(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);

	// sub  $t1,$t1,$t2
	cout <<"sub\t$t"<<t1<<",$t"<<t1<<",$t"<<t2<<endl;

	QUAT* tq = &quats[++cur_q];
	// bgez  $t1,label
	cout <<"bgez\t$t"<<t1<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean_t();
}
void asm_lessequ(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);

	// sub  $t1,$t1,$t2
	cout <<"sub\t$t"<<t1<<",$t"<<t1<<",$t"<<t2<<endl;

	QUAT* tq = &quats[++cur_q];
	// bgtz  $t1,label
	cout <<"bgtz\t$t"<<t1<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean_t();
}
void asm_equ(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);


	QUAT* tq = &quats[++cur_q];
	// bne  $t1,$t2,label
	cout <<"bne\t$t"<<t1<<",$t"<<t2<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean_t();
}
void asm_neq(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);


	QUAT* tq = &quats[++cur_q];
	// bne  $t1,$t2,label
	cout <<"beq\t$t"<<t1<<",$t"<<t2<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean_t();
}
void asm_jne(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	cout <<"!!!!!!!!!!!!!!!!!!!废弃的jne!!!!!!!!!!!!!!!!!!!!!"<<endl;
	//这个函数应该是用不上了
}
void asm_jmp(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif
	cout << "j\t"<< q->f3<<'\n'<<endl;
}
void asm_ret(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif


	if(q->f3 != ""){
		// 当函数有返回值

		int t0;

		t0 = reg_pool.get_t();

		get_from_stack(t0,q->f3);

		// 还原sp 回到本级的头部
		// add  $sp,$fp,0
		cout <<"add\t$sp,$fp,0"<<endl;

		// 还原ra 以保证正确返回
		// lw  $ra,-4($fp)
		cout <<"lw\t$ra,-4($fp)"<<endl;

		// 还原fp 返回上一级的fp
		// lw  $fp,0($fp)
		cout <<"lw\t$fp,0($fp)"<<endl;

		// sw  $t0,0($sp)
		cout <<"sw\t$t"<<t0<<",0($sp)"<<endl;
		// add  $sp,$sp,-4
		cout <<"add\t$sp,$sp,-4\n"<<endl;
		//  此处不必移动sp，
		// 因为如果该return执行了，那么sp会被重置，增加没有意义
		// 如果没有执行，则更不必增加sp
		// sp++;

		reg_pool.clean_t();
	}else{

		// 还原sp 回到本级的头部
		// add  $sp,$fp,0
		cout <<"add\t$sp,$fp,0"<<endl;

		// 还原ra 以保证正确返回
		// lw  $ra,-4($fp)
		cout <<"lw\t$ra,-4($fp)"<<endl;

		// 还原fp 返回上一级的fp
		// lw  $fp,0($fp)
		cout <<"lw\t$fp,0($fp)"<<endl;
	}

	if(quats[cur_q+1].f3 != "main"){
		// 跳转
		// jr  $ra
		cout <<"jr\t$ra\n"<<endl;
	}else{
		// 如果是在主函数中读到return则直接结束程序
		
		// li $v0,10
		cout <<"li\t$v0,10"<<endl;
		cout <<"syscall\n"<<endl;
	}

	// 吃掉没用的end四元式
	cur_q++;
	#ifdef showcomment
	cout << "# end\n" <<endl;
	#endif
}
void asm_scan(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int t0;

	t0 = reg_pool.get_t();

	get_from_stack(t0,q->f3,1);

	// 暴力得无地自容。。
	for(ap = cur_q;ap >=0;ap--)
		if(quats[ap].opt == "func")
			break;

	int func_loc = loc(quats[ap].f3,1,0);//符号表中该函数的位置
	int var_loc = locInSubSymTab(q->f3,func_loc); // 符号表中该常变量的位置

	if(symtable.element[var_loc].typ == typINT){
		// li  $v0,5
		cout <<"li\t$v0,5"<<endl;
		// syscall
		cout <<"syscall"<<endl;
		// sw  $v0,0($t0)
		cout <<"sw\t$v0,0($t"<<t0<<")\n"<<endl;
	}
	else if(symtable.element[var_loc].typ == typCHAR){
		// li  $v0,12
		cout <<"li\t$v0,12"<<endl;
		// syscall
		cout <<"syscall"<<endl;
		// sw  $v0,0($t0)
		cout <<"sw\t$v0,0($t"<<t0<<")\n"<<endl;
	}


}
void asm_print(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f1 != ""){
		// 输出纯字符串

		ss.str("");
		ss.clear();

		ss << "str" << strindex[cur_q];

		// la  $a0,str123
		cout <<"la\t$a0,"<<ss.str()<<endl;
		// li  $v0,4
		cout <<"li\t$v0,4"<<endl;
		// syscall
		cout <<"syscall\n"<<endl;
	}

	if(q->f3 == "int"){
		// 输出整数
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f2);

		// addi  $a0,$t0,0
		cout <<"addi\t$a0,$t"<<t0<<",0"<<endl;
		// li  $v0,1
		cout <<"li\t$v0,1"<<endl;
		// syscall
		cout <<"syscall\n"<<endl;

	}else if(q->f3 == "char"){
		// 输出单个字符
		int t0 = reg_pool.get_t();

		get_from_stack(t0,q->f2);

		// addi  $a0,$t0,0
		cout <<"addi\t$a0,$t"<<t0<<",0"<<endl;
		// li  $v0,11
		cout <<"li\t$v0,11"<<endl;
		// syscall
		cout <<"syscall\n"<<endl;
	#ifdef showcomment
		cout <<"# print \\n"<<endl;
	#endif
		// li  $a0,10
		cout <<"li\t$a0,10"<<endl;
		// li  $v0,11
		cout <<"li\t$v0,11"<<endl;
		// syscall
		cout <<"syscall\n"<<endl;

	}else if(q->f3 == "");
	else cout <<"!!!!!!!!print!!!!!!!!!!"<<endl;



}
void asm_fupa(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	int s = reg_pool.get_s();
	int t = reg_pool.get_t();

	get_from_stack(t,q->f3);

	// add  $s,$t,0
	cout <<"add\t$s"<<s<<",$t"<<t<<",0\n"<<endl;
}

void asm_switch(QUAT* q){
	#ifdef showcomment
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;
	#endif

	if(q->f3 == "start"){
		// start

		string t = nextVar();
		// 备份sp到栈顶
		switchstack[swp].name = t;
		switchstack[swp].index = sp;
		switchstack[swp++].key = q->f1;

		int t0 = reg_pool.get_t();

		// $sp入栈备份
		if( (switchstack[swp-1].key)[0] == '$'){
			// 如果key为临时寄存值则备份$sp+4

			switchstack[swp-1].index = sp - 1;

			// add $t0,$sp,4
			cout <<"add\t$t"<<t0<<",$sp,4"<<endl;
			// sw  $t0,0($sp);
			cout <<"sw\t$t"<<t0<<",0($sp)"<<endl;
			// add  $sp,$sp.-4
			cout <<"add\t$sp,$sp,-4\n"<<endl;
			sp++;
		}else{
			// 否则备份$sp
			// add $t0,$sp,0
			cout <<"add\t$t"<<t0<<",$sp,0"<<endl;
			// sw  $t0,0($sp);
			cout <<"sw\t$t"<<t0<<",0($sp)"<<endl;
			// add  $sp,$sp.-4
			cout <<"add\t$sp,$sp,-4\n"<<endl;
			sp++;
		}

		addtempvar(t);

		reg_pool.clean_t();

	}else{
		// end
		// 还原栈顶sp
		// 此处swp不能直接自减，因为需要完成了取值操作之后再把swp恢复为0
		sp = switchstack[swp-1].index;
		string t = switchstack[swp-1].name;
		// 还原$sp
		int t0 = reg_pool.get_t();
		get_from_stack(t0,t);
		swp--;


		//add  $sp,$t0,0
		cout <<"add\t$sp,$t"<<t0<<",0"<<endl;

		reg_pool.clean_t();

	}
}
