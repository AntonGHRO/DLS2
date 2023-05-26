#include "core.h"

static uint32_t fps = 0;
static uint32_t fps_frame_count = 0;
static uint32_t fps_frame_limit = 24;
static uint8_t  s_control                   = 0;
static uint8_t  s_shift                     = 0;
static vec2     s_cursor_relative_set_left  = {.0f, .0f};
static vec2     s_cursor_relative_set_right = {.0f, .0f};
static line     s_connection_line;
static line     s_selection_line_ox1;
static line     s_selection_line_ox2;
static line     s_selection_line_oy1;
static line     s_selection_line_oy2;
static quad     s_quad_screen;
static texture  s_texture_screen;
static int32_t  s_propagations_count = 0;
static int32_t  s_propagations_valid = 0;
static string   s_save_string;
static char     s_save_string_data[GATE_MAX_NAME_SIZE + 1];
static float    s_min_unit = .03f;
static float    s_max_unit = .2f;

static uint8_t s_saving_toggle = 0;
static uint8_t s_grid_toggle = 1;
static uint8_t s_drag_offset_toggle = 0;
static uint8_t s_selection_toggle = 0;
static uint8_t s_move_toggle = 0;
static uint8_t s_connection_toggle = 0;

static void s_key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {

    if(action == GLFW_PRESS) {
        if(s_saving_toggle) {
            if(key == GLFW_KEY_ENTER) {
                if(strlen(s_save_string_data) > 0) {
                    editor_save(s_save_string_data);
                    s_save_string_data[0] = '\0';
                    s_saving_toggle = 0;
                }
            }
            else if(key >= 'A' && key <= 'Z' || key >= '0' && key <= '9' || key == '-')
                strncat(s_save_string_data, &key, 1); // Append
            else if(key == GLFW_KEY_BACKSPACE)
                s_save_string_data[strlen(s_save_string_data) - 1] = '\0';
        }

        if(s_control && key == GLFW_KEY_G) {
            s_grid_toggle = !s_grid_toggle;
        }

        if(key == GLFW_KEY_BACKSPACE) {
            editor_selection_delete();
        }

        if(s_control) {
            if(key == GLFW_KEY_S) s_saving_toggle = 1;

            if(key == GLFW_KEY_D) editor_selection_duplicate();

            if(key == GLFW_KEY_SPACE) editor_place_toggle();

            if(key == GLFW_KEY_TAB) editor_wire_delay_toggle();

            if(key == GLFW_KEY_EQUAL) editor_ppf_up(1);
            else if(key == GLFW_KEY_MINUS) editor_ppf_down(1);
        }
    }

    if(action == GLFW_REPEAT) {
        if(key == GLFW_KEY_EQUAL) {
            editor_ppf_up(10);
        }
        else if(key == GLFW_KEY_MINUS) {
            editor_ppf_down(10);
        }
    }

}

static void s_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if(s_control) {
        if(!s_shift)
            if(yoffset >= .0f)  editor_selected_up();
            else                editor_selected_down();
        else if(yoffset >= .0f) editor_cp_up(1);
        else                    editor_cp_down(1);
    }
    else {
        unit_set(g_unit + 0.005f * (float)yoffset);
        if(g_unit <= s_min_unit) g_unit = s_min_unit;
        if(g_unit >= s_max_unit) g_unit = s_max_unit;
    }
}

static void s_mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
    
    if(action == GLFW_PRESS) {
        if(button == GLFW_MOUSE_BUTTON_RIGHT) {
            s_cursor_relative_set_right = g_cursor_relative;
            s_drag_offset_toggle = 1;
        }

        if(button == GLFW_MOUSE_BUTTON_LEFT) {
            s_cursor_relative_set_left = g_cursor_relative;

            if(s_shift) {
                s_selection_toggle = 1;
            }
            else if(s_control) {
                editor_place(floorf(g_cursor_relative.x + 0.5f), floorf(g_cursor_relative.y + 0.5f));
            }
            else {
                if(editor_selection_check()) {
                    s_move_toggle = 1;
                }
                else {
                    s_connection_toggle = 1;
                    line_set_position0(&s_connection_line, g_cursor_relative.x, g_cursor_relative.y);
                    editor_connection_set_input();
                }
            }
        }
    }
    else if(action == GLFW_RELEASE) {

        if(button == GLFW_MOUSE_BUTTON_RIGHT)   s_drag_offset_toggle = 0;

        if(button == GLFW_MOUSE_BUTTON_LEFT) {
            if(s_selection_toggle) {
                s_selection_toggle = 0;
                editor_selection_set(   s_selection_line_oy1.position_0.x, s_selection_line_oy1.position_0.y,
                                        s_selection_line_oy2.position_1.x, s_selection_line_oy2.position_1.y    );
            }
            else if(s_move_toggle) {
                s_move_toggle = 0;
                editor_selection_move(floorf(g_cursor_relative.x + 0.5f), floorf(g_cursor_relative.y + 0.5f));
            }
            else if(s_connection_toggle) {
                s_connection_toggle = 0;
                editor_connection_set_output();
                editor_connection_make();
            }
        }
    }
}

