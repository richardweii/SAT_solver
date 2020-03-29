#ifndef _SUDOKU_INC_
#define _SUDOKU_INC_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"sat.h"
/****************************************
 * 数独解析模块
 * 数独约束：
 *  rule1:在每一行、每一列中不允许有连续的3个1或3个0出现
 *  rule2:在每一行、每一列中1与0的个数相同
 *  rule3:不存在重复的行与重复的列
 ****************************************/

Sudoku sudoku_parse(char* path)
{
    Sudoku sudoku = (Sudoku)malloc(sizeof(struct _sudoku));
    sudoku->pre_num = 0;
    sudoku->clause = 0;
    FILE* file;
    char* buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);
    if((file = fopen(path, "r")) == NULL)
    {
        printf("EXCEPTION: Open failed!!!\n");
        return NULL;
    }
    int i;  // 循环控制
    int line = -1;   // 记录子句的行数
    while(True)
    {
        if(feof(file))
            break;
        fgets(buffer, MAX_BUFFER, file);
        switch (buffer[0])
        {
            // 行首为‘c’表示为注释
            case 'c':
            {
                break;
            }

            // 行首为‘p’则为基本信息行
            case 'p':
            {
                i = 1;
                while (buffer[i] != '\n')
                {
                    if(buffer[i] == ' ')
                    {
                        i++;
                        continue;
                    }
                    else if (buffer[i] <= '9' && buffer[i] >= '0')
                    {
                        sudoku->size = buffer[i] - '0';
                        while(buffer[++i] <= '9' && buffer[i] >= '0')
                        {
                            sudoku->size = sudoku->size * 10 + buffer[i] - '0';
                        }
                        sudoku->prefill = (int*)malloc(sizeof(int) * sudoku->size * sudoku->size);
                    }
                }
                sudoku->variable = get_variable_num(sudoku->size);
                sudoku->clause += get_addition_clause_num(sudoku->size);
                if(line == -1)
                    line = 0;
                else
                {
                    printf("EXCEPTION: sudoku information initialization duplicates");
                    exit(1);
                }
                break;
            }
            default:
            {
                if(line == -1)
                {
                    printf("EXCEPTION: sudoku information haven't been initialize");
                    exit(1);
                }
                if(line >= sudoku->size)
                    break;
                i = 0;
                int col;
                while (buffer[i] != '\n')
                {
                    if(buffer[i] == '-')
                    {
                        col = buffer[++i] - '0';
                        while(buffer[++i] <= '9' && buffer[i] >= '0')
                        {
                            col = col * 10 + buffer[i] - '0';
                        }
                        
                        sudoku->prefill[sudoku->pre_num] = -encode_var(line + 1, col, sudoku->size);
                        sudoku->pre_num++;
                    }
                    else if(buffer[i] > '0' && buffer[i] <= '9')
                    {
                        col = buffer[i] - '0';
                        while(buffer[++i] <= '9' && buffer[i] >= '0')
                        {
                            col = col * 10 + buffer[i] - '0';
                        }
                        
                        sudoku->prefill[sudoku->pre_num] = encode_var(line + 1, col, sudoku->size);
                        sudoku->pre_num++;
                    }
                    else if(buffer[i] == '0')
                    {
                        break;
                    }
                    i++;
                }
                line++;
                break;
            }
        }
    }
    sudoku->clause += sudoku->pre_num;
    sudoku->var_next = sudoku->size * sudoku->size + 1;
    sudoku->contents = (int**)malloc(sizeof(int*) * sudoku->clause);
    for(int i = 0; i < sudoku->clause; i++)
    {
        sudoku->contents[i] = (int*)malloc(sizeof(int) * (sudoku->size + 2));
    }
    for(int i = 0; i < sudoku->pre_num; i++)
    {
        sudoku->contents[i][0] = sudoku->prefill[i];
        sudoku->contents[i][1] = 0;
    }
    sudoku->cla_next = sudoku->pre_num;
    fclose(file);
    return sudoku;
}

void sudoku_rule1(Sudoku s)
{
    // 总共增加 (size - 2) * size * 2个子句
    int row;
    int col;
    for(row = 1; row <= s->size; row++)
    {
        for(col = 1; col <= s->size - 2; col++)
        {
            s->contents[s->cla_next][0] = encode_var(row, col, s->size);
            s->contents[s->cla_next][1] = encode_var(row, col + 1, s->size);
            s->contents[s->cla_next][2] = encode_var(row, col + 2, s->size);
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->contents[s->cla_next][0] = -encode_var(row, col, s->size);
            s->contents[s->cla_next][1] = -encode_var(row, col + 1, s->size);
            s->contents[s->cla_next][2] = -encode_var(row, col + 2, s->size);
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
        }
    }
    for(col = 1; col <= s->size; col++)
    {
        for(row = 1; row <= s->size - 2; row++)
        {
            s->contents[s->cla_next][0] = encode_var(row, col, s->size);
            s->contents[s->cla_next][1] = encode_var(row + 1, col, s->size);
            s->contents[s->cla_next][2] = encode_var(row + 2, col, s->size);
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->contents[s->cla_next][0] = -encode_var(row, col, s->size);
            s->contents[s->cla_next][1] = -encode_var(row + 1, col, s->size);
            s->contents[s->cla_next][2] = -encode_var(row + 2, col, s->size);
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
        }
    }
}

