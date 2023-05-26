#include "./graphics.h"

// Program
void program_init(program* p, const char *name, const char *vertex_filename, const char *fragment_filename)
{
    const char *vertex_src = string_from_filename(vertex_filename);
    const char *fragment_src = string_from_filename(fragment_filename);
    const uint32_t vertex = GL(glCreateShader(GL_VERTEX_SHADER));
    const uint32_t fragment = GL(glCreateShader(GL_FRAGMENT_SHADER));
    uint32_t i;

    int32_t success;
    char infoLog[1024];

    for(i=0; i<64 && name[i]!='\0'; i++)
        p->name[i] = name[i];
    p->name[i] = '\0';

    GL(glShaderSource(vertex, 1, &vertex_src, NULL));
    free(vertex_src);
    GL(glCompileShader(vertex));
    GL(glGetShaderiv(vertex, GL_COMPILE_STATUS, &success));
    if(!success) {
        GL(glGetShaderInfoLog(vertex, 1024, NULL, infoLog));
        debug_send("Opengl_VertexShader", "[%s]\n%s", p->name, infoLog);
    };

    GL(glShaderSource(fragment, 1, &fragment_src, NULL));
    free(fragment_src);
    GL(glCompileShader(fragment));
    GL(glGetShaderiv(fragment, GL_COMPILE_STATUS, &success));
    if(!success) {
        GL(glGetShaderInfoLog(fragment, 1024, NULL, infoLog));
        debug_send("Opengl_FragmentShader", "[%s]\n%s", p->name, infoLog);
    };

    GL(p->id = glCreateProgram());
    GL(glAttachShader(p->id, vertex));
    GL(glAttachShader(p->id, fragment));
    GL(glLinkProgram(p->id));
    GL(glGetProgramiv(p->id, GL_LINK_STATUS, &success));
    if(!success) {
        GL(glGetProgramInfoLog(p->id, 1024, NULL, infoLog));
        debug_send("Opengl_Program", "[%s] - %s", p->name, infoLog);
    };

    GL(glDeleteShader(vertex));
    GL(glDeleteShader(fragment));
}

void program_bind(program *p) {
    GL(glUseProgram(p->id));
}

void program_set_uniform_1i(program *p, uint32_t index, int32_t value) {
    GL(glUseProgram(p->id));
    GL(glUniform1i(index, value));
}

void program_set_uniform_1f(program *p, uint32_t index, float value) {
    GL(glUseProgram(p->id));
    GL(glUniform1f(index, value));
}

void program_set_uniform_2f(program *p, uint32_t index, vec2 value) {
    GL(glUseProgram(p->id));
    GL(glUniform2f(index, value.x, value.y));
}

void program_set_uniform_3f(program *p, uint32_t index, vec3 value) {
    GL(glUseProgram(p->id));
    GL(glUniform3f(index, value.x, value.y, value.z));
}

void program_set_uniform_m2f(program *p, uint32_t index, mat2 value) {
    GL(glUseProgram(p->id));
    GL(glUniformMatrix2fv(index, 1, 0, mat2_data(&value)));
}

void program_free(program *p) {
    GL(glDeleteProgram(p->id));
}

// Line
static program s_line_program;
static int32_t s_line_index[2] = {0, 1};
static uint32_t s_line_vao;
static uint32_t s_line_vbo;
static uint8_t s_line_initialized = 0;
static const uint32_t s_line_transform_uniform = 1;
static const uint32_t s_line_g_offset_uniform = 2;
static const uint32_t s_line_position0_uniform = 3;
static const uint32_t s_line_position1_uniform = 4;
static const uint32_t s_line_color0_uniform = 5;
static const uint32_t s_line_color1_uniform = 6;

void line_init(line *l, float x0, float y0, float x1, float y1) {
    if(!s_line_initialized) {
        program_init(&s_line_program, "Line", "shaders/vertex/line.glsl", "shaders/fragment/line.glsl");

        GL(glGenVertexArrays(1, &s_line_vao));
        GL(glGenBuffers(1, &s_line_vbo));

        GL(glBindVertexArray(s_line_vao));
        GL(glBindBuffer(GL_ARRAY_BUFFER, s_line_vbo));
        GL(glBufferData(GL_ARRAY_BUFFER, sizeof(s_line_index), s_line_index, GL_STATIC_DRAW));

        GL(glVertexAttribIPointer(0, 1, GL_INT, sizeof(int32_t), NULL));
        GL(glEnableVertexAttribArray(0));

        GL(glBindVertexArray(0));

        s_line_initialized = 1;
    }

    l->position_0.x = x0;
    l->position_0.y = y0;
    l->position_1.x = x1;
    l->position_1.y = y1;
    l->color_0.x = 1.0f;
    l->color_0.y = 1.0f;
    l->color_0.z = 1.0f;
    l->color_1.x = 1.0f;
    l->color_1.y = 1.0f;
    l->color_1.z = 1.0f;
}

