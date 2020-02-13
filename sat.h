/* 此头文件定义了项目用到的数据结构和声明了部分通用函数接口 */

/* 文字 */
typedef struct literal_
{
    int variable_num;  // 变量序号
    int sign;  //文字的正负
    int value; // 变量的真值, 1, 0 分别表示正负
}Literal;

// typedef Literal* PLiteral;  // 文字指针

/* 子句 */
typedef struct clause_
{
    int clause_num; // 子句序号
    int flag;   // 子句状态, 1, 0, -1 分别表示已满足, 待满足, 不满足
    int length; // 子句长度
    Literal* l_head; // 子句的头文字指针
}Clause;

// typedef Clause* PClause;    // 子句指针
