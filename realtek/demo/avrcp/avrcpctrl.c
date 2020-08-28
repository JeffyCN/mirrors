#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signalfd.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <glib.h>



#include "avrcpctrl.h"
#include "gdbus.h"

#define COLOR_OFF	"\x1B[0m"
#define COLOR_RED	"\x1B[0;91m"
#define COLOR_GREEN	"\x1B[0;92m"
#define COLOR_YELLOW	"\x1B[0;93m"
#define COLOR_BLUE	"\x1B[0;94m"
#define COLOR_BOLDGRAY	"\x1B[1;30m"
#define COLOR_BOLDWHITE	"\x1B[1;37m"


/* String display constants */
#define COLORED_NEW	COLOR_GREEN "NEW" COLOR_OFF
#define COLORED_CHG	COLOR_YELLOW "CHG" COLOR_OFF
#define COLORED_DEL	COLOR_RED "DEL" COLOR_OFF

#define PROMPT_ON	COLOR_BLUE "[bluetooth]" COLOR_OFF "# "
#define PROMPT_OFF	"[bluetooth]# "

#define BLUEZ_MEDIA_PLAYER_INTERFACE "org.bluez.MediaPlayer1"
#define BLUEZ_MEDIA_FOLDER_INTERFACE "org.bluez.MediaFolder1"
#define BLUEZ_MEDIA_ITEM_INTERFACE "org.bluez.MediaItem1"

DBusConnection *dbus_conn;
GDBusProxy *default_player;
GSList *players = NULL;
GSList *folders = NULL;
GSList *items = NULL;
GDBusClient *client;
GMainLoop *main_loop = NULL;


 void connect_handler(DBusConnection *connection, void *user_data)
 {
     rl_set_prompt(PROMPT_ON);
     printf("\r");
     rl_on_new_line();
     rl_redisplay();
 }
 
void disconnect_handler(DBusConnection *connection, void *user_data)
 {
     rl_set_prompt(PROMPT_OFF);
     printf("\r");
     rl_on_new_line();
     rl_redisplay();
 }
void print_folder(GDBusProxy *proxy, const char *description)
{
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	/*Log::debug("%s%s%sFolder %s\n", description ? "[" : "",
					description ? : "",
					description ? "] " : "",
					path);*/
}
 void folder_removed(GDBusProxy *proxy)
 {
     folders = g_slist_remove(folders, proxy);
 
     print_folder(proxy, COLORED_DEL);
 }
 
 char *proxy_description(GDBusProxy *proxy, const char *title,
						const char *description)
{
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	return g_strdup_printf("%s%s%s%s %s ",
					description ? "[" : "",
					description ? : "",
					description ? "] " : "",
					title, path);
}
 void print_player(GDBusProxy *proxy, const char *description)
{
	char *str;
    char strplay[256];
	str = proxy_description(proxy, "Player", description);

    memset(strplay, 0x00, 256);
	sprintf(strplay,"%s%s\n", str, (default_player == proxy ? "[default]" : ""));
    printf(strplay);
    
	g_free(str);
}
 void player_added(GDBusProxy *proxy)
{
    printf("player_added \n");
	players = g_slist_append(players, proxy);

	if (default_player == NULL){
	    printf("set default player");
		default_player = proxy;
    }

	print_player(proxy, COLORED_NEW);
}
void print_item(GDBusProxy *proxy, const char *description)
 {
     const char *path, *name;
     DBusMessageIter iter;
 
     path = g_dbus_proxy_get_path(proxy);
 
     if (g_dbus_proxy_get_property(proxy, "Name", &iter))
         dbus_message_iter_get_basic(&iter, &name);
     else
         name = "<unknown>";
 
     /*Log::debug("%s%s%sItem %s %s\n", description ? "[" : "",
                     description ? : "",
                     description ? "] " : "",
                     path, name);*/
 }

 void item_added(GDBusProxy *proxy)
 {
     items = g_slist_append(items, proxy);
 
     print_item(proxy, COLORED_NEW);
 }
 
 void folder_added(GDBusProxy *proxy)
 {
     folders = g_slist_append(folders, proxy);
 
     print_folder(proxy, COLORED_NEW);
 }

 void proxy_added(GDBusProxy *proxy, void *user_data)
 {
    printf("proxy_added \n");
     const char *interface;
     const char interfacestr[128] ;
     interface = g_dbus_proxy_get_interface(proxy);

     memset(interfacestr, 0x00, 128);
     sprintf(interfacestr," interface:%s \n", interface);
     printf(interfacestr);
     if (!strcmp(interface, BLUEZ_MEDIA_PLAYER_INTERFACE))
         player_added(proxy);
     else if (!strcmp(interface, BLUEZ_MEDIA_FOLDER_INTERFACE))
         folder_added(proxy);
     else if (!strcmp(interface, BLUEZ_MEDIA_ITEM_INTERFACE))
         item_added(proxy);
 }

 void player_removed(GDBusProxy *proxy)
{
	print_player(proxy, COLORED_DEL);

	if (default_player == proxy)
		default_player = NULL;

	players = g_slist_remove(players, proxy);
}

