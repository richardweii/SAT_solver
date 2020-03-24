#ifndef _RES_SAVE_INC_
#define _RES_SAVE_INC_
#include"sat.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

int res_save(const char* cnf_file, Solver s)
{
    char* result_path = (char*)calloc(80, sizeof(char));
    strcat(result_path, cnf_file);
    int length = strlen(result_path);
    result_path[length - 4] = 0;
    strcat(result_path, ".res");
    FILE* fp = fopen(result_path, "a+");

    if(s->sat != Unknown)
        fprintf(fp, "s: %d\n", s->sat);
    else
        fprintf(fp, "s: TIMEOUT\n");

    fprintf(fp, "v:");
    if(s->sat == True)
        for(int i = 0 ; i < s->variable_num; i++)
        {
            if(s->ref_sets[i]->status == True)
                fprintf(fp, "  %d", i+1);
            else
                fprintf(fp, " -%d", i+1);
        }

    fprintf(fp, "\n");
    fprintf(fp, "t: %dms\n", (int)(s->time * 1000));
    fprintf(fp, "args:\n"); // TODO:输出控制参数
    fprintf(fp, "other: ");
    fprintf(fp, "var decision times:%d rule times:%d clauses_num:%d\n\n",
        s->decision_times, s->rule_times, s->clause_num);
    return 0;
}
#endif