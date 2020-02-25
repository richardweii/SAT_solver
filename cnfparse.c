/* 输入解析模块 */
#define _CNFPARSE_
#define MAX_BUFFER 1000
#include"sat.h"
#include<stdio.h>
#include<stdlib.h>

Solver input_parse(const char* path)
{
    FILE* file;
    char* buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);
    if((file = fopen(path, "r")) == NULL)
    {
        printf("ERROR: Open failed!!!\n");
        return NULL;
    }
    int i;  // 循环控制
    int num;    // 字符串转化为数字时的中介变量
    int sign;   // 表示文字的正负:0,1 分别表示正, 负
    int line = -1;   // 记录子句的行数
    Solver solver = creat_solver();
    while(True)
    {
        fgets(buffer, MAX_BUFFER, file);
        if(feof(file))
            break;
        switch (buffer[0])
        {
            // 行首为‘c’表示为注释
            case 'c':
            {
                continue;
                break;
            }

            // 行首为‘p’则为基本信息行
            case 'p':
            {
                i = 1;
                while (buffer[i] != '\n')
                {
                    if(buffer[i] == ' ' || (buffer[i] == 'c' && buffer[++i] == 'n' && buffer[++i] == 'f'))
                    {
                        i++;
                        continue;
                    }
                    else if (buffer[i] <= '9' && buffer[i] >= '0')
                    {
                        // cnf后面紧跟的是变元数量
                        solver->variable_num = buffer[i] - '0';
                        while (buffer[++i] <= '9' && buffer[i] >= '0')
                        {
                            solver->variable_num = solver->variable_num * 10 + (buffer[i] - '0');
                        }
                        solver->clause_queue = creat_queue(MAX_CAP / 10);
                        solver->cause_queue = creat_queue(solver->variable_num * 3);
                        break;
                    }
                }
                solver->clause_set = creat_clause_set();
                solver->ref_sets = creat_clause_ref_set(solver->variable_num);
                if(line == -1)
                    line = 0;
                else
                {
                    printf("ERROR: cnf information initialization duplicates");
                    exit(0);
                }
                break;
            }

            // 子句行
            default:
            {
                if(line == -1)
                {
                    printf("ERROR: cnf information haven't been initialize");
                    exit(0);
                }
                if(buffer[0] != '-' && (buffer[0] < '0' || buffer[0] > '9'))
                    break;
                i = 0;
                add_clause(solver);
                while (buffer[i] != '\n')
                {
                    if(buffer[i] == '-')
                    {
                        sign = Negative;
                        num = buffer[++i] - '0';
                        while(buffer[++i] != ' ')
                        {
                            num = num * 10 + (buffer[i] - '0');
                        }
                        add_literal(solver->clause_set[line], num - 1, sign);
                        add_clause_ref(solver->ref_sets[num - 1], line, sign);
                    }
                    else if(buffer[i] > '0' && buffer[i] <= '9')
                    {
                        sign = Positive;
                        num = buffer[i] - '0';
                        while(buffer[++i] != ' ')
                        {
                            num = num * 10 + (buffer[i] - '0');
                        }
                        add_literal(solver->clause_set[line], num - 1, sign);
                        add_clause_ref(solver->ref_sets[num - 1], line, sign);
                    }
                    else if(buffer[i] == '0')
                    {
                        break;
                    }
                    i++;
                }
                if(solver->clause_set[line]->length == 1)
                    Q_put(solver->clause_queue, line);
                line++;
                break;
            }
        }
    }
    printf("Input successfully!!!\n");
    fclose(file);
    return solver;
}

Solver creat_solver()
{
    Solver s = (Solver)malloc(sizeof(struct solver));
    s->search_tree = (Tree)malloc(sizeof(struct tree_));
    s->search_tree->top = (Node)malloc(sizeof(struct node_));
    s->search_tree->top->ancestor = NULL;
    s->search_tree->top->level = 0;
    s->clause_num = 0;
    s->cluase_sat_num = 0;
    s->sat = Unknown;
    s->time = 0;
    s->decision_times = 0;
    s->rule_times = 0;
    s->search_tree->height = 0;
    return s;
}

