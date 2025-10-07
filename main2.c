/* Filename: main2.c
*  Author: Caleb Wilson
*  Date: 9/30/25
*  Description: User interface + parsing. Calls into vector.c for storage and math.
*  To compile: gcc -o test vector2.c main2.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vector2.h"

#define LINE_LEN 256

/* ---------- tiny string helpers ---------- */

// Trim leading/trailing whitespace and newlines.
static void trim(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))) { s[--n] = '\0'; }
    size_t i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
}

// Convert commas to spaces (so "1, 2, 3" works).
static void to_spaces(char *s) {
    for (; *s; ++s) if (*s == ',') *s = ' ';
}

// Check if the token is a valid floating number per strtod.
static int is_number(const char *s) {
    if (!s || !*s) return 0;
    char *end = NULL;
    strtod(s, &end);
    return end && *end == '\0';
}

// Parse exactly 3 doubles from a right-hand side.
static int parse_three_doubles(char *rhs, double out[3]) {
    to_spaces(rhs);
    char buf[LINE_LEN];
    int bi = 0, seen = 0;
    for (char *p = rhs; *p && bi < (int)sizeof(buf)-1; ++p) {
        if (isspace((unsigned char)*p)) {
            if (seen) { buf[bi++] = ' '; seen = 0; }
        } else {
            buf[bi++] = *p; seen = 1;
        }
    }
    if (bi && buf[bi-1] == ' ') bi--;
    buf[bi] = '\0';

    double x, y, z;
    int n = sscanf(buf, "%lf %lf %lf", &x, &y, &z);
    if (n != 3) return 0;
    out[0] = x; out[1] = y; out[2] = z;
    return 1;
}

/* ---------- usage/help ---------- */

static void usage(const char *prog) {
    printf("Usage: %s [-h]\n", prog);
    puts("Interactive 3D vector calculator (max 10 vectors). Spaces around operators are required.");
    puts("Commands:");
    puts("  name = x y z             Assign vector (also accepts commas between numbers)");
    puts("  name                     Display vector");
    puts("  a + b                    Add");
    puts("  a - b                    Subtract");
    puts("  a * s   or   s * a       Scalar multiply (s is a number)");
    puts("  c = a + b                Operation with assignment (also -, *)");
    puts("  list                     List stored vectors");
    puts("  clear                    Clear all vectors");
    puts("  quit                     Exit");
    puts("Extra credit:");
    puts("  dot a b                  Dot product (prints scalar)");
    puts("  cross a b                Cross product (prints vector)");
    puts("  c = cross a b            Assign cross product to vector c");
}

/* ---------- expression utilities ---------- */

static int valid_name(const char *name) {
    if (!name || !*name) return 0;
    for (const char *p = name; *p; ++p) {
        if (!(isalnum((unsigned char)*p) || *p == '_')) return 0;
    }
    return 1;
}

/* ---------- handlers ---------- */

