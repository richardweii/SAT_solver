/* DPLL算法 */
#define _DPLL_
#define B_PLAN
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>
#include"sat.h"
#define POSITIVE 1
#define NEGATIVE 0

double var_time = 0;
double single_time = 0;
double update_time = 0;
double pure_time = 0;

Result DPLL(CNF cnf)
{
    Result res; // 运行结果
    clock_t start_t, finish_t;  // 时刻点
    start_t = clock();  // 算法开始
    List search_list = searching_start(cnf);    // 获得搜索链头
    res.cut_1 = 0;
    res.cut_2 = 0;
    res.times = 0;
    Variable center_var; // 中间变量，记录每一步的中心变元
    int sat = -1;    // 记录当前cnf公式的满足状态
    res.cut_1 += single_detect(search_list, cnf);
    res.cut_2 += pure_detect(search_list, cnf);
    if(check_sat(cnf) == 0)
        res.sat = 0;
    else
    {   
        while(true)
        {
            while(sat == -1)
            {
                res.times++;
                center_var = var_decision(cnf);
                sat = assign_add(search_list, cnf, center_var, false);
                if(sat != -1)
                    break;
                res.cut_1 += single_detect(search_list, cnf);
                res.cut_2 += pure_detect(search_list, cnf);
                sat = check_sat(cnf);
            }
            // 出口一：公式可满足
            if(sat == 1)
            {
                res.sat = sat;
                break;
            }
            // 出口二：无法向上回溯
            if(!backing(search_list, cnf))
            {
                res.sat = sat;
                break;
            }
            sat = assign_modify(search_list, cnf);
            // 变量决策函数能保证每次选择中心变元时一定是该变元的表象最优情况，回溯到该节点时不会直接通过修改而使公式满足
            res.cut_1 += single_detect(search_list, cnf);
            res.cut_2 += pure_detect(search_list, cnf);
            sat = check_sat(cnf);
        }
    }
    res.search_list = search_list;
    finish_t = clock();
    res.time = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
    return res;
}

List searching_start(CNF cnf)
{
    List l = (List)malloc(sizeof(struct list_));
    l->head = (Node)malloc(sizeof(struct node_));
    l->head->father = NULL;
    l->head->child = NULL;
    l->head->var.value = l->head->fixed = l->head->var.order = -1;  // -1表示为头节点
    l->tail = l->head;
    return l;
}

int check_sat(CNF cnf)
{
    int result = 1;
    for(int i = 0; i < cnf->clause_num; i++)
    {
        if(cnf->clause_set[i]->flag == 0)
        {
            result = 0;
            break;
        }
        else if(cnf->clause_set[i]->flag == -1)
            result = -1;
    }
    return result;
}

int update_cnf(Variable v, CNF cnf)
{
    clock_t s, t;
    s = clock();
    int order = v.order;
    int value = v.value;
    cnf->variable[order]->value = value;
    Clause_ref cr = cnf->variable[order]->clause_ref;
    Literal L;
    for (; cr != NULL; cr = cr->next)
    {
        if(cnf->clause_set[cr->order]->flag == 1)
            continue;
        L = cnf->clause_set[cr->order]->l_head;
        for(; L->order != order && L != NULL; L = L->next);
        if(L == NULL)
        {
            printf("ERROR:cannot find that clause!!!");
            exit(0);
        }
        cnf->clause_set[cr->order]->length -= 1;
        if(L->sign == value)
        {
            L->flag = 1;
            cnf->clause_set[cr->order]->flag = 1;
            for(L = cnf->clause_set[cr->order]->l_head; L != NULL; L = L->next)
            {
                if(cnf->variable[L->order]->value == -1)
                {
                    if(L->sign == 1)
                        cnf->variable[L->order]->positive_freq -= 1;
                    else
                        cnf->variable[L->order]->negative_freq -= 1;
                }
            }
        }
        else
        {
            L->flag = 0;
            if(cnf->clause_set[cr->order]->length == 0)
                cnf->clause_set[cr->order]->flag = 0;
        }
    }
    t = clock();
    update_time += (double)(t - s)/ CLOCKS_PER_SEC;
    return check_sat(cnf);
}

