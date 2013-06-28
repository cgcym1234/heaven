#ifndef _DIR_LIST_H_
#define _DIR_LIST_H_

#ifdef  __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

typedef struct file_list_s file_list_t;
struct file_list_s
{
    char file_name[512];
    file_list_t *next;
};

typedef struct cur_dir_s cur_dir_t;
struct cur_dir_s
{
    char dir_name[512];
    file_list_t *dir;
    file_list_t *regular;
    file_list_t *slink;
    cur_dir_t *next;
};

cur_dir_t *dir_init();
int dir_file(cur_dir_t *head, char *file_path);
void dir_info(cur_dir_t *head);
void dir_free(cur_dir_t *head);

cur_dir_t *recursive_dir_file(char *file_path);
void recursive_dir_info(cur_dir_t *head);
void recursive_dir_free(cur_dir_t *head);

void recursive_dir_and_print(char *file_path);


#ifdef __cplusplus
}
#endif


#endif

