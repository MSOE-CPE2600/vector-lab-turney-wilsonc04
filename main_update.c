/* Filename: main_update.c
*  Author: Caleb Wilson
*  Date: 10/14/25
*  Description: User interface + parsing. Calls into vector.c for storage and math.
*  To compile: gcc -o test vector_update.c main_update.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vector_update.h"

#define LINE 256

static void trim_right(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static int isnum(const char *s) {
    char *end;
    strtod(s, &end);
    return end != NULL && *end == '\0';
}

/* normalize: turn commas into spaces, collapse whitespace, remove leading spaces */
static void normalize_numbers(char *in, char *out, size_t outsz) {
    size_t i = 0, j = 0;
    int in_space = 1;
    while (in[i] != '\0' && j + 1 < outsz) {
        char c = in[i++];
        if (c == ',') c = ' ';
        if (isspace((unsigned char)c)) {
            if (!in_space) {
                out[j++] = ' ';
                in_space = 1;
            }
        } else {
            out[j++] = c;
            in_space = 0;
        }
    }
    if (j > 0 && out[j - 1] == ' ') j--;
    out[j] = '\0';
}

/* -------- help screen -------- */
static void print_help(void) {
    printf("\nVector Calculator — Commands\n");
    printf("------------------------------------------------------------\n");
    printf("Assign / View\n");
    printf("  name = x y z           Set a vector (spaces)\n");
    printf("  name = x,y,z           Set a vector (commas)\n");
    printf("  name = x, y, z         Mix commas/spaces is fine\n");
    printf("  name                   Print the stored vector\n");
    printf("\nMath\n");
    printf("  a + b                  Vector addition\n");
    printf("  a - b                  Vector subtraction\n");
    printf("  a * s                  Scalar multiply (s is a number)\n");
    printf("  dot a b                Dot product (prints scalar result)\n");
    printf("  cross a b              Cross product (prints new vector)\n");
    printf("  c = cross a b          Assign cross product to vector c\n");
    printf("\nStorage\n");
    printf("  list                   List all stored vectors\n");
    printf("  clear                  Remove all vectors (frees memory)\n");
    printf("\nCSV I/O\n");
    printf("  load <file>            Load CSV (clears current vectors first)\n");
    printf("                         CSV format per line: name,x,y,z\n");
    printf("                         Accepts \\n or \\r\\n; ignores empty lines\n");
    printf("  save <file>            Save all vectors to CSV (overwrites file)\n");
    printf("\nOther\n");
    printf("  help or -h or ?        Show this help\n");
    printf("  quit                   Exit program\n");
    printf("------------------------------------------------------------\n\n");
}

