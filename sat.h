/* 此头文件定义了项目用到的数据结构和声明了部分通用函数接口 */
#ifndef _SAT_H_INC
#define _SAT_H_INC

/*********************** 数据存储结构 ************************/
// 变量的序号和子句的序号都从0开始计算

/* 变元 */
typedef struct variable_
{
    int order;
    int value;
}Variable;

/* 文字 */
typedef struct literal_
{
    int order; // 变量序号, 用于访问变量
    int sign; // 文字的正负:0, 1 分别表示 正, 负
    int flag;   // 文字的满足状态:1, 0, -1分别表示满足,不满足,待满足
    struct literal_* next;   // 构建链表
}* Literal;

/* 子句 */
typedef struct clause_
{
    int flag;   // 子句状态, 1, 0, -1 分别表示 已, 不, 待 满足
    int length; // 子句长度
    Literal l_head; // 子句的头文字指针
}* Clause;

/* 子句引用 */
typedef struct clause_ref_
{
    union
    {
        int order;  // 子句引用序号
        int flag;   // (链表头)变量真值状态, 1, 0, -1表示满足, 不满足, 待满足
    };
    union
    {
        int sign;   // 文字的正负
        int frequency; // (链表头)变量被引用的次数
    };
    struct clause_ref_* next; // 构建链表
}* Clause_ref;

/* CNF公式 */
typedef struct cnf_
{
    int variable_num;   // 子句集中所含的变元数量
    int clause_num;     // 子句集中所含子句的数量 
    Clause* clause_set;     // 指向子句指针区域的指针, 构建 子句-文字 邻接表
    Clause_ref* variable;   // 指向子句引用区域的指针, 构建 变量-子句引用
}* CNF;

/* 搜索链，记录DPLL过程，可进行有效链状回溯 */
typedef struct node_
{
    int variable_order; // 选取的中心变元的序号
    int flag;   // 变元在当前节点的真值
    int fixed;  // 变元的真值是否固定
    struct node_* father;   // 父节点
    struct node_* child; // 子节点
}* Node;

/* 运行结果 */
typedef struct result_
{
    int sat;    // 可满足性，1、0分别表示满足和不满足
    int time;   // 算法运行的时间
    int cut;    // dpll规则成功运用的次数，作为搜索指标参考
    Node search_list; // 搜索链用来得到变量的赋值情况
}Result;
/*********************** 函数接口 *************************/
#ifdef  _CNFPARSE_
// 文件读入解析模块函数接口

/****************************************
 * 函数:input_parse
 * 功能: 对读入的cnf文件进行公式解析
 * 参数:
 *      path: 输入文件的路径
 * 返回值: cnf公式
 * 条件: 文件路径存在,且内容格式正确
 ****************************************/
CNF input_parse(const char* path);

/****************************************
 * 函数: formula_display
 * 功能: 将建立的cnf公式输出
 * 输入参数:
 *      cnf: cnf公式
 * 返回值: 无
 * 条件: cnf公式已成功建立
 ****************************************/
void formula_display(CNF cnf);
#endif

#ifdef _RES_SAVE_
// 结果输出保存模块函数接口

/****************************************
 * 函数名:res_save
 * 功能: 将DPLL的结果保存在文件中
 * 输入参数:
 *      path: 保存的路径
 *      res：运行结果
 * 返回值：保存成功返回1，否则返回0
 * 条件: 路径正确, DPLL结果有效
 ****************************************/
int res_save(const char* path, Result res);
#endif

#ifdef _DPLL_
// DPLL算法模块的函数接口

/****************************************
 * 函数：DPLL
 * 功能：DPLL算法的入口，进行DPLL算法的运行
 * 输入参数：
 *      cnf：已建立好的cnf公式
 * 返回值：cnf公式的满足情况，1、0分别为满足和不满足
 * 条件：仅作为DPLL过程的
 ****************************************/
Result DPLL(CNF cnf);

/****************************************
 * 函数：single_detect
 * 功能：将单子句中的文字变为满足状态，同时对其他子句中的该变元进行确定性赋值，将该变元添加到搜索链尾
 * 输入参数：
 *      search_list：搜索链
 *      cnf：cnf公式
 * 返回值：成功运用单子句规则的次数
 ****************************************/
int single_detect(Node seach_list, CNF cnf);

/****************************************
 * 函数：pure_detect
 * 功能：将 在所有子句中都为同一符号的变元赋值，并将该变元添加到搜索链尾
 * 输入参数：
 *      search_list：搜索链
 *      cnf：cnf公式
 * 返回值：成功运用纯文字规则的次数
 ****************************************/
int pure_detect(Node search_list, CNF cnf);

/****************************************
 * 函数：assign_add
 * 功能：将已选好的中心变元赋予一个真假值，并将其添加到搜索链末尾，同时维护子句的满足情况和长度和变量的引用次数
 * 输入参数：
 *      search_list：搜索链
 *      cnf：cnf公式
 *      center_variable：选好的中心变元
 *      value：赋给变元的真假值，1、0分别为真假
 * 返回值：为变元指定真值之后cnf公式的满足情况，1，0，-1分别表示已满足，不满足和待满足
 ****************************************/
int assign_add(Node search_list, CNF cnf, int center_variable, int value);

/****************************************
 * 函数：assign_modify
 * 功能：修改搜索链末尾变元的真值状态，同时使其值当前固定，同时维护子句的满足情况和长度和变量的引用次数
 * 输入参数：
 *      search_list：搜索链表
 *      cnf：cnf公式
 * 条件：搜索链末尾变元的真值尚未固定
 * 返回值：为变元指定真值之后cnf公式的满足情况，1，0，-1分别表示已满足，不满足和待满足
 ****************************************/
int assign_modify(Node search_list, CNF cnf);

/****************************************
 * 函数：check_sat
 * 功能：检查当前的cnf公式是否满足
 * 输入参数：
 *      cnf：cnf公式
 * 返回值：已、不、未满足时分别返回 1、0、-1
 ****************************************/
int check_sat(CNF cnf);

/****************************************
 * 函数：var_decision
 * 功能：从当前的cnf公式中找出一个中心变元进行化简
 * 输入参数：
 *      cnf：cnf公式
 * 返回值：所找到的变元的序号以及为其赋值
 ****************************************/
Variable var_decision(CNF cnf);

/****************************************
 * 函数：backing
 * 功能：回溯搜索链，直到上一个未固定的变元，且在回溯过程中恢复cnf公式和删除搜索链中被回溯的部分
 * 输入参数：
 *      search_list：搜索链
 *      cnf：cnf公式
 * 返回值：回溯成功返回1，否则返回0
 * 条件：当前搜索过程使得公式不满足
 ****************************************/
int backing(Node search_list, CNF cnf);

/****************************************
 * 函数：searching_start
 * 功能：开始搜索，并生成一个搜索链头部
 * 输入参数：
 *      cnf：cnf公式
 * 返回值：搜索链
 * 条件：仅在搜索开始时用于创建搜索链
 ****************************************/
Node searching_start(CNF cnf);

#endif

// #endif
#endif