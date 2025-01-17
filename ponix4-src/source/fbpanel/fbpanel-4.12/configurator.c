

#include "plugin.h"
#include "panel.h"
#include "misc.h"
#include "bg.h"
#include "version.h"
#include "gtkbgbox.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

//#define DEBUG
#include "dbg.h"

void configure(void);
void restart(void);
void quit(void);

command commands[] = {
    { "configure", configure },
    { "restart", restart },
    { "quit", quit },
    { NULL, NULL },
};


#define FRAME_BORDER   6
#define INDENT_SIZE    20


static GtkWidget *dialog = NULL;
static GtkSizeGroup *sg;

//width
static GtkWidget *width_spinb, *width_opt;
static GtkAdjustment *width_adj;

//height
static GtkWidget *height_spinb, *height_opt;
static GtkAdjustment *height_adj;

//margin
static GtkWidget *margin_label, *margin_spinb;
static GtkAdjustment *margin_adj;

//allign
static GtkWidget *allign_opt;

//edge
static GtkWidget *edge_opt;

//transparency
static GtkWidget *tr_checkb, *tr_colorb;;
static GtkWidget *tr_box;

//properties
static GtkWidget *prop_dt_checkb, *prop_st_checkb, *prop_autohide_checkb;
static GtkWidget *height_when_hidden_box, *height_when_hidden_spinb;
static GtkAdjustment *height_when_hidden_adj;


extern panel *p;
extern gchar *cprofile;
extern int config;
extern FILE *pconf;

void global_config_save(FILE *fp);
void plugin_config_save(FILE *fp);

static void
add_hindent_box(GtkWidget *box)
{
    GtkWidget  *indent_box;
    
    indent_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (box), indent_box, FALSE, TRUE, 0);
    gtk_widget_set_size_request(indent_box, INDENT_SIZE, 1);
}



static int
mk_profile_dir()
{
    gchar fname[1024];
    struct stat buf;
    int ret;

    ENTER;
    sprintf(fname, "%s/.fbpanel", getenv("HOME"));
    if ((ret = stat(fname, &buf))) {
        LOG(LOG_INFO, "creating %s\n", fname);
        mkdir(fname, 0755);
        ret = stat(fname, &buf);
    }
    if (ret)
        RET(0);
    if (!(S_ISDIR(buf.st_mode) && (S_IWUSR & buf.st_mode) && (S_IXUSR & buf.st_mode)))
        RET(0);
    RET(1);
}


static void
response_event(GtkDialog *widget, gint arg1, gpointer user_data)
{
    gchar fname[1024];
    FILE *fp;
      
    ENTER;
    switch (arg1) {
    case GTK_RESPONSE_DELETE_EVENT:
        DBG("GTK_RESPONSE_DELETE_EVENT\n");
        gtk_widget_destroy(dialog);
        dialog = NULL;
        break;
    case GTK_RESPONSE_CLOSE:
        gtk_widget_hide(dialog);
        break;
    case GTK_RESPONSE_APPLY:
        if (!mk_profile_dir()) {
            ERR("can't make ~/.fbpanel direcory\n");
            RET();
        }
        sprintf(fname, "%s/.fbpanel/%s", getenv("HOME"), cprofile);
        LOG(LOG_WARN, "saving profile %s as %s\n", cprofile, fname);
        if (!(fp = fopen(fname, "w"))) {
            ERR("can't open for write %s:", fname);
            perror(NULL);
            RET();
        }
        global_config_save(fp);
        plugin_config_save(fp);
        fclose(fp);
        //sprintf(fname, "cat %s/.fbpanel/confplug >> %s/.fbpanel/%s", getenv("HOME"), getenv("HOME"), cprofile);
        //system(fname);
        gtk_main_quit();
        break;
    }
    RET();
}

static void
set_edge(GtkComboBox *widget, gpointer bp)
{
    int edge;
    
    ENTER;    
    edge = gtk_combo_box_get_active(widget) + 1;
    DBG("edge=%d\n", edge);    
    RET();
}


static void
set_allign(GtkComboBox *widget, gpointer bp)
{
    int allign;
    gboolean t;
    
    ENTER;	
    allign = gtk_combo_box_get_active(widget) + 1;
    DBG("allign=%d\n", allign);
    t = (allign != ALLIGN_CENTER);
    gtk_widget_set_sensitive(margin_label, t);
    gtk_widget_set_sensitive(margin_spinb, t);    
    RET();
}