void line_set_position0(line *l, float x, float y) {
    l->position_0.x = x;
    l->position_0.y = y;
}

void line_set_position1(line *l, float x, float y) {
    l->position_1.x = x;
    l->position_1.y = y;
}

void line_set_color0(line *l, float r, float g, float b) {
    l->color_0.x = r;
    l->color_0.y = g;
    l->color_0.z = b;
}

void line_set_color1(line *l, float r, float g, float b) {
    l->color_1.x = r;
    l->color_1.y = g;
    l->color_1.z = b;
}

void line_render(line *l) {
    program_set_uniform_m2f(&s_line_program, s_line_transform_uniform, g_transformation);
    program_set_uniform_2f(&s_line_program, s_line_g_offset_uniform, g_offset);
    program_set_uniform_2f(&s_line_program, s_line_position0_uniform, l->position_0);
    program_set_uniform_2f(&s_line_program, s_line_position1_uniform, l->position_1);
    program_set_uniform_3f(&s_line_program, s_line_color0_uniform, l->color_0);
    program_set_uniform_3f(&s_line_program, s_line_color1_uniform, l->color_1);

    program_bind(&s_line_program);
    GL(glBindVertexArray(s_line_vao));
    GL(glDrawArrays(GL_LINES, 0, 2));
    GL(glBindVertexArray(0));
}

// Texture
void texture_init(texture *t, const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if(!file) {
        debug_send("texture_init", "File '%s' not found.", filename);
        return;
    }

    uint32_t data_offset, pixel_count, i;

    fseek(file, 2 * sizeof(uint32_t) + sizeof(uint16_t), SEEK_SET);
    fread(&data_offset, sizeof(uint32_t), 1, file);

    fseek(file, sizeof(uint32_t), SEEK_CUR);
    fread(&t->width, sizeof(uint32_t), 1, file);
    fread(&t->height, sizeof(uint32_t), 1, file);

    fseek(file, sizeof(uint16_t), SEEK_CUR);
    fread(&t->bytes_per_pixel, sizeof(uint16_t), 1, file);
    t->bytes_per_pixel /= 8;

    fseek(file, data_offset, SEEK_SET);
    pixel_count = t->width * t->height;
    t->data = malloc(t->bytes_per_pixel * pixel_count * sizeof(unsigned char));

    GL(glGenTextures(1, &t->id));
    GL(glActiveTexture(GL_TEXTURE0)); // Temporar
    GL(glBindTexture(GL_TEXTURE_2D, t->id));

    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    if(t->bytes_per_pixel == 3)
    {
        for(i=0; i<pixel_count; i++)
        {
            t->data[i*t->bytes_per_pixel + 2] = fgetc(file);    // b
            t->data[i*t->bytes_per_pixel + 1] = fgetc(file);    // g
            t->data[i*t->bytes_per_pixel + 0] = fgetc(file);    // r
        }

        GL(glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, t->width, t->height, 0, GL_RGB, GL_UNSIGNED_BYTE, t->data));
    }
    else if(t->bytes_per_pixel == 4)
    {
        for(i=0; i<pixel_count; i++)
        {
            t->data[i*t->bytes_per_pixel + 2] = fgetc(file);    // b
            t->data[i*t->bytes_per_pixel + 1] = fgetc(file);    // g
            t->data[i*t->bytes_per_pixel + 0] = fgetc(file);    // r
            t->data[i*t->bytes_per_pixel + 3] = fgetc(file);    // a
        }

        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->data));
    }

    GL(glBindTexture(GL_TEXTURE_2D, 0));
    fclose(file);
}

