#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


int red[34] = {0};
int blue[17] = {0};
int red_excludes = 0;
int blue_excludes = 0;
char *exclude_name = "exclude.txt";
char *generate_name = "generate.txt";

static int fmt_number(char *str)
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

static int exclude_number(char *str)
{
    int len = strlen(str);
    char number[3] = {0};
    int exclude_idx = 0;

    number[0] = str[len-2];
    number[1] = str[len-1];
    exclude_idx = atoi(number);
    if(exclude_idx < 1 || exclude_idx > 16)
    {
        printf("blue ball error:[%d]", exclude_idx);
        return -1;
    }

    blue[exclude_idx] = -1;
    blue_excludes++;
    str[len-2] = str[len-1] = '\0';

    while(*str != '\0')
    {
        number[0] = str[0];
        number[1] = str[1];
        str += 2;

        exclude_idx = atoi(number);
        if(exclude_idx < 1 || exclude_idx > 33)
        {
            printf("red ball error:[%d]", exclude_idx);
            return -1;
        }
        red[exclude_idx] = -1;
        red_excludes++;
    }

    return 0;
}

void get_file_info()
{
    FILE *fp = fopen(exclude_name, "r");
    if(!fp)
    {
        fp = fopen(exclude_name, "w");
        if(fp) fclose(fp);
        return;
    }
    char buf[256] = {0};
    int ret = 0;
    int lines = 0;
    while(fgets(buf, sizeof(buf), fp))
    {
        lines++;
        ret = fmt_number(buf);
        if(ret == 0 || ret%2 == 1)
        {
            printf("format error of the number in the file:[%s], line:[%d]\n"
                   "corect:01020304...\n", exclude_name, lines);
            continue;
        }

        ret = exclude_number(buf);
        if(ret == -1) printf("number out of range in the line:[%d]\n", lines);
    }

    fclose(fp);
    return;
}

static void get_cur_date(char *str)
{
    if (str == NULL)
        return;

    time_t t;
    struct tm *ptm;
    t = time(NULL);
    ptm = localtime(&t);

    sprintf(str,"%02d-%02d-%02d %02d:%02d:%02d",ptm->tm_year+1900, ptm->tm_mon+1,
            ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

void show()
{
    char buf[1024] = {0};
    char tmp_num[8] = {0};
    char cur_date[32] = {0};
    get_cur_date(cur_date);
    sprintf(buf, "%s red:", cur_date);

    int i;
    for(i = 1; i < 34; i++)
    {
        if(red[i] == 1)
        {
            sprintf(tmp_num, "%02d ", i);
            strcat(buf, tmp_num);
        }
    }

    i = 0;
    for(i = 1; i < 17; i++)
    {
        if(blue[i] == 1)
        {
            sprintf(tmp_num, "blue:%02d", i);
            strcat(buf, tmp_num);
        }
    }

    printf("%s\n", buf);
    FILE *fp = fopen(generate_name, "a+");
    if(!fp) return;
    fprintf(fp, "%s\n", buf);
    fclose(fp);

    return;
}

void generate()
{
    if(blue_excludes >= 16 || red_excludes >= 27)
    {
        printf("error: exclude too many numbers!\n"
               "red_excludes:[%d], blue_excludes:[%d]\n", red_excludes, blue_excludes);
        return;
    }
    srand((int)time(NULL));
    int i = 0;
    int idx;
    while(i < 6)
    {
        idx = rand()%33 + 1;
        if(red[idx] == 0 && red[idx] != -1)
        {
            red[idx] = 1;
            i++;
        }
    }

    i = 0;
    while(i < 1)
    {
        idx = rand()%16 + 1;
        if(blue[idx] == 0 && blue[idx] != -1)
        {
            blue[idx] = 1;
            i++;
        }
    }
}

void start()
{
    get_file_info();
    generate();
    show();
}

int main()
{
    start();
    getchar();
    return 0;
}

