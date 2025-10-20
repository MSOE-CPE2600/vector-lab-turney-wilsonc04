/* Filename: vector_update.c
 * Author: Caleb Wilson
 * Date: 10/19/25
 * Description: Lab 7 dynamic storage (+ CSV) with Lab 5 math API.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vector_update.h"

typedef struct {
    Vec   *data;
    size_t size;
    size_t capacity;
} VecStore;

static VecStore g = { NULL, 0, 0 };

static void ensure_capacity(size_t need) {
    if (need <= g.capacity) return;
    size_t newcap = g.capacity ? g.capacity * 2 : 8;
    if (newcap < need) newcap = need;
    Vec *tmp = (Vec*)realloc(g.data, newcap * sizeof *tmp);
    if (!tmp) { fprintf(stderr, "Error: out of memory\n"); exit(1); }
    for (size_t i = g.capacity; i < newcap; ++i) {
        tmp[i].used = 0;
        tmp[i].name[0] = '\0';
        tmp[i].v[0] = tmp[i].v[1] = tmp[i].v[2] = 0.0;
    }
    g.data = tmp;
    g.capacity = newcap;
}

void init_store(void) {
    g.data = NULL;
    g.size = 0;
    g.capacity = 0;
}

void free_store(void) {
    free(g.data);
    g.data = NULL;
    g.size = 0;
    g.capacity = 0;
}

void clear_store(void) {
    /* Reset to empty but keep capacity to avoid churn */
    for (size_t i = 0; i < g.size; ++i) g.data[i].used = 0;
    g.size = 0;
}

static int find_index(const char *name) {
    for (size_t i = 0; i < g.size; ++i)
        if (g.data[i].used && strcmp(g.data[i].name, name) == 0) return (int)i;
    return -1;
}

int set_vector(const char *name, double x, double y, double z) {
    int idx = find_index(name);
    if (idx >= 0) {
        g.data[idx].v[0] = x; g.data[idx].v[1] = y; g.data[idx].v[2] = z;
        return 1;
    }
    ensure_capacity(g.size + 1);
    g.data[g.size].used = 1;
    strncpy(g.data[g.size].name, name, NAME_LEN - 1);
    g.data[g.size].name[NAME_LEN - 1] = '\0';
    g.data[g.size].v[0] = x; g.data[g.size].v[1] = y; g.data[g.size].v[2] = z;
    g.size++;
    return 1;
}

int get_vector(const char *name, double out[3]) {
    int idx = find_index(name);
    if (idx < 0) return 0;
    out[0] = g.data[idx].v[0];
    out[1] = g.data[idx].v[1];
    out[2] = g.data[idx].v[2];
    return 1;
}

void list_store(void) {
    int any = 0;
    for (size_t i = 0; i < g.size; ++i) {
        if (g.data[i].used) {
            print_vec_named(g.data[i].name, g.data[i].v);
            any = 1;
        }
    }
    if (!any) puts("(no vectors stored)");
}

/* ----- Vector math ----- */

void v_add(const double a[3], const double b[3], double r[3]) {
    r[0] = a[0] + b[0]; r[1] = a[1] + b[1]; r[2] = a[2] + b[2];
}

void v_sub(const double a[3], const double b[3], double r[3]) {
    r[0] = a[0] - b[0]; r[1] = a[1] - b[1]; r[2] = a[2] - b[2];
}

void v_scale(const double a[3], double s, double r[3]) {
    r[0] = a[0] * s; r[1] = a[1] * s; r[2] = a[2] * s;
}

double v_dot(const double a[3], const double b[3]) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void v_cross(const double a[3], const double b[3], double r[3]) {
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
}

/* ----- Display ----- */

void print_vec_named(const char *name, const double v[3]) {
    printf("%s = %.3f   %.3f   %.3f\n", name, v[0], v[1], v[2]);
}

/* ----- CSV ----- */

static void trim_right(char *s) {
    size_t n = strlen(s);
    while (n && (unsigned char)s[n-1] <= ' ') s[--n] = '\0';
}

int load_csv(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if (!fp) { printf("Error: cannot open %s\n", fname); return 0; }

    clear_store();

    char line[256];
    char name[NAME_LEN];
    double x, y, z;

    while (fgets(line, sizeof line, fp)) {
        trim_right(line);
        if (!*line) continue;
        if (sscanf(line, "%31[^,],%lf,%lf,%lf", name, &x, &y, &z) == 4) {
            set_vector(name, x, y, z);
        } else {
            printf("Warning: bad line ignored: %s\n", line);
        }
    }

    fclose(fp);
    return 1;
}

int save_csv(const char *fname) {
    FILE *fp = fopen(fname, "w");
    if (!fp) { printf("Error: Cannot open %s\n", fname); return 0; }
    for (size_t i = 0; i < g.size; ++i) {
        if (g.data[i].used) {
            fprintf(fp, "%s,%.6f,%.6f,%.6f\n",
                    g.data[i].name, g.data[i].v[0], g.data[i].v[1], g.data[i].v[2]);
        }
    }
    fclose(fp);
    return 1;
}
