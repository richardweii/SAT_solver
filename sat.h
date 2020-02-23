/* 此头文件定义了项目用到的数据结构和声明了部分通用函数接口 */
#ifndef _SAT_H_INC
#define _SAT_H_INC
#define MAX_CAP 1000
#define True 1
#define Flase 0
#define Unknown -1
#define Positive 1
#define Negative 0
/*********************** 数据存储结构 ************************/
// 变量的序号和子句的序号都从0开始计算

 /* 状态 */
typedef int Status;

/* 文字的符号 */
typedef int Sign;


/* 变量 */
typedef struct variable_
{
    int order;  // 变元序号
    Status status;  // 变元真值
}Variable;

/* 文字 */
typedef struct literal_
{
    int order; // 变量序号, 用于访问变量
    Sign sign; // 文字的正负:1, 0 分别表示 正, 负
    Status status;   // 文字的满足状态:1, 0, -1分别表示满足,不满足,待满足
    struct literal_* next;   // 构建链表
}* Literal;

/* 子句 */
typedef struct clause_
{
    Status status;   // 子句状态, 1, 0, -1 分别表示 已, 不, 待 满足
    int length; // 子句长度
    double changing_level_old; // 上一个修改长度的决策层
    double changing_level_now; // 当前修改长度的决策层
    Literal l_head; // 子句的头文字指针
}* Clause;

/* 变量对子句的引用 */
typedef struct clause_ref_
{
    int order;  // 子句引用序号
    Sign sign;   // 文字的正负1、0分别表示正负
    Status status; // 子句的状态
    struct clause_ref_* next; // 构建链表
}* Clause_ref;

/* 子句引用集，存储变量的信息 */
typedef struct clause_ref_set_
{
    int positive_score;  // 正文字的score
    int negative_score;  // 负文字的score
    Status status;  // 变元的赋值状态
    Clause_ref clause_ref_head; // 子句引用链表
}* Clause_ref_set;

/* 决策结点，记录DPLL过程，可进行有效链状回溯 */
typedef struct node_
{
    Variable var;
    int level; // 决策层
    Status is_decision_node; // 决策结点识别
    struct node_* father;   // 父节点
    struct node_* child; // 子节点
}* Node;

/* 决策树 */
typedef struct tree_
{
    int length; // 主枝高度
    Node root;  // 根
    Node top;   // 顶结点
}* Tree;

/* 求解器 */
typedef struct solver
{
    Status sat;    // 可满足性，1、0分别表示满足和不满足
    int variable_num;   // 子句集中所含的变元数量
    int clause_num;     // 子句集中所含子句的数量 
    int decision_times;  // 变量分支决策的次数
    double time;   // 算法运行的时间
    int rule_times;    // dpll规则成功运用的次数，作为搜索指标参考
    Clause* clause_set;     // 指向子句指针区域的指针, 构建 子句-文字 链表数组
    Clause_ref_set* ref_sets;   // 指向子句引用区域的指针, 构建 变量-子句引用 链表数组
    Tree search_tree; // 搜索链用来得到变量的赋值情况
}* Solver;
/*********************** 函数接口 *************************/

Solver creat_solver();   // 初始化一个空的solver
Solver input_parse(const char* path);   // 文件输入和解析

Clause* creat_clause_set();   // 创建空子句集
void add_clause(Solver s); // 添加空子句
void add_literal(Clause c, int var_order, Sign sign);   // 向子句中添加文字
Clause_ref_set* creat_clause_ref_set(int var_num); // 创建空的子句引用集
void add_clause_ref(Clause_ref_set cr_set, int clause_order, Sign sign);  // 添加子句引用

void cnf_display(Solver s);

int res_save(const char* path, Solver solver);
Solver DPLL(Solver solver);


/****************************************
 * 函数：single_detect
 * 功能：将单子句中的文字变为满足状态，同时对其他子句中的该变元进行确定性赋值，将该变元添加到搜索链尾
 * 输入参数：
 *      search_list：搜索链
 *      cnf：cnf公式
 * 返回值：成功运用单子句规则的次数
 ****************************************/
// int single_detect(List search_list, CNF cnf);

// /****************************************
//  * 函数：pure_detect
//  * 功能：纯文字规则，将只有一种文字符号的变元确定性赋值，并将其添加到搜索链尾
//  * 输入参数：
//  *      search_list：搜索链
//  *      cnf：cnf公式
//  * 返回值：成功运用纯文字规则的次数
//  ****************************************/
// int pure_detect(List search_list, CNF cnf);
// /****************************************
//  * 函数：assign_add
//  * 功能：为变元赋予一个真假值，并将其添加到搜索链末尾，同时维护子句的满足情况和长度和变量的引用次数
//  * 输入参数：
//  *      search_list：搜索链
//  *      cnf：cnf公式
//  *      var：选好的中心变元
//  *      fixed：该变元是否固定
//  * 返回值：为变元指定真值之后cnf公式的满足情况，1，0，-1分别表示已满足，不满足和待满足
//  ****************************************/
// int assign_add(List search_list, CNF cnf, Variable var, int fixed);


// /****************************************
//  * 函数：assign_modify
//  * 功能：修改搜索链末尾变元的真值状态，同时使其值当前固定，同时维护子句的满足情况和长度和变量的引用次数
//  * 输入参数：
//  *      search_list：搜索链表
//  *      cnf：cnf公式
//  * 条件：搜索链末尾变元的真值尚未固定
//  * 返回值：为变元指定真值之后cnf公式的满足情况，1，0，-1分别表示已满足，不满足和待满足
//  ****************************************/
// int assign_modify(List search_list, CNF cnf);


// /****************************************
//  * 函数：update_cnf
//  * 功能：按照变元的赋予真值情况更新cnf公式，assign_add 和assign_modify的基础函数
//  * 参数：
//  *      v：变元
//  *      cnf：当前的cnf公式
//  * 返回值：更新之后cnf公式的状态，1、0和-1分别表示满足、不满足和待满足
//  ****************************************/
// int update(Variable v, CNF cnf);


// /****************************************
//  * 函数：check_sat
//  * 功能：检查当前的cnf公式是否满足
//  * 输入参数：
//  *      cnf：cnf公式
//  * 返回值：已、不、未满足时分别返回 1、0、-1
//  ****************************************/
// int check_sat(CNF cnf);


// /****************************************
//  * 函数：var_decision
//  * 功能：从当前的cnf公式中找出一个中心变元进行化简
//  * 输入参数：
//  *      cnf：cnf公式
//  * 返回值：所找到的变元的序号以及为其赋值
//  ****************************************/
// Variable var_decision(CNF cnf);


// /****************************************
//  * 函数：backing
//  * 功能：回溯搜索链，直到上一个未固定的变元，且在回溯过程中恢复cnf公式和删除搜索链中被回溯的部分
//  * 输入参数：
//  *      search_list：搜索链
//  *      cnf：cnf公式
//  * 返回值：回溯成功返回1，否则返回0
//  * 条件：当前搜索过程使得公式不满足，使用该函数之后需要一次assign_modify过程
//  ****************************************/
// int backing(List search_list, CNF cnf);


// /****************************************
//  * 函数：searching_start
//  * 功能：开始搜索，并生成一个搜索链头部
//  * 输入参数：
//  *      cnf：cnf公式
//  * 返回值：搜索链
//  * 条件：仅在搜索开始时用于创建搜索链
//  ****************************************/
// List searching_start(CNF cnf);


// // #endif
#endif