void texture_init_selection(texture *d, texture *s, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
    if( x1 >= x2 || y1 >= y2 || x2 > s->width || y2 > s->height ) {
        debug_send("Texture_selection", "Coordinates are out of range.");
        return;
    }

    uint32_t i, j, k;

    d->width = x2 - x1 + 1;
    d->height = y2 - y1 + 1;
    d->bytes_per_pixel = s->bytes_per_pixel;

    d->data = malloc( d->width * d->height * d->bytes_per_pixel * sizeof(uint16_t) );

    GL(glGenTextures(1, &d->id));
    GL(glActiveTexture(GL_TEXTURE0));
    GL(glBindTexture(GL_TEXTURE_2D, d->id));

    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    if(d->bytes_per_pixel == 3)
    {
        k=0;
        for(i=y1; i<=y2; i++)
        {
            for(j=x1; j<=x2; j++)
            {
                d->data[k + 2] = s->data[3 * (i*s->width + j) + 2];
                d->data[k + 1] = s->data[3 * (i*s->width + j) + 1];
                d->data[k + 0] = s->data[3 * (i*s->width + j) + 0];
                k+=3;
            }
        }

        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, d->width, d->height, 0, GL_RGB, GL_UNSIGNED_BYTE, d->data));
    }
    else if(d->bytes_per_pixel == 4)
    {
        k=0;
        for(i=y1; i<=y2; i++)
        {
            for(j=x1; j<=x2; j++)
            {
                d->data[k + 2] = s->data[4 * (i*s->width + j) + 2];
                d->data[k + 1] = s->data[4 * (i*s->width + j) + 1];
                d->data[k + 0] = s->data[4 * (i*s->width + j) + 0];
                d->data[k + 3] = s->data[4 * (i*s->width + j) + 3];
                k+=4;
            }
        }

        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, d->width, d->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, d->data));
    }

    GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void texture_bind(texture *t, uint32_t slot) {
    GL(glActiveTexture(GL_TEXTURE0 + slot));
    GL(glBindTexture(GL_TEXTURE_2D, t->id));
}

void texture_free(texture *t) {
    free(t->data);
    GL(glDeleteTextures(1, &t->id));
    memset(t, 0, sizeof(texture));
}

void texture_3to4(texture *t, float r, float g, float b) {
    uint32_t pixel_count = t->width * t->height, i;
    uint8_t ri = map_ftoi(r, 0.0f, 1.0f, 0, 255);
    uint8_t gi = map_ftoi(g, 0.0f, 1.0f, 0, 255);
    uint8_t bi = map_ftoi(b, 0.0f, 1.0f, 0, 255);
    uint8_t dr, dg, db;

    t->bytes_per_pixel = 4;

    uint8_t *temp = malloc(pixel_count * t->bytes_per_pixel * sizeof(uint8_t));

    for(i=0; i<pixel_count; i++)
    {
        dr =  t->data[i*3 + 0];
        dg =  t->data[i*3 + 1];
        db =  t->data[i*3 + 2];

        temp[i*4 + 0] = dr;
        temp[i*4 + 1] = dg;
        temp[i*4 + 2] = db;

        if((dr == ri) && (dg == gi) && (db == bi)) temp[i*4 + 3] = 0;
        else temp[i*4 + 3] = 255;
    }

    free(t->data);
    t->data = temp;

    GL(glActiveTexture(GL_TEXTURE0));
    GL(glBindTexture(GL_TEXTURE_2D, t->id));
    GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->data));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL(glBindTexture(GL_TEXTURE_2D, 0));
}

// Dreptunghi
static program s_quad_texture_program;
static program s_quad_color_program;
static uint32_t s_quad_initialized = 0;
static uint32_t s_quad_vbo;
static uint32_t s_quad_vao;
static uint32_t s_quad_ebo;
static uint32_t s_quad_position_uniform = 2;
static uint32_t s_quad_size_uniform = 3;
static uint32_t s_quad_offset_uniform = 4;
static uint32_t s_quad_transform_uniform = 5;
static uint32_t s_quad_g_offset_uniform = 6;
static uint32_t s_quad_color_uniform = 7;
static uint32_t s_quad_alpha_uniform = 8;
static uint32_t s_quad_angle_uniform = 9;
static uint32_t s_ratio_uniform = 10;

static vec2 s_quad_vertices[] = {
    {-0.5f, -0.5f},   {0.0f, 0.0f},
    {+0.5f, -0.5f},   {1.0f, 0.0f},
    {-0.5f, +0.5f},   {0.0f, 1.0f},
    {+0.5f, +0.5f},   {1.0f, 1.0f}
};

static uint32_t s_quad_index[] = {
    0, 1, 2,
    1, 2, 3
};

