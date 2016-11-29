//此版本的运行栈以0x00000000为栈底，后面发现与data区冲突且无法解决，已废弃
#include<iostream>
#include<fstream>
#include "head.h"
#define fDEBUG

#ifdef fDEBUG
	#define cout afout
#endif

// 观察运行栈内容
// #define showsstack

using namespace std;

ofstream afout("asm.txt");

int sp = 0; // 栈顶指针
int ap = 0; // 临时记录函数四元式位置
int qp = 0; // 四元式读取指针

int abp_dis = 0;//指向全局常变量区的开始
int abp_cur = 0;//指向自己的头部
int preabp = 0;//指向上一级模块的头部
int ret_addr = 0;//指向返回地址

stringstream ss;

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

	void clean(){
		for(int i=0;i<8;i++)
			_t[i] = u_t[i] = 0;
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

string sstack[MAX_SSTACK_SIZE];// 模拟运行栈


void pushSStack(string in){ // 压栈操作
	sstack[sp++] = in;
}

void insertAddr2Symb(int idx,int addr){

}

void pushFuncIntoSStack(){

// 记录当前代码执行位置，即pc值，暂时寄存在t9中
	// ret_addr = cur_q + 1;
	// // add  $t9,$ra,0
	// cout <<"add\t$t9,$ra,0" << endl;

// 记录上一级的头部,此时abp_cur指向的是上一级的头部
// 对于汇编来说，上一级的头部此时存在$fp中,暂时寄存在t8中
	preabp = abp_cur;
	// add  $t8,$fp,0
	cout << "add\t$t8,$fp,0" << endl;
// 记录自己的头部位置，即当前sp值
	abp_cur = sp;
	// add  $fp,$0,$sp //用fp存储自己头部
	cout <<"add\t$fp,$0,$sp"<<endl;

// abp_dis
	cout <<"# abp_dis"<<endl;
	// sw  $0,0($sp)
	// pushSStack("abp_dis");
	cout <<"sw\t$0,0($sp)"<<endl;
	// add  $sp,$sp,4
	cout <<"add\t$sp,$sp,4\n"<<endl;

// abp_cur
	cout <<"# abp_cur"<<endl;
	// sw  $fp,0($sp)
	// pushSStack("abp_cur");
	cout <<"sw\t$fp,0($sp)"<<endl;
	// add  $sp,$sp,4
	cout <<"add\t$sp,$sp,4\n"<<endl;

// preabp
	cout <<"# preabp"<<endl;
	// pushSStack("preabp");
	// sw  $t8,0($sp)
	cout <<"sw\t$t8,0($sp)" << endl;
	// add  $sp,$sp,4
	cout <<"add\t$sp,$sp,4\n"<<endl;

// ret_addr
	cout <<"# ret_addr"<<endl;
	// pushSStack("ret_addr");
	// sw  $ra,0($sp)
	cout <<"sw\t$ra,0($sp)" << endl;
	// add  $sp,$sp,4
	cout <<"add\t$sp,$sp,4\n"<<endl;

}

// t是临时寄存器编号
void push_Const_Var(QUAT * q){

	int t = reg_pool.get_t();

	if(q->opt == "const"){
			// 常量

			// li  $t0,1
			cout <<"li\t$t"<<t<<","<<q->f2<<endl;

			// sw  $t0,0($sp)
			pushSStack(q->f3);
			cout <<"sw\t$t"<<t<<",0($sp)"<<endl;
			// add  $sp,$sp,4
			cout <<"add\t$sp,$sp,4\n"<<endl;

			//在符号表里记录相对地址

		}
		else if(q->opt == "int" || q->opt == "char"){
			// 非数组变量

			//sw  $0,0($sp)
			pushSStack(q->f3);
			cout <<"sw\t$0,0($sp)"<<endl;
			// add  $sp,$sp,4
			cout <<"add\t$sp,$sp,4\n"<<endl;
		}
		else if(q->opt == "int[]" || q->opt == "char[]"){
			// 数组变量

			int len =0;

			ss.str("");
			ss.clear();
			ss << q->f2;
			ss >> len;

			for(int j=0;j<len;j++){

				ss.str("");
				ss.clear();
				ss << q->f3 << '[' << j <<']';

				pushSStack(ss.str());
				//sw $0,0($sp)
				cout <<"sw\t$0,0($sp)"<<endl;
				//add  $sp,$sp,4
				cout <<"add\t$sp,$sp,4\n"<<endl;
			}
		}
    reg_pool.clean();
}

void midcode2MIPS(){

	// 初始化
	sp = 0;
	ap = 0;
	qp = 0;

	cout <<".text"<<endl;
	cout << "li\t$sp,0\n" << endl;



	qp = cur_q; // 获取四元式表的长度,cur_q之前用于生成四元式表，此时指向表尾部

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
	}

	cur_q = mainIndex; // 获取主函数头部


	for(; cur_q < qp ; cur_q++){
		//局部常变量声明
		push_Const_Var(&quats[cur_q]);

        if(cur_q == 26)
            cout << endl;

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
	}




	#ifdef showsstack
	// 观察运行栈状态
	cout <<"my sstack:"<<endl;
	for(int i=0;i<sp;i++)
		cout << sstack[i] << endl;
	#endif

}

