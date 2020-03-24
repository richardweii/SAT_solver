/* DPLL算法 */
#define _DPLL_
#define B_PLAN
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"sat.h"
#define INTERVAL 10
#define ORIGINAL_SERIES 100
#define RATIO 1.5
// #define DEBUG
// #define RESTART
#define SHRINK
// TODO:重构dpll参数部分，使其能够控制使用某些功能和参数
// #define RESTART_INTERVAL 60

void show_in_time(Solver s, int timer)
{
    printf("\nAfter %d second:\n", timer);
    printf("decision times:  %d\n", s->decision_times);
    printf("rule times:      %d\n", s->rule_times);
    printf("clause num:      %d\n", s->clause_num);
    printf("decision level:  %d\n", s->search_tree->top->level);
    printf("tree height:     %d\n", s->search_tree->height);
    printf("sat num:         %d\n", s->cluase_sat_num);
}

void DPLL(Solver solver)
{
    int timer_show = INTERVAL;
    int time_now = 0;
    #ifdef DEBUG
    int breakpoint = 0;
    #endif
    clock_t start_t, finish_t;  // 时刻点
    start_t = clock();  // 算法开始
    Variable center_var; // 中间变量，记录每一步的中心变元
    Status sat = Unknown;    // 记录当前cnf公式的满足状态
    solver->original_clauses = solver->clause_num;
    #ifdef SHRINK
    int gap = solver->original_clauses;
    #endif
    #ifdef RESTART
    int original_num = solver->clause_num;
    int restart_seq = ORIGINAL_SERIES;
    int i = 1;
    #endif
    int level;
    if((sat = simplify(solver)) != Unknown)
        solver->sat = sat;
    else
    {   
        while(True)
        {
            while(sat == Unknown)
            {
                center_var = var_decision(solver);
                tree_grows(solver, center_var, True);
                update(solver, center_var);
                solver->decision_times++;
                sat = simplify(solver);
            }
            // 出口一：公式可满足
            if(sat == True)
            {
                solver->sat = sat;
                break;
            }
            if((time_now = ((clock() - start_t) / CLOCKS_PER_SEC)) >= timer_show)
            {
                show_in_time(solver, time_now);
                timer_show += INTERVAL;
                if(timer_show  > 5000)
                {
                    // Timeout
                    solver->sat = sat;
                    printf("Time out\n");
                    break;
                }
            }
            #ifdef DEBUG
            if(time_now > INTERVAL)
            {
                breakpoint += 1;
            }
            #endif
            if((level = conflict_clause_learning(solver)) < 0)   // 回溯到第几层)
            {
            // 出口二：无法向上回溯
                solver->sat = sat;
                break;
            }
            #ifdef SHRINK
            if(solver->clause_num - solver->original_clauses > gap)
            {
                int a = solver->clause_num;
                gap *= 1.2;
                clause_delete(solver, 8);
                printf("Delete %d clause\n", a - solver->clause_num);
            }
            #endif
            #ifdef RESTART
            if(solver->clause_num - original_num >= restart_seq )
            {
                original_num = solver->clause_num;
                if(restart_seq < 3000)
                    restart_seq *= RATIO;
                backtrack(solver, 0, True);
                sat = Unknown;
                printf("\nRestart %d success in time: %f\n", i++ ,(double)(clock() - start_t) / CLOCKS_PER_SEC);
            }
            else
                backtrack(solver, level, False);
            #else
            backtrack(solver, level, False);
            #endif
            sat = simplify(solver);
        }
    }
    finish_t = clock();
    solver->time = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
}