void quad_init(quad *rc, float x, float y, float w, float h) {
    if(!s_quad_initialized) {
        program_init(&s_quad_texture_program, "quad_textured", "shaders/vertex/quad.glsl", "shaders/fragment/quad_textured.glsl");
        program_init(&s_quad_color_program, "quad_colored", "shaders/vertex/quad.glsl", "shaders/fragment/quad_colored.glsl");

        glGenVertexArrays(1, &s_quad_vao);
        glGenBuffers(1, &s_quad_vbo);
        glGenBuffers(1, &s_quad_ebo);

        glBindVertexArray(s_quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, s_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_quad_vertices), s_quad_vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_quad_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_quad_index), s_quad_index, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(float), NULL);
        glVertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        s_quad_initialized = 1;
    }

    rc->t = NULL;
    rc->color.x = 1.0f;
    rc->color.y = 1.0f;
    rc->color.z = 1.0f;
    rc->position.x = x;
    rc->position.y = y;
    rc->size.x = w;
    rc->size.y = h;
    rc->offset.x = 0.0f;
    rc->offset.y = 0.0f;
    rc->alpha = 1.0f;
    rc->angle = 0.0f;
    rc->global_offset_bypass = 0;
}

void quad_set_position(quad *rc, float x, float y) {
    rc->position.x = x;
    rc->position.y = y;
}

void quad_set_size(quad *rc, float x, float y) {
    rc->size.x = x;
    rc->size.y = y;
}

void quad_set_offset(quad *rc, float x, float y) {
    rc->offset.x = x * 0.5f;
    rc->offset.y = y * 0.5f;
}

void quad_set_color(quad *rc, float r, float g, float b, float a) {
    rc->color.x = r;
    rc->color.y = g;
    rc->color.z = b;
    rc->alpha = a;
}

void quad_set_angle(quad *rc, float a) {
    rc->angle = a;
}

void quad_set_texture(quad *rc, texture *t, uint8_t respect_ratio) {
    rc->t = t;
    if(respect_ratio) rc->size.y = rc->size.x * ( (float)rc->t->height / (float)rc->t->width );
}

