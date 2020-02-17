/* 输入解析和输出保存模块 */
#include"sat.h"
#include<stdio.h>
#include<stdlib.h>
#define MAX_BUFFER 100

CNF input_parse(const char* path)
{
    FILE* file;
    char* buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);
    if((file = fopen(path, "r")) == NULL)
    {
        printf("ERROR: Open failed!!!\n");
        return NULL;
    }
    // TODO

    int i;  // 循环控制
    int num;    // 字符串转化为数字时的中介变量
    int sign;   // 表示文字的正负:0,1 分别表示正, 负
    int line = -1;   // 记录子句的行数
    Clause_ref cr;  // l中介 子句引用 指针
    Literal l;  // 中介 文字 指针
    CNF cnf = (CNF)malloc(sizeof(struct cnf_));

    while(!feof(file))
    {
        fgets(buffer, MAX_BUFFER, file);
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
                        cnf->variable_num = buffer[i] - '0';
                        while (buffer[++i] <= '9' && buffer[i] >= '0')
                        {
                            cnf->variable_num = cnf->variable_num * 10 + (buffer[i] - '0');
                        }

                        while(buffer[++i] < '0' && buffer[i] >'9');

                        // 子句数量
                        cnf->clause_num = buffer[i] - '0';
                        while (buffer[++i] <= '9' && buffer[i] >= '0')
                        {
                            cnf->clause_num = cnf->clause_num * 10 + (buffer[i] - '0');
                        }

                        if(buffer[i] == '\n')
                            break;
                        i++;
                    }
                }
                // 分配邻接表的头空间并作基本初始化
                cnf->clause_set = (Clause)malloc(sizeof(struct clause_) * cnf->clause_num);

                for(int i = 0; i < cnf->clause_num; i++)
                {
                    cnf->clause_set[i]->flag = -1;
                    cnf->clause_set[i]->l_head = NULL;
                    cnf->clause_set[i]->length = 0;
                }

                cnf->variable = (Clause_ref)malloc(sizeof(struct clause_ref_) * cnf->variable_num);

                for(int i = 0; i < cnf->variable_num; i++)
                {
                    cnf->variable[i]->flag = -1;
                    cnf->variable[i]->next = NULL;
                }

                if(line = -1)
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
                else if(line >= cnf->clause_num)
                {
                    break;
                }
                i = 0;
                while (buffer[i] != '\n')
                {
                    if(buffer[i] == '-')
                    {
                        sign = 1;
                        num = buffer[++i];
                        while(buffer[++i] != ' ')
                        {
                            num = num * 10 + (buffer[i] - '0');
                        }
                        l = cnf->clause_set[line]->l_head;
                        cnf->clause_set[line]->l_head = (Literal)malloc(sizeof(struct literal_));
                        cnf->clause_set[line]->l_head->next = l;
                        cnf->clause_set[line]->l_head->order = num;
                        cnf->clause_set[line]->l_head->sign = sign;
                        cnf->clause_set[line]->l_head->flag = -1;
                        cnf->clause_set[line]->length += 1;
                        // TODO
                    }
                    else if(buffer[i] > '0' && buffer[i] <= '9')
                    {
                        sign = 0;
                        num = buffer[i];
                        while(buffer[++i] != ' ')
                        {
                            num = num * 10 + (buffer[i] - '0');
                        }
                        l = cnf->clause_set[line]->l_head;
                        cnf->clause_set[line]->l_head = (Literal)malloc(sizeof(struct literal_));
                        cnf->clause_set[line]->l_head->next = l;
                        cnf->clause_set[line]->l_head->order = num;
                        cnf->clause_set[line]->l_head->sign = sign;
                        cnf->clause_set[line]->l_head->flag = -1;
                        cnf->clause_set[line]->length += 1;
                        // TODO
                    }
                    else if(buffer == '0')
                    {
                        break;
                    }
                }
                line++;
                break;
            }
        }
    }
    // TODO
    fclose(file);
    return NULL;
}

// ********TEST*******
int main(int argc, char const *argv[])
{
    input_parse(argv[1]);
    return 0;
}
