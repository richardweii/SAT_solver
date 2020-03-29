/* 程序入口 */
// #define MODULE_TESTING
#include<stdio.h>
#include<stdlib.h>
#include"sat.h"
#include"sudoku.c"
#include"dpll.c"
#include"cnfparse.c"
#include"save.c"

#ifndef MODULE_TESTING
int main()
{
    printf("Choose the function:\n");
    printf(" 1. SAT problem solves.\n");
    printf(" 2. sudoku gaming\n");
    printf(" 3. exit\n");
    printf("Input the number of function:[1 or 2 or 3]: ");
    int choice;
    char temp;
    while (True)
    {
        scanf("%d", &choice);
        scanf("%c", &temp);
        switch (choice)
        {
            case 1:
            {
                printf("input the path of cnf file:");
                char* path = (char*)malloc(sizeof(char) * 80);
                scanf("%s", path);
                Solver solver = input_parse(path);
                DPLL(solver);
                sat_solution(solver);
                res_save(path, solver);
                break;
            }
            case 2:
            {
                int size;
                printf("input the size of sudoku:[6 0r 8 or 10 or 12 or 14]:");
                scanf("%d", &size);
                char* sudoku_path = sudoku_select(size);
                Sudoku s = sudoku_display(sudoku_path);

                sudoku_rule1(s);
                sudoku_rule2(s);
                sudoku_rule3(s);
                
                sudoku_solve(s);
                sudoku_solution_display(s);
                break;
            }
            case 3:
            {
                break;
            }
            default:
            {
                printf("Input invalid!!!");
                printf("  try again:[1 or 2 or 3]:");
                break;
            }
        }
        if (choice == 1 || choice == 2)
        {
            printf("Choose the function:\n");
            printf(" 1. SAT problem solves.\n");
            printf(" 2. sudoku gaming\n");
            printf(" 3. exit\n");
            printf("Input the number of function:[1 or 2 or 3]: ");
        }
        else if(choice == 3)
            break;
    }
    return 0;
}
#endif

#ifdef MODULE_TESTING
// 会执行下列模块测试中的最先被定义的那个，可以通过注释来选择测试的模块
// 不注释就会执行DPLL测试，注释第一个就会执行sudoku测试，注释掉前两个就会执行cnf文件解析测试
// #define DPLL_TESTING
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
    sat_solution(solver);
    res_save(path, solver);
    return 0;
}
#endif

#ifdef SUDOKU_TESTING
#undef CNF_PARSE_TESTING
int main()
{   
    // char* sudoku_path = sudoku_select(6);
    char* sudoku_path = "D:\\WorkSpace\\SAT\\example\\sudoku\\6_1.sudoku";
    Sudoku s = sudoku_display(sudoku_path);

    sudoku_rule1(s);
    sudoku_rule2(s);
    sudoku_rule3(s);

    sudoku_solve(s, True);
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

char* sudoku_select(int size)
{
    char* file = (char*)malloc(sizeof(char) * 80);
    char* path = (char*)calloc(80, sizeof(char));
    strcat(path, "example\\sudoku\\");
    int a, b;   // a表示高位，b表示低位
    int i = 0;      // 表示file字符串下一个位置
    if(size >= 10)
    {
        a = size / 10;
        b = size % 10;
        file[i++] = a + '0';
        file[i++] = b + '0';
        file[i++] = '_';
    }
    else
    {
        a = 0;
        b = size;
        file[i++] = b + '0';
        file[i++] = '_';
    }
    srand((unsigned)time(NULL));
    int order = rand();
    if(size == 6)
    {
        order %= 30;
        order += 1;
        if(order >= 10)
        {
            a = order / 10;
            b = order % 10;
            file[i++] = a + '0';
            file[i++] = b + '0';
        }
        else
        {
            b = order;
            file[i++] = b + '0';
        }
    }
    else
    {
        order %= 5;
        order += 1;
        file[i++] = order + '0';
    }
    file[i] = 0;
    strcat(file, ".sudoku");
    strcat(path, file);
    return path;
}

void sat_solution(Solver solver)
{
    printf("Time:%dms", (int)(solver->time * 1000));
    printf("   SAT:");
    if(solver->sat == True)
        printf("True\n");
    else
        printf("False\n");
    printf("SAT solution:\n");
    if(solver->sat == True)
    {
        for(int i = 0; i < solver->variable_num; i++)
        {
            printf("Variable %3d :", i + 1);
            if(solver->var_info_set[i]->status == True)
                printf("True\n");
            else
                printf("False\n");
        }
    }
}