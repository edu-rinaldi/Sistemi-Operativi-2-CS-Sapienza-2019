#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>



// --- FUNCTION DECLARATION ---

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
void list_files(char *path, int num_files, char **files);

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
    
    int paramLength = argc-optind, numDir = 0, numFiles = 0, numLinks = 0;
    char *param[paramLength];
    for(int i=0; i<paramLength; i++)
    {
        // array con tutti i parametri in [files]
        param[i] = argv[i+optind];
        // calcolo le lunghezze dei 3 array
        switch(path_type(param[i]))
        {
            // se è una directory
            case 0: numDir++; break;
            // se è un file
            case 1: numFiles++; break;
            // se è un link simbolico
            case 2: numLinks++; break;
        }
    }

    char *dirs[numDir];
    char *files[numFiles];
    char *links[numLinks];
    int dirsIndex = 0, filesIndex = 0, linksIndex = 0;
    
    // creo l'array con i path delle dir, dei file, dei link
    for(int i=0; i<paramLength; i++)
    {
        switch(path_type(param[i]))
        {
            case 0: dirs[dirsIndex++] = param[i]; break;
            case 1: files[filesIndex++] = param[i]; break;
            case 2: links[linksIndex++] = param[i]; break;
        }
    }

    // li ordino
    sort(dirs, dirsIndex);
    sort(files, filesIndex);
    sort(links, linksIndex);


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

    // int num_files = count_files(".");
    // char *dir_files[num_files];
    // list_files(".", num_files, dir_files);
    // for(int i=0; i<num_files; i++) printf("%s\t%d\n", dir_files[i], i);   

    // se [files] c'e esegui su ogni el in files (nell'ordine ..)
    if (paramLength > 0)
    {

    }
    // se non c'e files esegui su "."
    else
    {
        char *root = "/Users/eduardo/Downloads/grader.2/all/input_output.1/inp.6/";
        if(R_flag == 1) print_dir_recursive(root, l_flag, mod);
        else print_dir(root, l_flag, mod);
    }

    return 0;
}


void print_dir_recursive(char *dir, int l_flag, int mod)
{
    printf("\n%s:\n", dir);
    int num_files = count_files(dir);
    char *dir_files[num_files];
    list_files(dir, num_files, dir_files);
    for(int i=0; i<num_files; i++) print_line(dir_files[i], l_flag, mod);
    for(int i=0; i<num_files; i++)
        if (path_type(dir_files[i]) == 0) 
        {
            char newp[256];
            char *basename_dir = basename(dir_files[i]);
            strcpy(newp, dir);
            strcat(newp, "/");
            strcat(newp, basename_dir);
            print_dir_recursive(newp, l_flag, mod);
        }
}


void print_dir(char *dir, int l_flag, int mod)
{
    int num_files = count_files(dir);
    char *dir_files[num_files];
    list_files(dir, num_files, dir_files);
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
        else
        {
            print_permission(path);
            printf("\t%s\n", basename_path);   
        }
        
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
    struct dirent *dp;
    DIR *dir = opendir(path);
    int count = 0;
    // se non riesce ad aprire la directory
    if (!dir) return -1;
    //se riesce conta i file
    while ((dp = readdir(dir)) != NULL) 
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0 && dp->d_name[0] != '.' ) 
            count++;
    //chiudi la directory
    closedir(dir);
    return count;
}

// funzione che restituisce un array contenente i file in una directory (ordinati)
void list_files(char *path, int num_files, char *files[])
{
    struct dirent *dp;
    DIR *dir = opendir(path);
    int i = num_files;
    // se non riesce ad aprire la directory
    if (!dir) return;
    // se riesce aggiungi il path
    while ((dp = readdir(dir)) != NULL)
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0 && dp->d_name[0] != '.')
        {
            char *newp = malloc(256*sizeof(char));
            strcpy(newp, path);
            strcat(newp, "/");
            strcat(newp, dp->d_name);
            files[--i] = malloc(strlen(newp) + 1);
            strcpy(files[i], newp);
            free(newp);
            newp = NULL;
        }
    //chiudi la directory
    closedir(dir);
    sort(files, num_files);

}

// funzione che printa i permessi di un file a un certo path
int print_permission(char *path)
{
    struct stat st;
    if (stat(path, &st)<0) return 1;

    char permissions[11];
    permissions[0] = S_ISDIR(st.st_mode) ? 'd' : '-';
    permissions[1] = st.st_mode & S_IRUSR ? 'r' : '-';
    permissions[2] = st.st_mode & S_IWUSR ? 'w' : '-';
    permissions[3] = st.st_mode & S_IXUSR ? 'x' : '-';
    permissions[4] = st.st_mode & S_IRGRP ? 'r' : '-';
    permissions[5] = st.st_mode & S_IWGRP ? 'w' : '-';
    permissions[6] = st.st_mode & S_IXGRP ? 'x' : '-';
    permissions[7] = st.st_mode & S_IROTH ? 'r' : '-';
    permissions[8] = st.st_mode & S_IWOTH ? 'w' : '-';
    permissions[9] = st.st_mode & S_IXOTH ? 'x' : '-';
    printf("%s", permissions);
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