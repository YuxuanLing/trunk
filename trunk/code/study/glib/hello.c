#include <glib.h>  
#include <locale.h>  
GMainLoop* loop;  
gint counter = 10;  
gboolean callback(gpointer arg)  
{  
    g_print("hello world\n");  
    g_message("中文");  
    //fflush();  
    if(--counter ==0){  
        g_print("\n");  
        //退出循环  
        g_main_loop_quit(loop);  
        //注销定时器  
        return FALSE;  
    }  
    //定时器继续运行  
    return TRUE;  
}  
int main(int argc, char* argv[])  
{  
    setlocale (LC_ALL, "");/*将程序的locale设置成与console的一致*/  
    //g_thread_init是必需的，GMainLoop需要gthread库的支持。  
    if(g_thread_supported() == 0)  
        g_thread_init(NULL);  
    //创建一个循环体，先不管参数的意思。  
    g_print("g_main_loop_new\n");  
    loop = g_main_loop_new(NULL, FALSE);  
   //增加一个定时器，1000毫秒运行一次callback  
    g_timeout_add(10000,callback,NULL);  
    g_print("g_main_loop_run\n");  
    g_main_loop_run(loop);  
    g_print("g_main_loop_unref\n");  
    g_main_loop_unref(loop);  
    return 0;  
}  