void sudoku_rule2(Sudoku s)
{
    // 总共增加 2 * C(size / 2 + 1, size) * size * 2 个子句
    int m = s->size / 2 + 1;
    Comb c = comb(s->size, m);
    int row, col, i, j;
    for(row = 1; row <= s->size; row++)
    {
        for(i = 0; i < c->num; i++)
        {
            for(j = 0; j < m; j++)
                s->contents[s->cla_next][j] = encode_var(row, c->seq[i][j], s->size);
            s->contents[s->cla_next][j] = 0;
            s->cla_next++;
    
            for(j = 0; j < m; j++)
                s->contents[s->cla_next][j] = -encode_var(row, c->seq[i][j], s->size);
            s->contents[s->cla_next][j] = 0;
            s->cla_next++;
        }
    }
    for(col = 1; col <= s->size; col++)
    {
        for(i = 0; i < c->num; i++)
        {
            for(j = 0; j < m; j++)
                s->contents[s->cla_next][j] = encode_var(c->seq[i][j], col, s->size);
            s->contents[s->cla_next][j] = 0;
            s->cla_next++;
    
            for(j = 0; j < m; j++)
                s->contents[s->cla_next][j] = -encode_var(c->seq[i][j], col, s->size);
            s->contents[s->cla_next][j] = 0;
            s->cla_next++;
        }
    }
}

