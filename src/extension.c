/*
 *  extension.c
 *  A GTK+ interpreter for the LOGO language
 *
 *  Author: Kenneth Kam <kk4053@bris.ac.uk>
 *  Username: kk4053
 *  Declaration: This code for COMSM1201 is the original work of Kenneth Kam.
 */

#define SIZE_X          800     /* size of window */
#define SIZE_Y          600
#define DRAW_SIZE_X     800     /* size of draw area */
#define DRAW_SIZE_Y     400
#define TV_SIZE_X       550     /* size of text view */
#define TV_SIZE_Y       200
#define SLIDER_MAX      5.0     /* maximum factor for slider */     
#define SLIDER_MIN      0.2     /* minimum factor for slider */
#define SLIDER_STEP     0.05    /* step of slider */
#define SLIDER_LENGTH   250     /* length of slider in pixels */
#define PI              3.14159265
#define TO_RADS         PI / 180
#define LOGO2GUI_FACTOR 1.4     /* conversion factor between LOGO and postscript */
#define DEFAULT_ANGLE   -90     /* point north */
#define WHITE           1.0, 1.0, 1.0  /* RGB values */
#define GREY            0.1, 0.1, 0.1
#define RED             1.0, 0.0, 0.0
#define TAB_WIDTH       28
#define RETURN_KEY      0xff0d  /* return key code */
#define HISTORY_STR_LENGTH      27      /* string length for input name for input history list */
#define TRUNCATE_STR_LENGTH     1024
#define TRIANGLE_SIZE   10      /* size of triangle on screen */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "interpreter.h"
#include "extension.h"

/* for interpreting functions to draw on surface */
cairo_t *_cr = NULL;
/* current orientation */
float _angle = DEFAULT_ANGLE;
float _slider_factor = LOGO2GUI_FACTOR;

/********************************************
 Callback Funcs
 ********************************************/

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, DInfo *dinfo) {
    gtk_widget_destroy(widget);
    gtk_main_quit();
    return FALSE;
}

/*
 *  Callback to draw the picture
 */
static gboolean draw_callback(GtkWidget *widget, GdkEventExpose *event, DInfo *dinfo) {
    draw(dinfo);
    return FALSE;
}
                
/*
 *  Callback on slider value changed
 */ 
static gboolean slider_changed_callback(GtkRange *range, DInfo *dinfo) {
    // set global logo2gui conv. factor
    _slider_factor = (float) gtk_range_get_value(range);
    draw(dinfo);
    return TRUE;
}

/*
 *  Callback on draw button clicked
 */ 
static gboolean draw_clicked_callback(GtkButton *button, DInfo *dinfo) {
    process_text_view(dinfo);
    return TRUE;
}

/*
 *  Keyboard callback for CTRL+ENTER shortcut
 */
static gboolean keyboard_callback(GtkWidget *widget, GdkEventKey *event, DInfo *dinfo) {
    // if ctrl+enter is pressed, process the text view
    if (event->keyval == RETURN_KEY && 
        (event->state & GDK_CONTROL_MASK)) {
        process_text_view(dinfo);        
        return TRUE;
    }
    return FALSE;
}

/*
 *  Mouse motion callback for moving the picture
 */
static gboolean mouse_motion_callback(GtkWidget *widget, GdkEventButton *event, DInfo *dinfo) {
    if (dinfo->drag_on) {
        dinfo->x = event->x;
        dinfo->y = event->y;
        draw(dinfo);
    }
    return FALSE;
}

/*
 *  On button press callback
 */
static gboolean button_down_callback(GtkWidget *widget, GdkEventButton *event, DInfo *dinfo) {
    if (event->button == 1) {
        // change the cursor if left clicked and turn drag on
        dinfo->drag_on = 1;
        GdkCursor* cursor;
        cursor = gdk_cursor_new(GDK_FLEUR);
        gdk_window_set_cursor(gtk_widget_get_parent_window(widget), cursor);
        gdk_cursor_destroy(cursor);
    }
    return FALSE;
}

