#ifndef SIM_CORE_H
#define SIM_CORE_H

#include "../graphics/graphics.h"

// Macros
#define NATIVE_CUSTOM   0
#define NATIVE_NOT      1
#define NATIVE_AND      2
#define NATIVE_NAND     3
#define NATIVE_OR       4
#define NATIVE_NOR      5
#define NATIVE_XOR      6
#define NATIVE_XNOR     7

#define NODE_INPUT  0
#define NODE_WIRE   1
#define NODE_OUTPUT 2

#define NODE_SIZE               0.8f
#define EDITOR_CHAR_WIDTH       0.6f
#define GATE_FRAME_SIZE         0.2f * NODE_SIZE
#define GATE_MAX_NAME_SIZE      15
#define CONNECTION_SIZE         0.3f * NODE_SIZE
#define CONTROL_PANEL_HEIGHT    6.0f

#define INFO_CHAR_HEIGHT        0.05f

void editor_init(const char *save_name);
void editor_render(uint32_t fps);

void editor_selection_set(float x0, float y0, float x1, float y1);
void editor_selection_delete();
void editor_selection_move(float x, float y);
void editor_selection_duplicate();
uint8_t editor_selection_check();

void editor_connection_set_input();
void editor_connection_set_output();
void editor_connection_make();

void editor_place_toggle();
void editor_wire_delay_toggle();

void editor_selected_up();
void editor_selected_down();

void editor_ppf_up(uint32_t dif);
void editor_ppf_down(uint32_t dif);
int32_t editor_ppf();

void editor_cp_up(uint32_t dif);
void editor_cp_down(uint32_t dif);

void editor_place(float x, float y);

void editor_propagate();
void editor_save(char *name);
void editor_get_custom(char *filename);

#endif
