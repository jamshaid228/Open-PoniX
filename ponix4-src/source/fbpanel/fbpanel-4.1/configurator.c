

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

command commands[] = {
    { "configure", configure },
    { "restart", restart },
    { NULL, NULL },
};

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
static GtkWidget *tr_checkb,  *tr_colorl, *tr_colorb;;

//properties
static GtkWidget *prop_dt_checkb, *prop_st_checkb;

extern panel *p;
extern gchar *cprofile;
extern int config;
extern FILE *pconf;

void global_config_save(FILE *fp);
void plugin_config_save(FILE *fp);

static int
mk_profile_dir()
{
    gchar fname[1024];
    struct stat buf;
    int ret;

    ENTER;
    sprintf(fname, "%s/.fbpanel", getenv("HOME"));
    if ((ret = stat(fname, &buf))) {
        if (verbose)
            ERR("creating %s\n", fname);
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
    case GTK_RESPONSE_CANCEL:
        gtk_widget_hide(dialog);
        break;
    case GTK_RESPONSE_OK:
        config = 0;
        gtk_widget_hide(dialog);
    case GTK_RESPONSE_APPLY:
        if (!mk_profile_dir()) {
            ERR("can't make ~/.fbpanel direcory\n");
            RET();
        }
        sprintf(fname, "%s/.fbpanel/%s", getenv("HOME"), cprofile);
        if (verbose)
            ERR("saving profile %s to file %s\n", cprofile, fname);
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
set_edge(GtkWidget *item, gpointer bp)
{
    int edge;
    
    ENTER;    
    edge = gtk_option_menu_get_history(GTK_OPTION_MENU(item)) + 1;
    DBG("edge=%d\n", edge);    
    RET();
}


static void
set_allign(GtkWidget *item, gpointer bp)
{
    int allign;
    gboolean t;
    
    ENTER;	
    allign = gtk_option_menu_get_history(GTK_OPTION_MENU(item)) + 1;
    DBG("allign=%d\n", allign);
    t = (allign != ALLIGN_CENTER);
    gtk_widget_set_sensitive(margin_label, t);
    gtk_widget_set_sensitive(margin_spinb, t);    
    RET();
}


GtkWidget *
mk_position()
{
    GtkWidget *hbox, *hbox2, *label, *frame, *mi, *menu;
    GtkWidget *t;
   
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Position</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox2), 6);
    gtk_container_add (GTK_CONTAINER (frame), hbox2);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_set_size_request(hbox, 20, 1);
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
    
    menu = gtk_menu_new();
    
    mi = gtk_menu_item_new_with_label("Left");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("Right");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("Top");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("Bottom");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
  
    edge_opt = gtk_option_menu_new();
    gtk_widget_show(edge_opt);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(edge_opt), menu);
    g_signal_connect(G_OBJECT(edge_opt), "changed", G_CALLBACK(set_edge), NULL);

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
        
    menu = gtk_menu_new();
			
    mi = gtk_menu_item_new_with_label("Left");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("Center");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("Right");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    allign_opt = gtk_option_menu_new();
    gtk_widget_show(allign_opt);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(allign_opt), menu);
    g_signal_connect(G_OBJECT(allign_opt), "changed", G_CALLBACK(set_allign), NULL);

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
    
    widthtype = gtk_option_menu_get_history(GTK_OPTION_MENU(item)) + 1;
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
    GtkWidget *hbox, *hbox2, *label, *frame, *mi, *menu;
    GtkWidget *t;
  
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Size</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox2), 6);
    gtk_container_add (GTK_CONTAINER (frame), hbox2);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_set_size_request(hbox, 20, 1);
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
    
    menu = gtk_menu_new();
    
    mi = gtk_menu_item_new_with_label("dynamic");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("pixels");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    mi = gtk_menu_item_new_with_label("% of edge");
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    width_opt = gtk_option_menu_new();
    gtk_widget_show(width_opt);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(width_opt), menu);
    g_signal_connect(G_OBJECT(width_opt), "changed", G_CALLBACK(set_width), NULL);

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
    
    menu = gtk_menu_new();
			
    mi = gtk_menu_item_new_with_label("pixels");
    g_object_set_data(G_OBJECT(mi), "user-data", GUINT_TO_POINTER(HEIGHT_PIXEL));
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

    height_opt = gtk_option_menu_new();
    gtk_widget_show(height_opt);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(height_opt), menu);

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
    gtk_widget_set_sensitive(tr_colorl, t);
    gtk_widget_set_sensitive(tr_colorb, t);
    RET();
}

