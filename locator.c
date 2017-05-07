#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

//#define DEBUG 

#define PROGRAM "LOCATOR"
#define VERSION  "v 1.2b GTK"
#define COPYRIGHT "2012-2017 GPL"
#define COMMENT "Locator calculate the distance and bearing between different locator formats (QRA/WW-Locator/WW-Locator with microfield)."
#define URI "https://github.com/andi-f/locator"


typedef struct {
	GtkWidget *locator_1;
	GtkWidget *locator_2;
	GtkWidget *distance;
	GtkWidget *bearing;
} mainhead_data;

void qra_grad( char *str, double *b, double *l)
{
	unsigned char s;
	float l_g,b_g,b_min,l_min;

	s = toupper(*str);
	l_g = (s-65)*2;
	if(l_g > 44 )
	{
		l_g -= 52;
	}
	str ++;

	s = toupper(*str);
	b_g = (s-65) + 40;
	if(b_g > 73 )
	{
		b_g -= (s-65);
	}
	str ++;

	s = (toupper(*str)-48);
	b_min = 60 - (int)(s) * 7.5;
	str ++;


	s = (toupper(*str)-48) - 1;
	l_min = s * 12.0;
	if(l_min >= 60)
	{
		l_g = l_g + 1;
		l_min -= 60;
	}
    str ++;

	s = toupper(*str);
	switch(s)
	{
		case 'H':             b_min += 5; break;
		case 'A': l_min += 4; b_min += 5; break;
		case 'B': l_min += 8; b_min += 5; break;

		case 'G':             b_min += 2.5; break;
		case 'J': l_min += 4; b_min += 2.5; break;
		case 'C': l_min += 8; b_min += 2.5; break;

		case 'F':            break;
		case 'E': l_min += 4; break;
		case 'D': l_min += 8; break;
	}

	if(l_min >= 60)
	{
		l_g = l_g + 1;
		l_min -= 60;
	}

	*l = l_g + l_min/60.0;
	*b = b_g + b_min/60.0;
}

void loc_grad( char *str, double *b, double *l)
{
	unsigned char s;
	float l_g,b_g;
	size_t length;
	
	length = strlen(str);

	s = toupper(str[0]);
	l_g = (s-74) * 20;

	s = toupper(str[1]);
	b_g = (s-74) * 10;

    s = str[2];
	l_g = l_g + (s-48)*2;

    s = str[3];
	b_g = b_g + (s-48);

	s = toupper(str[4]);
	if (s > 76)
	{
		l_g = l_g + 1.0 + (s-77) / 12.0;
	}
	else if (s <= 76)
	{
		l_g = l_g + (s-65) / 12.0;
	}
	s = toupper(str[5]);
	b_g = b_g + (s-65) / 24.0;

	//microfield
	if (length == 8)
	{
		s = str[6];
		l_g = l_g + (s-48) * 30.0 / 3600 ;
		
#ifdef DEBUG
		g_print("microfield 1 %c \n\r", s);
		g_print("microfield 1 %f \n\r", (s-48) * 30.0 / 3600.0);
#endif
		s = str[7];
		b_g = b_g + (s-48) * 15.0 / 3600;
		
#ifdef DEBUG
		g_print("microfield 2 %c \n\r", s);
		g_print("microfield 2 %f \n\r", (s-48) * 15.0 / 3600.0);
#endif
		
	}

	*l = l_g;
	*b = b_g;
}

void calculate(GtkWidget *widget, mainhead_data *data)
{
	size_t length_l1;
	size_t length_l2;
	char qrb[10];
	char qtf[10];
	double bs,ls,be,le;	
	double ent;		
	double az;		

	gchar *input_1 = g_malloc (sizeof (gchar) * 10);
	gchar *input_2 = g_malloc (sizeof (gchar) * 10);

	bs = 0;
	ls = 0;
	be = 0;
	le = 0;
		
	input_1 = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->locator_1)));
	input_2 = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->locator_2)));
	
#ifdef DEBUG
	g_print("LOC 1 %s \n\r", input_1);
	g_print("LOC 2 %s \n\r", input_2);