/*
* 将name变量的值存入寄存器t中
* 		name可以是整数、临时寄存值
* 当get_addr=1时 返回name在运行栈中的绝对地址
* 当t_offset不为-1时，代表是name[t_offset]，需要加上偏移量
*/
void get_from_stack(int t, string name, bool get_addr = 0, int t_offset = -1){

	cout <<"# get " <<name <<endl;

//	const char* dis = name.c_str();


	if(name[0] == '-' || ('0' <= name[0] && name[0] <= '9') )
	{// 如果是数字
		// addi  $ti,$0,name
		cout <<"addi\t$t"<<t<<",$0,"<<name<<'\n'<<endl;
		return;
	}
	else if(name[0] == '$'){
	// 如果是临时寄存值

		// add  $t,$sp,0
		cout <<"add\t$t"<<t<<",$sp,0"<<endl;

		if(get_addr)
			return;
		else if(quats[cur_q-1].f3 == quats[cur_q].f2 && quats[cur_q].f1 == name){
			// 此时要取的数在sp的前一位,取出后将栈顶元素下降一位，sp回退一位

			int tt = reg_pool.get_t();

			// lw  $t,-4($t)
			cout <<"lw\t$t"<<t<<",-4($t"<<t<<")"<<endl;
			// lw  $tt,0($t)
			cout <<"lw\t$t"<<tt<<",0($sp)"<<endl;
			// sub  $sp,$sp,4
			cout <<"sub\t$sp,$sp,4\n"<<endl;
			// sw  $tt,0($sp)
			cout <<"sw\t$t"<<tt<<",0($sp)\n"<<endl;
			return;
		}
		else{
			// 此时sp未后移，仍然指向临时寄存值本身
			//	lw  $t,0($t)
			cout <<"lw\t$t"<<t<<",0($t"<<t<<")\n"<<endl;
			return;
		}
	}
	// 如果是常变量
	int func_loc = loc(quats[ap].f3,1,0);//符号表中该函数的位置
	int var_loc = locInSubSymTab(name,func_loc); // 符号表中该常变量的位置
	int s_addr;

	if(var_loc > func_loc)
        s_addr = var_loc - func_loc + 4 - 1; // 运行栈中该常变量相对于函数头部的位置
    else if(var_loc < func_loc)
        s_addr = var_loc;

    cout <<"li\t$t"<<t<<",0\n"<<endl;

	if(t_offset > -1){
		// t = offset
		// add  $t,$0,$t_offset
		cout <<"add\t$t"<<t<<",$0,$t"<<t_offset<<endl;
	}

	// t += s_addr
	// add  $t,$t,s_addr
	cout <<"add\t$t"<<t<<",$t"<<t<<","<<s_addr<<endl;
	// sll   $t,$t,2
	cout <<"sll\t$t"<<t<<",$t"<<t<<",2"<<endl;
	if(var_loc > func_loc){
		// add  $t,$fp,$t
		cout <<"add\t$t"<<t<<",$fp,$t"<<t<<endl;
	}


// 获取运行栈中的地址，提前返回，不执行寄存器加载
	if(get_addr)
		return;

	//此时$t中存的是变量name在运行栈中的地址
	// lw    $t,0($t)
	cout <<"lw\t$t"<<t<<",0($t"<<t<<")\n"<<endl;

	return;
}

void asm_become(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	int t1,t2;

//    const char* dp = q->opt.c_str();
//	const char* d1 = q->f1.c_str();
//	const char* d2 = q->f2.c_str();
//	const char* d3 = q->f3.c_str();

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	if(q->f2 != ""){
		// a = b[i] 形式
		int t3 = reg_pool.get_t();

		get_from_stack(t2,q->f2);
		get_from_stack(t1,q->f1,0,t2);
		get_from_stack(t3,q->f3,1);
	}else{
		// a = b 形式
		get_from_stack(t1,q->f1);
		get_from_stack(t2,q->f3,1);
	}

	// sw  $t1,0($t2)
	cout <<"sw\t$t"<<t1<<",0($t"<<t2<<")\n"<<endl;

	reg_pool.clean();
}