/*
 *  On button up callback
 */
static gboolean button_up_callback(GtkWidget *widget, GdkEventButton *event, DInfo *dinfo) {
    dinfo->drag_on = 0;
    GdkCursor* cursor;
    cursor = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor(gtk_widget_get_parent_window(widget), cursor);
    gdk_cursor_destroy(cursor);
    return FALSE;
}

/**
 *  New file callback
 */ 
static void new_file_callback(DInfo *dinfo, guint callback_action, GtkWidget *menuitem) {
    new_file(dinfo);
}

/**
 *  Open file callback
 */ 
static void open_file_callback(DInfo *dinfo, guint callback_action, GtkWidget *menuitem) {
    GtkWidget *dialog;
    // create a file chooser dialog
    dialog = gtk_file_chooser_dialog_new("Open File",
                          GTK_WINDOW(dinfo->window),
                          GTK_FILE_CHOOSER_ACTION_OPEN,
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                          NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        open_file(dinfo, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

/**
 *  Undo callback
 */
static void undo_callback(DInfo *dinfo, guint callback_action, GtkWidget *menuitem) {
    Logo input;
    remove_messages(dinfo);
    if (dinfo->input_items != NULL) {
        // pop the last logo input item
        input = input_items_pop(&(dinfo->input_items));
        // we don't want it anymore
        free_logo(input);
        // redraw
        paint_white(dinfo);
        draw(dinfo);
        gtk_statusbar_push(
            GTK_STATUSBAR(dinfo->statusbar), 
            gtk_statusbar_get_context_id(GTK_STATUSBAR(dinfo->statusbar), "OK"),
            "Undo"
        );
    }
    update_tree_view(dinfo);
}

/**
 *  Tree item selected callback. Used for history.
 */
static gboolean tree_item_selected_callback(GtkTreeView *tree_view, DInfo *dinfo) {
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTextBuffer *buffer;
    InputItems item;
    char *text;
    int i, num;
    
    model = gtk_tree_view_get_model(tree_view);
    selection = gtk_tree_view_get_selection(tree_view);
    
    // get the index of selected item
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
            1, &num,
        -1);
    }
    
    // get the text buffer of that selected item
    item = dinfo->input_items;
    if (item != NULL) {
        for (i=0; i<num; i++) {
            item = item->next;
        }
    }
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dinfo->text_view));
    text = item->buffer;
    gtk_text_buffer_set_text(buffer, text, -1);
    
    return FALSE;
}

/********************************************
 MAIN
 ********************************************/
int main(int argc, char * argv[]) {
    return gui_main(argc, argv);
}

/********************************************
 GUI
 ********************************************/

/*
 *  Main function that draws the GUI and registers callbacks
 */
