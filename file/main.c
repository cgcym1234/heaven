#include "filelist.h"

int main(int argc, char **argv)
{
    if(argc != 2) return 0;

    cur_dir_t *cur_dir = dir_init();
    dir_file(cur_dir, argv[1]);
    dir_info(cur_dir);
    dir_free(cur_dir);

    cur_dir_t *rec_dir = recursive_dir_file(argv[1]);
    //recursive_dir_info(rec_dir);
    //recursive_dir_free(rec_dir);

    return 0;
}