void init(int argc, char *argv[]) {
    glfwSetKeyCallback(g_window, s_key_callback);
    glfwSetScrollCallback(g_window, s_scroll_callback);
    glfwSetMouseButtonCallback(g_window, s_mouse_button_callback);

    glClearColor(.12f, .1f, .16f, 1.0f);

    text_init();

    quad_init(&s_quad_screen, .0f, .0f, .0f, .0f);
    texture_init(&s_texture_screen, "bmps/screen.bmp");
    quad_set_texture(&s_quad_screen, &s_texture_screen, 0);

    grid_init();
    grid_set_color(1.0f, 0.9f, 0.9f);

    line_init(&s_selection_line_ox1, .0f, .0f, 1.0f, 1.0f);
    line_init(&s_selection_line_ox2, .0f, .0f, 1.0f, 1.0f);
    line_init(&s_selection_line_oy1, .0f, .0f, 1.0f, 1.0f);
    line_init(&s_selection_line_oy2, .0f, .0f, 1.0f, 1.0f);

    line_set_color0(&s_selection_line_ox1, 1.0f, 1.0f, 1.0f);
    line_set_color0(&s_selection_line_ox2, 1.0f, 1.0f, 1.0f);
    line_set_color0(&s_selection_line_oy1, 1.0f, 1.0f, 1.0f);
    line_set_color0(&s_selection_line_oy2, 1.0f, 1.0f, 1.0f);

    line_set_color1(&s_selection_line_ox1, 1.0f, 1.0f, 1.0f);
    line_set_color1(&s_selection_line_ox2, 1.0f, 1.0f, 1.0f);
    line_set_color1(&s_selection_line_oy1, 1.0f, 1.0f, 1.0f);
    line_set_color1(&s_selection_line_oy2, 1.0f, 1.0f, 1.0f);

    line_init(&s_connection_line, .0f, .0f, 1.0f, 1.0f);
    line_set_color0(&s_connection_line, 1.0f, 1.0f, 1.0f);
    line_set_color1(&s_connection_line, 1.0f, 1.0f, 1.0f);

    string_init(&s_save_string, .0f, .0f, 2.0f);
    string_set_data(&s_save_string, s_save_string_data);

    if(argc > 1)    editor_init(argv[1]);
    else            editor_init("Default");
}

void event() {
    s_control = glfwGetKey(g_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    s_control |= glfwGetKey(g_window, GLFW_KEY_RIGHT_CONTROL)  == GLFW_PRESS;

    s_shift = glfwGetKey(g_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    s_shift |= glfwGetKey(g_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    glfwSetWindowShouldClose(g_window, s_control && glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS);
}

void update() {
    if(s_drag_offset_toggle) {
        g_offset.x = s_cursor_relative_set_right.x - g_cursor.x;
        g_offset.y = s_cursor_relative_set_right.y - g_cursor.y;
    }

    if(s_selection_toggle) {
        line_set_position0(&s_selection_line_ox1, s_cursor_relative_set_left.x, s_cursor_relative_set_left.y);
        line_set_position0(&s_selection_line_ox2, s_cursor_relative_set_left.x, g_cursor_relative.y);
        line_set_position1(&s_selection_line_ox1, g_cursor_relative.x, s_cursor_relative_set_left.y);
        line_set_position1(&s_selection_line_ox2, g_cursor_relative.x, g_cursor_relative.y);

        line_set_position0(&s_selection_line_oy1, s_cursor_relative_set_left.x, s_cursor_relative_set_left.y);
        line_set_position0(&s_selection_line_oy2, g_cursor_relative.x, s_cursor_relative_set_left.y);
        line_set_position1(&s_selection_line_oy1, s_cursor_relative_set_left.x, g_cursor_relative.y);
        line_set_position1(&s_selection_line_oy2, g_cursor_relative.x, g_cursor_relative.y);
    }

    if(s_move_toggle) editor_selection_move(g_cursor_relative.x, g_cursor_relative.y);

    s_propagations_valid = editor_ppf();

    if(s_propagations_valid < 0) {
            s_propagations_valid = -s_propagations_valid;
            if(s_propagations_count >= s_propagations_valid - 1) {
                editor_propagate();
                s_propagations_count = 0;
            }
            else s_propagations_count ++;
    }
    else if(s_propagations_valid > 0) {
        for(s_propagations_count = 0; s_propagations_count < s_propagations_valid; s_propagations_count ++)
            editor_propagate();
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if(s_grid_toggle) grid_render();

    if(s_connection_toggle) {
        line_set_position1(&s_connection_line, g_cursor_relative.x, g_cursor_relative.y);
        line_render(&s_connection_line);
    }

    if(fps_frame_count == fps_frame_limit){
        fps = (uint32_t) (1.0f / g_frame_time);
        fps_frame_count = 0;
    } else fps_frame_count ++;

    editor_render(fps);

    if(s_selection_toggle) {
        line_render(&s_selection_line_ox1);
        line_render(&s_selection_line_ox2);
        line_render(&s_selection_line_oy1);
        line_render(&s_selection_line_oy2);
    }

    if(s_saving_toggle) string_render(&s_save_string, 0);

    quad_render(&s_quad_screen);
}

void cleanup() {}
