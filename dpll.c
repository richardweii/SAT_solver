/* DPLL算法 */
#define _DPLL_
#define B_PLAN
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"sat.h"


double var_time = 0;
double single_time = 0;
double update_time = 0;
double pure_time = 0;

Solver DPLL(Solver solver)
{
    clock_t start_t, finish_t;  // 时刻点
    start_t = clock();  // 算法开始
    solver->cause_queue = creat_queue(solver->variable_num);
    solver->clause_queue = creat_queue(MAX_CAP / 10);
    Variable center_var; // 中间变量，记录每一步的中心变元
    Status sat = Unknown;    // 记录当前cnf公式的满足状态
    if(simplify(solver) == 0)
        solver->sat = 0;
    else
    {   
        while(True)
        {
            while(sat == Unknown)
            {
                center_var = var_decision(solver);
                solver->decision_times++;
                sat = simplify(solver);
            }
            // 出口一：公式可满足
            if(sat == True)
            {
                solver->sat = sat;
                break;
            }
            else if(!backtrack(solver))
            {
            // 出口二：无法向上回溯
                solver->sat = sat;
                break;
            }
            sat = simplify(solver);
        }
    }
    finish_t = clock();
    solver->time = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
    return solver;
}

// TODO:实现队列函数
Queue creat_queue(int cap)
{

}     
int Q_out(Queue q)
{

}
void Q_put(Queue q, int order)
{

}
void Q_clear(Queue q)
{

}
Status Q_empty(Queue q)
{

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

Status check_literal(Sign sign, Status v_status)
{
    return (sign == v_status) ? True : False;
}

void tree_grows(Solver solver, Variable v, Status is_decision)
{
    Node p = solver->search_tree->top;
    solver->search_tree->top = (Node)malloc(sizeof(struct node_));
    solver->search_tree->top->var = v;
    solver->search_tree->top->is_decision_node = is_decision;
    solver->search_tree->top->level = p->level;
    solver->search_tree->top->ancestor = NULL;
    solver->search_tree->height += 1;
    p->ancestor = solver->search_tree->top;
}

Status update(Solver solver, Variable v)
{
    clock_t s, t;
    s = clock();
    Status result = Unknown;
    solver->ref_sets[v.order]->status = v.status;
    solver->ref_sets[v.order]->level = solver->search_tree->top->level;
    Clause_ref cr = solver->ref_sets[v.order]->clause_ref_head;
    for (; cr != NULL; cr = cr->next)
    {
        if(solver->clause_set[cr->order]->status == True)
            continue;   // 滤去已满足的子句
        solver->clause_set[cr->order]->length -= 1;
        //  对长度变化链的更新
        // Change_list p = solver->clause_set[cr->order]->change_list;
        // solver->clause_set[cr->order]->change_list = (Change_list)malloc(sizeof(struct change_list_));
        // solver->clause_set[cr->order]->change_list->level = solver->search_tree->top->level;
        // solver->clause_set[cr->order]->change_list->ancestor = NULL;
        // p->ancestor = solver->clause_set[cr->order]->change_list;
        // 子句状态的改变
        if(check_literal(cr->sign, v.status))
        {
            solver->clause_set[cr->order]->status = True;
            if(solver->clause_num == solver->cluase_sat_num)
                result = True;
            solver->cluase_sat_num += 1;
            solver->clause_set[cr->order]->cause_order = v.order;
        }
        else if(solver->clause_set[cr->order]->length == 0)
        {
            solver->clause_set[cr->order]->status = False;
            result = False;
            solver->sat = False;
            solver->clause_set[cr->order]->cause_order = v.order;
        }
        if(solver->clause_set[cr->order]->length == 1)
            Q_put(solver->clause_queue, cr->order);
    }
    t = clock();
    update_time += (double)(t - s)/ CLOCKS_PER_SEC;
    return result;
}

Status simplify(Solver solver)
{
    Status status = single_rule(solver);
    if(status == False)
    {
        Q_clear(solver->clause_queue);
        return False;
    }
    return pure_rule(solver);
}

Status single_rule(Solver solver)
{
    int order;
    Status status = Unknown;
    Literal l;
    Variable v;
    // 布尔约束传播（单子句规则）
    while(!Q_empty(solver->clause_queue))
    {
        order = Q_out(solver->clause_queue);
        l = solver->clause_set[order]->l_head;
        for(; l != NULL && solver->ref_sets[l->order] != Unknown; l = l->next);
        if(l == NULL)
        {
            printf("ERROR:cannot find that literal!");
            exit(0);
        }
        v.order = l->order;
        v.status = (l->sign == Positive) ? True : False;
        tree_grows(solver, v, False);
        status = update(solver, v);
        if(status == False)
            break;
    }
    return status;
}

Status pure_rule(Solver solver)
{
    Status status;
    Variable v;
    Clause_ref cr;
    Sign sign;
    for(int i = 0; i < solver->variable_num; i++)
    {
        sign = Unknown;
        if(solver->ref_sets[i]->status != Unknown)
            continue;
        cr = solver->ref_sets[i]->clause_ref_head;
        for(;cr != NULL; cr = cr->next)
        {
            if(solver->clause_set[cr->order]->status != Unknown)
                continue;
            if(sign == Unknown)
                sign = cr->sign;
            else if(sign != cr->sign)
                return status;
        }
        v.order = i;
        v.status = sign == Positive ? True : False;
        tree_grows(solver, v, False);
        status = update(solver, v);
    }
    return status;
}

Status conflict_clause_learning(Solver solver)
{
    int level = solver->search_tree->top->level;
    int sign;
    Status* mark = (Status*)calloc(solver->variable_num, sizeof(Status));
    add_clause(solver);

    // 先找到决策结点
    Node p = solver->search_tree->top;
    for(;!p->is_decision_node && p != NULL; p = p->ancestor);
    if(p == NULL)
        return False;   // 学习失败，找不到决策结点，也意味着cnf公式不满足
    sign = p->var.status == True ? Negative : Positive;
    add_literal(solver->clause_set[solver->clause_num - 1], p->var.order, sign);
    add_clause_ref(solver->ref_sets[p->var.order], solver->clause_num - 1, sign);
    if(sign == Positive)
        solver->ref_sets[p->var.order]->positive_score += 1;
    else
        solver->ref_sets[p->var.order]->negative_score += 1;
    
    // 再找出直接冲突原因节点加入队列中
    Literal L = solver->clause_set[solver->conflict_clause_order]->l_head;
    for(; L != NULL; L = L->next)
    {
        if(solver->ref_sets[L->order]->level < level)
        {
            // 添加其中为其他层的冲突因子文字
            mark[L->order] = True;  // 表示该变元已经检查过了
            sign = solver->ref_sets[L->order]->status == True ? Negative : Positive;
            add_literal(solver->clause_set[solver->clause_num - 1], L->order, sign);
            add_clause_ref(solver->ref_sets[L->order], solver->clause_num - 1, sign);
            if(sign == Positive)
                solver->ref_sets[L->order]->positive_score += 1;
            else
                solver->ref_sets[L->order]->negative_score += 1;
        }
        else
            Q_put(solver->cause_queue, L->order);   // 将同层原因加入队列中搜索
    }

    // 从已有的原因节点开始BFS搜索原因节点和冲突因子
    int v_order;
    Clause_ref cr;
    while(!Q_empty(solver->cause_queue))
    {
        v_order = Q_out(solver->cause_queue);
        if(mark[v_order])
            continue;
        mark[v_order] = True;
        cr = solver->ref_sets[v_order]->clause_ref_head;
        for(; solver->clause_set[cr->order]->cause_order != v_order; cr = cr->next);
        L = solver->clause_set[cr->order]->l_head;
        for(; L != NULL; L = L->next)
        {
            if(solver->ref_sets[L->order]->level < level)
            {
                // 添加其他层的冲突因子文字
                mark[L->order] = True;  // 表示该变元已经检查过了
                sign = solver->ref_sets[L->order]->status == True ? Negative : Positive;
                add_literal(solver->clause_set[solver->clause_num - 1], L->order, sign);
                add_clause_ref(solver->ref_sets[L->order], solver->clause_num - 1, sign);
                // 计分
                if(sign == Positive)
                    solver->ref_sets[L->order]->positive_score += 1;
                else
                    solver->ref_sets[L->order]->negative_score += 1;
            }
            else
                Q_put(solver->cause_queue, L->order);   // 将同层原因加入队列中搜索
        }
    }
    return True;    // 成功学习
}

Status backtrack(Solver solver)
{
    int level = solver->search_tree->top->level;
    if(confilct_clause_learning(solver) == False)
        return False;
    decay(solver);
    Node p = solver->search_tree->top;
    Clause_ref cr;
    Literal L;
    Variable v;
    while(p->level == level)
    {
        v.order = p->var.order;
        v.status = p->var.status;
        for(cr = solver->ref_sets[v.order]->clause_ref_head; cr != NULL; cr = cr->next)
        {
            for(L = solver->clause_set[cr->order]->l_head; L != NULL && L->order != v.order; L = L->next);
            if(solver->ref_sets[L->order] == Unknown)
                continue;   // 表示这个子句在之前已经满足
            solver->clause_set[cr->order]->status = Unknown;
            solver->clause_set[cr->order]->length += 1;
        }
        p = p->ancestor;
        solver->ref_sets[v.order]->status = Unknown;
        solver->ref_sets[v.order]->level = Unknown;
        free(solver->search_tree->top);
        solver->search_tree->top = p;
    }
    return True;
}

Variable var_decision(Solver solver)
{
    clock_t s, t;
    s = clock();
    double score = 0;
    Variable v;
    Status status;
    int order;
    for(int i = 0; i < solver->variable_num; i++)
    {
        if(score < solver->ref_sets[i]->positive_score)
        {
            score = solver->ref_sets[i]->positive_score;
            order = i;
            status = True;
        }
        else if(score < solver->ref_sets[i]->negative_score)
        {
            score = solver->ref_sets[i]->negative_score;
            order = i;
            status = False;
        }
    }
    v.order = order;
    v.status = status;
    t = clock();
    var_time += (double)(t - s)/ CLOCKS_PER_SEC;
    return v;
}

void decay(Solver solver)
{
    for(int i = 0; i < solver->variable_num; i++)
    {
        solver->ref_sets[i]->positive_score /= 1.05;
        solver->ref_sets[i]->negative_score /= 1.05;
    }
}
// FIXME:debug
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
                fprintf(fp, "Level %d: the variable is %d and the status is %d\n", i, p->var.order + 1, p->var.status);
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
