#include "core.h"

// Macros
#define LISTNODE_NODE 0
#define LISTNODE_GATE 1

// Natives
static uint32_t s_native_not_delay  = 1;
static uint32_t s_native_and_delay  = 2;
static uint32_t s_native_nand_delay = 1;
static uint32_t s_native_or_delay   = 3;
static uint32_t s_native_nor_delay  = 1;
static uint32_t s_native_xor_delay  = 4;
static uint32_t s_native_xnor_delay = 5;

static uint8_t s_native_inputs[16];

static char     s_native_not_name[]     = "NOT";
static char     s_native_and_name[]     = "AND";
static char     s_native_nand_name[]    = "NAND";
static char     s_native_or_name[]      = "OR";
static char     s_native_nor_name[]     = "NOR";
static char     s_native_xor_name[]     = "XOR";
static char     s_native_xnor_name[]    = "XNOR";

static uint32_t native_delay(uint32_t native_gate) {
    switch(native_gate) {
        case NATIVE_NOT:    return s_native_not_delay;        
        case NATIVE_AND:    return s_native_and_delay;
        case NATIVE_NAND:   return s_native_nand_delay;
        case NATIVE_OR:     return s_native_or_delay;
        case NATIVE_NOR:    return s_native_nor_delay;
        case NATIVE_XOR:    return s_native_xor_delay;
        case NATIVE_XNOR:   return s_native_xnor_delay;
        default:            return 0;
    }
}

static void native_init() {
    for(uint32_t i = 0; i < 16; i ++) s_native_inputs[i] = 2;
}

static void native_load(uint8_t x) {
    uint32_t i = 0;
    while(s_native_inputs[i] != 2) i ++;
    s_native_inputs[i] = x;
}

static uint8_t native(uint32_t native_gate) {
    switch(native_gate) {
        case NATIVE_NOT:
            return !s_native_inputs[0];        
        case NATIVE_AND:
            return s_native_inputs[0] && s_native_inputs[1];
        case NATIVE_NAND:
            return !(s_native_inputs[0] && s_native_inputs[1]);
        case NATIVE_OR:
            return s_native_inputs[0] || s_native_inputs[1];
        case NATIVE_NOR:
            return !(s_native_inputs[0] || s_native_inputs[1]);
        case NATIVE_XOR:
            return (s_native_inputs[0] && !s_native_inputs[1]) || (!s_native_inputs[0] && s_native_inputs[1]);
        case NATIVE_XNOR:
            return !((s_native_inputs[0] && !s_native_inputs[1]) || (!s_native_inputs[0] && s_native_inputs[1]));
        default: return 0;
    }
}

// Structures
typedef struct exprnode_id exprnode;
typedef struct listnode_id listnode;

struct listnode_id{
    union {
        struct {
            listnode *input;
            listnode *gate;
            vec2 position;
            uint8_t value_1;
            uint8_t value_2;
            uint8_t updated;
            uint8_t type;
        } node;

        struct {
            listnode *input;
            listnode *output;
            listnode *impl;
            uint32_t type;
            uint32_t custom_index;
        } gate;
    } data;

    uint8_t type;

    listnode *prev;
    listnode *next;
};

// Editor Static Lists
static char         *s_save_name;

static listnode     *s_editor_list;

static struct {
    listnode *data;
    char **name;
    uint32_t count;
}                   s_custom_list;

static uint32_t     s_nodes_count;

static uint8_t      s_node_value_toggle;

listnode **s_selected_data = NULL;
uint32_t s_selected_count = 0;
uint32_t s_selected_capacity = 0;

// Render Statics
static quad     s_node_quad;
static texture  s_node_texture_input_off;
static texture  s_node_texture_input_on;
static texture  s_node_texture_wire_off;
static texture  s_node_texture_wire_on;
static texture  s_node_texture_output_off;
static texture  s_node_texture_output_on;

static quad     s_gate_quad;
static texture  s_gate_texture;
static string   s_gate_string;

static quad     s_connection_quad;
static texture  s_connection_texture_off;
static texture  s_connection_texture_on;

static quad     s_selection_quad;
static texture  s_selection_texture;

static vec2 ox = {1.0f, 0.0f};
static uint32_t s_control_panel_line_count = 14;

static struct {
    string  place;
    char    place_string[8];
    string  place_value;
    char    place_string_value[5][10];
    uint8_t place_selected;

    string  tool;
    char    tool_string[8];
    string  tool_value;
    char    tool_value_string[3][16];
    uint8_t tool_selected;

    string  node;
    char    node_string[8];
    string  node_value;
    char    node_value_string[3][8];
    uint8_t node_selected;

    string  native;
    char    native_string[8];
    string  native_value;
    char    native_value_string[16][16];
    uint8_t native_selected;

    string  custom_1;
    char    custom_1_string[16];
    string  custom_1_value;
    uint32_t custom_1_selected;

    string  custom_2;
    char    custom_2_string[16];
    string  custom_2_value;
    uint32_t custom_2_selected;

    string      tick;
    char        tick_string[8];
    string      tick_value;
    char        tick_value_string[32];
    uint32_t    tick_u32;

    string      propagations_per_frame;
    char        propagations_per_frame_string[32];
    string      propagations_per_frame_value;
    char        propagations_per_frame_value_string[32];
    int32_t     propagations_per_frame_i32;

    string      clock_period;
    char        clock_period_string[16];
    string      clock_period_value;
    char        clock_period_value_string[32];
    uint32_t    clock_period_u32;

    string      wire_delay;
    char        wire_delay_string[16];
    string      wire_delay_value;
    char        wire_delay_value_string[2][8];
    uint8_t     wire_delay_toggle;

} s_control_panel;

static struct {
    string      fps;
    char        fps_string[16];
    string      fps_value;
    char        fps_value_string[8];
    uint32_t    fps_u32;

    string  name;
    char    name_string[8];
    string  name_value;
    char    name_value_string[16];

} s_info_panel;

// Clock
static struct {
    quad        q;
    texture     t_off;
    texture     t_on;
    listnode    *input;
    uint32_t    propagation;
    uint8_t     toggle;
} s_clock;

// Disp
static struct {
    quad        q;
    texture     t;
    string      s1;
    string      s2;
    string      s3;
    string      s4;
    char        s_data1[64];
    char        s_data2[64];
    char        s_data3[64];
    char        s_data4[64];
    listnode    *nodes;
    uint8_t     active;
} s_disp;

// Natives
static uint32_t gate_input_count(listnode *this) {
    listnode *aux = this->data.gate.input;
    uint32_t count = 0;
    switch(this->data.gate.type) {
        case NATIVE_CUSTOM:
            while(aux != NULL) {
                count ++;
                aux = aux->next;
            } return count;
        case NATIVE_NOT: return 1;
        case NATIVE_AND: return 2;
        case NATIVE_NAND: return 2;
        case NATIVE_OR: return 2;
        case NATIVE_NOR: return 2;
        case NATIVE_XOR: return 2;
        case NATIVE_XNOR: return 2;
        default: return 0;
    }
}

static uint32_t gate_output_count(listnode *this) {
    listnode *aux = this->data.gate.output;
    uint32_t count = 0;
    switch(this->data.gate.type) {
        case NATIVE_CUSTOM:
            while(aux != NULL) {
                count ++;
                aux = aux->next;
            } return count;
        case NATIVE_NOT: return 1;
        case NATIVE_AND: return 1;
        case NATIVE_NAND: return 1;
        case NATIVE_OR: return 1;
        case NATIVE_NOR: return 1;
        case NATIVE_XOR: return 1;
        case NATIVE_XNOR: return 1;
        default: return 0;
    }
}

static char     *gate_name(listnode *this) {
    switch(this->data.gate.type) {
        case NATIVE_CUSTOM: return s_custom_list.name[this->data.gate.custom_index];
        case NATIVE_NOT: return s_native_not_name;
        case NATIVE_AND: return s_native_and_name;
        case NATIVE_NAND: return s_native_nand_name;
        case NATIVE_OR: return s_native_or_name;
        case NATIVE_NOR: return s_native_nor_name;
        case NATIVE_XOR: return s_native_xor_name;
        case NATIVE_XNOR: return s_native_xnor_name;
        default: return NULL;
    }
}

// ListNode Manipulation
static void listnode_move(listnode *this, float x, float y) {
    if(this->type == LISTNODE_NODE) {
        if(s_disp.nodes == this) {
            this->data.node.position.x = x;
            this->data.node.position.y = y;

            this = this->next;

            while(this != NULL) {
                listnode_move(this, this->prev->data.node.position.x + 1.0f, y);
                this = this->next;
            }
        }
        else {
            this->data.node.position.x = x;
            this->data.node.position.y = y;
        }
    }
    else if(this->type == LISTNODE_GATE) {
        listnode *aux = this->data.gate.input;

        float x_dif = x - this->data.gate.input->data.node.position.x;
        float y_dif = y - this->data.gate.input->data.node.position.y;

        while(aux != NULL) {
            aux->data.node.position.x += x_dif;
            aux->data.node.position.y += y_dif;
            aux = aux->next;
        }

        aux = this->data.gate.output;

        while(aux != NULL) {
            aux->data.node.position.x += x_dif;
            aux->data.node.position.y += y_dif;
            aux = aux->next;
        }
    }
}

static listnode *listnode_copy_custom(uint32_t custom_index);

// For node: float x, float y, uint8_t type
// For gate: float x, float y, uint32_t type, (only for custom, custom index)
static listnode *listnode_add(listnode *a, uint8_t type, ...) {

    listnode *new = malloc(sizeof(listnode));
    va_list valist;

    new->next = NULL;
    new->prev = NULL;
    new->type = type;

    va_start(valist, type);

    if(new->type == LISTNODE_NODE) {
        new->data.node.input = NULL;
        new->data.node.gate = NULL;
        new->data.node.value_1 = 0;
        new->data.node.value_2 = 0;
        new->data.node.updated = 1;

        new->data.node.position.x = (float) va_arg(valist, double);
        new->data.node.position.y = (float) va_arg(valist, double);
        new->data.node.type = (uint8_t) va_arg(valist, int32_t);

        s_nodes_count ++;
    }
    else if(new->type == LISTNODE_GATE){
        char *name = NULL;

        uint32_t input_count = 0;
        uint32_t output_count = 0;
        uint32_t custom_index = 0;
        uint32_t i;

        float x = .0f;
        float y = .0f;
        float width = .0f;

        x = (float) va_arg(valist, double);
        y = (float) va_arg(valist, double);
        new->data.gate.type = (uint32_t) va_arg(valist, int32_t);
        new->data.gate.input = NULL;
        new->data.gate.output = NULL;
        new->data.gate.impl = NULL;
        new->data.gate.custom_index = 0;

        if(new->data.gate.type == NATIVE_CUSTOM) {
            custom_index = (uint32_t) va_arg(valist, int32_t);

            new->data.gate.custom_index = custom_index;
            new = listnode_copy_custom(custom_index);

            listnode *aux = NULL;

            input_count = gate_input_count(new);
            output_count = gate_output_count(new);

            name = gate_name(new);
            width = floorf(2 * GATE_FRAME_SIZE + EDITOR_CHAR_WIDTH * strlen(name)) + 1.0f;

            aux = new->data.gate.input;
            while(aux->next != NULL) aux = aux->next;
            listnode_move(aux, x, y - ((float) (input_count - 1)));
            aux = aux->prev;

            while(aux != NULL) {
                aux->data.node.type = NODE_WIRE;
                listnode_move(aux, x, aux->next->data.node.position.y + 1.0f);

                aux = aux->prev;
            }

            aux = new->data.gate.output;
            while(aux->next != NULL) aux = aux->next;
            listnode_move(aux, x + width, y - ((float) (output_count - 1)));
            aux = aux->prev;

            while(aux != NULL) {
                aux->data.node.type = NODE_WIRE;
                listnode_move(aux, x + width, aux->next->data.node.position.y + 1.0f);

                aux = aux->prev;
            }
        }
        else {
            input_count = gate_input_count(new);
            output_count = gate_output_count(new);

            name = gate_name(new);
            width = floorf(2 * GATE_FRAME_SIZE + EDITOR_CHAR_WIDTH * strlen(name)) + 1.0f;

            new->data.gate.input = listnode_add(new->data.gate.input, LISTNODE_NODE, x, y - ((float) (input_count - 1)), NODE_WIRE);
            new->data.gate.input->data.node.gate = new;

            for(i = 1; i < input_count; i ++) {
                new->data.gate.input = listnode_add(new->data.gate.input, LISTNODE_NODE, x, new->data.gate.input->data.node.position.y + 1.0f, NODE_WIRE);
                new->data.gate.input->data.node.gate = new;
            }

            new->data.gate.output = listnode_add(new->data.gate.output, LISTNODE_NODE, x + width, y - ((float) (output_count - 1)), NODE_WIRE);
            new->data.gate.output->data.node.gate = new;

            for(i = 1; i < output_count; i ++) {
                new->data.gate.output = listnode_add(new->data.gate.output, LISTNODE_NODE, new->data.gate.output->data.node.position.x, new->data.gate.output->data.node.position.y + 1.0f, NODE_WIRE);
                new->data.gate.output->data.node.gate = new;
            }
        }
    }

    va_end(valist);

    // In case the 'a' list is empty, 'new' becomes the list
    if(a == NULL) return new;

    // Adding the listnode to the 'a' list
    a->prev = new;
    new->next = a;

    return new;
}

