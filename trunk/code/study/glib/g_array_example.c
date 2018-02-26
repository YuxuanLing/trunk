#include <glib.h>

void display_array(GArray *array, int len, const char *prompt)
{
    int i = 0;

    printf("%s: \n", prompt);
    for (i = 0; i < len; i++) {
        printf("%d ", g_array_index(array, int, i));
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    GArray *array = NULL;
    int i = 0;
    int cur_arr_len = 0;
    int len = 0;

    array = g_array_new(FALSE, TRUE, sizeof(int));
    
    len = 0;
    for (i = 10; i < 15; i++) {
        g_array_append_val(array, i);
        len++;
    }
    cur_arr_len += len;
    display_array(array, cur_arr_len, "Create array");

    int app_data[] = {30, 40, 50, 60};
    int app_len = sizeof(app_data) / sizeof(int);

    g_array_append_vals(array, app_data, app_len);
    cur_arr_len += app_len;
    display_array(array, cur_arr_len, "After append values 30 40 50 60");

    len = 0;
    for (i = 1; i < 4; i++) {
        g_array_prepend_val(array, i);
        len++;
    }
    cur_arr_len += len;
    display_array(array, cur_arr_len, "After prepend value 1 2 3");

    int prepend_data[] = {-10, -20, -30, -40};
    int prepend_len = sizeof(prepend_data) / sizeof(int);

    g_array_prepend_vals(array, prepend_data, prepend_len);
    cur_arr_len += prepend_len;
    display_array(array, cur_arr_len, "After prepend values -10 -20 -30 -40");

    int data = 100;
    g_array_insert_val(array, 5, data);
    cur_arr_len += 1;
    display_array(array, cur_arr_len, "After insert 100 at index 5");

    g_array_remove_index(array, 5);
    cur_arr_len -= 1;
    display_array(array, cur_arr_len, "After remove value at index 5");

    g_array_remove_index_fast(array, 10);
    cur_arr_len -= 1;
    display_array(array, cur_arr_len, "After remove value at index 10 fast");

    g_array_free(array, TRUE);
}
