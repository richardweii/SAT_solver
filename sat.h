/* 此头文件定义了项目用到的数据结构和声明了部分通用函数接口 */
#ifndef _SAT_H_
#define _SAT_H_

/*********************** 数据存储结构 ************************/

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
        int flag;   // 变量真值状态, 1, 0, -1表示满足, 不满足, 待满足
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

/*********************** 函数接口 *************************/
// #ifdef  _PARSE_

/********************* CNF公式解析模块 ********************/

/****************************************
 * 函数:input_parse
 * 参数:
 *      path: 输入文件的路径
 * 返回值: cnf公式
 * 条件: 文件路径存在,且内容格式正确
 * 功能: 对读入的cnf文件进行公式解析
 ****************************************/
CNF input_parse(const char* path);

/****************************************
 * 函数: formula_display
 * 输入参数:
 *      cnf: cnf公式
 * 返回值: 无
 * 条件: cnf公式已成功建立
 * 功能: 将建立的cnf公式输出
 ****************************************/
void formula_display(CNF cnf);

/****************************************
 * 函数名:res_save
 * 输入参数:
 *      path: 保存的路径
 *      cnf: 最终的cnf公式
 *      result: 可满足性
 *      time: DPLL算法运行时间
 * 条件: 路径正确, DPLL结果有效
 * 功能: 将DPLL的结果保存在文件中
 ****************************************/
int res_save(const char* path, CNF cnf, int result, int time);


// #endif
#endif