// Copies the inside of a custom gate
static listnode *listnode_copy_custom(uint32_t custom_index) {

    listnode *dest = malloc(sizeof(listnode));
    listnode *this = s_custom_list.data;

    listnode **source = NULL;
    listnode **copy = NULL;

    listnode *aux = NULL;
    listnode *aux2 = NULL;
    listnode *aux3 = NULL;

    uint32_t len = 0;
    uint32_t cap = 64;
    uint32_t i, j;

    for(i = 0; i < custom_index; i ++) this = this->next;

    dest->prev = NULL;
    dest->next = NULL;
    dest->type = LISTNODE_GATE;

    dest->data.gate.type = NATIVE_CUSTOM;
    dest->data.gate.custom_index = this->data.gate.custom_index;
    dest->data.gate.input = NULL;
    dest->data.gate.impl = NULL;
    dest->data.gate.output = NULL;

    source = calloc(cap, sizeof(listnode *));
    copy = calloc(cap, sizeof(listnode *));

    aux = this->data.gate.input;
    while(aux != NULL) {

        if(len == cap) {
            cap <<= 1;
            source = realloc(source, cap * sizeof(listnode *));
            copy = realloc(copy, cap * sizeof(listnode *));
        }

        dest->data.gate.input = listnode_add(dest->data.gate.input, LISTNODE_NODE, .0f, .0f, aux->data.node.type);
        dest->data.gate.input->data.node.gate = dest;

        source[len] = aux;
        copy[len] = dest->data.gate.input;
        len ++;

        aux = aux->next;
    }

    aux = this->data.gate.output;
    while(aux != NULL) {

        if(len == cap) {
            cap <<= 1;
            source = realloc(source, cap * sizeof(listnode *));
            copy = realloc(copy, cap * sizeof(listnode *));
        }

        dest->data.gate.output = listnode_add(dest->data.gate.output, LISTNODE_NODE, .0f, .0f, aux->data.node.type);
        dest->data.gate.output->data.node.gate = dest;

        source[len] = aux;
        copy[len] = dest->data.gate.output;
        len ++;

        aux = aux->next;
    }

    aux = this->data.gate.impl;
    while(aux != NULL) {
        if(aux->type == LISTNODE_NODE) {
            if(len == cap) {
                cap <<= 1;
                source = realloc(source, cap * sizeof(listnode *));
                copy = realloc(copy, cap * sizeof(listnode *));
            }

            dest->data.gate.impl = listnode_add(dest->data.gate.impl, LISTNODE_NODE, .0f, .0f, aux->data.node.type);

            source[len] = aux;
            copy[len] = dest->data.gate.impl;
            len ++;
        }
        else if(aux->type == LISTNODE_GATE) {
            dest->data.gate.impl = listnode_add(dest->data.gate.impl, LISTNODE_GATE, .0f, .0f, aux->data.gate.type);

            aux2 = aux->data.gate.input;
            aux3 = dest->data.gate.impl->data.gate.input;
            while(aux2 != NULL) {
                
                if(len == cap) {
                    cap <<= 1;
                    source = realloc(source, cap * sizeof(listnode *));
                    copy = realloc(copy, cap * sizeof(listnode *));
                }

                source[len] = aux2;
                copy[len] = aux3;
                len ++;

                aux2 = aux2->next;
                aux3 = aux3->next;
            }

            aux2 = aux->data.gate.output;
            aux3 = dest->data.gate.impl->data.gate.output;
            while(aux2 != NULL) {
                
                if(len == cap) {
                    cap <<= 1;
                    source = realloc(source, cap * sizeof(listnode *));
                    copy = realloc(copy, cap * sizeof(listnode *));
                }

                source[len] = aux2;
                copy[len] = aux3;
                len ++;

                aux2 = aux2->next;
                aux3 = aux3->next;
            }
        }

        aux = aux->next;
    }

    for(i = 0; i < len; i ++) {
        if(source[i]->data.node.input != NULL) {
            for(j = 0; j < len; j ++) {
                if(source[i]->data.node.input == source[j]) {
                    copy[i]->data.node.input = copy[j];
                }
            }
        }
    }

    free(source);
    free(copy);

    return dest;
}

// Only used by `listnode_delete`. Deletes all connections to 'this' from 's_editor_list' (all nodes, including gate nodes)
static void listnode_delete_input_connection(listnode *this) {
    listnode *current = NULL;
    listnode *aux = NULL;

    current = s_editor_list;

    while(current != NULL) {

        if(current->type == LISTNODE_NODE) {
            if(current->data.node.input == this) current->data.node.input = NULL;
        }
        else if(current->type == LISTNODE_GATE) {
            aux = current->data.gate.input;

            while(aux != NULL) {
                if(aux->data.node.input == this) aux->data.node.input = NULL;
                aux = aux->next;
            }

            aux = current->data.gate.output;

            while(aux != NULL) {
                if(aux->data.node.input == this) aux->data.node.input = NULL;
                aux = aux->next;
            }
        }

        current = current->next;
    }

    if(s_disp.active) {
        current = s_disp.nodes;

        while(current != NULL) {
            if(current->data.node.input == this) current->data.node.input = NULL;
            current = current->next;
        }
    }
}

static listnode *listnode_delete(listnode *a, listnode *this) {
    listnode *aux = NULL;

    listnode *prev = this->prev;
    listnode *next = this->next;

    // Removing clock if it was it's input
    if(s_clock.input == this) {
        s_clock.input = NULL;
        s_clock.toggle = 0;
        s_clock.propagation = 0;
    }

    // Removing the disp
    if(s_disp.nodes == a) {
        s_disp.active = 0;
    }

    if(next != NULL && prev != NULL) {          // In the middle
        next->prev = prev;
        prev->next = next;
    }
    else if(next == NULL && prev != NULL) {     // Most right
        prev->next = NULL;
    }
    else if(next != NULL && prev == NULL) {     // Most left
        a = next;
        a->prev = NULL;
    }
    else {                                      // Lonely listnode
        a = NULL;
    }

    if(this->type == LISTNODE_NODE) {
        listnode_delete_input_connection(this);
        s_nodes_count --;
    }
    else if(this->type == LISTNODE_GATE) {
        aux = this->data.gate.input;

        while(aux != NULL) {
            listnode_delete_input_connection(aux);
            aux = aux->next;
            if(aux != NULL) {
                free(aux->prev);
                aux->prev = NULL;   // Not really needed
            }
        }

        aux = this->data.gate.output;

        while(aux != NULL) {
            listnode_delete_input_connection(aux);
            aux = aux->next;
            if(aux != NULL) {
                free(aux->prev);
                aux->prev = NULL;   // Not really needed
            }
        }
    }

    free(this);

    return a;
}

static void listnode_render(listnode *this) {
    if(this->type == LISTNODE_NODE) {
        quad_set_position(&s_node_quad, this->data.node.position.x, this->data.node.position.y);

        if((s_node_value_toggle == 1 ? this->data.node.value_1 : this->data.node.value_2) == 0) {
            if(this->data.node.type == NODE_INPUT)
                quad_set_texture(&s_node_quad, &s_node_texture_input_off, 0);
            else if(this->data.node.type == NODE_WIRE)
                quad_set_texture(&s_node_quad, &s_node_texture_wire_off, 0);
            else if(this->data.node.type == NODE_OUTPUT)
                quad_set_texture(&s_node_quad, &s_node_texture_output_off, 0);
        }
        else {
            if(this->data.node.type == NODE_INPUT)
                quad_set_texture(&s_node_quad, &s_node_texture_input_on, 0);
            else if(this->data.node.type == NODE_WIRE)
                quad_set_texture(&s_node_quad, &s_node_texture_wire_on, 0);
            else if(this->data.node.type == NODE_OUTPUT)
                quad_set_texture(&s_node_quad, &s_node_texture_output_on, 0);
        }

        quad_render(&s_node_quad);
    }
    else if(this->type == LISTNODE_GATE) {
        listnode *input = this->data.gate.input;
        listnode *output = this->data.gate.output;

        char *name = gate_name(this);

        uint32_t input_count = gate_input_count(this);
        uint32_t output_count = gate_output_count(this);
        uint32_t max_node_count = max(input_count, output_count);

        float width = floorf(2 * GATE_FRAME_SIZE + EDITOR_CHAR_WIDTH * strlen(name)) + 1.0f;
        float height = 2 * GATE_FRAME_SIZE + (float) max_node_count;

        while(input->next != NULL) input = input->next;

        float rx = input->data.node.position.x + width / 2;
        float ry = input->data.node.position.y + .5f * (float) (max_node_count - 1);

        input = this->data.gate.input;

        quad_set_position(&s_gate_quad, rx, ry);
        quad_set_size(&s_gate_quad, width, height);

        string_set_position(&s_gate_string, rx, ry);
        string_set_data(&s_gate_string, name);

        quad_render(&s_gate_quad);
        string_render(&s_gate_string, 0);

        // Rendering the first input
        listnode_render(input);
        input = input->next;

        // Rendering the rest of the inputs (if there are any)
        while(input != NULL) {
            listnode_render(input);
            input = input->next;
        }

        // Rendering the outputs
        while(output != NULL) {
            listnode_render(output);
            output = output->next;
        }
    }
}