GtkWidget *
mk_position()
{
    GtkWidget *hbox, *hbox2, *label, *frame;
    GtkWidget *t;
   
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), FRAME_BORDER);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Position</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox2), 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox2);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_set_size_request(hbox, INDENT_SIZE, 1);
    gtk_box_pack_start(GTK_BOX (hbox2), hbox, FALSE, TRUE, 0);
    
    t = gtk_table_new(5, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(t), 3);
    gtk_table_set_col_spacings(GTK_TABLE(t), 5);
    gtk_box_pack_start(GTK_BOX (hbox2), t, FALSE, TRUE, 0);
    
    //Edge
    label = gtk_label_new("Edge:");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_widget_show(label);
    gtk_table_attach(GTK_TABLE(t), label, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
    gtk_size_group_add_widget(sg, label);
    
    edge_opt = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(edge_opt), "Left");
    gtk_combo_box_append_text(GTK_COMBO_BOX(edge_opt), "Right");
    gtk_combo_box_append_text(GTK_COMBO_BOX(edge_opt), "Top");
    gtk_combo_box_append_text(GTK_COMBO_BOX(edge_opt), "Bottom");
    g_signal_connect(G_OBJECT(edge_opt), "changed", G_CALLBACK(set_edge), NULL);
    
    gtk_widget_show(edge_opt);
  
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), edge_opt, FALSE, TRUE, 0);
    gtk_table_attach(GTK_TABLE(t), hbox, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), edge_opt);

    //Allignment
    label = gtk_label_new("Allignment:");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    
    gtk_table_attach(GTK_TABLE(t), label, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
        
    allign_opt = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(allign_opt), "Left");
    gtk_combo_box_append_text(GTK_COMBO_BOX(allign_opt), "Center");
    gtk_combo_box_append_text(GTK_COMBO_BOX(allign_opt), "Right");
    g_signal_connect(G_OBJECT(allign_opt), "changed", G_CALLBACK(set_allign), NULL);
    gtk_widget_show(allign_opt);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), allign_opt, FALSE, TRUE, 0);
    gtk_table_attach(GTK_TABLE(t), hbox, 1, 2, 1, 2, GTK_FILL, 0, 0, 0);                
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), allign_opt);


    //Margin
    margin_label = gtk_label_new("Margin:");
    gtk_misc_set_alignment(GTK_MISC(margin_label), 0.0, 0.5);
    gtk_widget_show(margin_label);

    gtk_table_attach(GTK_TABLE(t), margin_label, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);

    margin_adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, gdk_screen_width(), 1.0, 5.0, 5.0);
    margin_spinb = gtk_spin_button_new (margin_adj, 1.0, 0);
    gtk_table_attach(GTK_TABLE(t), margin_spinb, 3, 4, 1, 2, GTK_FILL, 0, 0, 0);
    gtk_table_set_col_spacing(GTK_TABLE(t), 1, 20);

    RET(frame);
}


static void
set_width(GtkWidget *item, gpointer bp)
{
    int widthtype;
    gboolean t;
    
    ENTER;
    
    widthtype = gtk_combo_box_get_active(GTK_COMBO_BOX(item)) + 1;
    DBG("widthtype=%d\n", widthtype);
    t = (widthtype != WIDTH_REQUEST);
    gtk_widget_set_sensitive(width_spinb, t);
    if (widthtype == WIDTH_PERCENT) {
        width_adj->upper = 100;
        width_adj->value = width_adj->upper;
    } else if  (widthtype == WIDTH_PIXEL) {
        width_adj->upper = gdk_screen_width();
        width_adj->value = width_adj->upper;
    } else
        RET();
    
    gtk_adjustment_changed(width_adj);
    gtk_adjustment_value_changed(width_adj);
    
    RET();
}


