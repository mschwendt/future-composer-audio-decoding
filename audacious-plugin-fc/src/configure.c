#include <audacious/configdb.h>
#include <audacious/plugin.h>
#include <gtk/gtk.h>

#include "configure.h"

FCpluginConfig fc_myConfig;

static void config_ok(GtkWidget *widget, gpointer data);

static GtkWidget *fc_config_window = NULL;

static gchar configSection[] = "FutureComposer";

static GtkWidget *Bits16;
static GtkWidget *Bits8;

static GtkWidget *Stereo;
static GtkWidget *Mono;

static GtkWidget *Sample_48;
static GtkWidget *Sample_44;
static GtkWidget *Sample_22;

static const gint FREQ_SAMPLE_48 = 48000;
static const gint FREQ_SAMPLE_44 = 44100;
static const gint FREQ_SAMPLE_22 = 22050;

void fc_ip_load_config()
{
    mcs_handle_t *cfg;

    fc_myConfig.frequency = FREQ_SAMPLE_44;
    fc_myConfig.precision = 8;
    fc_myConfig.channels = 1;
    
    if ((cfg = aud_cfg_db_open()))
    {
        aud_cfg_db_get_int(cfg, configSection, "frequency", &fc_myConfig.frequency);
        aud_cfg_db_get_int(cfg, configSection, "precision", &fc_myConfig.precision);
        aud_cfg_db_get_int(cfg, configSection, "channels", &fc_myConfig.channels);

        aud_cfg_db_close(cfg);
    }
}