void asm_func(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	cout << q->f3 << ":" <<endl;
	pushFuncIntoSStack();
	ap = cur_q;
}
void asm_param(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	// 从参数寄存器中取出参数，压入栈中
	int s = cur_q - ap - 1;
// sw  $si,0($sp)
	cout <<"sw\t$s"<<s<<",0($sp)"<<endl;
	//add  $sp,$sp,4
	cout <<"add\t$sp,$sp,4\n"<<endl;
}
void asm_end(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	if(q->f3 != "main"){
		// jr	$ra
		cout << "jr\t$ra\n" << endl;
	}

}
void asm_call(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	// jal  target
	cout << "jal\t" << q->f1 <<'\n'<< endl;
}
void asm_plus(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	if(quats[cur_q+1].f1 != quats[cur_q].f3 && quats[cur_q+1].f2 != quats[cur_q].f3){
		//add  $sp,$sp,4
		cout <<"add\t$sp,$sp,4\n"<<endl;
	}


	reg_pool.clean();
}
void asm_minus(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	if(quats[cur_q+1].f1 != quats[cur_q].f3 && quats[cur_q+1].f2 != quats[cur_q].f3){
		//add  $sp,$sp,4
		cout <<"add\t$sp,$sp,4\n"<<endl;
	}


	reg_pool.clean();
}
void asm_mult(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	if(quats[cur_q+1].f1 != quats[cur_q].f3 && quats[cur_q+1].f2 != quats[cur_q].f3){
		//add  $sp,$sp,4
		cout <<"add\t$sp,$sp,4\n"<<endl;
	}

	reg_pool.clean();
}
void asm_div(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	if(quats[cur_q+1].f1 != quats[cur_q].f3 && quats[cur_q+1].f2 != quats[cur_q].f3){
		//add  $sp,$sp,4
		cout <<"add\t$sp,$sp,4\n"<<endl;
	}


	reg_pool.clean();
}
void asm_label(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	cout << q->f3 << ":" <<endl;
}
void asm_arrybecome(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	int t1,t2,t3;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();
	t3 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);
	get_from_stack(t3,q->f3,1,t2);

	reg_pool.clean();
}
void asm_morethan(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	reg_pool.clean();
}
void asm_moreequ(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	reg_pool.clean();
}
void asm_lessthan(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	reg_pool.clean();
}
void asm_lessequ(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

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

	reg_pool.clean();
}
void asm_equ(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);


	QUAT* tq = &quats[++cur_q];
	// bne  $t1,$t2,label
	cout <<"bne\t$t"<<t1<<",$t"<<t2<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean();
}
void asm_neq(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	int t1,t2;

	t1 = reg_pool.get_t();
	t2 = reg_pool.get_t();

	get_from_stack(t1,q->f1);
	get_from_stack(t2,q->f2);


	QUAT* tq = &quats[++cur_q];
	// bne  $t1,$t2,label
	cout <<"beq\t$t"<<t1<<",$t"<<t2<<","<<tq->f3<<'\n'<<endl;

	reg_pool.clean();
}
void asm_jne(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	cout <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	//这个函数应该是用不上了
}
void asm_jmp(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	cout << "j\t"<< q->f3<<'\n'<<endl;
}
void asm_ret(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	if(q->f3 == ""){
		return;
	}
	else{
		int t0;

		t0 = reg_pool.get_t();

		get_from_stack(t0,q->f3);

		// // add $v0,$0,$t0
		// cout <<"add\t$v0,$0,$t"<<t0<<'\n'<<endl;

		// sw  $t0,0($sp)
		cout <<"sw\t$t"<<t0<<",0($sp)"<<endl;
		// add $sp,$sp,4
		cout <<"add\t$sp,$sp,4\n"<<endl;

		reg_pool.clean();
	}
}
void asm_scan(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	int t0;

	t0 = reg_pool.get_t();

	get_from_stack(t0,q->f3,1);

	// li  $v0,5
	cout <<"li\t$v0,5"<<endl;
	// syscall
	cout <<"syscall"<<endl;
	// sw  $v0,0($t0)
	cout <<"sw\t$v0,0($t"<<t0<<")\n"<<endl;
}
void asm_print(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;

	if(q->f3 == ""){
		// 输出纯字符串
	}
	else if(q->f3 == "int"){
		// 输出整数
	}else if(q->f3 == "char"){
		// 输出单个字符
	}else cout <<"!!!!!!!!!!!!!!!!!!"<<endl; 
}
void asm_fupa(QUAT* q){
	cout <<"# "<< q->opt <<','<< q->f1 <<','<< q->f2 <<',' << q->f3 <<endl;


}