// Only for listnodes of type node
static void connection_render(listnode *current) {
    if(current->data.node.input == NULL) return;

    listnode *input = current->data.node.input;

    vec2 n_dif;
    vec2 n_sum;

    n_dif = vec2_add(current->data.node.position, vec2_scale(input->data.node.position, -1.0f));
    n_sum = vec2_add(current->data.node.position, input->data.node.position);

    quad_set_position(&s_connection_quad, .5f * n_sum.x, .5f * n_sum.y);
    quad_set_size(&s_connection_quad, vec2_len(n_dif), s_connection_quad.size.y);
    if(n_dif.x < .0f)   quad_set_angle(&s_connection_quad, M_PI + atanf(n_dif.y / n_dif.x));
    else                quad_set_angle(&s_connection_quad, atanf(n_dif.y / n_dif.x));
    if((s_node_value_toggle == 1 ? current->data.node.value_1 : current->data.node.value_2) == 0)
        quad_set_texture(&s_connection_quad, &s_connection_texture_off, 0);
    else
        quad_set_texture(&s_connection_quad, &s_connection_texture_on, 0);

    quad_render(&s_connection_quad);
}

// Rendering
static void control_panel_init() {
    vec2 string_position = { -1.64f, -0.41f };
    vec2 string_value_position = { +1.64f, -0.41f };

    float y_dif = 0.005f;

    // Place
    string_init(&s_control_panel.place, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.place_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(&s_control_panel.place_string, "Place");
    strcpy(s_control_panel.place_string_value[0], "TOOL");
    strcpy(s_control_panel.place_string_value[1], "NODE");
    strcpy(s_control_panel.place_string_value[2], "NATIVE");
    strcpy(s_control_panel.place_string_value[3], "CUSTOM_1");
    strcpy(s_control_panel.place_string_value[4], "CUSTOM_2");
    s_control_panel.place_selected = 1;
    string_set_data(&s_control_panel.place, s_control_panel.place_string);
    string_set_data(&s_control_panel.place_value, s_control_panel.place_string_value[s_control_panel.place_selected]);
    string_set_char_height(&s_control_panel.place, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.place_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Tool
    string_init(&s_control_panel.tool, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.tool_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.tool_string, "Tool");
    strcpy(s_control_panel.tool_value_string[0], "CLOCK");
    strcpy(s_control_panel.tool_value_string[1], "DISP");
    s_control_panel.tool_selected = 0;
    string_set_data(&s_control_panel.tool, s_control_panel.tool_string);
    string_set_data(&s_control_panel.tool_value, s_control_panel.tool_value_string[s_control_panel.tool_selected]);
    string_set_char_height(&s_control_panel.tool, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.tool_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Node
    string_init(&s_control_panel.node, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.node_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.node_string, "Node");
    strcpy(s_control_panel.node_value_string[0], "INPUT");
    strcpy(s_control_panel.node_value_string[1], "WIRE");
    strcpy(s_control_panel.node_value_string[2], "OUTPUT");
    s_control_panel.node_selected = 0;
    string_set_data(&s_control_panel.node, s_control_panel.node_string);
    string_set_data(&s_control_panel.node_value, s_control_panel.node_value_string[s_control_panel.node_selected]);
    string_set_char_height(&s_control_panel.node, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.node_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Native
    string_init(&s_control_panel.native, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.native_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.native_string, "Native");
    strcpy(s_control_panel.native_value_string[0], "NOT");
    strcpy(s_control_panel.native_value_string[1], "AND");
    strcpy(s_control_panel.native_value_string[2], "NAND");
    strcpy(s_control_panel.native_value_string[3], "OR");
    strcpy(s_control_panel.native_value_string[4], "NOR");
    strcpy(s_control_panel.native_value_string[5], "XOR");
    strcpy(s_control_panel.native_value_string[6], "XNOR");
    s_control_panel.native_selected = 0;
    string_set_data(&s_control_panel.native, s_control_panel.native_string);
    string_set_data(&s_control_panel.native_value, s_control_panel.native_value_string[s_control_panel.native_selected]);
    string_set_char_height(&s_control_panel.native, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.native_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Custom 1
    string_init(&s_control_panel.custom_1, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.custom_1_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.custom_1_string, "Custom 1");
    s_control_panel.custom_1_selected = 0;
    string_set_data(&s_control_panel.custom_1, s_control_panel.custom_1_string);
    string_set_char_height(&s_control_panel.custom_1, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.custom_1_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Custom 2
    string_init(&s_control_panel.custom_2, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.custom_2_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.custom_2_string, "Custom 2");
    s_control_panel.custom_2_selected = 0;
    string_set_data(&s_control_panel.custom_2, s_control_panel.custom_2_string);
    string_set_char_height(&s_control_panel.custom_2, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.custom_2_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Tick
    string_init(&s_control_panel.tick, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.tick_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.tick_string, "Tick");
    s_control_panel.tick_u32 = 0;
    sprintf(s_control_panel.tick_value_string, "%4.4u", s_control_panel.tick_u32);
    string_set_data(&s_control_panel.tick, s_control_panel.tick_string);
    string_set_data(&s_control_panel.tick_value, s_control_panel.tick_value_string);
    string_set_char_height(&s_control_panel.tick, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.tick_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Propagations per frame
    string_init(&s_control_panel.propagations_per_frame, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.propagations_per_frame_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.propagations_per_frame_string, "Propagations/Frame");
    s_control_panel.propagations_per_frame_i32 = 0;
    sprintf(s_control_panel.propagations_per_frame_value_string, "%4.4u", s_control_panel.propagations_per_frame_i32);
    string_set_data(&s_control_panel.propagations_per_frame, s_control_panel.propagations_per_frame_string);
    string_set_data(&s_control_panel.propagations_per_frame_value, s_control_panel.propagations_per_frame_value_string);
    string_set_char_height(&s_control_panel.propagations_per_frame, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.propagations_per_frame_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Clock period
    string_init(&s_control_panel.clock_period, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.clock_period_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.clock_period_string, "Clock Period");
    s_control_panel.clock_period_u32 = 1;
    sprintf(s_control_panel.clock_period_value_string, "%4.4u", s_control_panel.clock_period_u32);
    string_set_data(&s_control_panel.clock_period, s_control_panel.clock_period_string);
    string_set_data(&s_control_panel.clock_period_value, s_control_panel.clock_period_value_string);
    string_set_char_height(&s_control_panel.clock_period, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.clock_period_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Wire delay
    string_init(&s_control_panel.wire_delay, string_position.x, string_position.y, 1.0f);
    string_init(&s_control_panel.wire_delay_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_control_panel.wire_delay_string, "Wire delay");
    strcpy(s_control_panel.wire_delay_value_string[0], "OFF");
    strcpy(s_control_panel.wire_delay_value_string[1], "ON");
    s_control_panel.wire_delay_toggle = 1;
    string_set_data(&s_control_panel.wire_delay, s_control_panel.wire_delay_string);
    string_set_data(&s_control_panel.wire_delay_value, s_control_panel.wire_delay_value_string[s_control_panel.wire_delay_toggle]);
    string_set_char_height(&s_control_panel.wire_delay, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_control_panel.wire_delay_value, INFO_CHAR_HEIGHT);

    // Offset bypass
    string_set_global_offset_bypass(&s_control_panel.place, 1);
    string_set_global_offset_bypass(&s_control_panel.place_value, 1);
    string_set_global_offset_bypass(&s_control_panel.tool, 1);
    string_set_global_offset_bypass(&s_control_panel.tool_value, 1);
    string_set_global_offset_bypass(&s_control_panel.node, 1);
    string_set_global_offset_bypass(&s_control_panel.node_value, 1);
    string_set_global_offset_bypass(&s_control_panel.native, 1);
    string_set_global_offset_bypass(&s_control_panel.native_value, 1);
    string_set_global_offset_bypass(&s_control_panel.custom_1, 1);
    string_set_global_offset_bypass(&s_control_panel.custom_1_value, 1);
    string_set_global_offset_bypass(&s_control_panel.custom_2, 1);
    string_set_global_offset_bypass(&s_control_panel.custom_2_value, 1);

    string_set_global_offset_bypass(&s_control_panel.tick, 1);
    string_set_global_offset_bypass(&s_control_panel.tick_value, 1);
    string_set_global_offset_bypass(&s_control_panel.propagations_per_frame, 1);
    string_set_global_offset_bypass(&s_control_panel.propagations_per_frame_value, 1);
    string_set_global_offset_bypass(&s_control_panel.clock_period, 1);
    string_set_global_offset_bypass(&s_control_panel.clock_period_value, 1);
    string_set_global_offset_bypass(&s_control_panel.wire_delay, 1);
    string_set_global_offset_bypass(&s_control_panel.wire_delay_value, 1);

    // Info panel init
    string_position.x = -1.64f;
    string_position.y = 0.9f;
    string_value_position.x = +1.64f;
    string_value_position.y = string_position.y;

    // Fps
    string_init(&s_info_panel.fps, string_position.x, string_position.y, 1.0f);
    string_init(&s_info_panel.fps_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_info_panel.fps_string, "Frames/s");
    s_info_panel.fps_u32 = 0;
    sprintf(s_info_panel.fps_value_string, "%4.4u", s_info_panel.fps_u32);
    string_set_data(&s_info_panel.fps, s_info_panel.fps_string);
    string_set_data(&s_info_panel.fps_value, s_info_panel.fps_value_string);
    string_set_char_height(&s_info_panel.fps, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_info_panel.fps_value, INFO_CHAR_HEIGHT);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Name
    string_init(&s_info_panel.name, string_position.x, string_position.y, 1.0f);
    string_init(&s_info_panel.name_value, string_value_position.x, string_value_position.y, 1.0f);
    strcpy(s_info_panel.name_string, "Save");
    strcpy(s_info_panel.name_value_string, s_save_name);
    string_set_data(&s_info_panel.name, s_info_panel.name_string);
    string_set_data(&s_info_panel.name_value, s_info_panel.name_value_string);
    string_set_char_height(&s_info_panel.name, INFO_CHAR_HEIGHT);
    string_set_char_height(&s_info_panel.name_value, INFO_CHAR_HEIGHT);

    // Offset bypass
    string_set_global_offset_bypass(&s_info_panel.fps, 1);
    string_set_global_offset_bypass(&s_info_panel.fps_value, 1);
    string_set_global_offset_bypass(&s_info_panel.name, 1);
    string_set_global_offset_bypass(&s_info_panel.name_value, 1);
}

static void control_panel_render() {
    vec2 string_position = { -1.64f, -0.41f };
    vec2 string_value_position = { +1.64f, -0.41f };

    float y_dif = 0.005f;

    // Place
    string_set_position(&s_control_panel.place, string_position.x, string_position.y);
    string_set_position(&s_control_panel.place_value, string_value_position.x, string_value_position.y);
    string_set_data(&s_control_panel.place_value, s_control_panel.place_string_value[s_control_panel.place_selected]);
    string_render(&s_control_panel.place, 1);
    string_render(&s_control_panel.place_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Tool
    string_set_position(&s_control_panel.tool, string_position.x, string_position.y);
    string_set_position(&s_control_panel.tool_value, string_value_position.x, string_value_position.y);
    string_set_data(&s_control_panel.tool_value, s_control_panel.tool_value_string[s_control_panel.tool_selected]);
    string_render(&s_control_panel.tool, 1);
    string_render(&s_control_panel.tool_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Node
    string_set_position(&s_control_panel.node, string_position.x, string_position.y);
    string_set_position(&s_control_panel.node_value, string_value_position.x, string_value_position.y);
    string_set_data(&s_control_panel.node_value, s_control_panel.node_value_string[s_control_panel.node_selected]);
    string_render(&s_control_panel.node, 1);
    string_render(&s_control_panel.node_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Native
    string_set_position(&s_control_panel.native, string_position.x, string_position.y);
    string_set_position(&s_control_panel.native_value, string_value_position.x, string_value_position.y);
    string_set_data(&s_control_panel.native_value, s_control_panel.native_value_string[s_control_panel.native_selected]);
    string_render(&s_control_panel.native, 1);
    string_render(&s_control_panel.native_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Custom 1
    string_set_position(&s_control_panel.custom_1, string_position.x, string_position.y);
    string_set_position(&s_control_panel.custom_1_value, string_value_position.x, string_value_position.y);
    if(s_control_panel.custom_1_selected + 1 <= s_custom_list.count) {
        string_set_data(&s_control_panel.custom_1_value, s_custom_list.name[s_custom_list.count - s_control_panel.custom_1_selected - 1]);
        string_render(&s_control_panel.custom_1_value, 2);
    }
    string_render(&s_control_panel.custom_1, 1);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Custom 2
    string_set_position(&s_control_panel.custom_2, string_position.x, string_position.y);
    string_set_position(&s_control_panel.custom_2_value, string_value_position.x, string_value_position.y);
    if(s_control_panel.custom_2_selected + 1 <= s_custom_list.count) {
        string_set_data(&s_control_panel.custom_2_value, s_custom_list.name[s_custom_list.count - s_control_panel.custom_2_selected - 1]);
        string_render(&s_control_panel.custom_2_value, 2);
    }
    string_render(&s_control_panel.custom_2, 1);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Tick
    string_set_position(&s_control_panel.tick, string_position.x, string_position.y);
    string_set_position(&s_control_panel.tick_value, string_value_position.x, string_value_position.y);
    string_render(&s_control_panel.tick, 1);
    string_render(&s_control_panel.tick_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Propagations per frame
    string_set_position(&s_control_panel.propagations_per_frame, string_position.x, string_position.y);
    string_set_position(&s_control_panel.propagations_per_frame_value, string_value_position.x, string_value_position.y);
    string_render(&s_control_panel.propagations_per_frame, 1);
    string_render(&s_control_panel.propagations_per_frame_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Clock period
    string_set_position(&s_control_panel.clock_period, string_position.x, string_position.y);
    string_set_position(&s_control_panel.clock_period_value, string_value_position.x, string_value_position.y);
    string_render(&s_control_panel.clock_period, 1);
    string_render(&s_control_panel.clock_period_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Wire delay
    string_set_position(&s_control_panel.wire_delay, string_position.x, string_position.y);
    string_set_position(&s_control_panel.wire_delay_value, string_value_position.x, string_value_position.y);
    string_set_data(&s_control_panel.wire_delay_value, s_control_panel.wire_delay_value_string[s_control_panel.wire_delay_toggle]);
    string_render(&s_control_panel.wire_delay, 1);
    string_render(&s_control_panel.wire_delay_value, 2);

    // Info panel
    string_position.x = -1.64f;
    string_position.y = 0.9f;
    string_value_position.x = +1.64f;
    string_value_position.y = string_position.y;

    // Fps
    string_set_position(&s_info_panel.fps, string_position.x, string_position.y);
    string_set_position(&s_info_panel.fps_value, string_value_position.x, string_value_position.y);
    string_render(&s_info_panel.fps, 1);
    string_render(&s_info_panel.fps_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;

    // Name
    string_set_position(&s_info_panel.name, string_position.x, string_position.y);
    string_set_position(&s_info_panel.name_value, string_value_position.x, string_value_position.y);
    string_render(&s_info_panel.name, 1);
    sprintf(s_info_panel.fps_value_string, "%4.4u", s_info_panel.fps_u32);
    string_render(&s_info_panel.name_value, 2);
    string_position.y -= INFO_CHAR_HEIGHT + y_dif;
    string_value_position.y -= INFO_CHAR_HEIGHT + y_dif;
}

void editor_init(const char *save_name) {
    s_editor_list = NULL;
    s_custom_list.data = NULL;
    s_custom_list.count = 0;
    s_custom_list.name = NULL;
    s_nodes_count = 0;
    s_node_value_toggle = 1;

    s_save_name = calloc(closest_pow2(strlen(save_name) + 1), sizeof(char));
    strcpy(s_save_name, save_name);

    // Interface inits
    text_init();

    texture_init(&s_node_texture_input_off, "bmps/node_input_off.bmp");
    texture_init(&s_node_texture_input_on, "bmps/node_input_on.bmp");
    texture_init(&s_node_texture_wire_off, "bmps/node_wire_off.bmp");
    texture_init(&s_node_texture_wire_on, "bmps/node_wire_on.bmp");
    texture_init(&s_node_texture_output_off, "bmps/node_output_off.bmp");
    texture_init(&s_node_texture_output_on, "bmps/node_output_on.bmp");

    texture_init(&s_gate_texture, "bmps/gate.bmp");

    texture_init(&s_connection_texture_off, "bmps/connection_off.bmp");
    texture_init(&s_connection_texture_on, "bmps/connection_on.bmp");

    texture_init(&s_selection_texture, "bmps/selection.bmp");

    quad_init(&s_node_quad, 0.0f, 0.0f, NODE_SIZE, NODE_SIZE);
    quad_init(&s_gate_quad, 0.0f, 0.0f, 1.0f, 1.0f);
    quad_init(&s_connection_quad, 0.0f, 0.0f, 1.0f, CONNECTION_SIZE);
    quad_init(&s_selection_quad, .0f, .0f, NODE_SIZE, NODE_SIZE);

    quad_set_texture(&s_gate_quad, &s_gate_texture, 0);
    quad_set_texture(&s_selection_quad, &s_selection_texture, 0);

    string_init(&s_gate_string, 0.0f, 0.0f, EDITOR_CHAR_WIDTH);

    // Control panel init
    control_panel_init();

    // Clock init
    quad_init(&s_clock.q, .0f, .0f, 3 * NODE_SIZE, 3 * NODE_SIZE);
    texture_init(&s_clock.t_off, "bmps/clock_off.bmp");
    texture_init(&s_clock.t_on, "bmps/clock_on.bmp");

    s_clock.input = NULL;
    s_clock.toggle = 0;
    s_clock.propagation = 0;

    // Disp init
    quad_init(&s_disp.q, .0f, .0f, 2 * GATE_FRAME_SIZE + 16.0f, 20 * GATE_FRAME_SIZE);
    texture_init(&s_disp.t, "bmps/disp.bmp");
    quad_set_texture(&s_disp.q, &s_disp.t, 0);
    string_init(&s_disp.s1, .0f, .0f, EDITOR_CHAR_WIDTH);
    string_init(&s_disp.s2, .0f, .0f, EDITOR_CHAR_WIDTH);
    string_init(&s_disp.s3, .0f, .0f, EDITOR_CHAR_WIDTH);
    string_init(&s_disp.s4, .0f, .0f, EDITOR_CHAR_WIDTH);
    string_set_data(&s_disp.s1, s_disp.s_data1);
    string_set_data(&s_disp.s2, s_disp.s_data2);
    string_set_data(&s_disp.s3, s_disp.s_data3);
    string_set_data(&s_disp.s4, s_disp.s_data4);

    s_disp.active = 0;

    for(uint32_t i = 0; i < 16; i ++) {
        s_disp.nodes = listnode_add(s_disp.nodes, LISTNODE_NODE, .0f, .0f, NODE_WIRE);
    }
}

void editor_render(uint32_t fps) {
    listnode *current = s_editor_list;
    listnode *aux = NULL;

    // Connections
    while(current != NULL) {

        if(current->type == LISTNODE_NODE) connection_render(current);
        else if(current->type == LISTNODE_GATE) {
            aux = current;

            current = aux->data.gate.input;

            while(current != NULL) {
                connection_render(current);
                current = current->next;
            }

            current = aux->data.gate.output;

            while(current != NULL) {
                // connection_render(current);
                current = current->next;
            }

            current = aux;
        }

        current = current->next;
    }

    // Clock
    if(s_clock.input != NULL) {
        if(s_clock.toggle) quad_set_texture(&s_clock.q, &s_clock.t_on, 0);
        else quad_set_texture(&s_clock.q, &s_clock.t_off, 0);

        quad_set_position(&s_clock.q, s_clock.input->data.node.position.x - 2 * NODE_SIZE, s_clock.input->data.node.position.y);

        quad_render(&s_clock.q);
    }

    // Disp
    if(s_disp.active) {
        quad_set_position(&s_disp.q, s_disp.nodes->data.node.position.x + 7.5f, s_disp.nodes->data.node.position.y + 10 * GATE_FRAME_SIZE);
        quad_render(&s_disp.q);

        uint32_t value = 0;
        int32_t value_neg[4];
        uint8_t msb[4];
        uint32_t index = 0;

        current = s_disp.nodes;

        while(current != NULL) {
            if(current->data.node.input != NULL) {
                if(s_node_value_toggle == 1) {
                    current->data.node.value_1 = current->data.node.input->data.node.value_1;
                    current->data.node.value_2 = current->data.node.input->data.node.value_1;
                }
                else if(s_node_value_toggle == 2) {
                    current->data.node.value_1 = current->data.node.input->data.node.value_2;
                    current->data.node.value_2 = current->data.node.input->data.node.value_2;
                }
            }

            connection_render(current);
            listnode_render(current);

            switch(index) {
                case 0:     msb[0] = current->data.node.value_1; break;
                case 4:     msb[1] = current->data.node.value_1; break;
                case 8:     msb[2] = current->data.node.value_1; break;
                case 12:    msb[3] = current->data.node.value_1; break;
                default: break;
            }

            if(index % 4 == 0) {
                switch(index) {
                    case 4: value >>= 1; value_neg[0] = value - 16; itoa(value, s_disp.s_data1, 10); break;
                    case 8: value >>= 1; value_neg[1] = value - 16; itoa(value, s_disp.s_data2, 10); break;
                    case 12: value >>= 1; value_neg[2] = value - 16; itoa(value, s_disp.s_data3, 10); break;
                    default: break;
                }
                value = 0;
            }

            value |= current->data.node.value_1;
            value <<= 1;
            index ++;

            current = current->next;
        }

        value >>= 1;
        value_neg[3] = value - 16;
        itoa(value, s_disp.s_data4, 10);

        if(msb[0] == 1) {
            string_set_position(&s_disp.s1, s_disp.nodes->data.node.position.x + 1.5f, s_disp.nodes->data.node.position.y + 15.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s1, 0);

            string_set_position(&s_disp.s1, s_disp.nodes->data.node.position.x + 1.5f, s_disp.nodes->data.node.position.y + 5 * GATE_FRAME_SIZE);
            itoa(value_neg[0], s_disp.s_data1, 10);
            string_render(&s_disp.s1, 0);
        }
        else {
            string_set_position(&s_disp.s1, s_disp.nodes->data.node.position.x + 1.5f, s_disp.nodes->data.node.position.y + 10.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s1, 0);
        }

        if(msb[1] == 1) {
            string_set_position(&s_disp.s2, s_disp.nodes->data.node.position.x + 5.5f, s_disp.nodes->data.node.position.y + 15.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s2, 0);

            string_set_position(&s_disp.s2, s_disp.nodes->data.node.position.x + 5.5f, s_disp.nodes->data.node.position.y + 5 * GATE_FRAME_SIZE);
            itoa(value_neg[1], s_disp.s_data2, 10);
            string_render(&s_disp.s2, 0);
        }
        else {
            string_set_position(&s_disp.s2, s_disp.nodes->data.node.position.x + 5.5f, s_disp.nodes->data.node.position.y + 10.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s2, 0);
        }

        if(msb[2] == 1) {
            string_set_position(&s_disp.s3, s_disp.nodes->data.node.position.x + 9.5f, s_disp.nodes->data.node.position.y + 15.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s3, 0);

            string_set_position(&s_disp.s3, s_disp.nodes->data.node.position.x + 9.5f, s_disp.nodes->data.node.position.y + 5 * GATE_FRAME_SIZE);
            itoa(value_neg[2], s_disp.s_data3, 10);
            string_render(&s_disp.s3, 0);
        }
        else {
            string_set_position(&s_disp.s3, s_disp.nodes->data.node.position.x + 9.5f, s_disp.nodes->data.node.position.y + 10.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s3, 0);
        }

        if(msb[3] == 1) {
            string_set_position(&s_disp.s4, s_disp.nodes->data.node.position.x + 13.5f, s_disp.nodes->data.node.position.y + 15.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s4, 0);

            string_set_position(&s_disp.s4, s_disp.nodes->data.node.position.x + 13.5f, s_disp.nodes->data.node.position.y + 5 * GATE_FRAME_SIZE);
            itoa(value_neg[3], s_disp.s_data4, 10);
            string_render(&s_disp.s4, 0);
        }
        else {
            string_set_position(&s_disp.s4, s_disp.nodes->data.node.position.x + 13.5f, s_disp.nodes->data.node.position.y + 10.0f * GATE_FRAME_SIZE);
            string_render(&s_disp.s4, 0);
        }
    }

    // Gates & Nodes & Selections
    current = s_editor_list;

    while(current != NULL) {
        listnode_render(current);
        current = current->next;
    }

    // Selections
    for(uint32_t i = 0; i < s_selected_count; i ++) {
        if(s_selected_data[i]->type == LISTNODE_NODE)
            quad_set_position(&s_selection_quad, s_selected_data[i]->data.node.position.x, s_selected_data[i]->data.node.position.y);
        else if(s_selected_data[i]->type == LISTNODE_GATE)
            quad_set_position(&s_selection_quad, s_selected_data[i]->data.gate.input->data.node.position.x, s_selected_data[i]->data.gate.input->data.node.position.y);
        quad_render(&s_selection_quad);
    }

    // Control Panel (Renders last)
    s_info_panel.fps_u32 = fps;
    control_panel_render();
}

// Interface
void editor_place_toggle() {
    s_control_panel.place_selected = s_control_panel.place_selected < 4 ? s_control_panel.place_selected + 1 : 0;
}

void editor_wire_delay_toggle() {
    s_control_panel.wire_delay_toggle = !s_control_panel.wire_delay_toggle;
}

void editor_selected_up() {
    switch(s_control_panel.place_selected) {
        case 0:     // Tool
            s_control_panel.tool_selected = s_control_panel.tool_selected < 1 ? s_control_panel.tool_selected + 1 : 0;
            break;
        
        case 1:     // Node
            s_control_panel.node_selected = s_control_panel.node_selected < 2 ? s_control_panel.node_selected + 1 : 0;
            break;
        
        case 2:     // Native
            s_control_panel.native_selected = s_control_panel.native_selected < 6 ? s_control_panel.native_selected + 1 : 0;
            break;

        case 3:     // Custom 1
            s_control_panel.custom_1_selected = s_control_panel.custom_1_selected < s_custom_list.count - 1 ? s_control_panel.custom_1_selected + 1 : 0;
            break;
        
        case 4:     // Custom 2
            s_control_panel.custom_2_selected = s_control_panel.custom_2_selected < s_custom_list.count - 1 ? s_control_panel.custom_2_selected + 1 : 0;
            break;

        default: break;
    }
}

void editor_selected_down() {
    switch(s_control_panel.place_selected) {
        case 0:     // Tool
            s_control_panel.tool_selected = s_control_panel.tool_selected > 0 ? s_control_panel.tool_selected - 1 : 1;
            break;
        
        case 1:     // Node
            s_control_panel.node_selected = s_control_panel.node_selected > 0 ? s_control_panel.node_selected - 1 : 2;
            break;
        
        case 2:     // Native
            s_control_panel.native_selected = s_control_panel.native_selected > 0 ? s_control_panel.native_selected - 1 : 6;
            break;

        case 3:     // Custom 1
            s_control_panel.custom_1_selected = s_control_panel.custom_1_selected > 0 ? s_control_panel.custom_1_selected - 1 : s_custom_list.count - 1;
            break;
        
        case 4:     // Custom 2
            s_control_panel.custom_2_selected = s_control_panel.custom_2_selected > 0 ? s_control_panel.custom_2_selected - 1 : s_custom_list.count - 1;
            break;
        
        default: break;
    }
}

void editor_ppf_up(uint32_t dif) {
    s_control_panel.propagations_per_frame_i32 += (int32_t) dif;
    if(s_control_panel.propagations_per_frame_i32 < 0)
        sprintf(s_control_panel.propagations_per_frame_value_string, "1/%.3u", (uint32_t) (-s_control_panel.propagations_per_frame_i32));
    else
        sprintf(s_control_panel.propagations_per_frame_value_string, "%.4u", s_control_panel.propagations_per_frame_i32);
}

void editor_ppf_down(uint32_t dif) {
    s_control_panel.propagations_per_frame_i32 -= (int32_t) dif;
    if(s_control_panel.propagations_per_frame_i32 < 0)
        sprintf(s_control_panel.propagations_per_frame_value_string, "1/%.3u", (uint32_t) (-s_control_panel.propagations_per_frame_i32));
    else
        sprintf(s_control_panel.propagations_per_frame_value_string, "%.4u", s_control_panel.propagations_per_frame_i32);
}

int32_t editor_ppf() {
    return s_control_panel.propagations_per_frame_i32;
}

void editor_cp_up(uint32_t dif) {
    s_control_panel.clock_period_u32 += dif;
    sprintf(s_control_panel.clock_period_value_string, "%4.4u", s_control_panel.clock_period_u32);
}

void editor_cp_down(uint32_t dif) {
    s_control_panel.clock_period_u32 = s_control_panel.clock_period_u32 > dif ? s_control_panel.clock_period_u32 - dif : 0;
    sprintf(s_control_panel.clock_period_value_string, "%4.4u", s_control_panel.clock_period_u32);
}

void editor_place(float x, float y) {
    switch(s_control_panel.place_selected) {
        case 0:     // Tool
            // Clock
            if(s_control_panel.tool_selected == 0) {
                s_editor_list = listnode_add(s_editor_list, LISTNODE_NODE, x, y, NODE_INPUT);
                s_clock.input = s_editor_list;
            }
            // Disp
            else if(s_control_panel.tool_selected == 1) {
                listnode *aux = s_disp.nodes;

                while(aux != NULL) {
                    listnode_move(aux, x, y);
                    x += 1.0f;
                    aux = aux->next;
                }

                s_disp.active = 1;
            }
            break;

        case 1:     // Node
            s_editor_list = listnode_add(s_editor_list, LISTNODE_NODE, x, y, s_control_panel.node_selected);
            break;
        
        case 2:     // Native
            s_editor_list = listnode_add(s_editor_list, LISTNODE_GATE, x, y, NATIVE_NOT + s_control_panel.native_selected);
            break;
        
        case 3:     // Custom 1
            if(s_control_panel.custom_1_selected + 1 <= s_custom_list.count)
                s_editor_list = listnode_add(s_editor_list, LISTNODE_GATE, x, y, NATIVE_CUSTOM, s_control_panel.custom_1_selected);
            break;
        
        case 4:     // Custom 2
            if(s_control_panel.custom_2_selected + 1 <= s_custom_list.count)
                s_editor_list = listnode_add(s_editor_list, LISTNODE_GATE, x, y, NATIVE_CUSTOM, s_control_panel.custom_2_selected);
            break;

        default: break;
    }
}

// Connection Statics
static listnode *connection_input = NULL;
static listnode *connection_output = NULL;

void editor_connection_set_input() {
    listnode *current = s_editor_list;
    listnode *aux = NULL;

    connection_input = NULL;

    while(current != NULL) {

        if(current->type == LISTNODE_NODE) {
            if( vec2_len(vec2_add(current->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                connection_input = current;
        }
        else if(current->type == LISTNODE_GATE) {
            aux = current->data.gate.input;

            while(aux != NULL) {
                if( vec2_len(vec2_add(aux->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                    connection_input = aux;

                aux = aux->next;
            }

            aux = current->data.gate.output;

            while(aux != NULL) {
                if( vec2_len(vec2_add(aux->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                    connection_input = aux;

                aux = aux->next;
            }
        }

        if(connection_input != NULL) break;

        current = current->next;
    }

    if(s_disp.active) {
        current = s_disp.nodes;

        while(current != NULL) {

            if( vec2_len(vec2_add(current->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                connection_input = current;

            if(connection_input != NULL) break;

            current = current->next;
        }
    }
}

void editor_connection_set_output() {
    listnode *current = s_editor_list;
    listnode *aux = NULL;

    connection_output = NULL;

    while(current != NULL) {

        if(current->type == LISTNODE_NODE) {
            if( vec2_len(vec2_add(current->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                connection_output = current;
        }
        else if(current->type == LISTNODE_GATE) {
            aux = current->data.gate.input;

            while(aux != NULL) {
                if( vec2_len(vec2_add(aux->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                    connection_output = aux;

                aux = aux->next;
            }

            aux = current->data.gate.output;

            while(aux != NULL) {
                if( vec2_len(vec2_add(aux->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                    connection_output = aux;

                aux = aux->next;
            }
        }

        if(connection_output != NULL) break;

        current = current->next;
    }

    if(s_disp.active) {
        current = s_disp.nodes;

        while(current != NULL) {

            if( vec2_len(vec2_add(current->data.node.position, vec2_scale(g_cursor_relative, -1.0f))) <= (NODE_SIZE / 2) )
                connection_output = current;

            if(connection_output != NULL) break;

            current = current->next;
        }
    }
}

void editor_connection_make() {
    if(connection_input != NULL && connection_output != NULL) {

        // Toggle node's value
        if(connection_input == connection_output) {

            // Toggle clock
            if(s_clock.input == connection_input) {
                s_clock.toggle = !s_clock.toggle;
                if(s_clock.toggle == 0) {
                    s_clock.input->data.node.value_1 = 0;
                    s_clock.input->data.node.value_2 = 0;
                }
                s_clock.propagation = 0;
            }
            else {
                // Toggle input
                if(connection_input->data.node.type == NODE_INPUT)
                    connection_input->data.node.value_1 = !connection_input->data.node.value_1;
                    connection_input->data.node.value_2 = connection_input->data.node.value_1;
            }
        }
        // Delete connection
        else if(connection_input->data.node.input == connection_output) {
                // Node starts floating :))
                connection_input->data.node.value_1 = 0;
                connection_input->data.node.value_2 = 0;
                // Obv. remove the input
                connection_input->data.node.input = NULL;
            }
        // Make the connection :))
        else if(connection_input->data.node.type != NODE_OUTPUT && connection_output->data.node.type != NODE_INPUT) {
            connection_output->data.node.input = connection_input;
        }
    }
}

// Selection
void editor_selection_set(float x0, float y0, float x1, float y1) {
    listnode *current;
    listnode *aux;

    float max_x = fmaxf(x0, x1);
    float max_y = fmaxf(y0, y1);
    float min_x = fminf(x0, x1);
    float min_y = fminf(y0, y1);

    s_selected_count = 0;

    if(s_selected_capacity == 0) {
        s_selected_capacity = 64;
        s_selected_data = calloc(s_selected_capacity, sizeof(listnode *));
    }

    current = s_editor_list;

    while(current != NULL) {

        if(current->type == LISTNODE_NODE) aux = current;
        else if(current->type == LISTNODE_GATE) aux = current->data.gate.input;

        if( aux->data.node.position.x >= min_x && aux->data.node.position.x <= max_x &&
            aux->data.node.position.y >= min_y && aux->data.node.position.y <= max_y )
                    s_selected_data[s_selected_count ++] = current;

        if(s_selected_count == s_selected_capacity) {
            listnode **temp = NULL;

            s_selected_capacity <<= 1;

            temp = realloc(s_selected_data, s_selected_capacity * sizeof(listnode *));
            if(temp == NULL) {
                debug_send("editor_selection_set", "Failed to reallocate selection list");
                s_selected_capacity >>= 1;
                return;
            }

            s_selected_data = temp;
        }

        current = current->next;
    }

    if(s_disp.active) {
        aux = s_disp.nodes;

        if( aux->data.node.position.x >= min_x && aux->data.node.position.x <= max_x &&
            aux->data.node.position.y >= min_y && aux->data.node.position.y <= max_y )
                    s_selected_data[s_selected_count ++] = aux;

        if(s_selected_count == s_selected_capacity) {
            listnode **temp = NULL;

            s_selected_capacity <<= 1;

            temp = realloc(s_selected_data, s_selected_capacity * sizeof(listnode *));
            if(temp == NULL) {
                debug_send("editor_selection_set", "Failed to reallocate selection list");
                s_selected_capacity >>= 1;
                return;
            }

            s_selected_data = temp;
        }
    }
}

void editor_selection_delete() {
    listnode *custom = NULL;
    listnode *aux = NULL;
    listnode *aux2 = NULL;
    listnode *next = NULL;
    listnode *next2 = NULL;

    for(uint32_t i = 0; i < s_selected_count; i ++) {

        if(s_selected_data[i]->data.node.gate == NULL) {                                // Simple node
            if(s_disp.nodes == s_selected_data[i]) s_disp.active = 0;
            else s_editor_list = listnode_delete(s_editor_list, s_selected_data[i]);
        }
        else if(s_selected_data[i]->data.node.gate->data.gate.type == NATIVE_CUSTOM) {  // Custom first input
            custom = s_selected_data[i]->data.node.gate;

            aux = custom->data.gate.output;
            while(aux != NULL) {
                next = aux->next;
                listnode_delete_input_connection(aux);
                free(aux);
                aux = next;
            }

            aux = custom->data.gate.impl;
            while(aux != NULL) {
                next = aux->next;
                if(aux->type == LISTNODE_GATE) {
                    aux2 = aux->data.gate.input;
                    while(aux2 != NULL) {
                        next2 = aux2->next;
                        free(aux2);
                        aux2 = next2;
                    }

                    aux2 = aux->data.gate.output;
                    while(aux2 != NULL) {
                        next2 = aux2->next;
                        free(aux2);
                        aux2 = next2;
                    }
                }
                free(aux);
                aux = next;
            }

            aux = custom->data.gate.input;
            while(aux != NULL) {
                next = aux->next;
                free(aux);
                aux = next;
            }
        }
        else {                                                                          // Native first input
            s_editor_list = listnode_delete(s_editor_list, s_selected_data[i]);
        }
    }

    s_selected_count = 0;
}

void editor_selection_move(float x, float y) {
    listnode *aux = NULL;

    float x_last = .0f;
    float y_last = .0f;
    
    float x_dif = .0f;
    float y_dif = .0f;

    if(s_selected_count != 0) {
        if(s_selected_data[0]->type == LISTNODE_NODE) aux = s_selected_data[0];
        else if(s_selected_data[0]->type == LISTNODE_GATE) aux = s_selected_data[0]->data.gate.input;

        x_last = aux->data.node.position.x;
        y_last = aux->data.node.position.y;

        listnode_move(s_selected_data[0], x, y);
    }

    for(uint32_t i = 1; i < s_selected_count; i ++) {
        if(s_selected_data[i]->type == LISTNODE_NODE) aux = s_selected_data[i];
        else if(s_selected_data[i]->type == LISTNODE_GATE) aux = s_selected_data[i]->data.gate.input;

        x_dif = aux->data.node.position.x - x_last;
        y_dif = aux->data.node.position.y - y_last;

        x_last = aux->data.node.position.x;
        y_last = aux->data.node.position.y;

        if(s_selected_data[i-1]->type == LISTNODE_NODE)
            listnode_move(
                s_selected_data[i],
                s_selected_data[i-1]->data.node.position.x + x_dif,
                s_selected_data[i-1]->data.node.position.y + y_dif
            );
        else if(s_selected_data[i-1]->type == LISTNODE_GATE)
            listnode_move(
                s_selected_data[i],
                s_selected_data[i-1]->data.gate.input->data.node.position.x + x_dif,
                s_selected_data[i-1]->data.node.input->data.node.position.y + y_dif
            );
    }
}

void editor_selection_duplicate() {
    listnode **source = NULL;
    listnode **copy = NULL;
    listnode **temp = NULL;
    listnode *aux1 = NULL;
    listnode *aux2 = NULL;
    uint32_t len = 0;
    uint32_t cap = 64;
    uint32_t i, j;

    source = calloc(cap, sizeof(listnode *));
    copy = calloc(cap, sizeof(listnode *));

    for(i = 0; i < s_selected_count; i ++) {

        if(len == cap) {
            cap <<= 1;
            source = realloc(source, cap * sizeof(listnode *));
            copy = realloc(copy, cap * sizeof(listnode *));
        }

        if(s_selected_data[i]->type == LISTNODE_NODE) {
            s_editor_list = listnode_add(
                                            s_editor_list, LISTNODE_NODE,
                                            s_selected_data[i]->data.node.position.x,
                                            s_selected_data[i]->data.node.position.y,
                                            s_selected_data[i]->data.node.type
                                        );

            source[len] = s_selected_data[i];
            copy[len] = s_editor_list;
            len ++;
        }
        else if(s_selected_data[i]->type == LISTNODE_GATE) {
            s_editor_list = listnode_add(
                                            s_editor_list, LISTNODE_GATE,
                                            s_selected_data[i]->data.gate.input->data.node.position.x,
                                            s_selected_data[i]->data.gate.input->data.node.position.y,
                                            s_selected_data[i]->data.gate.type,
                                            s_selected_data[i]->data.gate.custom_index
                                        );

            aux1 = s_selected_data[i]->data.gate.input;
            aux2 = s_editor_list->data.gate.input;
            while(aux1 != NULL) {
                if(len == cap) {
                    cap <<= 1;
                    source = realloc(source, cap * sizeof(listnode *));
                    copy = realloc(copy, cap * sizeof(listnode *));
                }

                source[len] = aux1;
                copy[len] = aux2;
                aux1 = aux1->next;
                aux2 = aux2->next;

                len ++;
            }

            aux1 = s_selected_data[i]->data.gate.output;
            aux2 = s_editor_list->data.gate.output;
            while(aux1 != NULL) {
                if(len == cap) {
                    cap <<= 1;
                    source = realloc(source, cap * sizeof(listnode *));
                    copy = realloc(copy, cap * sizeof(listnode *));
                }

                source[len] = aux1;
                copy[len] = aux2;
                aux1 = aux1->next;
                aux2 = aux2->next;

                len ++;
            }
        }

        s_selected_data[i] = s_editor_list;
    }

    for(i = 0; i < len; i ++) {
        if(source[i]->data.node.input != NULL) {
            for(j = 0; j < len; j ++) {
                if(source[i]->data.node.input == source[j]) {
                    copy[i]->data.node.input = copy[j];
                }
            }
        }
    }

    free(source);
    free(copy);
}

uint8_t editor_selection_check() {
    return s_selected_count != 0;
}

// Simulation
static listnode     **s_aux_list = NULL;
static uint32_t     s_aux_list_cap = 0;

static void change_node_value_helper(listnode *n, uint8_t x) {
    if(s_node_value_toggle == 1) n->data.node.value_2 = x;
    else if(s_node_value_toggle == 2) n->data.node.value_1 = x;

    n->data.node.updated = n->data.node.value_1 != n->data.node.value_2;
}

static void node_propagate_no_wire_delay_helper(listnode *current) {
    listnode *aux = NULL;

    uint32_t aux_count = 0;
    uint32_t i;
    uint8_t loop = 0;

    // Prepare the auxilary list (cap is an exaggeration)
    if(s_aux_list_cap != closest_pow2(s_nodes_count)) {
        listnode **temp = NULL;

        s_aux_list_cap = closest_pow2(s_nodes_count);

        temp = realloc(s_aux_list, s_aux_list_cap * sizeof(listnode *));
        if(temp == NULL) debug_send("editor_propagate", "Failed to reallocate auxilary list");
        else s_aux_list = temp;
    }

    // Start searching
    if(current->data.node.input != NULL) {
        aux = current->data.node.input;

        while(aux != NULL) {
            // Stopping the search
            if(aux->data.node.type != NODE_WIRE || aux->data.node.input == NULL) {

                // Solving the current node and the wires
                if(s_node_value_toggle == 1) {
                    change_node_value_helper(current, aux->data.node.value_1);
                    for(i = 0; i < aux_count; i ++) change_node_value_helper(s_aux_list[i], aux->data.node.value_1);
                }
                else if(s_node_value_toggle == 2) {
                    change_node_value_helper(current, aux->data.node.value_2);
                    for(i = 0; i < aux_count; i ++) change_node_value_helper(s_aux_list[i], aux->data.node.value_2);
                }

                break;
            }

            // Checking for loop
            loop = 0;
            for(i = 0; i < aux_count; i ++)
                // Loop detected, stopping the search
                if(aux->data.node.input == s_aux_list[i]) {
                    loop = 1;
                    break;
                }

            // aux is not floating wire -> added to the list (unless it creates a loop)
            if(!loop) {
                s_aux_list[aux_count ++] = aux;
                aux = aux->data.node.input;
            }
            // Stop the search and turn off the loop
            else {
                change_node_value_helper(aux, 0);
                for(i = 0; i < aux_count; i ++) change_node_value_helper(s_aux_list[i], 0);
                aux = NULL;
            }
        }

        // Could not find any input that is not a wire.. basically 'current' is a floating node
        if(aux == NULL) change_node_value_helper(current, 0);
    }
    // Preventing floating nodes from being high
    else change_node_value_helper(current, 0);
}

/*
// custom_propagate without wires
static void custom_propagate(listnode *this) {

    // Based on the fact that customs are missing non-gated wire nodes
    // The algorithm is basically wired propagated, but since wires are missing
    // This translates to non-wired propagation

    listnode *current = this->data.gate.impl;
    listnode *aux = NULL;

    while(current != NULL) {

        // Only NATIVE LISTNODE_GATE is expected
        aux = current->data.gate.input;

        while(aux != NULL) {

            if(aux->data.node.input != NULL) {
                if(s_node_value_toggle == 1) {
                    change_node_value_helper(aux, aux->data.node.input->data.node.value_1);
                    native_load(aux->data.node.value_2);
                }
                else if(s_node_value_toggle == 2) {
                    change_node_value_helper(aux, aux->data.node.input->data.node.value_2);
                    native_load(aux->data.node.value_1);
                }
            }
            else {
                change_node_value_helper(aux, 0);
                native_load(0);
            }

            aux = aux->next;
        }

        if(s_node_value_toggle == 1)
            current->data.gate.output->data.node.value_2 = native(current->data.gate.type);
        else if(s_node_value_toggle == 2)
            current->data.gate.output->data.node.value_1 = native(current->data.gate.type);

        current = current->next;
    }
} */

static void custom_propagate(listnode *this) {
    listnode *current = NULL;
    listnode *aux = NULL;

    // Nodes
    current = this->data.gate.input;

    while(current != NULL) {
        if(current->type == LISTNODE_NODE)
            if(current->data.node.input != NULL)
                if(s_node_value_toggle == 1) change_node_value_helper(current, current->data.node.input->data.node.value_1);
                else if(s_node_value_toggle == 2) change_node_value_helper(current, current->data.node.input->data.node.value_2);

            else change_node_value_helper(current, 0);

        current = current->next;
    }

    current = this->data.gate.impl;

    while(current != NULL) {
        if(current->type == LISTNODE_NODE)
            if(current->data.node.input != NULL)
                if(s_node_value_toggle == 1) change_node_value_helper(current, current->data.node.input->data.node.value_1);
                else if(s_node_value_toggle == 2) change_node_value_helper(current, current->data.node.input->data.node.value_2);

            else change_node_value_helper(current, 0);

        current = current->next;
    }

    current = this->data.gate.output;

    while(current != NULL) {
        if(current->type == LISTNODE_NODE)
            if(current->data.node.input != NULL)
                if(s_node_value_toggle == 1) change_node_value_helper(current, current->data.node.input->data.node.value_1);
                else if(s_node_value_toggle == 2) change_node_value_helper(current, current->data.node.input->data.node.value_2);

            else change_node_value_helper(current, 0);

        current = current->next;
    }

    // Natives
    current = this->data.gate.impl;

    while(current != NULL) {

        if(current->type == LISTNODE_GATE) {
            native_init();
            aux = current->data.gate.input;

            while(aux != NULL) {

                if(aux->data.node.input != NULL) {
                    if(s_node_value_toggle == 1) {
                        change_node_value_helper(aux, aux->data.node.input->data.node.value_1);
                        native_load(aux->data.node.value_2);
                    }
                    else if(s_node_value_toggle == 2) {
                        change_node_value_helper(aux, aux->data.node.input->data.node.value_2);
                        native_load(aux->data.node.value_1);
                    }
                }
                else {
                    change_node_value_helper(aux, 0);
                    native_load(0);
                }

                aux = aux->next;
            }

            if(s_node_value_toggle == 1)
                current->data.gate.output->data.node.value_2 = native(current->data.gate.type);
            else if(s_node_value_toggle == 2)
                current->data.gate.output->data.node.value_1 = native(current->data.gate.type);
        }

        current = current->next;
    }
}

void editor_propagate() {
    listnode *current = s_editor_list;
    listnode *aux = NULL;
    uint8_t solve_gate = 0;

    // Nodes first

    // Clock's node
    if(s_clock.input != NULL && s_clock.toggle) {

        if(s_clock.propagation < s_control_panel.clock_period_u32/2) change_node_value_helper(s_clock.input, 0);
        else change_node_value_helper(s_clock.input, 1);

        if(s_clock.propagation == 0) s_clock.propagation = s_control_panel.clock_period_u32 - 1;
        else s_clock.propagation --;
    }

    // Wire delay is on -> simulator will grab the value of the first input, no matter the type
    if(s_control_panel.wire_delay_toggle) {
        while(current != NULL) {
            if(current->type == LISTNODE_NODE && current->data.node.type != NODE_INPUT) {

                // Grabing the input
                if(current->data.node.input != NULL)
                    if(!current->data.node.input->data.node.updated || current->data.node.input->type == NODE_INPUT)
                        if(s_node_value_toggle == 1) change_node_value_helper(current, current->data.node.input->data.node.value_1);
                        else if(s_node_value_toggle == 2) change_node_value_helper(current, current->data.node.input->data.node.value_2);
                // Preventing floating nodes from being high
                else change_node_value_helper(current, 0);
            }

            current = current->next;
        }
    }
    // Wire delay is off -> the simulator grabs the input only from non-wire nodes
    // Only outputs and terminal wire nodes are analyzed
    else {
        while(current != NULL) {

            if(current->type == LISTNODE_NODE && current->data.node.type == NODE_OUTPUT)
                node_propagate_no_wire_delay_helper(current);

            current = current->next;
        }
    }

    // Gates after
    current = s_editor_list;
    while(current != NULL) {

        if(current->type == LISTNODE_GATE) {

            solve_gate = 0;

            if(current->data.gate.type != NATIVE_CUSTOM) {
                native_init();
                aux = current->data.gate.input;

                if(!s_control_panel.wire_delay_toggle) {
                    while(aux != NULL) {

                        if(aux->data.node.updated) solve_gate = 1;

                        node_propagate_no_wire_delay_helper(aux);

                        if(s_node_value_toggle == 1) native_load(aux->data.node.value_2);
                        else if(s_node_value_toggle == 2) native_load(aux->data.node.value_1);

                        aux = aux->next;
                    }
                }
                else {
                    while(aux != NULL) {

                        if(aux->data.node.updated) solve_gate = 1;

                        if(aux->data.node.input != NULL) {
                            if(s_node_value_toggle == 1) {
                                change_node_value_helper(aux, aux->data.node.input->data.node.value_1);
                                native_load(aux->data.node.value_2);
                            }
                            else if(s_node_value_toggle == 2) {
                                change_node_value_helper(aux, aux->data.node.input->data.node.value_2);
                                native_load(aux->data.node.value_1);
                            }
                        }
                        else {
                            change_node_value_helper(aux, 0);
                            native_load(0);
                        }

                        aux = aux->next;
                    }
                }

                    /*
                // Potential issues with this setup: TODO
                // Start counting
                if(solve_gate) current->data.gate.propagation = 0;
                else {
                    current->data.gate.propagation ++;

                    if(current->data.gate.propagation == current->data.gate.delay) {
                        if(native(current->data.gate.type) != current->data.gate.output->data.node.value_1)
                            current->data.gate.output->data.node.updated = 1;

                        current->data.gate.output->data.node.value_1 = native(current->data.gate.type);
                        current->data.gate.output->data.node.value_2 = native(current->data.gate.type);
                        current->data.gate.propagation = 0;
                    }
                }

                // ^^ Was supposed to implement gate delays for natives
                    */

                if(s_node_value_toggle == 1)
                    current->data.gate.output->data.node.value_2 = native(current->data.gate.type);
                else if(s_node_value_toggle == 2)
                    current->data.gate.output->data.node.value_1 = native(current->data.gate.type);
            }
            else {
                custom_propagate(current);
            }
        }

        current = current->next;
    }

    if(s_node_value_toggle == 1) s_node_value_toggle = 2;
    else if(s_node_value_toggle == 2) s_node_value_toggle = 1;
}

    /*
static void save_last_custom_to_disk() {

    char *name = NULL;
    FILE *file = NULL;
    listnode *custom = s_custom_list.data + (s_custom_list.count - 1);
    listnode *aux = NULL;

    uint32_t input_count = 0;
    uint32_t node_count = 0;
    uint32_t gate_count = 0;
    uint32_t output_count = 0;

    struct {
        listnode *ptr;
    } *inputs;

    struct {
        listnode *ptr;
        listnode *input;
    } *outputs;

    struct {
        listnode *ptr;
        listnode *input;
    } *nodes;

    struct {
        struct {
            listnode *ptr;
            listnode *input;
        } *inputs;

        struct {
            listnode *ptr;
        } *outputs;

        uint32_t type;
    } *gates;

    name = calloc(strlen(s_custom_list.name[s_custom_list.count - 1]) + 8, sizeof(char));
    strcpy(name, s_custom_list.name[s_custom_list.count - 1]);
    strcat(name, ".sld");
    file = fopen(name, "wb+");

    // Counting inputs
    aux = custom->data.gate.input;
    while(aux != NULL) {
        input_count ++;
        aux = aux->next;
    }

    // Counting impl
    aux = custom->data.gate.impl;
    while(aux != NULL) {

        if(aux->type == LISTNODE_NODE) impl_node_count;
        else if(aux->type == LISTNODE_GATE) impl_gate_count ++;

        aux = aux->next;
    }

    // Counting outputs
    aux = custom->data.gate.impl;
    while(aux != NULL) {
        output_count ++;
        aux = aux->next;
    }

    // Allocating buffers
    inputs = calloc(input_count, sizeof(*inputs));
    outputs = calloc(output_count, sizeof(*outputs));
    nodes = calloc(node_count, sizeof(*nodes));
    gates = calloc(gate_count), sizeof(*gates));

    // Writing lengths
    fwrite(&input_count, sizeof(uint32_t), 1, file);
    fwrite(&impl_node_count, sizeof(uint32_t), 1, file);
    fwrite(&impl_gate_count, sizeof(uint32_t), 1, file);
    fwrite(&output_count, sizeof(uint32_t), 1, file);

    // Preparing implementation data
    aux = custom->data.gate.input;
    while(aux != NULL) {

        

        aux = aux->next;
    }

    fclose(file);
} */

void editor_save(const char *name) {

    // TODO: Remove wire nodes from save implementation

    // Saving to customs
    listnode *current = NULL;
    listnode *next = NULL;
    listnode *next2 = NULL;
    listnode *new = malloc(sizeof(listnode));
    listnode *aux = NULL;
    listnode *aux2 = NULL;
    char **temp = NULL;

    new->type = LISTNODE_GATE;
    new->next = NULL;
    new->prev = NULL;

    new->data.gate.type = NATIVE_CUSTOM;
    new->data.gate.custom_index = s_custom_list.count;
    new->data.gate.input = NULL;
    new->data.gate.impl = NULL;
    new->data.gate.output = NULL;

    current = s_editor_list;
    while(current != NULL) {

        next = current->next;

        if(s_clock.input == current) {
            current = next;
            continue;
        }

        current->next = NULL;
        current->prev = NULL;

        if(current->type == LISTNODE_NODE) {
            if(current->data.node.type == NODE_INPUT) {
                if(new->data.gate.input != NULL) {
                    new->data.gate.input->prev = current;
                    current->next = new->data.gate.input;
                }
                new->data.gate.input = current;
            }
            else if(current->data.node.type == NODE_WIRE) {
                if(new->data.gate.impl != NULL) {
                    new->data.gate.impl->prev = current;
                    current->next = new->data.gate.impl;
                }
                new->data.gate.impl = current;
            }
            else if(current->data.node.type == NODE_OUTPUT) {
                if(new->data.gate.output != NULL) {
                    new->data.gate.output->prev = current;
                    current->next = new->data.gate.output;
                }
                new->data.gate.output = current;
            }

            current->data.node.type = NODE_WIRE;
        }
        else if(current->type == LISTNODE_GATE) {
            if(current->data.gate.type != NATIVE_CUSTOM) {
                if(new->data.gate.impl != NULL) {
                    new->data.gate.impl->prev = current;
                    current->next = new->data.gate.impl;
                }
                new->data.gate.impl = current;
            }
            else {

                // Adding the insides of a custom gate
                aux = current->data.gate.input;
                while(aux != NULL) {

                    next2 = aux->next;

                    aux->prev = NULL;
                    aux->next = NULL;

                    if(new->data.gate.impl != NULL) {
                        new->data.gate.impl->prev = aux;
                        aux->next = new->data.gate.impl;
                    }
                    new->data.gate.impl = aux;

                    aux = next2;
                }

                aux = current->data.gate.output;
                while(aux != NULL) {

                    next2 = aux->next;

                    aux->prev = NULL;
                    aux->next = NULL;

                    if(new->data.gate.impl != NULL) {
                        new->data.gate.impl->prev = aux;
                        aux->next = new->data.gate.impl;
                    }
                    new->data.gate.impl = aux;

                    aux = next2;
                }

                aux = current->data.gate.impl;
                while(aux != NULL) {

                    next2 = aux->next;

                    aux->prev = NULL;
                    aux->next = NULL;

                    if(new->data.gate.impl != NULL) {
                        new->data.gate.impl->prev = aux;
                        aux->next = new->data.gate.impl;
                    }
                    new->data.gate.impl = aux;

                    aux = next2;
                }

            }
        }

        current = next;
    }

    /*
    // REMOVE ADDING THE WIRES TO IMPL ^^^

    // Resolve the non gated wires. Connects inputs, outputs and gated wires directly. (Non gated wires are not even added lol)
    aux = new->data.gate.output; // Outputs first

    while(aux != NULL) {

        aux2 = aux->data.node.input;

        if(aux2 != NULL) {
            while(aux2->data.node.gate == NULL) {
                aux2 = aux2->data.node.input;
            }
        }

        aux = aux->next;
    }
    */

    if(s_custom_list.data != NULL) {
        s_custom_list.data->prev = new;
        new->next = s_custom_list.data;
    }

    s_custom_list.data = new;
    s_editor_list = s_clock.input;

    s_custom_list.count ++;

    temp = realloc(s_custom_list.name, s_custom_list.count * sizeof(char *));
    if(temp == NULL) {
        debug_send("editor_save", "Failed to reallocate names data");
        return;
    }
    s_custom_list.name = temp;

    s_custom_list.name[s_custom_list.count - 1] = malloc((strlen(name) + 1) * sizeof(char));
    strcpy(s_custom_list.name[s_custom_list.count - 1], name);

    // Saving to disk
        /*
    char *filename = malloc((strlen(name) + 8) * sizeof(char));
    strcpy(filename, name);
    strcat(filename, ".sld");

    FILE *f = fopen(filename, "wb+");

    // Copying new to disk (will encounter wires, natives and customs)
    uint32_t len = 0;
    uint32_t cap = 64;
    uint32_t i;

    listnode *disk = calloc(cap, sizeof(listnode));
    listnode **ptr = calloc(cap, sizeof(listnode *));

    // Writing inputs
    current = new->data.gate.input;

    while(current != NULL) {

        if(len == cap) {
            cap <<= 1;
            disk = realloc(disk, cap * sizeof(listnode));
            ptr = realloc(ptr, cap * sizeof(listnode));
        }

        ptr[len] = current;

        fwrite(current, sizeof(listnode), 1, f);
        len ++;

        current = current->next;
    }

    // Writing outputs
    current = new->data.gate.output;

    while(current != NULL) {

        if(len == cap) {
            cap <<= 1;
            disk = realloc(disk, cap * sizeof(listnode));
            ptr = realloc(ptr, cap * sizeof(listnode));
        }

        ptr[len] = current;

        fwrite(current, sizeof(listnode), 1, f);
        len ++;

        current = current->next;
    }

    current = new->data.gate.impl;

    while(current != NULL) {

        // Wire node
        if(current->type == LISTNODE_NODE) {

            if(len == cap) {
                cap <<= 1;
                disk = realloc(disk, cap * sizeof(listnode));
                ptr = realloc(ptr, cap * sizeof(listnode));
            }

            ptr[len] = current;

            fwrite(current, sizeof(listnode), 1, f);
            len ++;

        }
        // Gate (Custom / Native)
        else if(current->type == LISTNODE_GATE) {

            // Writing Gate
            if(len == cap) {
                cap <<= 1;
                disk = realloc(disk, cap * sizeof(listnode));
                ptr = realloc(ptr, cap * sizeof(listnode));
            }

            ptr[len] = current;

            fwrite(current, sizeof(listnode), 1, f);
            len ++;

            // Writing inputs
            aux = current->data.gate.input;

            while(aux != NULL) {

                if(len == cap) {
                    cap <<= 1;
                    disk = realloc(disk, cap * sizeof(listnode));
                    ptr = realloc(ptr, cap * sizeof(listnode));
                }

                ptr[len] = aux;

                fwrite(aux, sizeof(listnode), 1, f);
                len ++;
                aux = aux->next;
            }

            // Writing outputs
            aux = current->data.gate.output;

            while(aux != NULL) {

                if(len == cap) {
                    cap <<= 1;
                    disk = realloc(disk, cap * sizeof(listnode));
                    ptr = realloc(ptr, cap * sizeof(listnode));
                }

                ptr[len] = aux;

                fwrite(aux, sizeof(listnode), 1, f);
                len ++;
                aux = aux->next;
            }
        }

        current = current->next;
    }

    // Re-writing the inputs
    current = new->data.gate.impl;

    while(current != NULL) {

        // TODO

        current = current->next;
    }

    fclose(f);
        */
}

void editor_get_custom(char *filename) {
    FILE *f = fopen(filename, "wb+");

    if(f == NULL) {
        debug_send("editor_get_custom", "Failed to open save file");
        return;
    }

    

    fclose(f);
}

// Disk related
    /*
typedef struct link_listnode_id link_listnode;
typedef struct disk_listnode_id disk_listnode;
typedef struct disk_pair_id disk_pair;

struct link_listnode_id {
    listnode *ram;
    uint32_t index;
};

struct disk_pair_id {
    uint32_t index;
    uint32_t type;
};

struct disk_listnode_id {
    union {
        struct {
            disk_pair input;
            disk_pair output_1;
            disk_pair output_2;
            uint32_t gate;

            uint8_t type;
        } node;

        struct {
            disk_pair *input;
            disk_pair *impl;
            disk_pair *output;

            uint32_t input_count;
            uint32_t impl_count;
            uint32_t output_count;

            uint32_t type;
        } gate;
    } data;

    uint8_t type;
};

static disk_pair s_get_disk_pair(
    link_listnode *input,
    link_listnode *impl,
    link_listnode *output,
    uint32_t input_count,
    uint32_t impl_count,
    uint32_t output_count,
    listnode *ram) {

    disk_pair result = {.index = 0, .type = NODE_INPUT | NODE_WIRE | NODE_OUTPUT};
    uint32_t i;

    for(i = 0; i < input_count; i ++) {
        if(input[i].ram == ram) {
            result.index = i;
            result.type = NODE_INPUT;
            return result;
        }
    }

    for(i = 0; i < impl_count; i ++) {
        if(impl[i].ram == ram) {
            result.index = i;
            result.type = NODE_WIRE;
            return result;
        }
    }

    for(i = 0; i < output_count; i ++) {
        if(output[i].ram == ram) {
            result.index = i;
            result.type = NODE_OUTPUT;
            return result;
        }
    }

    return result;
}

void editor_save() {
    FILE *file = fopen(s_save_name, "wb+");
    if(file == NULL) {
        debug_send("editor_save", "Failed to open save file");
        return;
    }

    link_listnode *input_link = calloc(s_nodes_count, sizeof(link_listnode));
    link_listnode *impl_link = calloc(s_nodes_count, sizeof(link_listnode));
    link_listnode *output_link = calloc(s_nodes_count, sizeof(link_listnode));

    struct {
        disk_listnode *input;
        disk_listnode *impl;
        disk_listnode *output;
    } save;

    listnode *current = s_editor_list;

    uint32_t input_count = 0;
    uint32_t impl_count = 0;
    uint32_t output_count = 0;

    uint32_t input_index = 0;
    uint32_t impl_index = 0;
    uint32_t output_index = 0;

    // Setup the ram - disk link (better to link first because of connections having different indices)
    while(current != NULL) {

        if(current->type == LISTNODE_NODE) {
            switch(current->data.node.type) {
                case NODE_INPUT:
                    input_link[input_count].ram = current;
                    input_link[input_count].index = input_count;
                    input_count ++;
                    break;
                case NODE_WIRE:
                    impl_link[impl_count].ram = current;
                    impl_link[impl_count].index = impl_count;
                    impl_count ++;
                    break;
                case NODE_OUTPUT:
                    output_link[output_count].ram = current;
                    output_link[output_count].index = output_count;
                    output_count ++;
                    break;
            }
        }
        else if(current->type == LISTNODE_GATE) {
            if(current->data.gate.type != NATIVE_CUSTOM) {
                impl_link[impl_count].ram = current;
                impl_link[impl_count].index = impl_count;
                impl_count ++;
            }
            else {
                // TODO
            }
        }

        current = current->next;
    }

    // Uploading the data to the disk list
    save.input = calloc( input_count, sizeof(disk_listnode) );
    save.impl = calloc( impl_count, sizeof(disk_listnode) );
    save.output = calloc( output_count, sizeof(disk_listnode) );

    current = s_editor_list;

    while(current != NULL) {

        if(current->type == LISTNODE_NODE) {
            switch(current->data.node.type) {
                case NODE_INPUT:
                    save.input[input_index].type = LISTNODE_NODE;
                    save.input[input_index].data.node.gate = 0;
                    save.input[input_index].data.node.input.index = 0;
                    save.input[input_index].data.node.input.type = 0;

                    save.input[input_index].data.node.output_1 = s_get_disk_pair(
                        input_link, impl_link, output_link,
                        input_count, impl_count, output_count,
                        current->data.node.output_1
                    );

                    save.input[input_index].data.node.output_2 = s_get_disk_pair(
                        input_link, impl_link, output_link,
                        input_count, impl_count, output_count,
                        current->data.node.output_2
                    );

                    input_index ++;
                    break;

                case NODE_WIRE:
                    save.impl[impl_index].type = LISTNODE_NODE;
                    save.impl[impl_index].data.node.gate = 0;

                    save.impl[impl_index].data.node.input = s_get_disk_pair(
                        input_link, impl_link, output_link,
                        input_count, impl_count, output_count,
                        current->data.node.input
                    );

                    save.impl[impl_index].data.node.output_1 = s_get_disk_pair(
                        input_link, impl_link, output_link,
                        input_count, impl_count, output_count,
                        current->data.node.output_1
                    );

                    save.impl[impl_index].data.node.output_2 = s_get_disk_pair(
                        input_link, impl_link, output_link,
                        input_count, impl_count, output_count,
                        current->data.node.output_2
                    );

                    impl_index ++;
                    break;

                case NODE_OUTPUT:
                    save.output[output_index].type = LISTNODE_NODE;
                    save.output[output_index].data.node.gate = 0;

                    save.output[output_index].data.node.input = s_get_disk_pair(
                        input_link, impl_link, output_link,
                        input_count, impl_count, output_count,
                        current->data.node.input
                    );

                    save.output[output_index].data.node.output_1.index = 0;
                    save.output[output_index].data.node.output_1.type = 0;
                    save.output[output_index].data.node.output_2.index = 0;
                    save.output[output_index].data.node.output_2.type = 0;

                    output_index ++;
                    break;
            }
        }
        else if(current->type == LISTNODE_GATE) {
            if(current->data.gate.type != NATIVE_CUSTOM) {

            }
        }

        current = current->next;
    }

    fwrite(&input_count, sizeof(uint32_t), 1, file);
    fwrite(&impl_count, sizeof(uint32_t), 1, file);
    fwrite(&output_count, sizeof(uint32_t), 1, file);

    fwrite(save.input, sizeof(disk_listnode), input_count, file);
    fwrite(save.impl, sizeof(disk_listnode), impl_count, file);
    fwrite(save.output, sizeof(disk_listnode), output_count, file);

        /*
            debug_send(
                "editor_save",
                "Saved:\n%u Input: %u bytes\n%u Impl: %u bytes\n%u Output: %u bytes",
                input_count, input_count * sizeof(disk_listnode),
                impl_count, impl_count * sizeof(disk_listnode),
                output_count, output_count * sizeof(disk_listnode)
            );
        /*

    free(input_link);
    free(impl_link);
    free(output_link);
}
    */
