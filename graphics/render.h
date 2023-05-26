#ifndef GRAPHICS_RENDER_H
#define GRAPHICS_RENDER_H

#include "./graphics.h"

// TODO
// ADD AA AND BLOOM
// ANTI ALIASING
// MULTIPLE RENDER TARGETS, FRAMEBUFFERS

// Program
struct program {
    uint32_t id;
    char name[64];
};

typedef struct program program;

void program_init(program* p, const char *name, const char *vertex_filename, const char *fragment_filename);
void program_bind(program *p);
void program_set_uniform_1i(program *p, uint32_t index, int32_t value);
void program_set_uniform_1f(program *p, uint32_t index, float value);
void program_set_uniform_2f(program *p, uint32_t index, vec2 value);
void program_set_uniform_3f(program *p, uint32_t index, vec3 value);
void program_set_uniform_m2f(program *p, uint32_t index, mat2 value);
void program_free(program *p);

// Line
struct line {
    vec3 color_0, color_1;
    vec2 position_0, position_1;
};

typedef struct line line;

void line_init(line *l, float x0, float y0, float x1, float y1);
void line_set_position0(line *l, float x, float y);
void line_set_position1(line *l, float x, float y);
void line_set_color0(line *l, float r, float g, float b);
void line_set_color1(line *l, float r, float g, float b);
void line_render(line *l);

// Texture
struct texture {
    uint8_t *data;
    uint32_t id;
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;
};

typedef struct texture texture;

void texture_init(texture *t, const char *filename);
void texture_init_selection(texture *dest, texture *source, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
void texture_bind(texture *t, uint32_t slot);
void texture_free(texture *t);
void texture_3to4(texture *t, float r, float g, float b);

struct quad {
    texture *t;
    vec3 color;
    vec2 position;
    vec2 size;
    vec2 offset;
    float alpha;
    float angle;    // In radians, ofc..
    uint8_t global_offset_bypass;
};

typedef struct quad quad;

void quad_init(quad *rc, float x, float y, float w, float h);

void quad_set_position(quad *rc, float x, float y);
void quad_set_size(quad *rc, float x, float y);
void quad_set_offset(quad *rc, float x, float y);
void quad_set_color(quad *rc, float r, float g, float b, float a);
void quad_set_angle(quad *rc, float a);
void quad_set_texture(quad *rc, texture *t, uint8_t respect_ratio);
void quad_set_global_offset_bypass(quad *rc, uint8_t b);

void quad_render(quad *rc);

// Text
void text_init();
texture *text_textures(uint32_t index); // 0-255
float text_char_ratio();
void text_free();

struct string {
    char *data;
    vec2 position;
    float string_width;
    float char_width;
    uint8_t global_offset_bypass;
};

typedef struct string string;

void string_init(string *s, float x, float y, float char_width);
void string_render(string *s, uint8_t origin);

void string_set_data(string *s, char *data);
void string_set_position(string *s, float x, float y);
void string_set_char_width(string *s, float char_width);
void string_set_char_height(string *s, float char_height);
void string_set_width(string *s, float w);
void string_set_global_offset_bypass(string *s, uint8_t b);

float string_char_height(string *s);
float string_char_width(string *s);
float string_width(string *s);
quad *string_quad();

// Grid
void grid_init();
void grid_render();

void grid_set_line_width(float w);
void grid_set_color(float r, float g, float b);

#endif
