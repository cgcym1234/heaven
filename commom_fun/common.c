#include "common.h"

void del_specified_char_in_str(char *src, char *sub)
{
	char *begin = src;
	char *end = src;
	char hashtable[256] = {0};
	//memset(hashtable, 0, sizeof(hashtable));
	while(*sub) ++hashtable[*sub++];
	while(*end)
	{
		if(!hashtable[*end]) *begin++ = *end;
		++end;
	}
	*begin = '\0';
}

void my_trim(char *src, char k)
{
	char *begin = src;
	char *end = src;
	while(*end)
	{
		if(*end != k) *begin++ = *end;
		++end;
	}
	*begin = '\0';
}

void test_endian()
{
    int a = 0x12345678;
	char *p = (char *)&a;
	printf("%x\n",(int)*p);
}

int my_mkdir(const char *dir_name)
{
    if(!dir_name) return -1;

    if(access(dir_name, F_OK) == 0) return 0;
    if(mkdir(dir_name, S_IRUSR | S_IWUSR | S_IXUSR) == 0) return 0;

    const char *begin = dir_name + 1;
    const char *end;
    char parent_dir[512] = {0};
    while((end = strchr(begin, '/')) != NULL)
    {
        begin = end + 1;
        memcpy(parent_dir, dir_name, end - dir_name);
        parent_dir[end - dir_name] = '\0';

        if(access(parent_dir, F_OK) == 0) continue;
        if(mkdir(parent_dir, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
        {
            fprintf(stderr, "mkdir():[%s] failed when make parent_dir:[%s], errno:[%s]\n",
                    dir_name, parent_dir, strerror(errno));
            return -1;
        }
    }

    if(mkdir(dir_name, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
    {
        fprintf(stderr, "mkdir():[%s] failed, errno:[%s]\n",
                dir_name, strerror(errno));
        return -1;
    }

    return 0;
}

void make_absolute_path(char *out, char *parent, char *sub)
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

void get_cur_date(char *str)
{
    if(!str) return;

    struct timeval tv;
    struct tm ctm;
    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &ctm);

    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", ctm.tm_year+1900,
            ctm.tm_mon+1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec);

    return;
}
//date formate:  yyyymmddhh
void modify_date_in_hour(char *date, int hour)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(time_key, "%Y%m%d%H", &tm);
    time_t secs = mktime(&tm);
    secs += 3600*hour;
    localtime_r(&secs, &tm);
    strftime(time_key, sizeof(time_key), "%Y%m%d%H", &tm);
}

int extract_number_in_str(char *str)
{
    char *begin = str;
    int num = 0;
    while(*str != '\0')
    {
        if(*str >= '0' && *str <= '9')
        {
            begin[num++] = *str;
            str++;
        }
        else str++;
    }
    begin[num] = '\0';

    //printf("fmt_number:%s\n", begin);
    return num;
}

inline void my_swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

static int random_partition(int *a, int begin, int end)
{
/*
    int mid = (begin + end)/2;
    if(a[begin] > a[mid]) swap(a[begin], a[mid]);
    if(a[mid] > a[end]) swap(a[mid], a[end]);
    if(a[begin] > a[end]) swap(a[begin], a[end]);
*/

    int key = a[end];
    int i = begin - 1;
    int j;
    for(j = begin; j < end; j++)
    {
        if(a[j] <= key)
        {
            if(a[j] != a[++i]) my_swap(&a[i], &a[j]);
        }
    }
    my_swap(&a[++i], &a[end]);

    return i;
}

void quick_sort(int *a, int lo, int hi)
{
    if(lo < hi)
    {
        int k = random_partition(a, lo, hi);
        quick_sort(a, lo, k-1);
        quick_sort(a, k+1, hi);
    }
}

int b_search(int *a, int n, int key)
{
    int left = 0, right = n, mid;
    while(left < right)
    {
        mid = (left + right)/2;
        if(key < a[mid]) right = mid;
        else if(key > a[mid]) left = mid +1;
        else return mid;
    }

    return -1;
}

void my_daemon()
{
    int fd;

    if(fork() != 0) exit(0); /* parent exits */
    setsid(); /* create a new session */

    /* Every output goes to /dev/null.*/
    if((fd = open("/dev/null", O_RDWR, 0)) != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if(fd > STDERR_FILENO) close(fd);
    }
    //fprintf(stdout, "Run as daemon Ok!\n");
}

int my_read_config_file(const char *file_name)
{
    if(!file_name) return -1;

    FILE *fp = fopen(file_name, "r");
    if(!fp)
    {
        printf("fopen(r) file[%s] failed errno:[%s]", file_name, strerror(errno));
        return -1;
    }

    int linenum = 0;
    char buf[LEN_1024];
    while(fgets(buf, LEN_1024, fp) != NULL)
    {
        linenum++;
        del_specified_char_in_str(buf, " \r\t\n");

        /* Skip comments and blank lines*/
        if(buf[0] == '#' || buf[0] == '\0') continue;
    }

    fclose(fp);
    return 0;
 err:
    fprintf(stderr, "\n*** FATAL CONFIG FILE ERROR ***\n");
    fprintf(stderr, "Reading the configuration file, at line %d\n", linenum);
    fprintf(stderr, ">>> '%s'\n", buf);
    fprintf(stderr, "%s\n", );
    exit(1);
}

long ascii2hex(char *buf, int len)
{
	int i;
	long val;

	len = min(len, 16);
	val = 0;
	for (i = 0; i < len; i++) {
		char c;
		c = toupper(buf[i]);
		if (c >= '0' && c <= '9') {
			val <<= 4;
			val |= c - '0';
		} else if (c >= 'A' && c <= 'F') {
			val <<= 4;
			val |= c - 'A' + 10;
		} else {
			/* other chars are treated as 0s */
			val <<= 4;
		}
	}
	return val;
}


unsigned long get_file_size(char *file)
{
	struct stat buf;
	if(stat(file, &buf) == 0) return buf.st_size;

	return -1;
}

