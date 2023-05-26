#include "graphics.h"

float inv_sqrt(const float number) {
	int32_t i;
	float x2 = number * 0.5F, y = number;
	const float threehalfs = 1.5F;

	i = *(int32_t*)&y;
	i = 0x5f3759df - ( i >> 1 );
	y = *(int32_t*)&i;
	y = y * ( threehalfs - ( x2 * y * y ) );
    y = y * ( threehalfs - ( x2 * y * y ) );

	return y;
}

uint32_t closest_pow2(uint32_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
};

float mapf(float val, float min1, float max1, float min2, float max2) {
    return (val - min1) * (max2 - min2) / (max1 - min1) + min2;
}

int32_t mapi(int32_t val, int32_t min1, int32_t max1, int32_t min2, int32_t max2) {
    return (val - min1) * (max2 - min2) / (max1 - min1) + min2;
}

float map_itof(int32_t val, int32_t min1, int32_t max1, float min2, float max2) {
    return (val - min1) * (max2 - min2) / (max1 - min1) + min2;
}

int32_t map_ftoi(float val, float min1, float max1, int32_t min2, int32_t max2) {
    return (val - min1) * (max2 - min2) / (max1 - min1) + min2;
}

// VEC 2
vec2 vec2_adds(vec2 a, float s) {
    const vec2 r = {a.x + s, a.y + s};
    return r;
}

vec2 vec2_subs(vec2 a, float s) {
    const vec2 r = {a.x - s, a.y - s};
    return r;
}

vec2 vec2_scale(vec2 a, float s) {
    const vec2 r = {a.x * s, a.y * s};
    return r;
}

vec2 vec2_add(vec2 a, vec2 b) {
    const vec2 r = {a.x + b.x, a.y + b.y};
    return r;
}

vec2 vec2_sub(vec2 a, vec2 b) {
    const vec2 r = {a.x - b.x, a.y - b.y};
    return r;
}

float vec2_dot(vec2 a, vec2 b) {
    const float r = a.x * b.x + a.y * b.y;
    return r;
}

vec2 vec2_norm(vec2 a) {
    const float f = inv_sqrt(a.x*a.x + a.y*a.y);
    vec2 r = {a.x * f, a.y * f};
    return r;
}

vec2 vec2_norms(vec2 a, float s) {
    const float f = inv_sqrt(a.x*a.x + a.y*a.y) * s;
    vec2 r = {a.x * f, a.y * f};
    return r;
}

float   vec2_len(vec2 a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}

float *vec2_data(vec2 *a) {
    return &a->x;
}

// VEC 3
vec3 vec3_adds(vec3 a, float s) {
    const vec3 r = {a.x + s, a.y + s, a.z + s};
    return r;
}

vec3 vec3_subs(vec3 a, float s) {
    const vec3 r = {a.x - s, a.y - s, a.z - s};
    return r;
}

vec3 vec3_scale(vec3 a, float s) {
    const vec3 r = {a.x * s, a.y * s, a.z * s};
    return r;
}

vec3 vec3_add(vec3 a, vec3 b) {
    const vec3 r = {a.x + b.x, a.y + b.y, a.z + b.z};
    return r;
}

vec3 vec3_sub(vec3 a, vec3 b) {
    const vec3 r = {a.x - b.x, a.y - b.y, a.z - b.z};
    return r;
}

float vec3_dot(vec3 a, vec3 b) {
    const float r = a.x * b.x + a.y * b.y + a.z * b.z;
    return r;
}

vec3 vec3_norm(vec3 a) {
    const float f = inv_sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
    vec3 r = {a.x * f, a.y * f, a.z * f};
    return r;
}

vec3 vec3_norms(vec3 a, float s) {
    const float f = inv_sqrt(a.x*a.x + a.y*a.y + a.z*a.z) * s;
    vec3 r = {a.x * f, a.y * f, a.z * f};
    return r;
}

float *vec3_data(vec3 *a) {
    return &a->x;
}

// MAT 2
mat2 mat2_muls(mat2 a, float s) {
    mat2 r = { 
        {a.i.x * s, a.i.y * s},
        {a.j.x * s, a.j.y * s}
    };
    return r;
}

vec2 vec2_transform(mat2 t, vec2 a) {
    vec2 r = {a.x * t.i.x + a.y * t.j.x, a.x * t.i.y + a.y * t.j.y };
    return r;
}

mat2 mat2_add(mat2 a, mat2 b) {
    mat2 r = {
        {a.i.x + b.i.x, a.i.y + b.i.y},
        {a.j.x + b.j.x, a.j.y + b.j.y}
    };
    return r;
}

mat2 mat2_sub(mat2 a, mat2 b) {
    mat2 r = {
        {a.i.x - b.i.x, a.i.y - b.i.y},
        {a.j.x - b.j.x, a.j.y - b.j.y}
    };
    return r;
}

mat2 mat2_mul(mat2 a, mat2 b) {
    mat2 r = {
        {a.i.x*b.i.x + a.j.x*b.i.y, a.i.y*b.i.x + a.j.y*b.i.y},
        {a.i.x*b.j.x + a.j.x*b.j.y, a.i.y*b.j.x + a.j.y*b.j.y}
    };
    return r;
}

float mat2_det(mat2 a) {
    return a.i.x * a.j.y - a.j.x * a.i.y;
}

mat2 mat2_tran(mat2 a) {
    mat2 r = {
        {a.i.x, a.j.x},
        {a.i.y, a.j.y}
    };
    return r;
}

mat2 mat2_cof(mat2 a) {
    mat2 r = {
        {+a.j.y, -a.j.x},
        {-a.i.y, +a.i.x}
    };
    return r;
}

mat2 mat2_adj(mat2 a) {
    mat2 r = {
        {+a.j.y, -a.i.y},
        {-a.j.x, +a.i.x}
    };
    return r;
}

mat2 mat2_inv(mat2 a) {
    float det = a.i.x * a.j.y - a.j.x * a.i.y;
    mat2 r = {
        { +(a.j.y / det), -(a.i.y / det) },
        { -(a.j.x / det), +(a.i.x / det) }
    };
    return r;
}

float *mat2_data(mat2 *a) {
    return &a->i.x;
}

// Unghiuri
float to_degrees(float radians) {
    return 180*radians*M_1_PI;
}

float to_radians(float degrees) {
    return degrees*M_PI*0.00555555555555555555f;
}

float vec2_ang(vec2 a, vec2 b) {
    return acosf( vec2_dot(a, b) * inv_sqrt( (a.x*a.x + a.y*a.y) * (b.x*b.x + b.y*b.y) ) );
}

float vec2_ang_cos(vec2 a, vec2 b) {
    return vec2_dot(a, b) * inv_sqrt( (a.x*a.x + a.y*a.y) * (b.x*b.x + b.y*b.y) );
}

mat2 mat2_rot(float s) {
    float sin_v = sinf(s);
    float cos_v = cosf(s);
    mat2 r = {
        {+cos_v, +sin_v},
        {-sin_v, +cos_v}
    };
    return r;
}

mat2 mat2_rot_deg(float s) {
    float sin_v = sinf(s*M_PI*0.00555555555555555555f);
    float cos_v = cosf(s*M_PI*0.00555555555555555555f);
    mat2 r = {
        {+cos_v, +sin_v},
        {-sin_v, +cos_v}
    };
    return r;
}