GtkWidget *
mk_size()
{
    GtkWidget *hbox, *hbox2, *label, *frame;
    GtkWidget *t;
  
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), FRAME_BORDER);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Size</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox2), 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox2);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_set_size_request(hbox, INDENT_SIZE, 1);
    gtk_box_pack_start(GTK_BOX (hbox2), hbox, FALSE, TRUE, 0);
    
    t = gtk_table_new(3, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(t), 3);
    gtk_table_set_col_spacings(GTK_TABLE(t), 5);
    gtk_box_pack_start(GTK_BOX (hbox2), t, FALSE, TRUE, 0);
    
    //width
    label = gtk_label_new("Width:");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_widget_show(label);
    gtk_table_attach(GTK_TABLE(t), label, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
    gtk_size_group_add_widget(sg, label);
    
    width_adj = (GtkAdjustment *) gtk_adjustment_new (20.0, 0.0, 2100.0, 1.0, 5.0, 5.0);
    width_spinb = gtk_spin_button_new (width_adj, 1.0, 0);
    gtk_table_attach(GTK_TABLE(t), width_spinb, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
    

    width_opt = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(width_opt), "dynamic");
    gtk_combo_box_append_text(GTK_COMBO_BOX(width_opt), "pixels");
    gtk_combo_box_append_text(GTK_COMBO_BOX(width_opt), "% of edge");
    g_signal_connect(G_OBJECT(width_opt), "changed", G_CALLBACK(set_width), NULL);
    gtk_widget_show(width_opt);

    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), width_opt, FALSE, TRUE, 0);
    gtk_table_attach(GTK_TABLE(t), hbox, 2, 3, 0, 1, GTK_FILL, 0, 0, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), width_opt);


    //height
    label = gtk_label_new("Height:");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_widget_show(label);
    gtk_table_attach(GTK_TABLE(t), label, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
    gtk_size_group_add_widget(sg, label);
    
    height_adj = (GtkAdjustment *) gtk_adjustment_new (30.0, 0.0, 300.0, 1.0, 5.0, 5.0);
    height_spinb = gtk_spin_button_new (height_adj, 1.0, 0);
    gtk_table_attach(GTK_TABLE(t), height_spinb, 1, 2, 1, 2, GTK_FILL, 0, 0, 0);
    
    
    height_opt = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(height_opt), "pixels");
    //g_signal_connect(G_OBJECT(height_opt), "changed", G_CALLBACK(set_height), NULL);
    gtk_widget_show(height_opt);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), height_opt, FALSE, TRUE, 0);
    gtk_table_attach(GTK_TABLE(t), hbox, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), height_opt);

    RET(frame);
}

static void
transparency_toggle(GtkWidget *b, gpointer bp)
{
    gboolean t;

    ENTER;
    t = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b));
    gtk_widget_set_sensitive(tr_box, t);
    RET();
}

GtkWidget *
mk_effects()
{
    GtkWidget *hbox, *label, *frame, *vbox;
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), FRAME_BORDER);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Effects</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox);
    
    add_hindent_box(hbox);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), vbox, FALSE, TRUE, 0);

    
    tr_checkb = gtk_check_button_new_with_label("Transparency");
    gtk_box_pack_start(GTK_BOX (vbox), tr_checkb, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(tr_checkb), "toggled", G_CALLBACK(transparency_toggle), NULL);

    tr_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox), tr_box, FALSE, FALSE, 0);

    add_hindent_box(tr_box);
    
    label = gtk_label_new("Color settings:");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX (tr_box), label, FALSE, FALSE, 0);
    
    tr_colorb = gtk_color_button_new();
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(tr_colorb), TRUE);
    gtk_color_button_set_alpha (GTK_COLOR_BUTTON(tr_colorb), 65535/256*125);
    gtk_box_pack_start(GTK_BOX (tr_box), tr_colorb, FALSE, FALSE, 5);
    //gtk_widget_set_sensitive(tr_colorb, FALSE);
     
    RET(frame);
}

static void
autohide_toggle(GtkWidget *b, gpointer bp)
{
    gboolean t;

    ENTER;
    t = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b));
    gtk_widget_set_sensitive(height_when_hidden_box, t);
    RET();
}

GtkWidget *
mk_properties()
{
    GtkWidget *vbox, *hbox, *label, *frame;
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), FRAME_BORDER);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Properties</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox);

    add_hindent_box(hbox);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox), vbox, FALSE, TRUE, 0);
    
    prop_dt_checkb = gtk_check_button_new_with_label("Set Dock Type");
    gtk_box_pack_start(GTK_BOX (vbox), prop_dt_checkb, FALSE, FALSE, 0);

    prop_st_checkb = gtk_check_button_new_with_label("Do not cover by maximized windows");
    gtk_box_pack_start(GTK_BOX (vbox), prop_st_checkb, FALSE, FALSE, 0);

    prop_autohide_checkb = gtk_check_button_new_with_label("Autohide");
    gtk_box_pack_start(GTK_BOX (vbox), prop_autohide_checkb, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(prop_autohide_checkb), "toggled", G_CALLBACK(autohide_toggle), NULL);

    height_when_hidden_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (vbox), height_when_hidden_box, FALSE, FALSE, 0);

    add_hindent_box(height_when_hidden_box);
    
    label = gtk_label_new("Height when hidden");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX (height_when_hidden_box), label, FALSE, TRUE, 0);

    height_when_hidden_adj = (GtkAdjustment *) gtk_adjustment_new (2.0, 1.0, 10.0, 1.0, 1.0, 1.0);
    height_when_hidden_spinb = gtk_spin_button_new (height_when_hidden_adj, 1.0, 0);
    gtk_box_pack_start(GTK_BOX (height_when_hidden_box), height_when_hidden_spinb, FALSE, TRUE, 5);

    label = gtk_label_new("pixels");
    gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX (height_when_hidden_box), label, FALSE, TRUE, 0);

    
    RET(frame);
}