int gui_main(int argc, char * argv[]) {
    // setup GTK pointers
    GtkWidget *window;          /* main window */
    GtkWidget *main_box;        /* main packing box attached to window */
    GtkWidget *scale_box;       /* horizontal box for slider */
    GtkWidget *text_input_box;  /* box to contain both text view and history */
    GtkWidget *vbox;            /* vertical packing box attached to main_box */
    GtkWidget *button_box;      /* packing box for buttons at bottom */
    GtkWidget *menu_bar;        /* menu bar */
    GtkWidget *drawing_area;    /* drawing area for drawing logo */
    GtkWidget *h_sep;           /* horizontal separator */
    GtkWidget *text_view;       /* text view for editing */
    GtkWidget *tree_view;       /* tree view for input history */
    GtkWidget *scrolled_text;   /* scrolled window for text */
    GtkWidget *scrolled_tree;   /* scrolled window for input history */
    GtkWidget *button;          /* draw button */
    GtkWidget *undo_button;     /* undo button */
    GtkWidget *statusbar;       /* status bar */
    DInfo dinfo;                /* to pass to callbacks for drawing, etc */
    
    // init the gtk application
    gtk_init(&argc, &argv);
    // create the window
    window = create_window(&dinfo);
    // setup the box
    main_box = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    // create the menu bar
    menu_bar = create_menu_bar(&dinfo, window);
    gtk_box_pack_start(GTK_BOX(main_box), menu_bar, FALSE, TRUE, 0);
    // make a box and put it after the menu bar
    vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
    gtk_container_add(GTK_CONTAINER(main_box), vbox);
    
    // drawing area
    drawing_area = create_drawing_area();
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 5);

    // make a horizontal separator
    h_sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), h_sep, FALSE, FALSE, 5);
    
    // create box for text and history
    text_input_box = gtk_hbox_new(FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbox), text_input_box, FALSE, FALSE, 5);

    // create a text viewer
    text_view = create_text_view();
    // create the scrolled window
    scrolled_text = create_scrolled_window(text_view);
    gtk_widget_set_size_request(scrolled_text, TV_SIZE_X, TV_SIZE_Y);
    gtk_box_pack_start(GTK_BOX(text_input_box), scrolled_text, FALSE, FALSE, 5);
    // create the tree view
    tree_view = create_tree_view();
    scrolled_tree = create_scrolled_window(tree_view);
    gtk_box_pack_start(GTK_BOX(text_input_box), scrolled_tree, TRUE, TRUE, 5);

    // for button
    button_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 5);    

    // create the button
    button = gtk_button_new_with_label("Draw (Ctrl-Enter)");
    gtk_box_pack_start(GTK_BOX(button_box), button, FALSE, FALSE, 5);
    gtk_widget_set_size_request(button, 200, -1);
    // undo button
    undo_button = gtk_button_new_from_stock(GTK_STOCK_UNDO);
    gtk_box_pack_end(GTK_BOX(button_box), undo_button, FALSE, FALSE, 5);
    // create the slider box
    scale_box = create_slider_box(&dinfo);
    gtk_widget_set_size_request(scale_box, SLIDER_LENGTH, -1);
    gtk_box_pack_end(GTK_BOX(button_box), scale_box, FALSE, TRUE, 5);
    
    // create the statusbar
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, TRUE, FALSE, 0);
    
    // setup data structure
    dinfo.drawing_area = drawing_area;
    dinfo.input_items = NULL;
    dinfo.text_view = text_view;
    dinfo.window = window;
    dinfo.statusbar = statusbar;
    dinfo.tree_view = tree_view;
    dinfo.drag_on = 0;
    dinfo.x = DRAW_SIZE_X / 2;
    dinfo.y = DRAW_SIZE_Y / 2;

    // callbacks
    // connect callback to button clicked to draw picture
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(draw_clicked_callback), &dinfo);
    // connect callback on keyboard pressed to capture ctrl-enter for both
    // tree view and text view
    g_signal_connect(G_OBJECT(text_view), "key-press-event",
                     G_CALLBACK(keyboard_callback), &dinfo);
    g_signal_connect(G_OBJECT(tree_view), "key-press-event",
                     G_CALLBACK(keyboard_callback), &dinfo);
    // connect callback on motion on drawing_area for drag panning
    g_signal_connect(G_OBJECT(drawing_area), "button-press-event",
                     G_CALLBACK(button_down_callback), &dinfo);
    g_signal_connect(G_OBJECT(drawing_area), "button-release-event",
                     G_CALLBACK(button_up_callback), &dinfo);
    g_signal_connect(G_OBJECT(drawing_area), "motion-notify-event",
                     G_CALLBACK(mouse_motion_callback), &dinfo);
    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(drawing_area, GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(drawing_area, GDK_POINTER_MOTION_MASK);
    // connect callback on tree_view select to load input history
    g_signal_connect(G_OBJECT(tree_view), "cursor-changed",
                     G_CALLBACK(tree_item_selected_callback), &dinfo);
    // connect callback on undo button
    g_signal_connect_swapped(G_OBJECT(undo_button), "clicked",
                             G_CALLBACK(undo_callback), &dinfo);
    // text view should have focus
    gtk_widget_grab_focus(text_view);
    // show all
    gtk_widget_show_all(window);
    
    // create the drawing surface
    _cr = gdk_cairo_create(drawing_area->window);
    cairo_set_line_width(_cr, 1.0);
    cairo_set_line_cap(_cr, CAIRO_LINE_CAP_SQUARE);
    
    // connect callback to expose event so it knows how to redraw itself
    g_signal_connect(G_OBJECT(window), "expose-event",
                     G_CALLBACK(draw_callback), &dinfo);

    gtk_main();
    /* free the input structure */
    free_input_items(dinfo.input_items);
    cairo_destroy(_cr);
    return 0;
}