#endif
	
	length_l1 =strlen(input_1);
	length_l2 =strlen(input_2);

		
	if(length_l1 == 6) {
		loc_grad(input_1,&bs,&ls);		
	}
	else
	if(length_l1 == 8) {
		loc_grad(input_1,&bs,&ls);		
	}
	else
	if(length_l1 == 5) {
		qra_grad(input_1,&bs,&ls);		
	}


	if (length_l2 == 6) {
		loc_grad(input_2,&be,&le);
	}
	else
	if (length_l2 == 8) {
		loc_grad(input_2,&be,&le);
	}
	else	
	if (length_l2 == 5) {
		qra_grad(input_2,&be,&le);
	}
	
#ifdef DEBUG	
	g_print("ls %f° bs %f° le %f° be %f°\n\r", ls,bs,le,be);
#endif
	
	ls = ls / 180 * M_PI;
	bs = bs / 180 * M_PI;
	le = le / 180 * M_PI;
	be = be / 180 * M_PI;

	ent = acos(sin(be)*sin(bs) + cos(be)*cos(bs)*cos(le-ls));
		
#ifdef DEBUG		
	g_print("ls %f bs %f le %f be %f\n\r", ls,bs,le,be);

	g_print("QRB: %5.1f km ",ent*6371);
#endif
	sprintf(qrb,"%5.1f",ent*6371);
		
	gtk_label_set_text(GTK_LABEL(data->distance),qrb); 
		
	if( ent != 0.0)
	{
		az  = sin( be ) - (sin( bs ) * cos( ent ));
		az  = az / cos( bs);
		az  = az / sin( ent );
	}
	else
		az = 0;

	if(az < -1)
		az = -1;
	if(az > 1)
		az = 1;

	az  = acos( az );
	if(sin(ls-le) >= 0)
	az = 2*M_PI-az;

#ifdef DEBUG
	g_print("QTF: %3.0f °\n\r",az*180/M_PI);
#endif

	sprintf(qtf,"%3.0f",az*180/M_PI);
	gtk_label_set_text(GTK_LABEL(data->bearing),qtf); 
	
	g_free (input_1);
	g_free (input_2);
}

// Callback for about
void about(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), PROGRAM);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION); 
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), COPYRIGHT);
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), COMMENT);
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), URI);
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
 }  

 
int main(int argc, char *argv[]) {

	mainhead_data data;
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *vbox;

	GtkWidget *toolbar;
	GtkToolItem *exit;
	GtkToolItem *info;

	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *label4;
	GtkWidget *label5;
	GtkWidget *label6;
	GtkWidget *label7;
	GtkWidget *label8;
	
	GtkWidget *entry1;
	GtkWidget *entry2;
  
	GtkWidget *button;
	
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "Locator");
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);

	exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);
	info = gtk_tool_button_new_from_stock(GTK_STOCK_HELP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), info, -1);

	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);

	g_signal_connect(G_OBJECT(exit), "clicked", 
        G_CALLBACK(gtk_main_quit), NULL);

	g_signal_connect(G_OBJECT(info), "clicked", GTK_SIGNAL_FUNC(about), NULL);

	g_signal_connect_swapped(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), NULL);


	table = gtk_table_new(6, 3, FALSE);
	gtk_box_pack_end (GTK_BOX (vbox), table, TRUE, TRUE, 20);

	label1 = gtk_label_new("First Locator");
	label2 = gtk_label_new("Second Locator");
	label3 = gtk_label_new("Distance");
	label4 = gtk_label_new("Azimut");
	label5 = gtk_label_new("km");
	label6 = gtk_label_new("°");
	label7 = gtk_label_new("0");
	label8 = gtk_label_new("0");

	button = gtk_button_new_with_label("Calculate");
	gtk_signal_connect(GTK_OBJECT(button), "clicked",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);
  
	gtk_table_attach(GTK_TABLE(table), label1, 0, 1, 1, 2,
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 2, 3, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label3, 0, 1, 3, 4, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label4, 0, 1, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label5, 2, 3, 3, 4, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label6, 2, 3, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	entry1 = gtk_entry_new();
	entry2 = gtk_entry_new();
	
	gtk_entry_set_max_length(GTK_ENTRY(entry1),8);
	gtk_entry_set_max_length(GTK_ENTRY(entry2),8);
	
	data.locator_1 = entry1;
	data.locator_2 = entry2;
	data.distance = label7;
	data.bearing = label8;

	gtk_table_attach(GTK_TABLE(table), entry1, 1, 2, 1, 2, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), entry2, 1, 2, 2, 3, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label7, 1, 2, 3, 4, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label8, 1, 2, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);


	gtk_table_attach(GTK_TABLE(table), button, 1, 2, 5, 6, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy",
		G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	return 0;
}
