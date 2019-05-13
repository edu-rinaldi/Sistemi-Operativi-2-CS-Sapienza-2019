#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>



// --- FUNCTION DECLARATION ---

void print_usage(char *n);
int isnumber(char c);
int string_is_number(char *s);
int path_type(char *path);
void list_files(char *path, int num_files, char **files);
int count_files(char *path);
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
    int num_files = count_files(".");
    char *dir_files[num_files];
    list_files(".", num_files, dir_files);
    for(int i=0; i<num_files; i++) printf("%s\n", dir_files[i]);
        

    return 0;
}

//funzione del print usage
void print_usage(char *n) { printf("Usage %s [-dR] [-l mod] [files]\n", n);}

int isnumber(char c) {return c >= '0' || c <= '9' ? 1 : 0;}

//funzione che restituisce true se la stringa contiene SOLO numeri
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
int count_files(char *path)
{
    struct dirent *dp;
    DIR *dir = opendir(path);
    int count = 0;
    // se non riesce ad aprire la directory
    if (!dir) return -1;
    //se riesce conta i file
    while ((dp = readdir(dir)) != NULL) count++;
    //chiudi la directory
    closedir(dir);
    return count;
}
void list_files(char *path, int num_files, char **files)
{
    struct dirent *dp;
    DIR *dir = opendir(path);
    // se non riesce ad aprire la directory
    if (!dir) return;
    // se riesce aggiungi il path
    while ((dp = readdir(dir)) != NULL) files[--num_files] = dp->d_name;
    //chiudi la directory
    closedir(dir);
}