Queue creat_queue(int cap)
{
    Queue q = (Queue)malloc(sizeof(struct queue_));
    q->items = (int*)malloc(sizeof(int) * cap);
    q->front = q->rear = -1;
    q->size = cap;
    return q;
}     
int Q_out(Queue q)
{
    q->front  = (q->front + 1) % q->size;
    return q->items[q->front];
}
void Q_put(Queue q, int order)
{
    q->rear = (q->rear + 1) % q->size;
    if(q->front == q->rear)
    {
        printf("EXCEPTION:queue is full");
        exit(1);
    }
    q->items[q->rear] = order;
}
void Q_clear(Queue q)
{
    q->front = q->rear = -1;
}
Status Q_empty(Queue q)
{
    return (q->front == q->rear);
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
    if(is_decision)
        solver->search_tree->top->level = p->level + 1;
    else
        solver->search_tree->top->level = p->level;
    solver->search_tree->height += 1;
    solver->search_tree->top->ancestor = p;
}

Status update(Solver solver, Variable v)
{
    Status result = Unknown;
    solver->ref_sets[v.order]->status = v.status;
    solver->ref_sets[v.order]->is_decision_var = solver->search_tree->top->is_decision_node;
    solver->ref_sets[v.order]->level = solver->search_tree->top->level;
    Clause_ref cr = solver->ref_sets[v.order]->clause_ref_head;
    for (; cr != NULL; cr = cr->next)
    {
        if(solver->clause_set[cr->order]->status == True)
            continue;   // 滤去已满足的子句
        solver->clause_set[cr->order]->length -= 1;
        if(check_literal(cr->sign, v.status))
        {
            solver->clause_set[cr->order]->status = True;
            solver->cluase_sat_num += 1;
            solver->clause_set[cr->order]->cause_order = v.order;
            if(solver->clause_num == solver->cluase_sat_num)
                result = True;
        }
        else if(solver->clause_set[cr->order]->length == 0)
        {
            solver->clause_set[cr->order]->status = False;
            result = False;
            solver->sat = False;
            solver->clause_set[cr->order]->cause_order = v.order;
        }
        else if(solver->clause_set[cr->order]->length == 1)
        {
            // printf("put %d into the clause queue\n", cr->order);
            Q_put(solver->clause_queue, cr->order);
        }
    }
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
        if(solver->clause_set[order]->status != Unknown)
            continue;
        l = solver->clause_set[order]->l_head;
        for(; l != NULL && solver->ref_sets[l->order]->status != Unknown; l = l->next);
        if(l == NULL)
        {
            printf("EXCEPTION:cannot find that literal in clause %d", order + 1);
            exit(1);
        }
        v.order = l->order;
        v.status = (l->sign == Positive) ? True : False;
        tree_grows(solver, v, False);
        status = update(solver, v);
        solver->rule_times += 1;
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
                break;
        }
        if(cr != NULL || sign == Unknown)
            continue;   // 上一个循环中途就停止了，说明该变元不pure，或者该变元的所有子句都已满足
        v.order = i;
        v.status = sign == Positive ? True : False;
        tree_grows(solver, v, False);
        status = update(solver, v);
        solver->rule_times += 1;
    }
    return status;
}