int assign_add(List search_list, CNF cnf, Variable var, int fixed)
{
    Node p =(Node)malloc(sizeof(struct node_));
    p->child = NULL;
    p->father = search_list->tail;
    p->fixed = fixed == 1 ? 1 : 0;
    p->var.order = var.order;
    p->var.value = var.value;
    search_list->tail->child = p;
    search_list->tail = p;
    return update_cnf(var, cnf);
}

int assign_modify(List search_list, CNF cnf)
{
    Node tail = search_list->tail;
    tail->fixed = 1;
    if(tail->var.value == 1)
        tail->var.value = 0;
    else
        tail->var.value = 1;
    Variable v = {tail->var.order, tail->var.value};
    return update_cnf(v, cnf);
}

int single_detect(List search_list, CNF cnf)
{
    clock_t s, t;
    int count = 0;
    Variable v;
    Literal L;
    s = clock();
    for(int i = 0; i < cnf->clause_num; i++)
    {
        if(cnf->clause_set[i]->length == 1 && cnf->clause_set[i]->flag == -1)
        {
            L = cnf->clause_set[i]->l_head;
            for(; L != NULL && L->flag != -1; L = L->next);
            v.order = L->order;
            v.value = (L->sign == 1) ? 1: 0;
            assign_add(search_list, cnf, v, true);
            count++;
        }
    }
    t = clock();
    single_time += (double)(t - s)/ CLOCKS_PER_SEC;
    return count;
}

int pure_detect(List search_list, CNF cnf)
{
    clock_t s, t;
    s = clock();
    int count = 0;
    Variable v;
    for(int i = 0; i < cnf->variable_num; i++)
    {
        if(cnf->variable[i]->value == -1 && (cnf->variable[i]->positive_freq + cnf->variable[i]->negative_freq) != 0)
        {
            if(cnf->variable[i]->positive_freq == 0)
            {
                v.order = i;
                v.value = 0;
                assign_add(search_list, cnf, v, true);
                count++;
            }
            else if(cnf->variable[i]->negative_freq == 0)
            {
                v.order = i;
                v.value = 1;
                assign_add(search_list, cnf, v, true);
                count++;
            }
        }
    }
    t = clock();
    pure_time += (double)(t - s)/ CLOCKS_PER_SEC;
    return count;
}

int backing(List search_list, CNF cnf)
{
    Node p = search_list->tail;
    Clause_ref cr;
    Literal L;
    Variable v;
    while(p->fixed != -1)
    {
        v.order = search_list->tail->var.order;
        v.value = search_list->tail->var.value;
        for(cr = cnf->variable[v.order]->clause_ref; cr != NULL; cr = cr->next)
        {
            for(L = cnf->clause_set[cr->order]->l_head; L != NULL && L->order != v.order; L = L->next);
            if(L->flag == -1)
                continue;
            L->flag = -1;
            if(cr->sign == v.value)
            {
                for(L = cnf->clause_set[cr->order]->l_head; L != NULL; L = L->next)
                {
                    if(cnf->variable[L->order]->value == -1)
                    {
                        if(L->sign == 1)
                            cnf->variable[L->order]->positive_freq += 1;
                        else    
                            cnf->variable[L->order]->negative_freq += 1;
                    }
                }
            }
            cnf->clause_set[cr->order]->flag = -1;
            cnf->clause_set[cr->order]->length += 1;
        }
        if(p->fixed == 0)
            break;
        p = p->father;
        cnf->variable[v.order]->value = -1;
        free(search_list->tail);
        search_list->tail = p;
        search_list->tail->child = NULL;
    }
    if(p->fixed == -1)
        return 0;
    else
        return 1;
}