void quad_render(quad *rc) {
   if(rc->t) {
        program_set_uniform_2f(&s_quad_texture_program, s_quad_position_uniform, rc->position);
        program_set_uniform_2f(&s_quad_texture_program, s_quad_size_uniform, rc->size);
        program_set_uniform_2f(&s_quad_texture_program, s_quad_offset_uniform, rc->offset);
        program_set_uniform_m2f(&s_quad_texture_program, s_quad_transform_uniform, g_transformation);
        program_set_uniform_2f(&s_quad_texture_program, s_quad_g_offset_uniform, g_offset);
        program_set_uniform_1f(&s_quad_texture_program, s_quad_angle_uniform, rc->angle);
        if(rc->global_offset_bypass)
            program_set_uniform_1f(&s_quad_texture_program, s_ratio_uniform, g_ratio);
        else
            program_set_uniform_1f(&s_quad_texture_program, s_ratio_uniform, 0.0f);

        program_bind(&s_quad_texture_program);
        texture_bind(rc->t, 0);
   }
   else {
        program_set_uniform_2f(&s_quad_color_program, s_quad_position_uniform, rc->position);
        program_set_uniform_2f(&s_quad_color_program, s_quad_size_uniform, rc->size);
        program_set_uniform_2f(&s_quad_color_program, s_quad_offset_uniform, rc->offset);
        program_set_uniform_m2f(&s_quad_color_program, s_quad_transform_uniform, g_transformation);
        program_set_uniform_2f(&s_quad_color_program, s_quad_g_offset_uniform, g_offset);
        program_set_uniform_3f(&s_quad_color_program, s_quad_color_uniform, rc->color);
        program_set_uniform_1f(&s_quad_color_program, s_quad_alpha_uniform, rc->alpha);
        program_set_uniform_1f(&s_quad_color_program, s_quad_angle_uniform, rc->angle);
        if(rc->global_offset_bypass)
            program_set_uniform_1f(&s_quad_color_program, s_ratio_uniform, g_ratio);
        else
            program_set_uniform_1f(&s_quad_color_program, s_ratio_uniform, 0.0f);

        program_bind(&s_quad_color_program);
   }

    glBindVertexArray(s_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

void quad_set_global_offset_bypass(quad *rc, uint8_t b) {
    rc->global_offset_bypass = b;
}

// Text
static texture s_text_source;
static texture s_text_chars[256];
static quad s_text_char_quad;
static float s_text_char_ratio;
static uint8_t s_text_initialized = 0;

void text_init() {
    if(s_text_initialized) return;

    uint32_t w, h, px_per_x, px_per_y;
    uint32_t i, j;

    texture_init(&s_text_source, "bmps/chars.bmp");
    texture_3to4(&s_text_source, 0.0f, 0.0f, 0.0f);
    w = s_text_source.width;
    h = s_text_source.height;
    px_per_x = w / 16;
    px_per_y = h / 16;

    for(i=0; i<16; i++)
        for(j=0; j<16; j++)
            texture_init_selection(s_text_chars + 16*i + j, &s_text_source, j*px_per_x, h - (i+1)*px_per_y, (j+1)*px_per_x, h - i*px_per_y);

    s_text_char_ratio = ( (float) s_text_chars->height ) / ( (float) s_text_chars->width );

    quad_init(&s_text_char_quad, 0.0f, 0.0f, 1.0f, 1.0f);

    s_text_initialized = 1;
}

texture *text_textures(uint32_t index) {
    if(!s_text_initialized) {
        debug_send("text_textures", "Text has not been initialized.");
        return NULL;
    }

    return s_text_chars + index;
}

float text_char_ratio() {
    if(!s_text_initialized) {
        debug_send("text_char_ratio", "Text has not been initialized.");
        return 0.0f;
    }

    return s_text_char_ratio;
}

void text_free() {
    for(uint32_t i=0; i<256; i++) texture_free(s_text_chars + i);
}

void string_init(string *s, float x, float y, float char_width) {
    if(!s_text_initialized) {
        debug_send("string_init", "Text has not been initialized.");
        return;
    }

    s->data = NULL;
    s->position.x = x;
    s->position.y = y;
    s->char_width = char_width;
    s->string_width = 0.0f;
    s->global_offset_bypass = 0;
}

void string_render(string *s, uint8_t origin) {
    if(!s_text_initialized) {
        debug_send("string_render", "Text has not been initialized.");
        return;
    }

    int32_t count = strlen(s->data);
    int32_t i;
    quad_set_size(&s_text_char_quad, s->char_width, 0.0f);
    quad_set_global_offset_bypass(&s_text_char_quad, s->global_offset_bypass);

    switch(origin) {
        case 0:
            if(count % 2 == 0) {
                quad_set_offset(&s_text_char_quad, 1.0f, 0.0f);
                for(i=0; i<count; i++) {
                    quad_set_position(&s_text_char_quad, s->position.x + (i - count/2 + 1) * s->char_width, s->position.y);
                    quad_set_texture(&s_text_char_quad, s_text_chars + s->data[i], 1);
                    quad_render(&s_text_char_quad);
                }
            }
            else {
                quad_set_offset(&s_text_char_quad, 0.0f, 0.0f);
                for(i=0; i<count; i++) {
                    quad_set_position(&s_text_char_quad, s->position.x + (i - count/2) * s->char_width, s->position.y);
                    quad_set_texture(&s_text_char_quad, s_text_chars + s->data[i], 1);
                    quad_render(&s_text_char_quad);
                }
            }
            break;
        case 1:
            quad_set_offset(&s_text_char_quad, 0.0f, 0.0f);
            for(i=0; i<count; i++) {
                quad_set_position(&s_text_char_quad, s->position.x + i * s->char_width, s->position.y);
                quad_set_texture(&s_text_char_quad, s_text_chars + s->data[i], 1);
                quad_render(&s_text_char_quad);
            }
            break;
        case 2:
            quad_set_offset(&s_text_char_quad, 0.0f, 0.0f);
            for(i=0; i<count; i++) {
                quad_set_position(&s_text_char_quad, s->position.x + (i - count + 1) * s->char_width, s->position.y);
                quad_set_texture(&s_text_char_quad, s_text_chars + s->data[i], 1);
                quad_render(&s_text_char_quad);
            }
            break;
    }
}

void string_set_data(string *s, char *data) {
    s->data = data;
    s->string_width = strlen(s->data) * s->char_width;
}

void string_set_position(string *s, float x, float y) {
    s->position.x = x;
    s->position.y = y;
}

void string_set_char_width(string *s, float char_width) {
    s->char_width = char_width;
    s->string_width = char_width * strlen(s->data);
}

void string_set_char_height(string *s, float char_height) {
    s->char_width = char_height / s_text_char_ratio;
}

void string_set_width(string *s, float w) {
    s->char_width = w / ((float) strlen(s->data));
}

void string_set_global_offset_bypass(string *s, uint8_t b) {
    s->global_offset_bypass = b;
}

float string_char_height(string *s) {
    return s->char_width * s_text_char_ratio;
}

float string_char_width(string *s) {
    return s->char_width;
}

float string_width(string *s) {
    return s->string_width;
}

quad *string_quad() {
    return &s_text_char_quad;
}

// Grid
static program s_grid_program;
static vec3 s_grid_color = {1.0f, 1.0f, 1.0f};
static vec2 s_grid_win_size_value;
static float s_grid_unit_boundary_value = 0.0f;
static uint32_t s_grid_vao;
static uint32_t s_grid_vbo;
static uint32_t s_grid_ebo;
static uint32_t s_grid_initialized = 0;
static uint32_t s_grid_transform_uniform = 1;
static uint32_t s_grid_color_uniform = 2;
static uint32_t s_grid_g_offset_uniform = 3;
static uint32_t s_grid_win_size_uniform = 4;
static uint32_t s_grid_px_per_unit_uniform = 5;
static uint32_t s_grid_unit_boundary_uniform = 6;

static vec2 s_grid_vertices[4] = {
    {-1.0f, +1.0f},
    {-1.0f, -1.0f},
    {+1.0f, +1.0f},
    {+1.0f, -1.0f}
};

static uint32_t s_grid_index[6] = {
    0, 1, 2,
    1, 2, 3
};

void grid_init() {
    if(!s_grid_initialized) {
        s_grid_unit_boundary_value = 2.0f / (float)g_window_width;

        s_grid_win_size_value.x = (float)g_window_width;
        s_grid_win_size_value.y = (float)g_window_height;

        program_init(&s_grid_program, "Grid", "shaders/vertex/grid.glsl", "shaders/fragment/grid.glsl");
        program_set_uniform_m2f(&s_grid_program, s_grid_transform_uniform, g_transformation);
        program_set_uniform_3f(&s_grid_program, s_grid_color_uniform, s_grid_color);
        program_set_uniform_2f(&s_grid_program, s_grid_g_offset_uniform, g_offset);
        program_set_uniform_2f(&s_grid_program, s_grid_win_size_uniform, s_grid_win_size_value);
        program_set_uniform_1f(&s_grid_program, s_grid_px_per_unit_uniform, g_px_per_unit);
        program_set_uniform_1f(&s_grid_program, s_grid_unit_boundary_uniform, s_grid_unit_boundary_value);

        GL(glGenVertexArrays(1, &s_grid_vao));
        GL(glGenBuffers(1, &s_grid_vbo));
        GL(glGenBuffers(1, &s_grid_ebo));

        GL(glBindVertexArray(s_grid_vao));
        GL(glBindBuffer(GL_ARRAY_BUFFER, s_grid_vbo));
        GL(glBufferData(GL_ARRAY_BUFFER, sizeof(s_grid_vertices), s_grid_vertices, GL_STATIC_DRAW));
        GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_grid_ebo));
        GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_grid_index), s_grid_index, GL_STATIC_DRAW));

        GL(glVertexAttribPointer(0, 2, GL_FLOAT, 0, 2 * sizeof(float), NULL));
        GL(glEnableVertexAttribArray(0));

        GL(glBindVertexArray(0));
    }
}

void grid_render() {
    s_grid_win_size_value.x = (float)g_window_width;
    s_grid_win_size_value.y = (float)g_window_height;

    program_set_uniform_m2f(&s_grid_program, s_grid_transform_uniform, g_transformation);
    program_set_uniform_2f(&s_grid_program, s_grid_g_offset_uniform, g_offset);
    program_set_uniform_2f(&s_grid_program, s_grid_win_size_uniform, s_grid_win_size_value);
    program_set_uniform_1f(&s_grid_program, s_grid_px_per_unit_uniform, g_px_per_unit);
    program_set_uniform_1f(&s_grid_program, s_grid_unit_boundary_uniform, s_grid_unit_boundary_value / g_unit);

    program_bind(&s_grid_program);

    GL(glBindVertexArray(s_grid_vao));
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));
    GL(glBindVertexArray(0));
}

void grid_set_line_width(float w) {
    s_grid_unit_boundary_value = w;
}

void grid_set_color(float r, float g, float b) {
    s_grid_color.x = r;
    s_grid_color.y = g;
    s_grid_color.z = b;
    program_set_uniform_3f(&s_grid_program, s_grid_color_uniform, s_grid_color);
}