int conflict_clause_learning(Solver solver)
{
    decay(solver);
    int level = solver->search_tree->top->level;
    int level_back = 0;
    int sign;
    Status result = False;  // 标记是否学习成功
    Status* mark = (Status*)calloc(solver->variable_num, sizeof(Status));
    add_clause(solver);

    // 再将直接冲突原因节点加入队列中
    int conflict_var = solver->search_tree->top->var.order;
    Q_put(solver->cause_queue, conflict_var);
    mark[conflict_var] = True;
    // 从已有的原因节点开始BFS搜索原因节点和冲突因子
    int v_order;
    Clause_ref cr;
    Literal L;
    while(!Q_empty(solver->cause_queue))
    {
        v_order = Q_out(solver->cause_queue);
        if(Q_empty(solver->cause_queue) && v_order != conflict_var)
        {
            sign = solver->ref_sets[v_order]->status == True ? Negative : Positive;
            add_literal(solver->clause_set[solver->next_place - 1], v_order, sign);
            add_clause_ref(solver->ref_sets[v_order], solver->next_place - 1, sign);
            result = True;
            break;
        }
        cr = solver->ref_sets[v_order]->clause_ref_head;
        for(; cr != NULL; cr = cr->next)
        {
            if(solver->clause_set[cr->order]->cause_order != v_order || solver->clause_set[cr->order]->length != 0) 
                continue;
            L = solver->clause_set[cr->order]->l_head;
            for(; L != NULL; L = L->next)
            {
                if(mark[L->order] == True || solver->ref_sets[L->order]->level == 0)
                    continue;
                if(solver->ref_sets[L->order]->level < level || solver->ref_sets[L->order]->is_decision_var == True)
                {
                    result = True;
                    if(solver->ref_sets[L->order]->level != level && solver->ref_sets[L->order]->level > level_back)
                        level_back = solver->ref_sets[L->order]->level;
                    // 添加其他层的冲突因子文字
                    mark[L->order] = True;  // 表示该变元已经检查过了
                    sign = solver->ref_sets[L->order]->status == True ? Negative : Positive;
                    add_literal(solver->clause_set[solver->next_place - 1], L->order, sign);
                    add_clause_ref(solver->ref_sets[L->order], solver->next_place - 1, sign);
                }
                else if(!mark[L->order])
                {
                    Q_put(solver->cause_queue, L->order);   // 将同层原因加入队列中搜索
                    mark[L->order] = True;
                }
            }
        }
    }
    count_score(solver, solver->clause_set[solver->next_place - 1], 0);
    solver->clause_set[solver->next_place - 1]->length = 0;
    solver->clause_set[solver->next_place - 1]->cause_order = Unknown;
    free(mark);
    if(result)
        return level_back;    // 成功学习
    else
        return -1;  // 学习失败
}

void backtrack(Solver solver, int level, Status is_restart)
{
    Node p = solver->search_tree->top;
    Clause_ref cr;
    Variable v;
    while(p->level > level)
    {
        v.order = p->var.order;
        v.status = p->var.status;
        for(cr = solver->ref_sets[v.order]->clause_ref_head; cr != NULL; cr = cr->next)
        {
            if(solver->clause_set[cr->order]->status == True && solver->clause_set[cr->order]->cause_order != v.order)
                continue;   // 表示这个子句在之前已经满足
            else if(solver->clause_set[cr->order]->status == True)
                solver->cluase_sat_num -= 1;    // 恢复一个已满足的子句
            solver->clause_set[cr->order]->status = Unknown;
            solver->clause_set[cr->order]->length += 1;
            solver->clause_set[cr->order]->cause_order = Unknown;
        }
        p = p->ancestor;
        solver->ref_sets[v.order]->status = Unknown;
        solver->ref_sets[v.order]->level = Unknown;
        solver->ref_sets[v.order]->is_decision_var = Unknown;
        free(solver->search_tree->top);
        solver->search_tree->top = p;
        solver->search_tree->height -= 1;
    }
    if(solver->clause_set[solver->next_place - 1]->length == 1 && is_restart == False)
        Q_put(solver->clause_queue, solver->next_place - 1);
}

Variable var_decision(Solver solver)
{
    double score = 0;
    Variable v;
    Status status;
    int order;
    for(int i = 0; i < solver->variable_num; i++)
    {
        if(solver->ref_sets[i]->status != Unknown)
            continue;
        if(score < solver->ref_sets[i]->positive_score)
        {
            score = solver->ref_sets[i]->positive_score;
            order = i;
            status = True;
        }
        if(score < solver->ref_sets[i]->negative_score)
        {
            score = solver->ref_sets[i]->negative_score;
            order = i;
            status = False;
        }
    }
    v.order = order;
    v.status = status;
    return v;
}

/****************************************
 * 参数：
 *  solver: 求解器
 *  threshold: 长度阈
 * 功能:
 *  遍历学习子句集,将当前长度大于threshold的子句删除,同时恢复solver的相关信息
 ****************************************/
