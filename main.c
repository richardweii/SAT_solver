/* 程序入口 */
#define _DPLL_INC_
#define MODULE_TESTING
#include<stdio.h>
#include<stdlib.h>
#include"sat.h"
#include"sudoku.c"
#include"dpll.c"
#include"cnfparse.c"
#include"save.c"

#ifndef MODULE_TESTING
// TODO:主程序入口
int main()
{
    return 0;
}
#endif

#ifdef MODULE_TESTING
// 会执行下列模块测试中的最先被定义的那个，可以通过注释来选择测试的模块
// 不注释就会执行DPLL测试，注释第一个就会执行sudoku测试，注释掉前两个就会执行cnf文件解析测试
#define DPLL_TESTING
#define SUDOKU_TESTING
#define CNF_PARSE_TESTING

#ifdef DPLL_TESTING
#undef SUDOKU_TESTING
#undef CNF_PARSE_TESTING
int main()
{
    printf("input the path of cnf file:");
    char* path = (char*)malloc(sizeof(char) * 80);
    scanf("%s", path);
    Solver solver = input_parse(path);
    DPLL(solver);
    res_save(path, solver);
    return 0;
}
#endif

#ifdef SUDOKU_TESTING
#undef CNF_PARSE_TESTING
int main()
{   
    char* sudoku_path = "example\\sudoku\\vh14_1.sudoku";
    Sudoku s = sudoku_display(sudoku_path);

    sudoku_rule1(s);
    sudoku_rule2(s);
    sudoku_rule3(s);

    sudoku_solve(s, False);
    sudoku_solution_display(s);
    return 0;
}
#endif

#ifdef CNF_PARSE_TESTING
int main()
{
    printf("input the path of cnf file:");
    char* path = (char*)malloc(sizeof(char) * 80);
    scanf("%s", path);
    Solver solver = input_parse(path);
    cnf_display(solver);
    return 0;
}
#endif



#endif

void sudoku_solution_display(Sudoku sudoku)
{
    printf("\nThe solution as follow:\n");
    int row, col;
    int i = 0;
    for(row = 1; row <= sudoku->size && i < sudoku->size * sudoku->size; row++)
    {
        for(col = 1; col <= sudoku->size && i < sudoku->size * sudoku->size; col++)
        {
            if(sudoku->solution[i] > 0)
                printf("1");
            else
                printf("0");
            i++;
            if(col == sudoku->size)
                printf("\n");
            else
                printf("  ");
        }
    }
}


Sudoku sudoku_display(char* path)
{
    Sudoku s = sudoku_parse(path);
    int row, col, index;
    int i = 0;
    for(row = 1; row <= s->size; row++)
    {
        for(col = 1; col <= s->size; col++)
        {
            index = encode_var(row, col, s->size);
            if((s->prefill[i] < 0 && -s->prefill[i] > index)||
               (s->prefill[i] > 0 && s->prefill[i] > index))
            {
                printf("_");
            }
            else
            {
                if(s->prefill[i] > 0)
                    printf("1");
                else
                    printf("0");
                i++;
            }
            if(col == s->size)
                printf("\n");
            else
                printf("  ");
        }
    }
    return s;
}

void cnf_display(Solver solver)
{
    Literal l;  // 中介 文字 指针
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
    // Clause_ref cr;  // 中介 子句引用 指针
    // printf("\ndisplay the clause in variable-clause_ref method\n");
    // for(int i = 0; i < solver->variable_num; i++)
    // {
    //     cr = solver->ref_sets[i]->clause_ref_head;
    //     printf("variable %d in score of %f/%f occurs in these clause : { ",
    //         i + 1, solver->ref_sets[i]->positive_score, solver->ref_sets[i]->negative_score);
    //     while(cr != NULL)
    //     {
    //         if(cr->sign)
    //             printf("%d ", cr->order + 1);
    //         else
    //             printf("(-)%d ", cr->order + 1);
    //         cr = cr->next;
    //     }
    //     printf("}\n");
    // }
}