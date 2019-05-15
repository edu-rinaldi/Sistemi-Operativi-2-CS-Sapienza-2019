#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <locale.h>



// --- FUNCTION DECLARATION ---

int prova(char *root, int d_flag, int l_flag, int mod, int R_flag);
void print_dir_recursive(char *dir, int l_flag, int mod);
void print_dir(char *dir, int l_flag, int mod);
// funzione che stampa in base ai flag una directory
void print_line(char *path, int l_flag, int mod);

// funzione del print usage
void print_usage(char *n);

// funzione che restituisce true se il carattere è un numero
int isnumber(char c);

// funzione che restituisce true se la stringa contiene SOLO numeri
int string_is_number(char *s);

// if path is directory --> return 0;
// if path is regFile -->   return 1;
// if path is symlink -->   return 2;
int path_type(char *path);

// funzione che conta quanti file ci sono in una directory
int count_files(char *path);

// funzione che restituisce un array contenente i file in una directory (ordinati)
void list_files(char *path, char **f, int num_files);
// void list_files2(char *path, int num_files, char **files);

static char *lsperms(int mode);
static int filetypeletter(int mode);

// funzione che printa i permessi di un file a un certo path
int print_permission(char *path);

// funzione che resistuisce la dimensione in byte di un file/directory
int get_file_dim(char *path);

// funzione che restituisce il numero di hard link di un file
int get_hlink_count(char *path);

// funzione comparatrice di stringhe
static int string_comparator(const void* a, const void* b);

// funzione che ordina un array di stringhe
void sort(char **array, int n);

// --- END FUNCTION DECLARATION ---


int main(int argc, char **argv)
{
    setlocale(LC_ALL, "C");
    // --- INIZIO PARSE PARAMETRI ---
    int option;
    int d_flag = 0, R_flag = 0, l_flag = 0, mod = 0;
    char currentDir[] = "./";
    while ((option = getopt(argc, argv, ":dRl:")) != -1 )
    {
        switch(option)
        {
            
            // -d
            case 'd': d_flag = 1; break;
            // -R
            case 'R': R_flag = 1; break;
            // -l
            case 'l':
                //setta il flag a true
                l_flag = 1;
                // se non è un numero --> exit 20
                if (!string_is_number(optarg)) { print_usage(argv[0]); exit(20); }
                //altrimenti convertilo
                mod = atoi(optarg);
                break;
            // -l senza argomento
            case ':':
                print_usage(argv[0]); exit(20); break;
            // parametro inesistente
            case '?': print_usage(argv[0]); exit(20); break;
        }
    }
    
    int paramLength = argc-optind, numDir = 0, numFiles = 0, numLinks = 0, numFilesAndLinks = 0;
    char *param[paramLength];
    for(int i=0; i<paramLength; i++)
    {
        // array con tutti i parametri in [files]
        param[i] = argv[i+optind];
        
        //se è una directory togli /
        // if (path_type(param[i]) == 0 && param[i][strlen(param[i])-1] == '/' ) 
        //     param[i][strlen(param[i])-1] = 0;
        // calcolo le lunghezze dei 3 array
        switch(path_type(param[i]))
        {
            // se è una directory
            case 0: numDir++; break;
            // se è un file
            case 1: numFiles++; numFilesAndLinks++; break;
            // se è un link simbolico
            case 2: numLinks++; numFilesAndLinks++; break;
        }
    }

    char *dirs[numDir];
    char *files[numFiles];
    char *links[numLinks];
    char *filesAndLinks[numFilesAndLinks];
    int dirsIndex = 0, filesIndex = 0, linksIndex = 0, filesAndLinksIndex = 0;
    
    // creo l'array con i path delle dir, dei file, dei link
    for(int i=0; i<paramLength; i++)
    {
        switch(path_type(param[i]))
        {
            case 0: dirs[dirsIndex++] = param[i]; break;
            case 1: files[filesIndex++] = param[i]; filesAndLinks[filesAndLinksIndex++] = param[i]; break;
            case 2: links[linksIndex++] = param[i]; filesAndLinks[filesAndLinksIndex++] = param[i]; break;
        }
    }

    // li ordino
    sort(dirs, dirsIndex);
    sort(files, filesIndex);
    sort(links, linksIndex);
    sort(filesAndLinks, filesAndLinksIndex);


    // printf("Dirs:\n");
    // for(int i=0; i<numDir; i++) printf("\t%s\n",dirs[i]);
    // printf("Files:\n");
    // for(int i=0; i<numFiles; i++) printf("\t%s\n",files[i]);
    // printf("Links:\n");
    // for(int i=0; i<numLinks; i++) printf("\t%s\n",links[i]);
    // printf("numDir: %d\tnumFiles: %d\tnumLinks: %d\n",numDir,numFiles,numLinks);  
    // printf("Flag d settato a %d\nFlag R settato a %d\nFlag l settato a %d con valore %d\n",
    //         d_flag,
    //         R_flag,
    //         l_flag, 
    //         mod);
    

    // --- FINE PARSE PARAMETRI ---

    // se [files] c'e esegui su ogni el in files (nell'ordine ..)
    if (paramLength > 0)
    {
        for(int i=0; i<numFilesAndLinks; i++)
            print_line(filesAndLinks[i], l_flag, mod);
        for(int i=0; i<numDir; i++)
            prova(dirs[i], d_flag, l_flag, mod, R_flag);
    }
    // se non c'e files esegui su "."
    else
        prova(".", d_flag, l_flag, mod, R_flag);
    return 0;
}