static void
dialog_destroy_event(GtkWidget * widget, GdkEvent * event, gpointer data)
{
    ENTER;  
    dialog = NULL;
    RET();
}

static gint
dialog_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{

    ENTER;  
    //if (!p->self_destroy)
    RET(FALSE);
}

static GtkWidget *
mk_tab_plugins()
{
    GtkWidget *hbox, *label;
    gchar *msg;
    
    hbox = gtk_vbox_new(FALSE, 0);
    msg = g_strdup_printf("Graphical plugin configuration is not implemented yet.\n"
          "Please edit manually\n\t~/.fbpanel/%s\n\n"
          "You can use as example files in \n\t%s/share/fbpanel/\n"
          "or visit\n"
          "\thttp://fbpanel.sourceforge.net/docs.html", cprofile, PREFIX);
    label = gtk_label_new(msg);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    gtk_box_pack_end(GTK_BOX(hbox), label, TRUE, TRUE, 5);
    g_free(msg);
    
    RET(hbox);
}

static GtkWidget *
mk_tab_general()
{
    GtkWidget *frame, *page;

    /*
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(sw), 0);
    */
    page = gtk_vbox_new(FALSE, 1);
        
    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    
    //position 
    frame = mk_position();
    DBG("here\n");
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
    //size 
    frame = mk_size();
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
    //effects 
    frame = mk_effects();
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
    //properties 
    frame = mk_properties();
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    /*
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (sw), page);
    */
    RET(page);
}


static GtkWidget *
mk_dialog()
{
    GtkWidget *sw, *nb, *label;
           
    DBG("creating dialog\n");
    dialog = gtk_dialog_new_with_buttons ("fbpanel configurator",
          NULL,
          0, //GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_STOCK_APPLY,
          GTK_RESPONSE_APPLY,
          GTK_STOCK_CLOSE,
          GTK_RESPONSE_CLOSE,
          NULL);
    DBG("connecting sugnal to %p\n",  dialog);
    g_signal_connect (G_OBJECT(dialog), "response",     (GCallback) response_event,       NULL);
    g_signal_connect (G_OBJECT(dialog), "destroy",      (GCallback) dialog_destroy_event, NULL);
    g_signal_connect (G_OBJECT(dialog), "delete_event", (GCallback) dialog_delete_event,  NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);

    //gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
    //gtk_window_set_skip_pager_hint(GTK_WINDOW(dialog), TRUE);
      
    nb = gtk_notebook_new();
    gtk_notebook_set_show_border (GTK_NOTEBOOK(nb), FALSE);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), nb);

    sw = mk_tab_general();
    label = gtk_label_new("General");
    gtk_misc_set_padding(GTK_MISC(label), 4, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), sw, label);

    sw = mk_tab_plugins();
    label = gtk_label_new("Plugins");
    gtk_misc_set_padding(GTK_MISC(label), 4, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), sw, label);
    
    g_object_unref(sg);
    
    //gtk_widget_show_all(page);
    gtk_widget_show_all(dialog);
    
    RET(dialog);
}

static void
update_opt_menu(GtkWidget *w, int ind)
{
    int i;

    ENTER;
    /* this trick will trigger "changed" signal even if active entry is
     * not actually changing */
    i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    if (i == ind) {
        i = i ? 0 : 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(w), i);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), ind);
    RET();
}

static void
update_toggle_button(GtkWidget *w, gboolean n)
{
    gboolean c;

    ENTER;
    /* this trick will trigger "changed" signal even if active entry is
     * not actually changing */
    c = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    if (c == n) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), !n);
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), n);
    RET();
}