typedef struct freq_counter
{
    int freq;
    int positive;
    int negative;
}* Freq_counter;

Variable var_decision(CNF cnf)
{
    clock_t s, t;
    s = clock();
    int freq = 0;   // 变量被引用次数
    int mark = 0;
    Variable v;
    for(int i = 0; i < cnf->variable_num; i++)
    {
        if(cnf->variable[i]->value == -1 && (cnf->variable[i]->negative_freq) + (cnf->variable[i]->positive_freq) > freq)
        {
            v.order = i;
            freq = (cnf->variable[i]->negative_freq) + (cnf->variable[i]->positive_freq);
        }
    }
    v.value = cnf->variable[v.order]->positive_freq > cnf->variable[v.order]->negative_freq ? 1 : 0;
    #ifdef B_PLAN
    Literal L;
    Freq_counter counter = (Freq_counter)calloc(cnf->variable_num, sizeof(struct freq_counter));
    for(int i = 0; i < cnf->clause_num; i++)
    {
        if(cnf->clause_set[i]->flag != 1 && cnf->clause_set[i]->length < 3)
        {
            mark += 1;
            for(L = cnf->clause_set[i]->l_head; L != NULL; L = L->next)
            {
                if(L->flag == -1)
                {
                    counter[L->order].freq += 1;
                    if(L->sign == 1)
                        counter[L->order].positive += 1;
                    else
                        counter[L->order].negative += 1;
                }
            }
        }
    }
    freq = 0;
    int count = 1;  // 频率相同变元的数量
    for(int i = 0; i < cnf->variable_num; i++)
    {
        if(counter[i].freq > freq)
        {
            freq = counter[i].freq;
            v.order = i;
            count = 1;
        }
        else if(counter[i].freq == freq)
        {
            count++;
        }
    }
    double inclination = 0; // 正负占比
    double temp;    // 缓存值
    if(count != 1)
    {
        for(int i = 0; i < cnf->variable_num; i++)
        {
            if(counter[i].freq == freq)
            {
                temp = (double)counter[i].positive / counter[i].freq;
                temp = temp > 0.5 ? temp : 1 - temp;
                if(temp > inclination)
                {
                    v.order = i;
                    inclination = temp;
                }
            }
        }
    }
    if(mark)
    {
        v.value = counter[v.order].positive>= counter[v.order].negative ? 1 : 0;
    }
    free(counter);
    #endif
    t = clock();
    var_time += (double)(t - s)/ CLOCKS_PER_SEC;
    return v;
}

#ifndef _DPLL_INC_
#define _CNFPARSE_INC_
#include"cnfparse.c"
int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        printf("ERROR: need two arguments!!!");
        exit(0);
    }
    char* path = "D:\\WorkSpace\\SAT\\result.res";
    FILE* fp = fopen(path, "a+");
    for(int i = 1; i < argc; i++)
    {
        var_time = 0;
        single_time = 0;
        update_time = 0;
        pure_time = 0;
        CNF cnf = input_parse(argv[i]);
        Result res = DPLL(cnf);
        fprintf(fp, "\nEXAMPLE:%s\n", argv[i]);
        fprintf(fp, "Result:%d time:%f cut_single:%d cut_pure:%d times:%d\n", res.sat, res.time, res.cut_1, res.cut_2, res.times);
        fprintf(fp, "TIME:update:%f decision:%f single:%f  pure:%f\n", update_time, var_time, single_time, pure_time);
        if(res.sat == 1)
        {
            int i = 1;
            Node p = res.search_list->head->child;
            while(p != NULL)
            {
                fprintf(fp, "Level %d: the variable is %d and the value is %d\n", i, p->var.order + 1, p->var.value);
                i++; 
                p = p->child;
            }
        }
        printf("Finished %s\n", argv[i]);
    }
    // CNF cnf = input_parse("D:\\WorkSpace\\SAT\\example\\sat-20.cnf");
    // formula_display(cnf);
    fclose(fp);
    return 0;
}
#endif