Clause* creat_clause_set()
{
    Clause* c_set = (Clause*)calloc(MAX_CAP, sizeof(Clause));
    return c_set;
}

void add_clause(Solver solver)
{
    Clause c = (Clause)malloc(sizeof(struct clause_));
    c->l_head = NULL;
    c->cause_order = Unknown;
    c->length = 0;
    c->status = Unknown;
    solver->clause_set[solver->clause_num] = c;
    solver->clause_num += 1;
}

void add_literal(Clause c, int var_order, Sign sign)
{
    Literal p = c->l_head;
    c->l_head = malloc(sizeof(struct literal_));
    c->l_head->order = var_order;
    c->l_head->sign = sign;
    c->l_head->next = p;
    c->length += 1;
}

Clause_ref_set* creat_clause_ref_set(int var_num)
{
    Clause_ref_set* cr_sets = (Clause_ref_set*)malloc(sizeof(Clause_ref_set) * var_num);
    for(int i = 0; i < var_num; i++)
    {
        cr_sets[i] = (Clause_ref_set)malloc(sizeof(struct clause_ref_set_));
        cr_sets[i]->positive_score = cr_sets[i]->negative_score = 0;
        cr_sets[i]->status = Unknown;
        cr_sets[i]->level = 0;
        cr_sets[i]->clause_ref_head = NULL;
    }
    return cr_sets;
}

void add_clause_ref(Clause_ref_set cr_set, int clause_order, Sign sign)
{
    Clause_ref p = cr_set->clause_ref_head;
    cr_set->clause_ref_head = (Clause_ref)malloc(sizeof(struct clause_ref_));
    cr_set->clause_ref_head->order = clause_order;
    cr_set->clause_ref_head->sign = sign;
    cr_set->clause_ref_head->next = p;
    if(sign == Positive)
        cr_set->positive_score += 1;
    else
        cr_set->negative_score += 1;
}

void cnf_display(Solver solver)
{
    Literal l;  // 中介 文字 指针
    Clause_ref cr;  // 中介 子句引用 指针
    // 按子句输出cnf公式
    printf("display the clause in clause-literal method\n");
    for(int i = 0; i < solver->clause_num; i++)
    {
        l = solver->clause_set[i]->l_head;
        printf("clause %d in length of %d : { ", i + 1, solver->clause_set[i]->length);
        while(l != NULL)
        {
            if(l->sign)
                printf("%d ", l->order + 1);
            else
                printf("-%d ", l->order + 1);
            l = l->next;
        }
        printf("}\n");
    }
    // 按变量输出cnf公式
    printf("\ndisplay the clause in variable-clause_ref method\n");
    for(int i = 0; i < solver->variable_num; i++)
    {
        cr = solver->ref_sets[i]->clause_ref_head;
        printf("variable %d in score of %f/%f occurs in these clause : { ",
            i + 1, solver->ref_sets[i]->positive_score, solver->ref_sets[i]->negative_score);
        while(cr != NULL)
        {
            if(cr->sign)
                printf("%d ", cr->order + 1);
            else
                printf("(-)%d ", cr->order + 1);
            cr = cr->next;
        }
        printf("}\n");
    }
}
#ifndef _CNFPARSE_INC_
// 通过带命令行参数的程序来验证cnf公式的正确性
int main(int argc, char const *argv[])
{
    if(argc != 2)
    {
        printf("ERROR: need two arguments!!!");
        exit(0);
    }

    Solver s = input_parse(argv[1]);
    // Solver s = input_parse("D:\\WorkSpace\\SAT\\example\\M\\mysample_sat.cnf");
    cnf_display(s);
    return 0;
}
#endif