/*
 *  Creates the window widget
 */
GtkWidget * create_window(DInfo *dinfo) {
    GtkWidget *window;
    // create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // set the size
    gtk_window_set_default_size(GTK_WINDOW(window), SIZE_X, SIZE_Y);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), dinfo);
    // position it
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    // set the title
    gtk_window_set_title(GTK_WINDOW(window), "Turtle Graphics Extension");
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    return window;
}

/**
 *  Creates the menu bar
 */
GtkWidget * create_menu_bar(DInfo *dinfo, GtkWidget *window) {
    GtkWidget *menu_bar;
    // define the items in menubar
    static GtkItemFactoryEntry entries[] = {
      { "/_File",         NULL,      NULL,           0, "<Branch>" },
      { "/File/_New",     "<CTRL>N", new_file_callback, 1, "<Item>" },
      { "/File/_Open...", "<CTRL>O", open_file_callback, 1, "<Item>" },
      { "/File/sep1",     NULL,      NULL,           0, "<Separator>" },
      { "/File/_Quit",    "<CTRL>Q", gtk_main_quit,  0, "<StockItem>", GTK_STOCK_QUIT },
      { "/_Edit",         NULL,      NULL,           0, "<Branch>" },
      { "/Edit/_Undo",    "<CTRL>Z", undo_callback,  1, "<Item>" }};
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    static gint num_entries = sizeof(entries) / sizeof(entries[0]);
  
    GtkItemFactory *item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
    gtk_item_factory_create_items(item_factory, num_entries, entries, dinfo);
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
    
    menu_bar = gtk_item_factory_get_widget(item_factory, "<main>");
    return menu_bar;
}

/**
 *  Create scrolled window
 */
GtkWidget * create_scrolled_window(GtkWidget *child) {
    GtkWidget * scrolled_window;
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), child);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    return scrolled_window;
}

/**
 *  Creates drawing area
 */
GtkWidget * create_drawing_area(void) {
    GtkWidget *drawing_area;
    
    // setup the drawing area    
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, DRAW_SIZE_X, DRAW_SIZE_Y);
    
    GtkRcStyle *rc_style;
    GdkColor color;
    
    // change some colours here
    rc_style = gtk_rc_style_new();
    gdk_color_parse("white", &color);
    rc_style->bg[GTK_STATE_NORMAL] = color;
    rc_style->color_flags[GTK_STATE_NORMAL] |= (GTK_RC_BG);
    gtk_widget_modify_style(drawing_area, rc_style);
    gtk_rc_style_unref(rc_style);
    
    return drawing_area;
}

/**
 *  Creates the slider widget and returns it in a hbox
 */