void item_removed(GDBusProxy *proxy)
{
	items = g_slist_remove(items, proxy);

	print_item(proxy, COLORED_DEL);
}
 void proxy_removed(GDBusProxy *proxy, void *user_data)
{
   printf("proxy_removed \n");
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, BLUEZ_MEDIA_PLAYER_INTERFACE))
		player_removed(proxy);
	if (!strcmp(interface, BLUEZ_MEDIA_FOLDER_INTERFACE))
		folder_removed(proxy);
	if (!strcmp(interface, BLUEZ_MEDIA_ITEM_INTERFACE))
		item_removed(proxy);
}
 void print_iter(const char *label, const char *name,
                         DBusMessageIter *iter)
 {
     dbus_bool_t valbool;
     dbus_uint32_t valu32;
     dbus_uint16_t valu16;
     dbus_int16_t vals16;
     const char *valstr;
     DBusMessageIter subiter;
 
     if (iter == NULL) {
         //Log::debug("%s%s is nil\n", label, name);
         return;
     }
 
     switch (dbus_message_iter_get_arg_type(iter)) {
     case DBUS_TYPE_INVALID:
         //Log::debug("%s%s is invalid\n", label, name);
         break;
     case DBUS_TYPE_STRING:
     case DBUS_TYPE_OBJECT_PATH:
         dbus_message_iter_get_basic(iter, &valstr);
         //Log::debug("%s%s: %s\n", label, name, valstr);
         break;
     case DBUS_TYPE_BOOLEAN:
         dbus_message_iter_get_basic(iter, &valbool);
         //Log::debug("%s%s: %s\n", label, name,
         //            valbool == TRUE ? "yes" : "no");
         break;
     case DBUS_TYPE_UINT32:
         dbus_message_iter_get_basic(iter, &valu32);
         //Log::debug("%s%s: 0x%06x\n", label, name, valu32);
         break;
     case DBUS_TYPE_UINT16:
         dbus_message_iter_get_basic(iter, &valu16);
         //Log::debug("%s%s: 0x%04x\n", label, name, valu16);
         break;
     case DBUS_TYPE_INT16:
         dbus_message_iter_get_basic(iter, &vals16);
         //Log::debug("%s%s: %d\n", label, name, vals16);
         break;
     case DBUS_TYPE_VARIANT:
         dbus_message_iter_recurse(iter, &subiter);
         print_iter(label, name, &subiter);
         break;
     case DBUS_TYPE_ARRAY:
         dbus_message_iter_recurse(iter, &subiter);
         while (dbus_message_iter_get_arg_type(&subiter) !=
                             DBUS_TYPE_INVALID) {
             print_iter(label, name, &subiter);
             dbus_message_iter_next(&subiter);
         }
         break;
     case DBUS_TYPE_DICT_ENTRY:
         dbus_message_iter_recurse(iter, &subiter);
         dbus_message_iter_get_basic(&subiter, &valstr);
         dbus_message_iter_next(&subiter);
         print_iter(label, valstr, &subiter);
         break;
     default:
         //Log::debug("%s%s has unsupported type\n", label, name);
         break;
     }
 }

 void player_property_changed(GDBusProxy *proxy, const char *name,
                         DBusMessageIter *iter)
 {
     char *str;
 
     str = proxy_description(proxy, "Player", COLORED_CHG);
     print_iter(str, name, iter);
     g_free(str);
 }
 void folder_property_changed(GDBusProxy *proxy, const char *name,
                         DBusMessageIter *iter)
 {
     char *str;
 
     str = proxy_description(proxy, "Folder", COLORED_CHG);
     print_iter(str, name, iter);
     g_free(str);
 }
 void item_property_changed(GDBusProxy *proxy, const char *name,
						DBusMessageIter *iter)
{
	char *str;

	str = proxy_description(proxy, "Item", COLORED_CHG);
	print_iter(str, name, iter);
	g_free(str);
}