int main(void) {
    init_store();
    atexit(free_store);

    char line[LINE];
    printf("Vector Calculator (Dynamic Memory + CSV)\n");
    printf("Type 'help' for instructions.\n");

    while (1) {
        printf(">> ");
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        trim_right(line);
        if (line[0] == '\0') continue;

        /* help / quit */
        if (strcmp(line, "help") == 0 || strcmp(line, "-h") == 0 || strcmp(line, "?") == 0) {
            print_help();
            continue;
        }
        if (strcmp(line, "quit") == 0) break;

        /* list / clear */
        if (strcmp(line, "list") == 0) {
            list_store();
            continue;
        }
        if (strcmp(line, "clear") == 0) {
            clear_store();
            continue;
        }

        /* CSV */
        if (strncmp(line, "load ", 5) == 0) {
            load_csv(line + 5);
            continue;
        }
        if (strncmp(line, "save ", 5) == 0) {
            save_csv(line + 5);
            continue;
        }

        /* dot / cross (print only) */
        if (strncmp(line, "dot ", 4) == 0) {
            char a[NAME_LEN], b[NAME_LEN];
            if (sscanf(line + 4, "%31s %31s", a, b) == 2) {
                double va[3], vb[3];
                if (!get_vector(a, va)) { printf("Left vector not found\n"); continue; }
                if (!get_vector(b, vb)) { printf("Right vector not found\n"); continue; }
                double d = va[0]*vb[0] + va[1]*vb[1] + va[2]*vb[2];
                printf("dot(%s,%s) = %.3f\n", a, b, d);
            } else {
                printf("Error: usage dot a b\n");
            }
            continue;
        }

        if (strncmp(line, "cross ", 6) == 0) {
            char a[NAME_LEN], b[NAME_LEN];
            if (sscanf(line + 6, "%31s %31s", a, b) == 2) {
                double va[3], vb[3], r[3];
                if (!get_vector(a, va)) { printf("Left vector not found\n"); continue; }
                if (!get_vector(b, vb)) { printf("Right vector not found\n"); continue; }
                r[0] = va[1]*vb[2] - va[2]*vb[1];
                r[1] = va[2]*vb[0] - va[0]*vb[2];
                r[2] = va[0]*vb[1] - va[1]*vb[0];
                print_vec_named("cross", r);
            } else {
                printf("Error: usage cross a b\n");
            }
            continue;
        }

        /* assignment: name = x y z  OR  name = x,y,z  (mix allowed) */
        {
            char left[NAME_LEN];
            char rhs[LINE];
            if (sscanf(line, "%31s = %255[^\n]", left, rhs) == 2) {
                char buf[LINE];
                double x, y, z;
                normalize_numbers(rhs, buf, sizeof(buf));
                if (strncmp(buf, "cross ", 6) == 0) {
                    char a[NAME_LEN], b[NAME_LEN];
                    if (sscanf(buf + 6, "%31s %31s", a, b) == 2) {
                        double va[3], vb[3], r[3];
                        if (!get_vector(a, va)) { printf("Left vector not found\n"); continue; }
                        if (!get_vector(b, vb)) { printf("Right vector not found\n"); continue; }
                        r[0] = va[1]*vb[2] - va[2]*vb[1];
                        r[1] = va[2]*vb[0] - va[0]*vb[2];
                        r[2] = va[0]*vb[1] - va[1]*vb[0];
                        set_vector(left, r[0], r[1], r[2]);
                        print_vec_named(left, r);
                        continue;
                    } else {
                        printf("Error: usage c = cross a b\n");
                        continue;
                    }
                }

                normalize_numbers(rhs, buf, sizeof(buf));
                if (sscanf(buf, "%lf %lf %lf", &x, &y, &z) == 3) {
                    set_vector(left, x, y, z);
                    double vals[3] = {x, y, z};
                    print_vec_named(left, vals);
                } else {
                    printf("Error: expected three numbers\n");
                }
                continue;
            }
        }

        /* vector math: a + b, a - b, a * n */
        {
            char a[NAME_LEN], op[4], b[NAME_LEN];
            if (sscanf(line, "%31s %3s %31s", a, op, b) == 3) {
                double va[3], vb[3], r[3];
                if (!get_vector(a, va)) { printf("Left vector not found\n"); continue; }

                if (strcmp(op, "+") == 0) {
                    if (!get_vector(b, vb)) { printf("Right vector not found\n"); continue; }
                    v_add(va, vb, r);
                    print_vec_named("ans", r);
                } else if (strcmp(op, "-") == 0) {
                    if (!get_vector(b, vb)) { printf("Right vector not found\n"); continue; }
                    v_sub(va, vb, r);
                    print_vec_named("ans", r);
                } else if (strcmp(op, "*") == 0) {
                    if (isnum(b)) {
                        double scale = strtod(b, NULL);
                        v_scale(va, scale, r);
                        print_vec_named("ans", r);
                    } else {
                        printf("Error: scalar multiply requires a number\n");
                    }
                } else {
                    printf("Unknown operator\n");
                }
                continue;
            }
        }

        /* single name -> print vector */
        {
            double v[3];
            if (get_vector(line, v)) {
                print_vec_named(line, v);
            } else {
                printf("Unknown command or vector (type 'help')\n");
            }
        }
    }

    return 0;
}