GtkWidget *
mk_transparency()
{
    GtkWidget *hbox, *hbox2, *label, *frame;
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Transparency</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox2), 6);
    gtk_container_add (GTK_CONTAINER (frame), hbox2);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox2), hbox, FALSE, TRUE, 0);
    gtk_widget_set_size_request(hbox, 20, 1);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX (hbox2), hbox, FALSE, TRUE, 0);

    tr_checkb = gtk_check_button_new_with_label("Enable Transparency");
    gtk_widget_show(tr_checkb);
    gtk_box_pack_start(GTK_BOX (hbox), tr_checkb, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(tr_checkb), "toggled", G_CALLBACK(transparency_toggle), NULL);
    //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tr_checkb), FALSE);
    
    tr_colorl = gtk_label_new("Tint color:");
    gtk_misc_set_alignment(GTK_MISC(tr_colorl), 0.0, 0.5);
    gtk_widget_show(tr_colorl);
    gtk_box_pack_start(GTK_BOX (hbox), tr_colorl, FALSE, FALSE, 5);
    //gtk_widget_set_sensitive(tr_colorl, FALSE);
    
    tr_colorb = gtk_color_button_new();
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(tr_colorb), TRUE);
    gtk_color_button_set_alpha (GTK_COLOR_BUTTON(tr_colorb), 65535/256*125);
    gtk_box_pack_start(GTK_BOX (hbox), tr_colorb, FALSE, FALSE, 0);
    //gtk_widget_set_sensitive(tr_colorb, FALSE);
     
    RET(frame);
}

GtkWidget *
mk_properties()
{
    GtkWidget *vbox, *hbox, *hbox2, *label, *frame;
   
    ENTER;
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL (label),"<span weight=\"bold\">Properties</span>");
    gtk_frame_set_label_widget(GTK_FRAME (frame), label);    

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame), hbox2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox2), 6);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox2), hbox, FALSE, TRUE, 0);
    gtk_widget_set_size_request(hbox, 20, 1);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (hbox2), vbox, FALSE, TRUE, 0);
    
    prop_dt_checkb = gtk_check_button_new_with_label("Set Dock Type");
    gtk_box_pack_start(GTK_BOX (vbox), prop_dt_checkb, FALSE, FALSE, 0);

    prop_st_checkb = gtk_check_button_new_with_label("Set Strut");
    gtk_box_pack_start(GTK_BOX (vbox), prop_st_checkb, FALSE, FALSE, 0);

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
mk_dialog()
{
    GtkWidget *sw, *frame, *page;
           
    DBG("creating dialog\n");
    dialog = gtk_dialog_new_with_buttons ("fbpanel configurator",
          NULL,
          0, //GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_STOCK_APPLY,
          GTK_RESPONSE_APPLY,
          GTK_STOCK_OK,
          GTK_RESPONSE_OK,
          GTK_STOCK_CANCEL,
          GTK_RESPONSE_CANCEL,
          NULL);
    DBG("connecting sugnal to %p\n",  dialog);
    g_signal_connect (G_OBJECT(dialog), "response",     (GCallback) response_event,       NULL);
    g_signal_connect (G_OBJECT(dialog), "destroy",      (GCallback) dialog_destroy_event, NULL);
    g_signal_connect (G_OBJECT(dialog), "delete_event", (GCallback) dialog_delete_event,  NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 400);

    //gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
    //gtk_window_set_skip_pager_hint(GTK_WINDOW(dialog), TRUE);

    
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), sw);
    
    page = gtk_vbox_new(FALSE, 1);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (sw), page);
    
    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    
    //position 
    frame = mk_position();
    DBG("here\n");
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
    //size 
    frame = mk_size();
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
    //transparency 
    frame = mk_transparency();
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
    //properties 
    frame = mk_properties();
    gtk_box_pack_start(GTK_BOX (page), frame, FALSE, TRUE, 0);
    
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
    i = gtk_option_menu_get_history(GTK_OPTION_MENU(w));
    if (i == ind) {
        i = i ? 0 : 1;
        gtk_option_menu_set_history(GTK_OPTION_MENU(w), i);
    }
    gtk_option_menu_set_history(GTK_OPTION_MENU(w), ind);
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
    gtk_adjustment_set_value(height_adj, p->height);

    update_toggle_button(tr_checkb, p->transparent);
    gtk_color_button_set_color(GTK_COLOR_BUTTON(tr_colorb), &p->gtintcolor);
    gtk_color_button_set_alpha (GTK_COLOR_BUTTON(tr_colorb), 256*p->alpha);
    gtk_widget_show(dialog);

    update_toggle_button(prop_dt_checkb, p->setdocktype);
    update_toggle_button(prop_st_checkb, p->setstrut);
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
          num2str(edge_pair, gtk_option_menu_get_history(GTK_OPTION_MENU(edge_opt)) + 1, "none"));
    fprintf(fp, "    allign = %s\n",
          num2str(allign_pair, gtk_option_menu_get_history(GTK_OPTION_MENU(allign_opt)) + 1, "none"));
    fprintf(fp, "    margin = %d\n", (int) margin_adj->value);
    fprintf(fp, "    widthtype = %s\n",
          num2str(width_pair, gtk_option_menu_get_history(GTK_OPTION_MENU(width_opt)) + 1, "none"));
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
    RET();
}

