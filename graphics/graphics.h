#ifndef GRAPHICS_H
#define GRAPHICS_H

#define GLEW_STATIC
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define GRAPHICS_DEBUG

// Standard
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Windows
#include <Windows.h>

// GL
#include "glad.h"
#include <glfw3.h>

// Altele
#include "./math.h"
#include "./files.h"
#include "./render.h"

// Windowing
extern float g_frame_time;
extern float g_unit;
extern float g_ratio;
extern float g_px_per_unit;

extern uint32_t g_window_width;
extern uint32_t g_window_height;

extern vec2 g_cursor;
extern vec2 g_cursor_relative;
extern vec2 g_offset;

extern mat2 g_transformation;

void unit_set(float unit);
void offset_set(float x, float y);

// Glfw
extern GLFWwindow *g_window;
extern GLFWmonitor *g_monitor;
extern GLFWvidmode *g_video_mode;

// Scene

    // Scena initiala
    void init(int32_t argc, char *argv[]);      // Definit de utilizator
    void event();                               // Definit de utilizator
    void update();                              // Definit de utilizator
    void cleanup();                             // Definit de utilizator

    // Alte scene
    void scene_set(void (*init_f)(int32_t argc, char *argv[]), void (*event_f)(), void (*update_f)(), void (*cleanup_f)());

// Erori
void GLClearErr();
void GLCheckErr(const char *f, const char* filename, int32_t line);
void debug_send(const char *module, const char *format, ...);

#ifdef ENGINE_DEBUG
    #define GL(func) \
            GLClearErr(); \
            func; \
            GLCheckErr(#func, __FILE__, __LINE__)
#else
    #define GL(func) func
#endif

#endif
