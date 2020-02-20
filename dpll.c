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
    Node search_list = searching_start(cnf);    // 获得搜索链头
    // TODO
    res.cut = 0;
    Variable center_var; // 中间变量，记录每一步的中心变元
    int sat = -1;    // 记录当前cnf公式的满足状态
    while(true)
    {
        while(sat == -1)
        {
            center_var = var_decision(cnf);
            sat = assign_add(search_list, cnf, center_var.order, center_var.value);
            if(sat != -1)
                break;
            res.cut += single_detect(search_list, cnf);
            res.cut += pure_detect(search_list, cnf);
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
        res.cut += pure_detect(search_list, cnf);
        sat = check_sat(cnf);
    }
    res.search_list = search_list;
    finish_t = clock();
    res.time = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
    return res;
}