// Handle: left = (three numbers) OR left = (expr with +, -, *, or cross a b)
static void handle_assignment(char *left, char *right) {
    trim(left); trim(right);
    if (!valid_name(left)) { puts("Error: invalid vector name."); return; }

    // Case 1: direct vector literal "x y z"
    double vals[3];
    if (parse_three_doubles(right, vals)) {
        if (!set_vector(left, vals[0], vals[1], vals[2])) { puts("Error: storage full."); return; }
        print_vec_named(left, vals);
        return;
    }

    // Case 2: extra credit "cross a b"
    if (strncmp(right, "cross ", 6) == 0) {
        char a[NAME_LEN] = {0}, b[NAME_LEN] = {0};
        if (sscanf(right + 6, "%31s %31s", a, b) == 2) {
            double va[3], vb[3], r[3];
            if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
            if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
            v_cross(va, vb, r);
            if (!set_vector(left, r[0], r[1], r[2])) { puts("Error: storage full."); return; }
            print_vec_named(left, r);
            return;
        } else {
            puts("Error: syntax: c = cross a b");
            return;
        }
    }

    // Disallow "left = dot a b" because dot returns a scalar (not a 3D vector).
    if (strncmp(right, "dot ", 4) == 0) {
        puts("Error: dot product is a scalar and cannot be assigned to a vector.");
        return;
    }

    // Case 3: binary ops with required spaces ("a + b", "a - b", "a * s", "s * a").
    char *op_plus  = strstr(right, " + ");
    char *op_minus = strstr(right, " - ");
    char *op_mul   = strstr(right, " * ");
    double res[3];

    if (op_plus) {
        *op_plus = '\0';
        char *a = right;
        char *b = op_plus + 3;
        trim(a); trim(b);
        double va[3], vb[3];
        if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
        if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
        v_add(va, vb, res);
    } else if (op_minus) {
        *op_minus = '\0';
        char *a = right;
        char *b = op_minus + 3;
        trim(a); trim(b);
        double va[3], vb[3];
        if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
        if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
        v_sub(va, vb, res);
    } else if (op_mul) {
        *op_mul = '\0';
        char *lhs = right;
        char *rhs = op_mul + 3;
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
    } else {
        puts("Error: invalid assignment expression.");
        return;
    }

    if (!set_vector(left, res[0], res[1], res[2])) { puts("Error: storage full."); return; }
    print_vec_named(left, res);
}

// Handle: single name, a + b, a - b, a * s, s * a, and extra credit: dot a b, cross a b
static void handle_expression(char *line) {
    // Extra credit commands: "dot a b", "cross a b"
    if (strncmp(line, "dot ", 4) == 0) {
        char a[NAME_LEN] = {0}, b[NAME_LEN] = {0};
        if (sscanf(line + 4, "%31s %31s", a, b) == 2) {
            double va[3], vb[3];
            if (!get_vector(a, va)) { puts("Error: left operand not found.");  return; }
            if (!get_vector(b, vb)) { puts("Error: right operand not found."); return; }
            double d = v_dot(va, vb);
            printf("dot(%s,%s) = %.3f\n", a, b, d);
            return;
        } else {
            puts("Error: syntax: dot a b");
            return;
        }
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
        } else {
            puts("Error: syntax: cross a b");
            return;
        }
    }

    // Required operators with spaces.
    char *op_plus  = strstr(line, " + ");
    char *op_minus = strstr(line, " - ");
    char *op_mul   = strstr(line, " * ");

    // Single name: print the vector.
    if (!op_plus && !op_minus && !op_mul) {
        trim(line);
        if (!valid_name(line)) { puts("Error: invalid input."); return; }
        double v[3];
        if (!get_vector(line, v)) { puts("Error: vector not found."); return; }
        print_vec_named(line, v);
        return;
    }

    // Binary ops
    double res[3];
    if (op_plus) {
        *op_plus = '\0';
        char *a = line;
        char *b = op_plus + 3;
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
        char *a = line;
        char *b = op_minus + 3;
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
        char *lhs = line;
        char *rhs = op_mul + 3;
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
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        usage(argv[0]);
        return 0;
    }

    clear_store();

    char line[LINE_LEN];
    while (1) {
        printf("minimat> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        trim(line);
        if (!*line) continue;

        if (strcmp(line, "quit") == 0) { break; }
        if (strcmp(line, "clear") == 0) { clear_store(); continue; }
        if (strcmp(line, "list")  == 0) { list_store();   continue; }

        // Assignment requires spaces around '='
        char *eq = strstr(line, " = ");
        if (eq) {
            *eq = '\0';
            char *left  = line;
            char *right = eq + 3;
            if (!*left || !*right) { puts("Error: invalid assignment."); continue; }
            handle_assignment(left, right);
            continue;
        }

        // Otherwise treat as expression or command (dot/cross/single var)
        handle_expression(line);
    }
    return 0;
}