void property_changed(GDBusProxy *proxy, const char *name,
                     DBusMessageIter *iter, void *user_data)
 {
    printf("property_changed \n");
     const char *interface;
 
     interface = g_dbus_proxy_get_interface(proxy);
 
     if (!strcmp(interface, BLUEZ_MEDIA_PLAYER_INTERFACE))
         player_property_changed(proxy, name, iter);
     else if (!strcmp(interface, BLUEZ_MEDIA_FOLDER_INTERFACE))
         folder_property_changed(proxy, name, iter);
     else if (!strcmp(interface, BLUEZ_MEDIA_ITEM_INTERFACE))
         item_property_changed(proxy, name, iter);
 }
bool check_default_player(void)
{
    if (!default_player) {
        if(NULL != players) {
            GSList *l;
            l = players;
            GDBusProxy *proxy = l->data;
            default_player = proxy;
            printf("set default player\n");
            return TRUE;
        }
     printf("No default player available\n");
     return FALSE;
    }
    //printf(" player ok\n");

    return TRUE;
}
 gboolean option_version = FALSE;
 
 GOptionEntry options[] = {
     { "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
                 "Show version information and exit" },
     { NULL },
 };

int init_avrcp() {
  GError *error = NULL;

 

  dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);

  if(NULL== dbus_conn){
    printf("dbus init fail!");
    return -1;
  }
  client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");
  if(NULL == client) {
    printf("client inti fail");
    dbus_connection_unref(dbus_conn);
    return -1;
  }
  main_loop = g_main_loop_new(NULL, FALSE);

  rl_set_prompt(PROMPT_OFF);
  rl_redisplay();

  g_dbus_client_set_connect_watch(client, connect_handler, NULL);
  g_dbus_client_set_disconnect_watch(client, disconnect_handler, NULL);

  g_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
                          property_changed, NULL);
  printf("init ok\n");
  g_main_loop_run(main_loop);
    
}

 int init_avrcp_ctrl() {
    pthread_t avrcp_thread;
    
    pthread_create(&avrcp_thread, NULL, &init_avrcp, NULL);
    return 1;
}
  int release_avrcp_ctrl() {
    
    g_dbus_client_unref(client);    

    dbus_connection_unref(dbus_conn);
    g_main_loop_unref(main_loop);

}

void play_reply(DBusMessage *message, void *user_data)
{
    DBusError error;

    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
      //rl_printf("Failed to play: %s\n", error.name);
      dbus_error_free(&error);
      return;
    }

    printf("Play successful\n");
}