GtkWidget * create_slider_box(DInfo *dinfo) {
    GtkWidget *label;
    GtkWidget *slider;
    GtkWidget *scale_box;
   
    // make a new box
    scale_box = gtk_hbox_new(FALSE, 0);
   
    // make a label
    label = gtk_label_new("Zoom");
    gtk_box_pack_start(GTK_BOX(scale_box), label, FALSE, FALSE, 5);
    
    // make a slider
    slider = gtk_hscale_new_with_range(SLIDER_MIN, SLIDER_MAX, SLIDER_STEP);
    gtk_scale_set_draw_value(GTK_SCALE(slider), FALSE);
    gtk_scale_set_value_pos(GTK_SCALE(slider), GTK_POS_TOP);
    gtk_range_set_value(GTK_RANGE(slider), LOGO2GUI_FACTOR);
    gtk_range_set_update_policy(GTK_RANGE(slider), GTK_UPDATE_CONTINUOUS);
    
    gtk_box_pack_start(GTK_BOX(scale_box), slider, TRUE, TRUE, 5);
    
    // connect callback to slider 
    g_signal_connect(G_OBJECT(slider), "value-changed",
                     G_CALLBACK(slider_changed_callback), dinfo);
                    
    return scale_box;
}

/*
 *  Creates the text_view widget and returns it
 */
GtkWidget * create_text_view(void) {
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkRcStyle *rc_style;
    GdkColor color;
    PangoFontDescription *font_desc;
    PangoTabArray *tab_array;

    text_view = gtk_text_view_new();
    
    // set the wrapping mode
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    
    // change some colours here
    rc_style = gtk_rc_style_new();
    gdk_color_parse("white", &color);
    rc_style->text[GTK_STATE_NORMAL] = color;
    gdk_color_parse("#666666", &color);
    rc_style->base[GTK_STATE_NORMAL] = color;
    rc_style->color_flags[GTK_STATE_NORMAL] |= (GTK_RC_TEXT + GTK_RC_BASE);
    gtk_widget_modify_style(text_view, rc_style);
    gtk_rc_style_unref(rc_style);

    // set monospace
    font_desc = pango_font_description_from_string("monospace 9");
    gtk_widget_modify_font(text_view, font_desc);
    
    // tabs are TAB_WIDTH long
    tab_array = pango_tab_array_new(1, TRUE);
    pango_tab_array_set_tab(tab_array, 0, PANGO_TAB_LEFT, TAB_WIDTH);
    gtk_text_view_set_tabs(GTK_TEXT_VIEW(text_view), tab_array);

    // get the buffer
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // set the buffer to default value
    gtk_text_buffer_set_text(buffer, 
"{\n\
    DO A FROM 1 TO 100 {\n\
        SET C := A 1.5 * ;\n\
        FD C\n\
        RT 62\n\
    }\n\
}" , -1);

    pango_tab_array_free(tab_array);
    return text_view;
}

/**
 *  Create a tree view for showing list of input
 */
GtkWidget * create_tree_view(void) {
    GtkWidget *tree_view;
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;
    GtkListStore *store;
    
    tree_view = gtk_tree_view_new();
    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Input History");
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", 0);
    
    // only one things can be selected
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    
    // set the model
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(store));
    
    return tree_view;
}

/********************************************
 Helper Functions
 ********************************************/

/*
 *  Draw on canvas
 */ 
int draw(DInfo *dinfo) {
    int ret;
    double cur_x, cur_y;
    cur_x = dinfo->x;
    cur_y = dinfo->y;
    ret = 0;
    // reset the angle
    _angle = DEFAULT_ANGLE;

    cairo_move_to(_cr, dinfo->x, dinfo->y);
    paint_white(dinfo);
    /* run the interpreter here */
    /* is there input? */
    if (dinfo->input_items != NULL) {
        // interpret it
        // set the lines are grey
        cairo_set_source_rgb(_cr, GREY);
        ret = parse_items(dinfo->input_items);
        cairo_get_current_point(_cr, &cur_x, &cur_y);
        cairo_stroke(_cr);
    }
    draw_triangle(dinfo, cur_x, cur_y);

    return ret;
}

/*
 *  Draws triangle for current position
 */
