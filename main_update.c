/* Filename: main_update.c
 * Author: Caleb Wilson
 * Date: 10/19/25
 * Description: Lab 7 UI + parsing. Keeps Lab 5 behaviors, adds CSV + dynamic store.
 * To compile: gcc -Wall -Wextra -Wpedantic -O2 -o vectorcalc vector_update.c main_update.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vector_update.h"

#define LINE_LEN 256

/* ---------- tiny string helpers ---------- */

static void trim_right(char *s) {
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n-1])) s[--n] = '\0';
}

static void trim_left(char *s) {
    size_t i = 0, n = strlen(s);
    while (i < n && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, n - i + 1);
}

static void trim(char *s) { trim_right(s); trim_left(s); }

/* Convert commas to spaces (so "1, 2, 3" works). */
static void to_spaces(char *s) {
    for (; *s; ++s) if (*s == ',') *s = ' ';
}

/* Collapse runs of whitespace to single spaces. */
static void collapse_ws(const char *in, char *out, size_t outsz) {
    size_t i = 0, j = 0;
    int in_space = 1;
    while (in[i] && j + 1 < outsz) {
        char c = in[i++];
        if (isspace((unsigned char)c)) {
            if (!in_space) { out[j++] = ' '; in_space = 1; }
        } else {
            out[j++] = c; in_space = 0;
        }
    }
    if (j && out[j-1] == ' ') j--;
    out[j] = '\0';
}

static int is_number(const char *s) {
    if (!s || !*s) return 0;
    char *end = NULL;
    strtod(s, &end);
    return end && *end == '\0';
}

/* ---------- usage/help ---------- */

static void print_help(void) {
    puts("\nVector Calculator — Commands");
    puts("------------------------------------------------------------");
    puts("Assign / View");
    puts("  name = x y z           Set a vector (spaces)");
    puts("  name = x,y,z           Set a vector (commas)");
    puts("  name = x y             Set (z defaults to 0.0)");
    puts("  name                   Print the stored vector");
    puts("");
    puts("Math");
    puts("  a + b                  Vector addition");
    puts("  a - b                  Vector subtraction");
    puts("  a * s   or   s * a     Scalar multiply (s is a number)");
    puts("  dot a b                Dot product (prints scalar)");
    puts("  cross a b              Cross product (prints vector)");
    puts("  c = a + b              Operation w/ assignment (also -, *, s * a)");
    puts("  c = cross a b          Assign cross product");
    puts("");
    puts("Storage");
    puts("  list                   List all stored vectors");
    puts("  clear                  Remove all vectors");
    puts("");
    puts("CSV I/O");
    puts("  load <file>            Load CSV (clears current vectors first)");
    puts("                         CSV line format: name,x,y,z");
    puts("  save <file>            Save all vectors to CSV (overwrite)");
    puts("");
    puts("Other");
    puts("  help or -h or ?        Show this help");
    puts("  quit                   Exit program");
    puts("------------------------------------------------------------\n");
}

/* ---------- expression utilities ---------- */

static int valid_name(const char *name) {
    if (!name || !*name) return 0;
    for (const char *p = name; *p; ++p)
        if (!(isalnum((unsigned char)*p) || *p == '_')) return 0;
    return 1;
}

/* Evaluate binary expression into out[3]. Supports: +  -  * (scalar on either side). */
static int eval_binary_expr(const char *lhs_tok, const char *op, const char *rhs_tok,
                            double out[3]) {
    double va[3], vb[3];
    if (strcmp(op, "+") == 0) {
        if (!get_vector(lhs_tok, va) || !get_vector(rhs_tok, vb)) return 0;
        v_add(va, vb, out);
        return 1;
    } else if (strcmp(op, "-") == 0) {
        if (!get_vector(lhs_tok, va) || !get_vector(rhs_tok, vb)) return 0;
        v_sub(va, vb, out);
        return 1;
    } else if (strcmp(op, "*") == 0) {
        if (is_number(rhs_tok) && get_vector(lhs_tok, va)) {
            v_scale(va, strtod(rhs_tok, NULL), out);
            return 1;
        } else if (is_number(lhs_tok) && get_vector(rhs_tok, vb)) {
            v_scale(vb, strtod(lhs_tok, NULL), out);
            return 1;
        }
        return 0;
    }
    return 0;
}

