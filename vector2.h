/* Filename: vector2.h
*  Author: Caleb Wilson
*  Date: 9/30/25
*  Description: Defines the Vec structure and declares all the storage and math functions
*/
#define MAX_VECS 10
#define NAME_LEN 32

// A stored 3D vector with a simple name and 3 doubles.
typedef struct {
    int used;
    char name[NAME_LEN];
    double v[3];
} Vec;

/* Storage management */
void clear_store(void);
void list_store(void);
int  set_vector(const char *name, double x, double y, double z);
int  get_vector(const char *name, double out[3]);

/* Vector math (required) */
void   v_add  (const double a[3], const double b[3], double r[3]);
void   v_sub  (const double a[3], const double b[3], double r[3]);
void   v_scale(const double a[3], double s,        double r[3]);

/* Extra credit */
double v_dot  (const double a[3], const double b[3]);           // scalar
void   v_cross(const double a[3], const double b[3], double r[3]); // vector

/* Display helper */
void print_vec_named(const char *name, const double v[3]);