void draw_triangle(DInfo *dinfo, double x, double y) {
    // constant used to draw the triangle
    double k = (TRIANGLE_SIZE/2/sin(20 * TO_RADS));
    // do the triangle
    cairo_move_to(_cr, x, y);
    cairo_rel_move_to(_cr, k*cos(_angle * TO_RADS), k*sin(_angle * TO_RADS));
    _angle = _angle - 160;
    cairo_rel_line_to(_cr, k*cos(_angle * TO_RADS), k*sin(_angle * TO_RADS));
    _angle = _angle - 110;
    cairo_rel_line_to(_cr, TRIANGLE_SIZE*cos(_angle * TO_RADS), TRIANGLE_SIZE*sin(_angle * TO_RADS));
    _angle = _angle - 110;
    cairo_rel_line_to(_cr, k*cos(_angle * TO_RADS), k*sin(_angle * TO_RADS));
    _angle = _angle + 380;
    cairo_set_source_rgb(_cr, RED);
    // fill it at last
    cairo_fill( _cr);
}

/**
 *  Paints cairo surface white
 */ 
void paint_white(DInfo *dinfo) {
    // move to the center
    cairo_move_to(_cr, dinfo->x, dinfo->y);
    // paint a white canvas
    cairo_set_source_rgb(_cr, WHITE);
    cairo_paint(_cr);
}

/**
 *  Gets what's in the textbox and draws 'em
 */ 
int process_text_view(DInfo *dinfo) {
    GtkWidget *errmsg;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dinfo->text_view));
    gchar *text;

    /* remove status messages */
    remove_messages(dinfo);

    /* Obtain iters for the start and end of points of the buffer */
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    
    /* Get the entire buffer text. */
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    /* load it onto input */
    input_items_add(&(dinfo->input_items), read_input(text, gtk_text_buffer_get_line_count(buffer)), text);
    if (draw(dinfo) != 0) {
        // set error message
        gtk_statusbar_push(
            GTK_STATUSBAR(dinfo->statusbar), 
            gtk_statusbar_get_context_id(GTK_STATUSBAR(dinfo->statusbar), "Failed"),
            "Failed to parse"
        );
        free(input_items_pop(&(dinfo->input_items)));
        errmsg = gtk_message_dialog_new(GTK_WINDOW(dinfo->window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "Error parsing Logo.");
        g_signal_connect(errmsg, "response", G_CALLBACK(gtk_widget_destroy), NULL);
        gtk_widget_show(errmsg);
        // redraw without bad pic
        draw(dinfo);
        return -1;
    }

    // create a new message
    gtk_statusbar_push(
        GTK_STATUSBAR(dinfo->statusbar), 
        gtk_statusbar_get_context_id(GTK_STATUSBAR(dinfo->statusbar), "OK"),
        "OK"
    );
    // clear the text area
    clear_text(dinfo);
    update_tree_view(dinfo);
    gtk_widget_grab_focus(dinfo->text_view);
    return 0;
}

/**
 *  Create model for input history
 */
void update_tree_view(DInfo *dinfo) {
    GtkListStore *store;
    GtkTreeIter iter;
    int count, i;
    InputItems item;
    char *tok, *end;
    char str[HISTORY_STR_LENGTH] = "";
    char truncated_input[TRUNCATE_STR_LENGTH] = "";
    
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(dinfo->tree_view)));
    gtk_list_store_clear(store);
    count = 0;
    item = dinfo->input_items;
    while (item != NULL) {
        // copy the string of the input
        strncpy(truncated_input, item->buffer, TRUNCATE_STR_LENGTH-1);
        // null char
        truncated_input[TRUNCATE_STR_LENGTH-1] = '\0';
        tok = strtok(truncated_input, "\n");
        // this gets rid of the first open bracket
        tok = strtok(NULL, "\n");
        // because the gui allows whitespace, we skip all those first
        tok = trim_space(tok);
        // now truncate the tok
        end = tok + HISTORY_STR_LENGTH-5;
        *(end) = '\0';
        for (i=0; i<3; i++) {
            end = end - 1;
            *(end) = '.';
        }
        
        sprintf(str, "#%d:", count+1);
        strcat(str, tok);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 
            0, str,
            1, count,
        -1);
        count = count + 1;
        item = item->next;
    }
}

