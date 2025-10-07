/* Filename: vector2.c
*  Author: Caleb Wilson
*  Date: 9/30/25
*  Description: Implements the storage layer (fixed-size array) and vector math.
*/
#include <stdio.h>
#include <string.h>
#include "vector2.h"

// Module-private storage; not visible outside this file.
static Vec store[MAX_VECS];

// Find an existing vector by name; return index or -1.
static int find_index_by_name(const char *name) {
    for (int i = 0; i < MAX_VECS; ++i) {
        if (store[i].used && strcmp(store[i].name, name) == 0) return i;
    }
    return -1;
}

// Find first free slot; return index or -1 if full.
static int first_free_index(void) {
    for (int i = 0; i < MAX_VECS; ++i)
        if (!store[i].used) return i;
    return -1;
}

/* ----- Storage API ----- */

void clear_store(void) {
    for (int i = 0; i < MAX_VECS; ++i) store[i].used = 0;
}

void list_store(void) {
    int any = 0;
    for (int i = 0; i < MAX_VECS; ++i) {
        if (store[i].used) {
            print_vec_named(store[i].name, store[i].v);
            any = 1;
        }
    }
    if (!any) puts("(no vectors stored)");
}

int set_vector(const char *name, double x, double y, double z) {
    int idx = find_index_by_name(name);
    if (idx >= 0) { // replace in place
        store[idx].v[0] = x; store[idx].v[1] = y; store[idx].v[2] = z;
        return 1;
    }
    idx = first_free_index();
    if (idx < 0) return 0; // full
    store[idx].used = 1;
    strncpy(store[idx].name, name, NAME_LEN - 1);
    store[idx].name[NAME_LEN - 1] = '\0';
    store[idx].v[0] = x; store[idx].v[1] = y; store[idx].v[2] = z;
    return 1;
}

int get_vector(const char *name, double out[3]) {
    int idx = find_index_by_name(name);
    if (idx < 0) return 0;
    out[0] = store[idx].v[0];
    out[1] = store[idx].v[1];
    out[2] = store[idx].v[2];
    return 1;
}

/* ----- Vector math (required) ----- */

void v_add(const double a[3], const double b[3], double r[3]) {
    r[0] = a[0] + b[0]; r[1] = a[1] + b[1]; r[2] = a[2] + b[2];
}

void v_sub(const double a[3], const double b[3], double r[3]) {
    r[0] = a[0] - b[0]; r[1] = a[1] - b[1]; r[2] = a[2] - b[2];
}

void v_scale(const double a[3], double s, double r[3]) {
    r[0] = a[0] * s; r[1] = a[1] * s; r[2] = a[2] * s;
}

/* ----- Extra credit ----- */

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
    // Fixed precision for readability.
    printf("%s = %.3f   %.3f   %.3f\n", name, v[0], v[1], v[2]);
}