/* ---------- handlers ---------- */

/* Handle: left = (numbers) | left = (expr) | left = cross a b */
static void handle_assignment(char *left, char *right) {
    trim(left); trim(right);
    if (!valid_name(left)) { puts("Error: invalid vector name."); return; }

    /* Try numbers first: x y z OR x,y,z OR x y (z=0) */
    {
        char tmp[LINE_LEN];
        strncpy(tmp, right, sizeof tmp - 1); tmp[sizeof tmp - 1] = '\0';
        to_spaces(tmp);
        char buf[LINE_LEN]; collapse_ws(tmp, buf, sizeof buf);

        double x, y, z;
        int n = sscanf(buf, "%lf %lf %lf", &x, &y, &z);
        if (n == 3 || n == 2) {
            if (n == 2) z = 0.0;
            set_vector(left, x, y, z);
            double v[3] = {x, y, z};
            print_vec_named(left, v);
            return;
        }
    }

    /* cross: c = cross a b */
    if (strncmp(right, "cross ", 6) == 0) {
        char a[NAME_LEN] = {0}, b[NAME_LEN] = {0};
        if (sscanf(right + 6, "%31s %31s", a, b) == 2) {
            double va[3], vb[3], r[3];
            if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
            if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
            v_cross(va, vb, r);
            set_vector(left, r[0], r[1], r[2]);
            print_vec_named(left, r);
            return;
        } else {
            puts("Error: syntax: c = cross a b");
            return;
        }
    }

    /* Disallow assigning dot (scalar) into a vector. */
    if (strncmp(right, "dot ", 4) == 0) {
        puts("Error: dot product is a scalar and cannot be assigned to a vector.");
        return;
    }

    /* Binary ops (Lab 5 style: spaces required around operators) */
    {
        char *op_plus  = strstr(right, " + ");
        char *op_minus = strstr(right, " - ");
        char *op_mul   = strstr(right, " * ");
        if (op_plus || op_minus || op_mul) {
            char op[4];
            if (op_plus)  { *op_plus  = '\0'; strcpy(op, "+"); }
            else if (op_minus){ *op_minus = '\0'; strcpy(op, "-"); }
            else          { *op_mul   = '\0'; strcpy(op, "*"); }

            char *lhs = right;
            char *rhs = (op_plus ? op_plus + 3 : op_minus ? op_minus + 3 : op_mul + 3);
            trim(lhs); trim(rhs);

            double r[3];
            if (!eval_binary_expr(lhs, op, rhs, r)) {
                puts("Error: invalid assignment expression.");
                return;
            }
            set_vector(left, r[0], r[1], r[2]);
            print_vec_named(left, r);
            return;
        }
    }

    puts("Error: expected numbers or an expression after '='");
}

