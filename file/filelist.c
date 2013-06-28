#include "filelist.h"

//static fix_mpool_t *mem_pool = NULL;
#define POOL_NUM 10000

static void destroy_list(file_list_t *head)
{
    if(!head) return;

    file_list_t *tmp;
    while(head)
    {
        tmp = head->next;
        //fmem_free(mem_pool, head);
        free(head);
        head = tmp;
    }

    return;
}

static void display_file(file_list_t *node)
{
    if(!node) return;

    while(node)
    {
        printf("name : [%s]\n", node->file_name);
        //LOG(LOG_DEBUG, "show list node : [%s]", p->file_name);
        node = node->next;
    }

    return;
}

cur_dir_t *dir_init()
{
    cur_dir_t *tmp = (cur_dir_t *)malloc(sizeof(cur_dir_t));
    if(!tmp)
    {
        fprintf(stderr,"dir_init() failed, Out Of Memory!!!\n");
        abort();
    }
    memset(tmp, 0, sizeof(cur_dir_t));
    tmp->dir = NULL;
    tmp->regular = NULL;
    tmp->slink = NULL;
    tmp->next = NULL;

    //if(!mem_pool) mem_pool = fmem_init(POOL_NUM, sizeof(file_list_t));

    return tmp;
}

void dir_free(cur_dir_t *head)
{
    if(!head) return;

    destroy_list(head->dir);
    destroy_list(head->regular);
    destroy_list(head->slink);
    free(head);

    return ;
}

int dir_file(cur_dir_t *head, char *file_path)
{
    if(!head || !file_path) return -1;

/*
    char work_dir[512] = {0};
    if(getcwd(work_dir, sizeof(work_dir)) == NULL)
        snprintf(work_dir, sizeof(work_dir), "%s/bin", getenv("HOME"));

    if(chdir(file_path) == -1)
    {
        fprintf(stderr, "chdir() failed, path:[%s], error:[%s]\n",
                        file_path, strerror(errno));
        return -1;
    }
*/

    DIR *dir = NULL;
    struct dirent *entry;
    struct stat st;
    memset(&st, 0, sizeof(struct stat));

    if((dir = opendir(file_path)) == NULL)
    {
        fprintf(stderr, "opendir() failed, path:[%s], error:[%s]\n",
                        file_path, strerror(errno));
        return -1;
    }
    else
    {
        strncpy(head->dir_name, file_path, sizeof(head->dir_name));
        char full_name[512];
        while((entry = readdir(dir)) != NULL)
        {
            sprintf(full_name, "%s/%s", file_path, entry->d_name);
            if(stat(full_name, &st) == 0)
            {
                if(strncmp(entry->d_name, ".", 1) == 0
                   || strncmp(entry->d_name, "..", 2) == 0) continue;

                if(S_ISREG(st.st_mode))
                {
                    file_list_t *node = (file_list_t *)malloc(sizeof(*node));
                    strncpy(node->file_name, entry->d_name, sizeof(node->file_name));
                    node->next = head->regular;
                    head->regular = node;
                }
                else if(S_ISDIR(st.st_mode))
                {
                    file_list_t *node = (file_list_t *)malloc(sizeof(*node));
                    strncpy(node->file_name, entry->d_name, sizeof(node->file_name));
                    node->next = head->dir;
                    head->dir = node;
                }
/*
                else if(S_ISLNK(st.st_mode))
                {
                    file_list_t *node = (file_list_t *)malloc(sizeof(*node));
                    strncpy(node->file_name, entry->d_name, sizeof(node->file_name));
                    node->next = head->slink;
                    head->slink = node;
                }
*/

            }
        }
        closedir(dir);
    }

/*
    if(chdir(work_dir) == -1)
    {
        fprintf(stderr, "chdir() failed, path:[%s], error:[%s]\n",
                        work_dir, strerror(errno));
    }
*/

    return 0;
}


void dir_info(cur_dir_t *head)
{
    if(!head) return;

    printf("\n>>>>>>>>>>>>>>>cur dir name:[%s]<<<<<<<<<<<<<<<<<\n", head->dir_name);
    printf("show sub_directory info:\n");
    printf("-----------------------\n");
    display_file(head->dir);
    printf("-----------------------\n\n");

    printf("show regular file info:\n");
    printf("-----------------------\n");
    display_file(head->regular);
    printf("-----------------------\n\n");

    printf("show link file info:\n");
    printf("-----------------------\n");
    display_file(head->slink);
    printf("-----------------------\n\n");

    return ;
}

static void make_absolute_path(char *out, char *parent, char *sub)
{
    int len = strlen(parent);
    memcpy(out, parent, len+1);

    if(out[len-1] != '/')
    {
        out[len]='/';
        out[len+1] = '\0';
    }
    strcat(out, sub);

    return;
}

cur_dir_t *recursive_dir_file(char *file_path)
{
    if(!file_path) return NULL;

    cur_dir_t *dir_head = dir_init();
    int ret = dir_file(dir_head, file_path);
    if(ret == -1)
    {
        dir_free(dir_head);
        return NULL;
    }

    cur_dir_t *tail = dir_head;
    cur_dir_t *head = dir_head;
    char absolute_path[512];
    while(head)
    {
        file_list_t *sub_dir = head->dir;
        while(sub_dir)
        {
            cur_dir_t *sub_dir_info = dir_init();
            make_absolute_path(absolute_path, head->dir_name, sub_dir->file_name);
            int ret = dir_file(sub_dir_info, absolute_path);
            if(ret == -1)
            {
                dir_free(sub_dir_info);
                sub_dir = sub_dir->next;
                continue;
            }
            tail->next = sub_dir_info;
            tail = sub_dir_info;

            sub_dir = sub_dir->next;
        }
        head = head->next;
    }

    return dir_head;
}

void recursive_dir_info(cur_dir_t *head)
{
    while(head)
    {
        dir_info(head);
        head = head->next;
    }

    //fmem_info(mem_pool);
    return;
}
void recursive_dir_free(cur_dir_t *head)
{
    cur_dir_t *tmp = head;
    while(head)
    {
        tmp = head->next;
        dir_free(head);
        head = tmp;
    }
    printf("recursive_dir_free ok\n");
    //fmem_info(mem_pool);
    //fmem_destroy(mem_pool);

    return;
}

void recursive_dir_and_print(char *file_path)
{
    if(!file_path) return;

    cur_dir_t *dir_head = dir_init();
    int ret = dir_file(dir_head, file_path);
    if(ret == -1)
    {
        dir_free(dir_head);
        return;
    }

    cur_dir_t *tail = dir_head;
    cur_dir_t *head = dir_head;
    cur_dir_t *tmp;
    char absolute_path[512];
    while(head)
    {
        file_list_t *sub_dir = head->dir;
        while(sub_dir)
        {
            cur_dir_t *sub_dir_info = dir_init();
            make_absolute_path(absolute_path, head->dir_name, sub_dir->file_name);
            int ret = dir_file(sub_dir_info, absolute_path);
            if(ret == -1)
            {
                dir_free(sub_dir_info);
                sub_dir = sub_dir->next;
                continue;
            }
            tail->next = sub_dir_info;
            tail = sub_dir_info;

            sub_dir = sub_dir->next;
        }
        tmp = head;
        head = head->next;
        dir_info(tmp);
        dir_free(tmp);
    }

    //fmem_info(mem_pool);
    //fmem_destroy(mem_pool);

    return;
}