int prova(char *root, int d_flag, int l_flag, int mod, int R_flag)
{
    if(d_flag) {print_line(root, l_flag, mod); return 0;}
    if(R_flag == 1) print_dir_recursive(root, l_flag, mod);
    else print_dir(root, l_flag, mod);
}

void print_dir_recursive(char *dir, int l_flag, int mod)
{
    printf("\n%s:\n", dir);
    int num_files = count_files(dir);
    char *dir_files[num_files];
    list_files(dir, dir_files, num_files);
    for(int i=0; i<num_files; i++) print_line(dir_files[i], l_flag, mod);
    for(int i=0; i<num_files; i++)
        if (path_type(dir_files[i]) == 0) 
        {
            char newp[256];
            char *basename_dir = basename(dir_files[i]);
            strcpy(newp, dir);
            if(dir[strlen(dir)-1] != '/') strcat(newp, "/");
            strcat(newp, basename_dir);
            print_dir_recursive(newp, l_flag, mod);
        }
}


void print_dir(char *dir, int l_flag, int mod)
{
    int num_files = count_files(dir);
    char *dir_files[num_files];
    list_files(dir, dir_files, num_files);
    for(int i=0; i<num_files; i++) print_line(dir_files[i], l_flag, mod);
}


// funzione che stampa in base ai flag una directory
void print_line(char *path, int l_flag, int mod)
{
    char *basename_path = basename(path);
    if (l_flag == 0)
        printf("%s\n", basename_path);
    else
    {
        // stampa "permessi \t hlink \t dim \t nome"
        if( mod == 0 )
        {
            print_permission(path);
            printf("\t%d\t%d\t%s\n", get_hlink_count(path), get_file_dim(path), basename_path);
        }
        // stampa "permessi \t nome"
        else { print_permission(path); printf("\t%s\n", basename_path);  }
    }
    
}

// funzione del print usage
void print_usage(char *n) { printf("Usage %s [-dR] [-l mod] [files]\n", n);}

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

// if path is directory --> return 0;
// if path is regFile -->   return 1;
// if path is symlink -->   return 2;
int path_type(char *path)
{
    struct stat st;
    if ( lstat(path, &st) == 0 && S_ISDIR(st.st_mode) ) return 0;
    if ( lstat(path, &st) == 0 && S_ISREG(st.st_mode) ) return 1;
    if ( lstat(path, &st) == 0 && S_ISLNK(st.st_mode) ) return 2;
    return -1;
}

// funzione che conta quanti file ci sono in una directory
int count_files(char *path)
{
    struct dirent **filesTMP;
    int num_of_filesTMP = scandir(path, &filesTMP, NULL, alphasort);
    int num_of_files = 0;
    for(int i=0; i<num_of_filesTMP; i++)
        if(filesTMP[i]->d_name[0] != '.') num_of_files++;
    return num_of_files;
}

void list_files(char *path, char *f[], int num_files)
{
    struct dirent **filesTMP;
    int num_of_filesTMP = scandir(path, &filesTMP, NULL, alphasort);
    int j=0; 
    for(int i=0; i<num_of_filesTMP; i++)
        if(filesTMP[i]->d_name[0] != '.')
        {
            // char *newp = malloc(256*sizeof(char));
            // strcpy(newp, path);
            // strcat(newp, "/");
            // strcat(newp, filesTMP[i]->d_name);
            // strcpy(f[--num_files], newp);
            // free(newp);
            // newp = NULL;
            f[j++] = malloc(256*sizeof(char*));
            strcpy(f[j-1], path);
            strcat(f[j-1], "/");
            strcat(f[j-1], filesTMP[i]->d_name);
            //strcpy(f[--num_files], newp);
            // f[--num_files] = filesTMP[i]->d_name;
        }
}


static char *lsperms(int mode)
{
    static const char *rwx[] = {"---", "--x", "-w-", "-wx",
    "r--", "r-x", "rw-", "rwx"};
    static char bits[11];

    bits[0] = filetypeletter(mode);
    strcpy(&bits[1], rwx[(mode >> 6)& 7]);
    strcpy(&bits[4], rwx[(mode >> 3)& 7]);
    strcpy(&bits[7], rwx[(mode & 7)]);
    if (mode & S_ISUID)
        bits[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        bits[6] = (mode & S_IXGRP) ? 's' : 'l';
    if (mode & S_ISVTX)
        bits[9] = (mode & S_IXOTH) ? 't' : 'T';
    bits[10] = '\0';
    return(bits);
}

static int filetypeletter(int mode)
{
    char c;
    if (S_ISDIR(mode)) c = 'd';
    else if (S_ISREG(mode)) c = '-';
    else if (S_ISLNK(mode)) c = 'l';
    else c = '?';

    return (c);
}


// funzione che printa i permessi di un file a un certo path
int print_permission(char *path)
{
    struct stat st;
    if (stat(path, &st)<0) return 1;

    printf("%s", lsperms(st.st_mode));
    return 0;
}

// funzione che resistuisce la dimensione in byte di un file/directory
int get_file_dim(char *path)
{
    struct stat st;
    if (stat(path, &st)<0) return -1;
    return st.st_size;
}

// funzione che restituisce il numero di hard link di un file
int get_hlink_count(char *path)
{
    struct stat st;
    if (stat(path, &st)<0) return -1;
    return st.st_nlink;
}

// funzione comparatrice di stringhe
static int string_comparator(const void* a, const void* b) { return strcmp(*(const char**)a, *(const char**)b); }   

// funzione che ordina un array di stringhe
void sort(char **array, int n) { qsort(array, n, sizeof(const char*), string_comparator);} 