#include "ast.h"
#include <math.h>
#include <string.h>

double eval_expression(ast_node_sexp *node) {
    if (!node) return 0.0;
    
    if (node->type == ST_ATOM) {
        ast_node_atom *atom = node->value.atom;
        if (atom->type == AT_NUMBER) {
            return atom->value.number;
        }
        // 处理常量标识符
        if (atom->type == AT_IDENTIFIER) {
            if (strcmp(atom->value.string, "pi") == 0) {
                return 3.141592653589793;
            } else if (strcmp(atom->value.string, "e") == 0) {
                return 2.718281828459045;
            }
        }
        return 0.0; // 未知标识符返回0
    } 
    else if (node->type == ST_LIST) {
        ast_node_list *list = node->value.list;
        if (list->length < 1) return 0.0;
        
        // 获取操作符/函数名
        ast_node_sexp *first = list->list[0];
        if (first->type != ST_ATOM || first->value.atom->type != AT_IDENTIFIER) {
            return 0.0;
        }
        
        char *func = first->value.atom->value.string;
        
        // 二元运算符处理
        if (list->length == 3) {
            double left = eval_expression(list->list[1]);
            double right = eval_expression(list->list[2]);
            
            if (strcmp(func, "+") == 0) return left + right;
            if (strcmp(func, "-") == 0) return left - right;
            if (strcmp(func, "*") == 0) return left * right;
            if (strcmp(func, "/") == 0) {
                if (right == 0) {
                    fprintf(stderr, "Error: Division by zero\n");
                    return 0;
                }
                return left / right;
            }
            if (strcmp(func, "^") == 0) return pow(left, right);
        }
        
        // 一元运算符和函数处理
        if (list->length == 2) {
            double arg = eval_expression(list->list[1]);
            
            if (strcmp(func, "-") == 0) return -arg;
            if (strcmp(func, "sin") == 0) return sin(arg);
            if (strcmp(func, "cos") == 0) return cos(arg);
            if (strcmp(func, "tan") == 0) return tan(arg);
            if (strcmp(func, "log") == 0) return log(arg);
            if (strcmp(func, "exp") == 0) return exp(arg);
            if (strcmp(func, "sqrt") == 0) {
                if (arg < 0) {
                    fprintf(stderr, "Error: Square root of negative number\n");
                    return 0;
                }
                return sqrt(arg);
            }
        }
    }
    return 0.0;
}