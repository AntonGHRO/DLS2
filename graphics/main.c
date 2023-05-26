#include "./graphics.h"

static char **s_argv;
static int32_t s_argc;

// Glfw
GLFWwindow *g_window;
GLFWmonitor *g_monitor;
GLFWvidmode *g_video_mode;

// Windowing
uint32_t g_window_width;
uint32_t g_window_height;

float g_frame_time;
float g_unit = 0.05f;
float g_ratio;
float g_px_per_unit;

vec2 g_cursor;
vec2 g_cursor_relative;
vec2 g_offset;

mat2 g_transformation;

// Scene
static struct {
    void (*init_f)();
    void (*event_f)();
    void (*update_f)();
    void (*cleanup_f)();
} current_scene;

void scene_set(void (*init_f)(int argc, char *argv[]), void (*event_f)(), void (*update_f)(), void (*cleanup_f)()) {
    current_scene.cleanup_f();

    current_scene.init_f = init_f;
    current_scene.event_f = event_f;
    current_scene.update_f = update_f;
    current_scene.cleanup_f = cleanup_f;

    current_scene.init_f(s_argc, s_argv);
}

// Unit and offset
void unit_set(float unit) {
    g_unit = unit;

    g_transformation.i.x = 2.0f * g_unit * g_ratio;
    g_transformation.j.y = 2.0f * g_unit;

    g_px_per_unit = g_unit * g_window_height;
}

void offset_set(float x, float y) {
    g_offset.x = x;
    g_offset.y = y;
}

// Error handling
void GLClearErr() { while(glGetError()); }

void GLCheckErr(const char *f, const char* filename, int line) {
    unsigned err = glGetError();
    while(err) fprintf(stderr, "[OpenGL]: %08x in file '%s' at line '%i'\nFunction: %s\n", err, filename, line, f);
}

void debug_send(const char *module, const char *format, ...) {
    #ifdef GRAPHICS_DEBUG
        va_list args; va_start(args, format);
        fprintf(stderr, "[%s]: ", module);
        vfprintf(stderr, format, args);
        fputc('\n', stderr);
        va_end(args);
    #endif
}

static void s_glfw_debug_callback(int32_t error, const char *description) {
    fprintf(stderr, "[GLFW_Callback][%i]: %s\n", error, description);
}

static void s_glfw_framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
    g_window_width = width;
    g_window_height = height;
    g_ratio = ((float) g_window_height) / ((float) g_window_width);
    fprintf(stderr, "[GLFW_Framebuffer]: Window resized to %i by %i\n", width, height);
}

// Main
int32_t main(int32_t argc, char **argv) {
    s_argc = argc;
    s_argv = argv;

    glfwSetErrorCallback(s_glfw_debug_callback);

    if(!glfwInit()) fprintf(stderr, "[GLFW]: Initializare esuata\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
    // glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

    g_monitor = glfwGetPrimaryMonitor();
    g_video_mode = glfwGetVideoMode(g_monitor);

    g_window_width = g_video_mode->width;
    g_window_height = g_video_mode->height;

    g_window = glfwCreateWindow(g_window_width, g_window_height, "Test", g_monitor, NULL);

    if(!g_window) fprintf(stderr, "[GLFW]: Initializare fereastra esuata\n");

    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, s_glfw_framebuffer_size_callback);
    glfwSwapInterval(1);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) fprintf(stderr, "[GLEW]: Initializare esuata\n");

    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glEnable(GL_BLEND));

    glfwSetWindowShouldClose(g_window, 0);

    GL(glViewport(0, 0, g_window_width, g_window_height));

    g_ratio = (float)g_window_height / (float)g_window_width;
    g_transformation.i.x = 2.0f * g_unit * g_ratio;
    g_transformation.i.y = 0.0f;
    g_transformation.j.x = 0.0f;
    g_transformation.j.y = 2.0f * g_unit;

    g_offset.x = 0.0f;
    g_offset.y = 0.0f;

    g_px_per_unit = g_unit * g_window_height;

    current_scene.init_f = init;
    current_scene.event_f = event;
    current_scene.update_f = update;
    current_scene.cleanup_f = cleanup;

    double cursor_x, cursor_y;
    double frame_time, current_time;

    glfwSetTime(0);
    current_scene.init_f(s_argc, s_argv);
    while(!glfwWindowShouldClose(g_window))
    {
        glfwGetCursorPos(g_window, &cursor_x, &cursor_y);
        g_cursor.x = (cursor_x - (float)g_window_width / 2) / g_px_per_unit;
        g_cursor.y = ((float)g_window_height / 2 - cursor_y) / g_px_per_unit;

        g_cursor_relative = vec2_add(g_cursor, g_offset);

        current_scene.event_f();

        current_scene.update_f();

        glfwSwapBuffers(g_window);
        glfwPollEvents();

        frame_time = glfwGetTime() - current_time;
        g_frame_time = frame_time;
        current_time = glfwGetTime();
    }

    current_scene.cleanup_f();

    glfwDestroyWindow(g_window);
    glfwTerminate();
    return 0;
}
