/* 此头文件定义了项目用到的数据结构和声明了部分通用函数接口 */
#ifndef _SAT_H_INC
#define _SAT_H_INC
#define MAX_CAP 500000
#define MAX_BUFFER 1000
#define True 1
#define False 0
#define Unknown -1
#define Positive 1
#define Negative 0
#define DECAY 0.9
/*********************** 数据存储结构 ************************/
// 变量的序号和子句的序号都从0开始计算

/*************** cnf 公式以及 dpll ****************/
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
    // Status status;   // 文字的满足状态:1, 0, -1分别表示满足,不满足,待满足
    struct literal_* next;   // 构建链表
}* Literal;

// /* 长度变化链，用于子句恢复 */
// typedef struct change_list_
// {
//     int var_order;
//     struct change_list_* ancestor;
// }* Change_list;

/* 子句 */
typedef struct clause_
{
    Status status;   // 子句状态, 1, 0, -1 分别表示 已, 不, 待 满足
    int cause_order; // 导致子句状态改变的变元
    int length; // 子句长度
    Literal l_head; // 子句的头文字指针
}* Clause;

/* 待处理队列 */
typedef struct queue_
{
    // 循环队列
    int size;
    int front; // 队列开始 
    int rear;   // 队列尾
    int* items; // 空间
}* Queue;

/* 变量对子句的引用 */
typedef struct clause_ref_
{
    int order;  // 子句引用序号
    Sign sign;   // 文字的正负1、0分别表示正负
    // Status status; // 子句的状态
    struct clause_ref_* next; // 构建链表
}* Clause_ref;

/* 子句引用集，存储变量的信息 */
typedef struct clause_ref_set_
{
    double positive_score;  // 正文字的score
    double negative_score;  // 负文字的score
    Status status;  // 变元的赋值状态
    int level;  // 变量所被赋值的决策层
    Status is_decision_var; // 是否是决策变量
    Clause_ref clause_ref_head; // 子句引用链表
}* Clause_ref_set;

/* 决策结点，记录DPLL过程，可进行有效链状回溯 */
typedef struct node_
{
    Variable var;
    int level; // 决策层
    Status is_decision_node; // 决策结点识别
    struct node_* ancestor;   // 父节点
}* Node;

/* 决策树 */
typedef struct tree_
{
    int height; // 主枝高度
    Node top;   // 顶结点
}* Tree;

/* SAT求解器 */
typedef struct solver
{
    Status sat;                 // 可满足性，1、0分别表示满足和不满足
    int variable_num;           // 子句集中所含的变元数量
    int cluase_sat_num;         // 子句集中已满足的子句的数量
    int clause_num;             // 子句集中所含子句的数量 
    int original_clauses;       // 原生子句的数量
    int next_place;             // 下一个安放学习子句的位置索引
    int decision_times;         // 变量分支决策的次数
    double time;                // 算法运行的时间
    int rule_times;             // dpll规则成功运用的次数，作为搜索指标参考
    Queue clause_queue;         // 长度为1的子句装入队列中
    Queue cause_queue;          // 冲突节点队列
    Clause* clause_set;         // 指向子句指针区域的指针, 构建 子句-文字 链表数组
    Clause_ref_set* ref_sets;   // 指向子句引用区域的指针, 构建 变量-子句引用 链表数组
    Tree search_tree;           // 搜索链用来得到变量的赋值情况
}* Solver;

/*************** 数独 *****************/

typedef struct _sudoku
{
    int size;       // 数独的棋盘规模
    int variable;   // 最终的变元个数
    int clause;     // 最终的子句个数
    int var_next;  // 下一个顺序附加变元的序号
    int cla_next;  // 下一个顺序子句的序号
    int* prefill;   // 预填的变量
    int pre_num;    // 预填的变量的数目
    int** contents; // 以数组的形式构建子句集
}* Sudoku;



/*********************** 函数接口 *************************/

Queue creat_queue(int cap); // 创建队列     
int Q_out(Queue q); // 出队
void Q_put(Queue q, int order); // 放入队列
void Q_clear(Queue q);  // 清空队列
Status Q_empty(Queue q); // 是否为空，为空返回True否则False

Solver creat_solver();   // 初始化一个空的solver
Solver input_parse(const char* path);   // 文件输入和解析

Clause* creat_clause_set();   // 创建空子句集
void add_clause(Solver s); // 添加空子句
void add_literal(Clause c, int var_order, Sign sign);   // 向子句中添加文字
Clause_ref_set* creat_clause_ref_set(int var_num); // 创建空的子句引用集
void add_clause_ref(Clause_ref_set cr_set, int clause_order, Sign sign);  // 添加子句引用
void count_score(Solver solver, Clause c, int mode);  // 对指定子句中的文字进行计分更新

void cnf_display(Solver s);

int res_save(const char* path, Solver solver);

Solver DPLL(Solver solver); // dpll入口
Status update(Solver solver, Variable v);   // 变量赋值对cnf公式的更新
void tree_grows(Solver solver, Variable v, Status is_decision);  // 搜索树的增长
Status simplify(Solver solver); // 运用单子句规则和纯文字规则化简
Status single_rule(Solver solver);  // 单子句规则
Status pure_rule(Solver solver);    // 纯文字规则
void backtrack(Solver solver, int level, Status is_restart);   // 回溯
int conflict_clause_learning(Solver solver); // 冲突子句学习，返回回溯的层数，返回-1则代表回溯失败
Variable var_decision(Solver solver);   // 变量分支决策
Status check_literal(Sign sign, Status v_status); // 检查一个被赋值的文字是否满足
void decay(Solver solver);  // 计分衰减
void clause_delete(Solver solver, int threshold);   // 删除长度大于threshold且已满足的学习子句

Sudoku sudoku_parse(char* path);    // 从文件读入数独基本信息
int get_clause_num(int size);       // 指定大小的数独格局需要的约束子句个数
int get_variable_num(int size);     // 指定大小的数独格局需要的变元个数
int encode_var(int row, int col, int size); // 从指定的行列转换为棋盘变元序号
void sudoku_rule1(Sudoku sudoku);   // 数独约束1
void sudoku_rule2(Sudoku sudoku);   // 数独约束2
void sudoku_rule3(Sudoku sudoku);   // 数独约束3
Solver sudoku_to_cnf(Sudoku sudolu); // 将已经化好的sudoku转化和输出为cnf公式

#endif