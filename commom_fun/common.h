#ifndef _MY_COMMON_H_
#define _MY_COMMON_H_

void del_specified_char_in_str(char *src, char *sub);
void my_trim(char *src, char k);
int extract_number_in_str(char *str);

long ascii2hex(char *buf, int len);

int my_daemon();
void test_endian();
int my_mkdir(const char *dir_name);
void make_absolute_path(char *out, char *parent, char *sub);

void modify_date_in_hour(char *date, int hour);
void get_cur_date(char *str);


void my_swap(int *a, int *b);
void quick_sort(int *a, int lo, int hi);
int b_search(int *a, int n, int key);

int my_read_config_file(const char *file_name);

#endif