/**
 *  Clears the text editor
 */
void clear_text(DInfo *dinfo) {
    GtkTextIter iter;
    // set a new slate for writing on
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dinfo->text_view));
    gtk_text_buffer_set_text(buffer, "{\n\t\n}", 5);
    gtk_text_buffer_get_iter_at_line_offset(buffer, &iter, 1, 1);
    gtk_text_buffer_place_cursor(buffer, &iter);
}

/**
 *  Removes all statusbar messages
 */
void remove_messages(DInfo *dinfo) {
    gtk_statusbar_pop(
        GTK_STATUSBAR(dinfo->statusbar),
        gtk_statusbar_get_context_id(GTK_STATUSBAR(dinfo->statusbar), "OK")
    );
    gtk_statusbar_pop(
        GTK_STATUSBAR(dinfo->statusbar),
        gtk_statusbar_get_context_id(GTK_STATUSBAR(dinfo->statusbar), "Failed")
    );
}

/**
 *  Prepares a new drawing surface
 */
void new_file(DInfo *dinfo) {
    clear_text(dinfo);
    // paint the area white
    paint_white(dinfo);
    // free what was in the last edit
    free_input_items(dinfo->input_items);
    // remove status messages
    remove_messages(dinfo);
    dinfo->input_items = NULL;
    // text area has focus
    gtk_widget_grab_focus(dinfo->text_view);
    update_tree_view(dinfo);
}

/**
 *  Open the file and process it, putting into the text buffer and drawing it
 */
int open_file(DInfo *dinfo, char *filename) {
    FILE *in_file;
    char *buffer;
    long size;
    size_t result;
    GtkTextBuffer *text_buffer;
    
    in_file = fopen(filename, "r");
    if (in_file == NULL) {
        fprintf(stderr, "Error: cannot open file %s\n", filename);
        return -1;
    }
    
    fseek(in_file ,0 , SEEK_END);
    size = ftell(in_file);
    rewind(in_file);
    
    buffer = (char *) calloc(size, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Error: cannot allocate memory for buffer\n");
        fclose(in_file);
        return -1;
    }
    
    result = fread(buffer, 1, size, in_file);
    if (result != size) {
        fprintf(stderr, "Error: failed to read %s\n", filename);
        fclose(in_file);
        free(buffer);
        return -1;
    }

    // get a new surface
    new_file(dinfo);

    // set the buffer
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dinfo->text_view));
    gtk_text_buffer_set_text(text_buffer, buffer, size);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(dinfo->text_view), text_buffer);

    process_text_view(dinfo);
    
    fclose(in_file);
    free(buffer);
    
    return 0;
}
 
/**
 *  Read text from text_view and put it into a Logo struct.
 */
Logo read_input(char *buffer, int count) {
    Logo input;
    char *copy;
    char *tok;
    int i;

    if (strlen(buffer) == 0) {
        return NULL;
    } else {
        copy = (char *) calloc(strlen(buffer) + 1, sizeof(char));
        if (copy == NULL) {
            fprintf(stderr, "Error: cannot allocate memory to buffer\n");
            return NULL;
        }
    }
    
    /* malloc the data structure */
    input = (Logo) malloc(sizeof(*input));
    if (input == NULL) {
        return NULL;
    }
    input->lines = (char **) malloc(count * sizeof(char *));
    if (input->lines == NULL) {
        /* free input */
        free(input);
        return NULL;
    }
    for (i=0; i<count; i++) {
        input->lines[i] = (char *) malloc(LINE_LENGTH * sizeof(char));
        if (input->lines[i] == NULL) {
            /* free input and lines */
            free(input->lines);
            free(input);
            return NULL;
        }
        /* input made properly, memset it */
        memset(input->lines[i], '\0', LINE_LENGTH);
    }
    
    strncpy(copy, buffer, strlen(buffer));
    copy[strlen(buffer) + 1] = '\0';

    count = 0;
    tok = strtok(copy, "\n");
    while(tok != NULL) {
        // take away the \n in tok
        if (tok[strlen(tok)-1] == '\n') {
            tok[strlen(tok)-1] = '\0';
        }
        tok = trim_space(tok);
        // be a bit lenient here, if not whitespace...
        if (strlen(tok) > 0) {
            strcpy(input->lines[count], tok);
            count = count + 1;
        }
        tok = strtok(NULL, "\n");
    }
    input->num_lines = count;
    input->counter = 0;
    free(copy);
    return input;
}