void play_avrcp(){
    if (!check_default_player())
        return;

    if (g_dbus_proxy_method_call(default_player, "Play", NULL, play_reply,
                  NULL, NULL) == FALSE) {
        printf("Failed to play\n");
        return;
    }

    printf("Attempting to play\n");
}

void pause_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		printf("Failed to pause: %s\n");
		dbus_error_free(&error);
		return;
	}

	printf("Pause successful\n");
}

void pause_avrcp(){
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Pause", NULL,
					pause_reply, NULL, NULL) == FALSE) {
		printf("Failed to pause\n");
		return;
	}

	printf("Attempting to pause\n");

}

void stop_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		//rl_printf("Failed to stop: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	printf("Stop successful\n");
}

void stop_avrcp(){
    if (!check_default_player())
            return;

    if (g_dbus_proxy_method_call(default_player, "Stop", NULL, stop_reply,
                            NULL, NULL) == FALSE) {
        printf("Failed to stop\n");
        return;
    }

    printf("Attempting to stop\n");

}

void next_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		printf("Failed to jump to next\n");
		dbus_error_free(&error);
		return;
	}

	printf("Next successful\n");
}

void next_avrcp()
{
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Next", NULL, next_reply,
							NULL, NULL) == FALSE) {
		printf("Failed to jump to next\n");
		return;
	}

	printf("Attempting to jump to next\n");
}

void previous_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		//rl_printf("Failed to jump to previous: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	printf("Previous successful\n");
}

void previous_avrcp(){
	if (!check_default_player())
		return;

	if (g_dbus_proxy_method_call(default_player, "Previous", NULL,
					previous_reply, NULL, NULL) == FALSE) {
		printf("Failed to jump to previous\n");
		return;
	}

	printf("Attempting to jump to previous\n");

}

void fast_forward_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		//rl_printf("Failed to fast forward: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	printf("FastForward successful\n");
}

void fast_forward_avrcp() {
    if (!check_default_player())
            return;
    
    if (g_dbus_proxy_method_call(default_player, "FastForward", NULL,
                fast_forward_reply, NULL, NULL) == FALSE) {
        printf("Failed to jump to previous\n");
        return;
    }

    printf("Fast forward playback\n");

}

void rewind_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		//rl_printf("Failed to rewind: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	printf("Rewind successful\n");
}

void rewind_avrcp(){
    if (!check_default_player())
            return;

    if (g_dbus_proxy_method_call(default_player, "Rewind", NULL,
                    rewind_reply, NULL, NULL) == FALSE) {
        printf("Failed to rewind\n");
        return;
    }

    printf("Rewind playback\n");

}

int getstatus_avrcp(){
    GDBusProxy *proxy;
    DBusMessageIter iter;
    const char *valstr;

    if (check_default_player() == FALSE)
        return AVRCP_PLAY_STATUS_ERROR; //default player no find
    
    proxy = default_player;

    if (g_dbus_proxy_get_property(proxy, "Status", &iter) == FALSE)
            return AVRCP_PLAY_STATUS_ERROR; //unkonw status

    dbus_message_iter_get_basic(&iter, &valstr);

    if (!strcasecmp(valstr, "stopped"))
		return AVRCP_PLAY_STATUS_STOPPED;
	else if (!strcasecmp(valstr, "playing"))
		return AVRCP_PLAY_STATUS_PLAYING;
	else if (!strcasecmp(valstr, "paused"))
		return AVRCP_PLAY_STATUS_PAUSED;
	else if (!strcasecmp(valstr, "forward-seek"))
		return AVRCP_PLAY_STATUS_FWD_SEEK;
	else if (!strcasecmp(valstr, "reverse-seek"))
		return AVRCP_PLAY_STATUS_REV_SEEK;
	else if (!strcasecmp(valstr, "error"))
		return AVRCP_PLAY_STATUS_ERROR;

	return AVRCP_PLAY_STATUS_ERROR;
}



