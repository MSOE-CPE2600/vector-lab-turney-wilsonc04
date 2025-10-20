/* Filename: vector_update.h
 * Author: Caleb Wilson
 * Date: 10/19/25
 * Description: Lab 7 dynamic storage + Lab 5 API surface.
 */
#ifndef VECTOR_UPDATE_H
#define VECTOR_UPDATE_H

#include <stddef.h>

#define NAME_LEN 32

typedef struct {
    int used;
    char name[NAME_LEN];
    double v[3];
} Vec;

/* Init / teardown */
void init_store(void);
void free_store(void);

/* Storage management */
void clear_store(void);
void list_store(void);
int  set_vector(const char *name, double x, double y, double z);
int  get_vector(const char *name, double out[3]);

/* Vector math (required) */
void   v_add  (const double a[3], const double b[3], double r[3]);
void   v_sub  (const double a[3], const double b[3], double r[3]);
void   v_scale(const double a[3], double s,          double r[3]);

/* Extra credit */
double v_dot  (const double a[3], const double b[3]);              // scalar
void   v_cross(const double a[3], const double b[3], double r[3]); // vector

/* Display helper */
void print_vec_named(const char *name, const double v[3]);

/* CSV I/O */
int load_csv(const char *fname); // clears store first
int save_csv(const char *fname); // overwrites

#endif /* VECTOR_UPDATE_H */
