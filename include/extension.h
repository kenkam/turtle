/*
 *  extension.h
 *  A GTK+ interpreter for the LOGO language
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#include <gtk/gtk.h>

// linked list of Logo inputs
struct _input_items {
    Logo input;
    gchar *buffer;  /* for use for re-reading history */
    struct _input_items *next;
};
typedef struct _input_items * InputItems;

struct _draw_info {
    InputItems input_items;
    GtkWidget *drawing_area;
    GtkWidget *text_view;
    GtkWidget *window;
    GtkWidget *statusbar;
    GtkWidget *tree_view;
    int drag_on;    /* dragging is on */
    double x;       /* coordinates to move to before draw */
    double y;       
};
typedef struct _draw_info DInfo;

// GUI
int gui_main(int argc, char * argv[]);
GtkWidget * create_window(DInfo *dinfo);
GtkWidget * create_menu_bar(DInfo *dinfo, GtkWidget *window);
GtkWidget * create_scrolled_window(GtkWidget *child);
GtkWidget * create_drawing_area(void);
GtkWidget * create_slider_box(DInfo *dinfo);
GtkWidget * create_text_view(void);
GtkWidget * create_tree_view(void);

// Helper functions
int draw(DInfo *dinfo);
void draw_triangle(DInfo *dinfo, double x, double y);
void paint_white(DInfo *dinfo);
void remove_messages(DInfo *dinfo);
void update_tree_view(DInfo *dinfo);
void clear_text(DInfo *dinfo);
void new_file(DInfo *dinfo);
int open_file(DInfo *dinfo, char *filename);
int process_text_view(DInfo * dinfo);
Logo read_input(char * buffer, int count);
int parse_items(InputItems input_items);
int input_items_add(InputItems *input_items, Logo input, char *buffer);
Logo input_items_pop(InputItems *input_items);
void free_input_items(InputItems input_items);

// GUI Interpreter Backend
void gui_ipt_fd(Logo input, float op);
void gui_ipt_lt(Logo input, float op);
void gui_ipt_rt(Logo input, float op);