/* Handle: dot/cross/single name and binary expressions */
static void handle_expression(char *line) {
    if (strncmp(line, "dot ", 4) == 0) {
        char a[NAME_LEN] = {0}, b[NAME_LEN] = {0};
        if (sscanf(line + 4, "%31s %31s", a, b) == 2) {
            double va[3], vb[3];
            if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
            if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
            double d = v_dot(va, vb);
            printf("dot(%s,%s) = %.3f\n", a, b, d);
            return;
        } else { puts("Error: syntax: dot a b"); return; }
    }
    if (strncmp(line, "cross ", 6) == 0) {
        char a[NAME_LEN] = {0}, b[NAME_LEN] = {0};
        if (sscanf(line + 6, "%31s %31s", a, b) == 2) {
            double va[3], vb[3], r[3];
            if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
            if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
            v_cross(va, vb, r);
            print_vec_named("ans", r);
            return;
        } else { puts("Error: syntax: cross a b"); return; }
    }

    char *op_plus  = strstr(line, " + ");
    char *op_minus = strstr(line, " - ");
    char *op_mul   = strstr(line, " * ");

    if (!op_plus && !op_minus && !op_mul) {
        trim(line);
        if (!valid_name(line)) { puts("Error: invalid input."); return; }
        double v[3];
        if (!get_vector(line, v)) { puts("Error: vector not found."); return; }
        print_vec_named(line, v);
        return;
    }

    double res[3];
    if (op_plus) {
        *op_plus = '\0';
        char *a = line, *b = op_plus + 3;
        trim(a); trim(b);
        double va[3], vb[3];
        if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
        if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
        v_add(va, vb, res);
        print_vec_named("ans", res);
        return;
    }
    if (op_minus) {
        *op_minus = '\0';
        char *a = line, *b = op_minus + 3;
        trim(a); trim(b);
        double va[3], vb[3];
        if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
        if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
        v_sub(va, vb, res);
        print_vec_named("ans", res);
        return;
    }
    if (op_mul) {
        *op_mul = '\0';
        char *lhs = line, *rhs = op_mul + 3;
        trim(lhs); trim(rhs);
        double v[3], s;
        if (is_number(lhs)) {
            s = strtod(lhs, NULL);
            if (!get_vector(rhs, v)) { puts("Error: vector operand not found."); return; }
            v_scale(v, s, res);
        } else if (is_number(rhs)) {
            if (!get_vector(lhs, v)) { puts("Error: vector operand not found."); return; }
            s = strtod(rhs, NULL);
            v_scale(v, s, res);
        } else {
            puts("Error: scalar multiplication requires one number and one vector.");
            return;
        }
        print_vec_named("ans", res);
        return;
    }
}

/* ---------- main loop ---------- */

int main(int argc, char **argv) {
    init_store();
    atexit(free_store);

    if (argc == 2 && (strcmp(argv[1], "-h") == 0)) {
        print_help();
        return 0;
    }

    char line[LINE_LEN];
    printf("minimat> "); fflush(stdout);

    while (fgets(line, sizeof(line), stdin)) {
        trim(line);
        if (!*line) { printf("minimat> "); fflush(stdout); continue; }

        if (strcmp(line, "quit") == 0) break;
        if (strcmp(line, "help") == 0 || strcmp(line, "-h") == 0 || strcmp(line, "?") == 0) { print_help(); printf("minimat> "); fflush(stdout); continue; }
        if (strcmp(line, "clear") == 0) { clear_store(); printf("minimat> "); fflush(stdout); continue; }
        if (strcmp(line, "list")  == 0) { list_store();  printf("minimat> "); fflush(stdout); continue; }

        if (strncmp(line, "load ", 5) == 0) { load_csv(line + 5); printf("minimat> "); fflush(stdout); continue; }
        if (strncmp(line, "save ", 5) == 0) { save_csv(line + 5); printf("minimat> "); fflush(stdout); continue; }

        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char left[NAME_LEN], rhs_raw[LINE_LEN];
            strncpy(left, line, sizeof left - 1); left[sizeof left - 1] = '\0';
            strncpy(rhs_raw, eq + 1, sizeof rhs_raw - 1); rhs_raw[sizeof rhs_raw - 1] = '\0';
            trim(left); trim(rhs_raw);
            if (!*left || !*rhs_raw) { puts("Error: invalid assignment."); printf("minimat> "); fflush(stdout); continue; }
            handle_assignment(left, rhs_raw);
            printf("minimat> "); fflush(stdout);
            continue;
        }

        handle_expression(line);
        printf("minimat> "); fflush(stdout);
    }

    return 0;
}
