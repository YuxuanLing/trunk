#include <iostream>
#include <stdlib.h>
#include <glib.h>

void each_callback(gpointer data, gpointer user_data)  
{  
    g_print("element:%s, user param:%s\n", (gchar*)data, (gchar*)user_data);  
}  
  
int main( int argc,  
		          char *argv[] )  
{  
  GList *list = NULL;  
  list = g_list_append(list, (gpointer)"second");  
  list = g_list_prepend(list,(gpointer) "first");  
  
  g_list_foreach(list, each_callback, (gpointer)"user_data");  
  
  GList *second = g_list_find(list, (gpointer)"second");  
  g_print("findElement:%s\n", (gchar*)second->data);  
  
  list = g_list_remove(list, (gpointer)"second");  
  
  g_list_foreach(list, each_callback, (gpointer)"user_data");  
  
  return 0;  
} 