void clause_delete(Solver solver, int threshold)
{
    int i;
    Literal l, p;
    for(i = solver->original_clauses;i < solver->next_place; i++)
    {
        if(solver->clause_set[i] == NULL || solver->clause_set[i]->length < threshold || solver->clause_set[i]->status != True)
            continue;
        solver->clause_num --;
        solver->cluase_sat_num --;
        l = solver->clause_set[i]->l_head;
        while(l != NULL)
        {
            p = l->next;
            free(l);
            l = p;
        }
        free(solver->clause_set[i]);
        solver->clause_set[i] = NULL;
    }

    Clause_ref cr, pr;
    for(i = 0; i < solver->variable_num; i++)
    {
        cr = solver->ref_sets[i]->clause_ref_head;
        while(solver->clause_set[cr->order] == NULL)
        {
            pr = cr->next;
            // EXCEPTION
            if(pr == NULL)
            {
                printf("EXCEPTION: A not used variable occur!!!\n");
                exit(1);
            }
            free(cr);
            solver->ref_sets[i]->clause_ref_head = pr;
            cr = pr;
        }
        pr = cr;
        while(pr != NULL)
        {
            cr = pr->next;
            if(cr != NULL && solver->clause_set[cr->order] == NULL)
            {
                pr->next = cr->next;
                free(cr);
            }
            else
            {
                pr = cr;
            }
        }
    }
}

void decay(Solver solver)
{
    for(int i = 0; i < solver->variable_num; i++)
    {
        solver->ref_sets[i]->positive_score *= DECAY;
        solver->ref_sets[i]->negative_score *= DECAY;
    }
}

#ifndef _DPLL_INC_
#ifndef _CNFPARSE_INC_
#define _CNFPARSE_INC_
#include"cnfparse.c"
#endif
void show_tree(Node n, int num)
{
    if(n->ancestor != NULL)
    {
        show_tree(n->ancestor, num - 1);
        printf("Level %d and tree height %d: the variable is %d and the status is %d\n", n->level, num, n->var.order + 1, n->var.status);
    }
}

#ifndef DEBUG
int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        printf("EXCEPTION:need at least two arguments!!");
        exit(1);
    }
    char* res_path = "result.res";
    FILE* fp = fopen(res_path, "a+");
    for(int i = 1; i < argc; i++)
    {
        int restart = 0;
        char const* path = argv[i];
        // char* path = "example\\sat-20.cnf";
        Solver solver = input_parse(path);
        // cnf_display(solver);
        DPLL(solver);
        // printf("Finish:  %s\n", argv[i]);
        fprintf(fp, "SAMPLE:  %s\n", argv[i]);
        #ifdef RESTART
        restart = 1;
        #endif
        fprintf(fp, "USINGE RESTART: %d\n", restart);
        fprintf(fp, "Result:%d time:%f  var decision times:%d rule times:%d\n\n", solver->sat, solver->time, solver->decision_times, solver->rule_times);
        printf("Result:%d time:%f  var decision times:%d rule times:%d clauses_num:%d\n\n",
         solver->sat, solver->time, solver->decision_times, solver->rule_times, solver->clause_num);
        if(solver->sat == True)
        {
            // show_tree(solver->search_tree->top, solver->search_tree->height);
        }
    }
    return 0;
}
#else
int main(int argc, char const *argv[])
{
    char* path = "example\\sat-20.cnf";
    // char* path = "example\\UF225.960.100\\uf200-03.cnf";
    // char* path = "example\\M\\m-mod2c-rand3bip-sat-220-3.shuffled-as.sat05-2490-311.cnf";
    Solver solver = input_parse(path);
    cnf_display(solver);
    solver = DPLL(solver);
    if(solver->sat == True)
    {
        // show_tree(solver->search_tree->top, solver->search_tree->height);
    }
    return 0;
}
#endif

#endif
