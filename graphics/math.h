#ifndef GRAPHICS_MATH_H
#define GRAPHICS_MATH_H

#include "./graphics.h"

float inv_sqrt(const float number);     // 1 / sqrt(number)
uint32_t closest_pow2(uint32_t n);      // Cea mai apropiata putere a lui doi mai mare ca n
float mapf(float val, float min1, float max1, float min2, float max2);              // Mapare pentru floats
int32_t mapi(int32_t val, int32_t min1, int32_t max1, int32_t min2, int32_t max2);  // Mapare pentru int32_ts
float map_itof(int32_t val, int32_t min1, int32_t max1, float min2, float max2);    // Mapare int la float
int32_t map_ftoi(float val, float min1, float max1, int32_t min2, int32_t max2);    // Mapare float la int

// VEC 2
struct vec2 {
    float x, y;
};

typedef struct vec2 vec2;

vec2    vec2_adds(vec2 a, float s);     // r = a + s
vec2    vec2_subs(vec2 a, float s);     // r = a - s
vec2    vec2_scale(vec2 a, float s);    // r = s * a

vec2    vec2_add(vec2 a, vec2 b);       // r = a + b
vec2    vec2_sub(vec2 a, vec2 b);       // r = a - b
float   vec2_dot(vec2 a, vec2 b);       // r = a * b

vec2    vec2_norm(vec2 a);              // Normalizare la 1
vec2    vec2_norms(vec2 a, float s);    // Normalizare la s

float   vec2_len(vec2 a);               // Euclidian length

float   *vec2_data(vec2 *a);            // Returneaza adresa

// VEC 3
struct vec3 {
    float x, y, z;
};

typedef struct vec3 vec3;

vec3    vec3_adds(vec3 a, float s);
vec3    vec3_subs(vec3 a, float s);
vec3    vec3_scale(vec3 a, float s);

vec3    vec3_add(vec3 a, vec3 b);
vec3    vec3_sub(vec3 a, vec3 b);
float   vec3_dot(vec3 a, vec3 b);

vec3    vec3_norm(vec3 a);
vec3    vec3_norms(vec3 a, float s);

float   *vec3_data(vec3 *a);

// MAT 2
struct mat2 {
    vec2 i, j;
};

typedef struct mat2 mat2;

mat2    mat2_muls(mat2 a, float s);         // r = s * a
vec2    vec2_transform(mat2 t, vec2 a);     // r = t * a

mat2    mat2_add(mat2 a, mat2 b);           // r = a + b
mat2    mat2_sub(mat2 a, mat2 b);           // r = a - b
mat2    mat2_mul(mat2 a, mat2 b);           // r = a * b

float   mat2_det(mat2 a);                   // r = determinant(a)
mat2    mat2_tran(mat2 a);                  // r = transpusa(a)
mat2    mat2_cof(mat2 a);                   // r = cofactor(a)
mat2    mat2_adj(mat2 a);                   // r = adjuncta(a)
mat2    mat2_inv(mat2 a);                   // r = inversa(a)

float   *mat2_data(mat2 *a);                // Returneaza adresa

// Pentru conversie radiani->grade sau grade->radiani
float to_degrees(float radians);
float to_radians(float degrees);

float vec2_ang(vec2 a, vec2 b);     // Returneaza unghiul dintre a si b in radiani
float vec2_ang_cos(vec2 a, vec2 b); // Returneaza cosinusul unghiului dintre a si b

mat2 mat2_rot(float s);             // Returneaza matrice de rotatie pentru s radiani
mat2 mat2_rot_deg(float s);         // Returneaza matrice de rotatie pentru s grade

#endif
