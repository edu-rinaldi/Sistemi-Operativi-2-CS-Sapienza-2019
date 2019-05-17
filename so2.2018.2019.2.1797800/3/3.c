#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

char *pname, *f_in, *f_out, *s;
int i1, i2;

void print_usage() {fprintf(stderr, "Usage: %s filein fileout awk_script i1 i2\n", pname); exit(10); }

int file_exist(char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

int read_permission(char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && st.st_mode & S_IRUSR && st.st_mode & S_IRGRP && st.st_mode & S_IROTH;
}

int write_permission(char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && st.st_mode & S_IWUSR && st.st_mode & S_IWGRP && st.st_mode & S_IWOTH;
}

//funzione che restituisce true se il carattere è un numero
int isnumber(char c) {return c >= '0' || c <= '9' ? 1 : 0;}

// funzione che restituisce true se la stringa contiene SOLO numeri
int string_is_number(char *s) 
{
    int slen = strlen(s);
    for(int i=0; i<slen; i++) 
        if(!isnumber(s[i])) return 0;
    return 1;
}

int check_integer(char *s) { return string_is_number(s) && atoi(s)>=0;}

void print_parameter(int debugMode)
{
    if (debugMode) { printf("FIn: %s\nFOut: %s\nS: %s\nI1: %d\nI2: %d\n", f_in, f_out, s, i1, i2);}
}

// ./3 f_in f_out s i1 i2
int main(int argc, char **argv)
{
    const int DEBUG_MODE = 1;
    pname = argv[0];
    if(argc < 6 || !check_integer(argv[4]) || !check_integer(argv[5])) print_usage();
    f_in = argv[1], f_out = argv[2], s = argv[3];
    i1 = atoi(argv[4]), i2 = atoi(argv[5]);
    print_parameter(DEBUG_MODE);

    FILE *fileBin1 = fopen(f_in, "rb");
    // char primo_num[4], secondo_num[4];
    int primo_num, secondo_num;
    int n = fread(&primo_num, 4, 1, fileBin1);
    printf("Ho letto %d bytes\n", n);
    printf("Primo num: %d\n", primo_num);
    n += fread(&secondo_num, 4, 1, fileBin1);
    printf("Ho letto %d bytes\n", n);
    printf("Secondo num: %d\n", secondo_num);

    char ascii_negato[8+primo_num+1];
    n += fread(ascii_negato, 8+primo_num, 1, fileBin1);
    for(int i=0; i<8+primo_num; i++)
        ascii_negato[i] = ~ascii_negato[i];
    // printf("%s\n", ascii_negato);
    // char c = 69;
    // printf("%c\n", c);
    // printf("%d\n", ~c & 239);


}