void fc_ip_configure()
{
	GtkWidget *notebook1;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *bitsPerSample_Frame;
	GtkWidget *vbox4;
	GSList *bitsPerSample_group = NULL;
	GtkWidget *Channels_Frame;
	GtkWidget *vbox5;
	GSList *vbox5_group = NULL;
	GtkWidget *frequency_Frame;
	GtkWidget *vbox3;
	GSList *sample_group = NULL;
	GtkWidget *Quality_Label;
	GtkWidget *bbox;
	GtkWidget *ok;
	GtkWidget *cancel;

    if (!fc_config_window)
    {
        fc_config_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_type_hint(GTK_WINDOW(fc_config_window), GDK_WINDOW_TYPE_HINT_DIALOG);
        g_object_set_data(G_OBJECT(fc_config_window), "fc_config_window", fc_config_window);
        gtk_window_set_title(GTK_WINDOW(fc_config_window), "Future Composer player configuration");
        gtk_window_set_resizable(GTK_WINDOW(fc_config_window), FALSE);
        gtk_window_set_position(GTK_WINDOW(fc_config_window), GTK_WIN_POS_MOUSE);
        g_signal_connect(G_OBJECT(fc_config_window), "destroy", G_CALLBACK(gtk_widget_destroyed), &fc_config_window);
        gtk_container_set_border_width(GTK_CONTAINER(fc_config_window), 10);

		vbox = gtk_vbox_new(FALSE, 10);
		gtk_container_add(GTK_CONTAINER(fc_config_window), vbox);

		notebook1 = gtk_notebook_new();
		g_object_set_data(G_OBJECT(fc_config_window), "notebook1", notebook1);
		gtk_box_pack_start(GTK_BOX(vbox), notebook1, TRUE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(notebook1), 3);

		vbox1 = gtk_vbox_new(FALSE, 0);
		g_object_set_data(G_OBJECT(fc_config_window), "vbox1", vbox1);

		hbox1 = gtk_hbox_new(FALSE, 0);
		g_object_set_data(G_OBJECT(fc_config_window), "hbox1", hbox1);
		gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 0);

		bitsPerSample_Frame = gtk_frame_new("Bits per sample:");
		g_object_set_data(G_OBJECT(fc_config_window), "bitsPerSample_Frame", bitsPerSample_Frame);
		gtk_box_pack_start(GTK_BOX(hbox1), bitsPerSample_Frame, TRUE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(bitsPerSample_Frame), 5);

		vbox4 = gtk_vbox_new(FALSE, 0);
		g_object_set_data(G_OBJECT(fc_config_window), "vbox4", vbox4);
		gtk_container_add(GTK_CONTAINER(bitsPerSample_Frame), vbox4);

		Bits16 = gtk_radio_button_new_with_label(bitsPerSample_group, "16 bit");
		bitsPerSample_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Bits16));
		g_object_set_data(G_OBJECT(fc_config_window), "Bits16", Bits16);
		gtk_box_pack_start(GTK_BOX(vbox4), Bits16, TRUE, TRUE, 0);
		if (fc_myConfig.precision == 16)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Bits16), TRUE);

		Bits8 = gtk_radio_button_new_with_label(bitsPerSample_group, "8 bit");
		bitsPerSample_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Bits8));
		g_object_set_data(G_OBJECT(fc_config_window), "Bits8", Bits8);
		gtk_box_pack_start(GTK_BOX(vbox4), Bits8, TRUE, TRUE, 0);
		if (fc_myConfig.precision == 8)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Bits8), TRUE);

		Channels_Frame = gtk_frame_new("Channels:");
		g_object_set_data(G_OBJECT(fc_config_window), "Channels_Frame", Channels_Frame);
		gtk_box_pack_start(GTK_BOX(hbox1), Channels_Frame, TRUE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(Channels_Frame), 5);

		vbox5 = gtk_vbox_new(FALSE, 0);
		g_object_set_data(G_OBJECT(fc_config_window), "vbox5", vbox5);
		gtk_container_add(GTK_CONTAINER(Channels_Frame), vbox5);

		Stereo = gtk_radio_button_new_with_label(vbox5_group, "Stereo");
		vbox5_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Stereo));
		g_object_set_data(G_OBJECT(fc_config_window), "Stereo", Stereo);
		gtk_box_pack_start(GTK_BOX(vbox5), Stereo, TRUE, TRUE, 0);
		if (fc_myConfig.channels == 2)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Stereo), TRUE);

		Mono = gtk_radio_button_new_with_label(vbox5_group, "Mono");
		vbox5_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Mono));
		g_object_set_data(G_OBJECT(fc_config_window), "Mono", Mono);
		gtk_box_pack_start(GTK_BOX(vbox5), Mono, TRUE, TRUE, 0);
		if (fc_myConfig.channels == 1)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Mono), TRUE);

		frequency_Frame = gtk_frame_new("Sample frequency:");
		g_object_set_data(G_OBJECT(fc_config_window), "Frequency_Frame", frequency_Frame);
		gtk_box_pack_start(GTK_BOX(vbox1), frequency_Frame, TRUE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(frequency_Frame), 5);

		vbox3 = gtk_vbox_new(FALSE, 0);
		g_object_set_data(G_OBJECT(fc_config_window), "vbox3", vbox3);
		gtk_container_add(GTK_CONTAINER(frequency_Frame), vbox3);

        Sample_48 = gtk_radio_button_new_with_label(sample_group, "48000 Hz");
		sample_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Sample_48));
		g_object_set_data(G_OBJECT(fc_config_window), "Sample_48", Sample_48);
		gtk_box_pack_start(GTK_BOX(vbox3), Sample_48, TRUE, TRUE, 0);
		if (fc_myConfig.frequency == FREQ_SAMPLE_48)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Sample_48), TRUE);

		Sample_44 = gtk_radio_button_new_with_label(sample_group, "44100 Hz");
		sample_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Sample_44));
		g_object_set_data(G_OBJECT(fc_config_window), "Sample_44", Sample_44);
		gtk_box_pack_start(GTK_BOX(vbox3), Sample_44, TRUE, TRUE, 0);
		if (fc_myConfig.frequency == FREQ_SAMPLE_44)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Sample_44), TRUE);

		Sample_22 = gtk_radio_button_new_with_label(sample_group, "22050 Hz");
		sample_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(Sample_22));
		g_object_set_data(G_OBJECT(fc_config_window), "Sample_22", Sample_22);
		gtk_box_pack_start(GTK_BOX(vbox3), Sample_22, TRUE, TRUE, 0);
		if (fc_myConfig.frequency == FREQ_SAMPLE_22)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(Sample_22), TRUE);

		Quality_Label = gtk_label_new("Quality");
		g_object_set_data(G_OBJECT(fc_config_window), "Quality_Label", Quality_Label);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook1), vbox1, Quality_Label);

		bbox = gtk_hbutton_box_new();
		gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
		gtk_box_set_spacing(GTK_BOX(bbox), 5);
		gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

		ok = gtk_button_new_with_label("Ok");
		g_signal_connect(G_OBJECT(ok), "clicked", G_CALLBACK(config_ok), NULL);
        gtk_widget_set_can_default(ok, TRUE);
		gtk_box_pack_start(GTK_BOX(bbox), ok, TRUE, TRUE, 0);
		gtk_widget_grab_default(ok);

		cancel = gtk_button_new_with_label("Cancel");
		g_signal_connect_swapped(G_OBJECT(cancel), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(fc_config_window));
        gtk_widget_set_can_default(cancel, TRUE);
		gtk_box_pack_start(GTK_BOX(bbox), cancel, TRUE, TRUE, 0);

		gtk_widget_show_all(fc_config_window);
	}
	else
	{
		gdk_window_raise(fc_config_window->window);
	}

}

static void config_ok(GtkWidget * widget, gpointer data)
{
    mcs_handle_t *cfg;

	if (GTK_TOGGLE_BUTTON(Bits16)->active)
		fc_myConfig.precision = 16;
	if (GTK_TOGGLE_BUTTON(Bits8)->active)
		fc_myConfig.precision = 8;

	if (GTK_TOGGLE_BUTTON(Stereo)->active)
		fc_myConfig.channels = 2;
	if (GTK_TOGGLE_BUTTON(Mono)->active)
		fc_myConfig.channels = 1;

	if (GTK_TOGGLE_BUTTON(Sample_48)->active)
		fc_myConfig.frequency = FREQ_SAMPLE_48;
	if (GTK_TOGGLE_BUTTON(Sample_44)->active)
		fc_myConfig.frequency = FREQ_SAMPLE_44;
	if (GTK_TOGGLE_BUTTON(Sample_22)->active)
		fc_myConfig.frequency = FREQ_SAMPLE_22;

	if ((cfg = aud_cfg_db_open()))
    {
        aud_cfg_db_set_int(cfg, configSection, "frequency", fc_myConfig.frequency);
        aud_cfg_db_set_int(cfg, configSection, "precision", fc_myConfig.precision);
        aud_cfg_db_set_int(cfg, configSection, "channels", fc_myConfig.channels);
        aud_cfg_db_close(cfg);
    }
	gtk_widget_destroy(fc_config_window);
}