/*
 *  Runs the interpreter on inputs in input_items->input_list
 */
int parse_items(InputItems input_items) {
    InputItems input;
    int ret;
    input = input_items;
    while(input != NULL) {
        if (input->input != NULL) {
            ret = parse(input->input);
            if (ret < 0) {
                // error parsing file
                return -1;
            }
            // reset it so it can be redrawn
            input->input->counter = 0;
        }
        input = input->next;
    }
    return 0;
}

/*
 *  LIFO list for managing input data
 */
int input_items_add(InputItems *input_items, Logo input, char *buffer) {
    /* create a new node first */
    InputItems node = malloc(sizeof(*node));
    InputItems iter, last;
    /* check that it is created correctly */
    if (node == NULL) {
        fprintf(stderr, "Error: cannot allocate memory for node\n");
        return -1;
    }
    node->input = input;
    node->buffer = (gchar *) malloc(strlen(buffer) * sizeof(gchar));
    if (node->buffer == NULL) {
        fprintf(stderr, "Error: cannot allocate memory for buffer\n");
        return -1;
    }
    node->buffer = buffer;
    node->next = NULL;
    iter = *input_items;
    last = NULL;
    // get the last node
    while(iter != NULL) {
        last = iter;
        iter = iter->next;
    }
    // if there is a last node, add node to the list
    if (last != NULL) {
        last->next = node;
    } else {
        // no last node, the node is the head
        *input_items = node;
    }
    return 0;
}

Logo input_items_pop(InputItems *input_items) {
    InputItems node, iter, last;
    Logo input = NULL;
    if (input_items == NULL) {
        return NULL;
    }
    iter = *input_items;
    node = NULL;
    while(iter != NULL) {
        last = node;
        node = iter;
        iter = iter->next;
    }
    // if there's a last item, set it's next to NULL
    if (last != NULL) {
        last->next = NULL;
    }
    // if there is a node, get the logo and free everything else
    if (node != NULL) {
        input = node->input;
        free(node->buffer);
        free(node);
    }
    // if there's no last item, the input_item now points to NULL
    if (last == NULL) {
        *input_items = NULL;
    }
    return input;
}

/*
 *  Frees input items and the input_list in it
 */
void free_input_items(InputItems input_items) {
    Logo input;
    while(input_items != NULL) {
        input = input_items_pop(&input_items);
        if (input != NULL) {
            free_logo(input);
        }
    }
}

/********************************************
 GUI Interpreter Backend
 ********************************************/

/**
 *  Draw a line depending on current orientation
 */ 
void gui_ipt_fd(Logo input, float op) {
    // some clever trigonometry here
    cairo_rel_line_to(_cr, _slider_factor*op*cos(_angle * TO_RADS), _slider_factor*op*sin(_angle * TO_RADS));
}

/**
 *  Turn _angle op degrees
 */ 
void gui_ipt_lt(Logo input, float op) {
    float div, times;
    _angle = (_angle - op);
    div = _angle / 360;
    times = floor(div);
    _angle = _angle - (360 * times);
}

/**
 *  Turn _angle -op degrees
 */ 
void gui_ipt_rt(Logo input, float op) {
    float div, times;
    _angle = (_angle + op);
    div = _angle / 360;
    times = floor(div);
    _angle = _angle - (360 * times);
}