void
configure(void)
{
    ENTER;
    DBG("dialog %p\n",  dialog);
    if (!dialog) 
        dialog = mk_dialog();
    gtk_widget_show(dialog);

    update_opt_menu(edge_opt, p->edge - 1);
    update_opt_menu(allign_opt, p->allign - 1);
    //gtk_adjustment_set_value(margin_adj, p->margin);
    //gtk_adjustment_value_changed(margin_adj);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(margin_spinb), p->margin);
    
    update_opt_menu(width_opt, p->widthtype - 1);
    gtk_adjustment_set_value(width_adj, p->width);
    update_opt_menu(height_opt, HEIGHT_PIXEL - 1);
    gtk_adjustment_set_value(height_adj, p->height);

    update_toggle_button(tr_checkb, p->transparent);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(tr_colorb), &p->gtintcolor);
    gtk_color_button_set_alpha (GTK_COLOR_BUTTON(tr_colorb), 256*p->alpha);
    gtk_widget_show(dialog);

    update_toggle_button(prop_dt_checkb, p->setdocktype);
    update_toggle_button(prop_st_checkb, p->setstrut);
    update_toggle_button(prop_autohide_checkb, p->autohide);
    gtk_adjustment_set_value(height_when_hidden_adj, p->height_when_hidden);
    RET();
}

void
global_config_save(FILE *fp)
{
    GdkColor c;
    
    fprintf(fp, "# fbpanel <profile> config file\n");
    fprintf(fp, "# see http://fbpanel.sf.net/docs.html for complete configuration guide\n");
    fprintf(fp, "\n\n");
    fprintf(fp, "Global {\n");
    fprintf(fp, "    edge = %s\n",
          num2str(edge_pair, gtk_combo_box_get_active(GTK_COMBO_BOX(edge_opt)) + 1, "none"));
    fprintf(fp, "    allign = %s\n",
          num2str(allign_pair, gtk_combo_box_get_active(GTK_COMBO_BOX(allign_opt)) + 1, "none"));
    fprintf(fp, "    margin = %d\n", (int) margin_adj->value);
    fprintf(fp, "    widthtype = %s\n",
          num2str(width_pair, gtk_combo_box_get_active(GTK_COMBO_BOX(width_opt)) + 1, "none"));
    fprintf(fp, "    width = %d\n", (int) width_adj->value);
    fprintf(fp, "    height = %d\n", (int) height_adj->value);
    fprintf(fp, "    transparent = %s\n",
          num2str(bool_pair, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tr_checkb)), "false"));
    gtk_color_button_get_color(GTK_COLOR_BUTTON(tr_colorb), &c);
    fprintf(fp, "    tintcolor = #%06x\n", gcolor2rgb24(&c));
    fprintf(fp, "    alpha = %d\n", gtk_color_button_get_alpha(GTK_COLOR_BUTTON(tr_colorb)) * 0xff / 0xffff);
    fprintf(fp, "    setdocktype = %s\n",
          num2str(bool_pair, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prop_dt_checkb)), "true"));
    fprintf(fp, "    setpartialstrut = %s\n",
          num2str(bool_pair, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prop_st_checkb)), "true"));
    fprintf(fp, "    autohide = %s\n",
          num2str(bool_pair, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(prop_autohide_checkb)), "false"));
    fprintf(fp, "    heightWhenHidden = %d\n", (int) height_when_hidden_adj->value);

    fprintf(fp, "}\n\n");
}

#define TAB_WIDTH 4
#define STR_LEN  254
void
plugin_config_save(FILE *fp)
{
    char space[STR_LEN];
    line s;
    int i = 0;
    fseek(pconf, 0, SEEK_SET);
    /*
    while (fgets(s, 254, pconf))
        fprintf(fp, "%s\n", s);
    */
    memset(space, ' ', STR_LEN);
    space[STR_LEN -1] = 0;
    s.len = 256;
    while (get_line(pconf, &s) != LINE_NONE) {
        switch (s.type) {
        case LINE_BLOCK_START:
            space[i*TAB_WIDTH] = 0;
            fprintf(fp, "%s%s {\n", space, s.t[0]);
            space[i*TAB_WIDTH] = ' ';
            i++;
            //just for the case
            if (i > STR_LEN/TAB_WIDTH) {
                i = STR_LEN/TAB_WIDTH;
                ERR("too long line in config file\n");
            }
            break;
        case LINE_BLOCK_END:
            i--;
            //just for the case
            if (i < 0) {
                ERR("unbalansed parenthesis in config file\n");
                i = 0;
            }
            space[i*TAB_WIDTH] = 0;
            fprintf(fp, "%s}\n", space);
            space[i*TAB_WIDTH] = ' ';
            if (!i)
                fprintf(fp, "\n\n");
            break;

        case LINE_VAR:
            space[i*TAB_WIDTH] = 0;
            fprintf(fp, "%s%s = %s\n", space, s.t[0], s.t[1]);
            space[i*TAB_WIDTH] = ' ';
            break;
        }
    }
    
}


void
restart(void)
{
    ENTER;
    gtk_main_quit();
    RET();
}


void
quit(void)
{
    ENTER;
    gtk_main_quit();
    force_quit = 1;
    RET();
}

