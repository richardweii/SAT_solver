/* DPLL算法 */
#define _DPLL_
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>
#include"sat.h"

Result DPLL(CNF cnf)
{
    Result res; // 运行结果
    clock_t start_t, finish_t;  // 时刻点
    start_t = clock();  // 算法开始
    List search_list = searching_start(cnf);    // 获得搜索链头
    res.cut = 0;
    Variable center_var; // 中间变量，记录每一步的中心变元
    int sat = -1;    // 记录当前cnf公式的满足状态
    while(true)
    {
        while(sat == -1)
        {
            center_var = var_decision(cnf);
            sat = assign_add(search_list, cnf, center_var);
            if(sat != -1)
                break;
            res.cut += single_detect(search_list, cnf);
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
        res.cut += single_detect(search_list, cnf);
        sat = check_sat(cnf);
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
    int order = v.order;
    int value = v.value;
    cnf->variable[order]->value = value;
    Clause_ref cr = cnf->variable[order]->next;
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
                    cnf->variable[L->order]->frequency -= 1;
            }
        }
        else
        {
            L->flag = 0;
            if(cnf->clause_set[cr->order]->length == 0)
                cnf->clause_set[cr->order]->flag = 0;
        }
    }
    return check_sat(cnf);
}

int assign_add(List search_list, CNF cnf, Variable center_var)
{
    Node p =(Node)malloc(sizeof(struct node_));
    p->child = NULL;
    p->father = search_list->tail;
    p->fixed = 0;
    p->var.order = center_var.order;
    p->var.value = center_var.value;
    search_list->tail->child = p;
    search_list->tail = p;
    return update_cnf(center_var, cnf);
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
    int count = 0;
    Node p;
    Variable v;
    Literal L;
    for(int i = 0; i < cnf->clause_num; i++)
    {
        if(cnf->clause_set[i]->length == 1 && cnf->clause_set[i]->flag == -1)
        {
            L = cnf->clause_set[i]->l_head;
            for(; L != NULL && L->flag != -1; L = L->next);
            p =(Node)malloc(sizeof(struct node_));
            p->child = NULL;
            p->father = search_list->tail;
            p->fixed = 1;
            p->var.order = L->order;
            p->var.value = (L->sign == 1) ? 1: 0;
            v.order = p->var.order;
            v.value = p->var.value;
            search_list->tail->child = p;
            search_list->tail = p;
            update_cnf(v, cnf);
            count++;
        }
    }
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
        for(cr = cnf->variable[v.order]->next; cr != NULL; cr = cr->next)
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
                        cnf->variable[L->order]->frequency += 1;
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

Variable var_decision(CNF cnf)
{
    int freq = 0;   // 变量被引用次数   
    int Pfreq = 0;  // 正引用次数
    int Nfreq = 0;  // 负引用次数
    Variable v;
    for(int i = 0; i < cnf->variable_num; i++)
    {
        if(cnf->variable[i]->value != -1)
            continue;
        if(freq <= cnf->variable[i]->frequency)
        {
            freq = cnf->variable[i]->frequency;
            v.order = i;
        }
    }
    Clause_ref p = cnf->variable[v.order]->next;
    for(; p != NULL; p = p->next)
    {
        if(cnf->clause_set[p->order]->flag != -1)
            continue;
        if(p->sign == 1)
            Pfreq++;
        else
            Nfreq++;
    }
    v.value = Pfreq >= Nfreq ? 1 : 0;
    return v;
}

#ifndef _DPLL_INC_
#define _CNFPARSE_INC_
#include"cnfparse.c"
int main(int argc, char const *argv[])
{
    if(argc != 2)
    {
        printf("ERROR: need two arguments!!!");
        exit(0);
    }
    CNF cnf = input_parse(argv[1]);
    // formula_display(cnf);
    Result res = DPLL(cnf);
    printf("Result:%d time:%f\n", res.sat, res.time);
    if(res.sat == 1)
    {
        int i = 1;
        Node p = res.search_list->head->child;
        while(p != NULL)
        {
            printf("Level %d: the variable is %d and the value is %d\n", i, p->var.order, p->var.value);
            i++;
            p = p->child;
        }
    }
    return 0;
}
#endif