void sudoku_rule3(Sudoku s)
{
    Comb c = comb(s->size, 2);
    // 每层循环增加 3 * size + 1个变元，10 * size + 1个子句
    int row1,row2, col;
    // 按行
    for(int i = 0; i < c->num; i++)
    {
        row1 = c->seq[i][0];
        row2 = c->seq[i][1];
        for(col = 1; col <= s->size; col++)
        {
            s->contents[s->cla_next][0] = encode_var(row1, col, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = encode_var(row2, col, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = -encode_var(row1, col, s->size);
            s->contents[s->cla_next][1] = -encode_var(row2, col, s->size);
            s->contents[s->cla_next][2] = s->var_next;
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->var_next++;

            s->contents[s->cla_next][0] = -encode_var(row1, col, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = -encode_var(row2, col, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = encode_var(row1, col, s->size);
            s->contents[s->cla_next][1] = encode_var(row2, col, s->size);
            s->contents[s->cla_next][2] = s->var_next;
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->var_next++;

            s->contents[s->cla_next][0] = -(s->var_next - 1);
            s->contents[s->cla_next][1] = s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = -(s->var_next - 2);
            s->contents[s->cla_next][1] = s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = s->var_next - 1;
            s->contents[s->cla_next][1] = s->var_next - 2;
            s->contents[s->cla_next][2] = -s->var_next;
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->var_next++;
        }
        s->contents[s->cla_next][0] = -s->var_next;
        for(col = 1; col <= s->size; col++)
        {
            s->contents[s->cla_next][col] = -(s->var_next - 1 - 3*(col - 1));
        }
        s->contents[s->cla_next][col] = 0;
        s->cla_next++;
        for(col = 1; col <= s->size; col++)
        {
            s->contents[s->cla_next][0] = s->var_next;
            s->contents[s->cla_next][1] = s->var_next - 1 - 3*(col - 1);
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;
        }
        s->contents[s->cla_next][0] = s->var_next;
        s->contents[s->cla_next][1] = 0;
        s->cla_next++;
        s->var_next++;
    }
    int col1,col2, row;
    // 按列
    for(int i = 0; i < c->num; i++)
    {
        col1 = c->seq[i][0];
        col2 = c->seq[i][1];
        for(row = 1; row <= s->size; row++)
        {
            s->contents[s->cla_next][0] = encode_var(row, col1, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = encode_var(row, col2, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = -encode_var(row, col1, s->size);
            s->contents[s->cla_next][1] = -encode_var(row, col2, s->size);
            s->contents[s->cla_next][2] = s->var_next;
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->var_next++;

            s->contents[s->cla_next][0] = -encode_var(row, col1, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = -encode_var(row, col2, s->size);
            s->contents[s->cla_next][1] = -s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = encode_var(row, col1, s->size);
            s->contents[s->cla_next][1] = encode_var(row, col2, s->size);
            s->contents[s->cla_next][2] = s->var_next;
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->var_next++;

            s->contents[s->cla_next][0] = -(s->var_next - 1);
            s->contents[s->cla_next][1] = s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = -(s->var_next - 2);
            s->contents[s->cla_next][1] = s->var_next;
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;

            s->contents[s->cla_next][0] = s->var_next - 1;
            s->contents[s->cla_next][1] = s->var_next - 2;
            s->contents[s->cla_next][2] = -s->var_next;
            s->contents[s->cla_next][3] = 0;
            s->cla_next++;
            s->var_next++;
        }
        s->contents[s->cla_next][0] = -s->var_next;
        for(row = 1; row <= s->size; row++)
        {
            s->contents[s->cla_next][row] = -(s->var_next - 1 - 3*(row - 1));
        }
        s->contents[s->cla_next][row] = 0;
        s->cla_next++;
        for(row = 1; row <= s->size; row++)
        {
            s->contents[s->cla_next][0] = s->var_next;
            s->contents[s->cla_next][1] = s->var_next - 1 - 3*(row - 1);
            s->contents[s->cla_next][2] = 0;
            s->cla_next++;
        }
        s->contents[s->cla_next][0] = s->var_next;
        s->contents[s->cla_next][1] = 0;
        s->cla_next++;
        s->var_next++;
    }
}

Solver sudoku_to_cnf(Sudoku s)
{
    char* temp_path = "example\\sudoku\\temp.cnf";
    FILE* file = fopen(temp_path, "w");
    fprintf(file, "p cnf %d %d\n", s->variable, s->clause);
    for(int i = 0; i < s->clause; i++)
    {
        for(int j = 0;;j++)
        {
            if(s->contents[i][j] == 0)
            {
                fprintf(file, "%d\n", s->contents[i][j]);
                break;
            }
            if(j >= s->size+1)
            {
                printf("EXCEPTION:no end in the clause");
                exit(1);
            }
            else
                fprintf(file, "%d ", s->contents[i][j]);
        }
    }
    fclose(file);

    // 读取cnf公式
    Solver solver = input_parse((const char*)temp_path);
    return solver;
}

int get_addition_clause_num(int size)
{
    int r1 = 2 * (size - 2) * size * 2;
    Comb c = comb(size, size / 2 + 1);
    int r2 = c->num * size * 4;
    int r3 = size * (size - 1) * (9 * size + size + 2);
    return r1 + r2 + r3;
}

int get_variable_num(int size)
{
    return size * (size - 1) * ( size * 3 + 1) + size * size;
}

Comb comb(int size, int m)
{
    Comb c = (Comb)malloc(sizeof(struct _combination));
    c->size = size;
    c->choose = m;
    c->num = 1;
    for(int i = 0; i < m; i++)
        c->num *= (size - i);
    for(int i = 0; i < m; i++)
        c->num /= (i + 1);
    c->seq = (int**)malloc(sizeof(int*) * c->num);
    c->num = 0;
    int* d = (int*)malloc(sizeof(int) * m);
    cbNK(size, m, d, m, c);
    return c;
}

void cbNK(int n,int k,int* d,const int NUM, Comb c)
{
	int i = n;
	while(i>=k)
	{
		d[NUM-k] = i;
		if(k>1) cbNK(i-1,k-1,d,NUM, c);
		else
		{
			int j = 0;
            c->seq[c->num] = (int*)malloc(sizeof(int) * NUM);
			while(j<NUM)
			{
				c->seq[c->num][j] = d[j];
				j++;
			}
            c->num++;
		}
		i--;
	}
}

int encode_var(int row, int col, int size)
{
    return (row - 1)* size + col;
}

void sudoku_solve(Sudoku sudoku)
{
    Solver solver = sudoku_to_cnf(sudoku);
    char* temp_path = "example\\sudoku\\temp.cnf";
    DPLL(solver);
    sudoku_solution(sudoku, solver);
    char* newname = (char*)malloc(sizeof(char) * 40);
    char* buffer = (char*)malloc(sizeof(char) * 20);
    printf("Input the save name of cnf file converted from sudoku with end of '.cnf' or \
just input the '0' to skip the save:  ");
    strcat(newname, "example\\sudoku\\");
    scanf("%s", buffer);
    if(buffer[0] == '0' && buffer[1] == 0)
    {
        remove(temp_path);
    }
    else
    {
        strcat(newname, buffer);
        rename(temp_path, newname);
    }
    printf("press the Enter key to show the answer");
    getchar();
    getchar();
}

void sudoku_solution(Sudoku sudoku, Solver solver)
{
    sudoku->solution = (int*)malloc(sizeof(int) * sudoku->size * sudoku->size);
    for(int i = 0; i < sudoku->size * sudoku->size; i++)
        sudoku->solution[i] = solver->var_info_set[i]->status;
}
#endif