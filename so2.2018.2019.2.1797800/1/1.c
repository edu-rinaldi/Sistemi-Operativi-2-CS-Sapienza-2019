#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <locale.h>
#include <unistd.h>

int countPrint = 0;
char fname[256];
int countNotFile = 0;
// --- FUNCTION DECLARATION ---
// funzione del print usage
void print_usage() { fprintf(stderr, "Usage %s [-dR] [-l mod] [files]\n", fname);}

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

/* 
    se e' una directory --> return 0
    se e' un file       --> return 1
    se e' un link       --> return 2
    se non esiste       --> return -1 
*/
int path_type(char *path)
{
    struct stat st;
    int res_lstat = lstat(path, &st);
    if ( res_lstat == 0 && S_ISDIR(st.st_mode) ) return 0;
    if ( res_lstat == 0 && S_ISREG(st.st_mode) ) return 1;
    if ( res_lstat == 0 && S_ISLNK(st.st_mode) ) return 2;
    return -1;
}

// funzione che restituisce quanti file ci sono in una directory (escludendo i file nascosti)
int count_files(char *path)
{
    //usa scandir per contare
    struct dirent **filesTMP;
    int num_of_filesTMP = scandir(path, &filesTMP, NULL, alphasort);
    int num_of_files = 0;
    // toglie dal conteggio i file nascosti e libera la memoria man mano
    for(int i=0; i<num_of_filesTMP; i++)
    {
        if(filesTMP[i]->d_name[0] != '.') num_of_files++;
        free(filesTMP[i]);
    }
    // libera la memoria
    free(filesTMP);
    return num_of_files;
}

/*
    Funzione che dato un path, riempie "l'array di stringhe" f con
    tutti i file all'interno di un path.
    num_files è il length(f)
*/
void list_files(char *path, char *f[], int num_files)
{
    // mette in filesTMP tutti i file nella directory, ordinandoli
    struct dirent **filesTMP;
    int num_of_filesTMP = scandir(path, &filesTMP, NULL, alphasort);
    int j=0; 
    for(int i=0; i<num_of_filesTMP; i++)
        {
            // mette in f il file nel seguente modo path+"/"+nome_file, solo se non è un file nascosto
            if(filesTMP[i]->d_name[0] != '.')
            {
                f[j++] = malloc(256*sizeof(char*));
                strcpy(f[j-1], path);
                if (f[j-1][strlen(f[j-1])-1] != '/') strcat(f[j-1], "/");
                strcat(f[j-1], filesTMP[i]->d_name);
            }
            // libera la memoria
            free(filesTMP[i]);
        }
    // libera la memoria
    free(filesTMP);
}

/*
    funzione usata per la stampa dei permessi
    e' una dir --> 'd'
    e' un file --> '-'
    e' un link --> 'l'
    altro ?    --> '?'

*/
char tipo_file(int mode) { return S_ISDIR(mode) ? 'd' : (S_ISREG(mode) ? '-' : (S_ISLNK(mode) ? 'l' : '?'));}

// funzione che restituisce la stringa con i permessi
char *get_ls_permissions(char *path)
{
    struct stat st;
    lstat(path, &st);

    static char bits[11];
    bits[1] = st.st_mode & S_IRUSR ? 'r' : '-';
    bits[2] = st.st_mode & S_IWUSR ? 'w' : '-';
    bits[3] = st.st_mode & S_IXUSR ? 'x' : '-';
    bits[4] = st.st_mode & S_IRGRP ? 'r' : '-';
    bits[5] = st.st_mode & S_IWGRP ? 'w' : '-';
    bits[6] = st.st_mode & S_IXGRP ? 'x' : '-';
    bits[7] = st.st_mode & S_IROTH ? 'r' : '-';
    bits[8] = st.st_mode & S_IWOTH ? 'w' : '-';
    bits[9] = st.st_mode & S_IXOTH ? 'x' : '-';
    bits[0] = tipo_file(st.st_mode);
    // settaggio degli sticky bits
    if (st.st_mode & S_ISUID)
        bits[3] = (st.st_mode & S_IXUSR) ? 's' : 'S';
    if (st.st_mode & S_ISGID)
        bits[6] = (st.st_mode & S_IXGRP) ? 's' : 'S';
    if (st.st_mode & S_ISVTX)
        bits[9] = (st.st_mode & S_IXOTH) ? 't' : 'T';
    bits[10] = '\0';
    return bits;
}

// funzione che printa i permessi di un file a un certo path
void print_permission(char *path) {printf("%s", get_ls_permissions(path));}

// funzione che resistuisce la dimensione in byte di un file/directory
int get_file_dim(char *path) { struct stat st; lstat(path, &st); return st.st_size; }

// funzione che restituisce il numero di hard link di un file
int get_hlink_count(char *path) { struct stat st; stat(path, &st); return st.st_nlink; }

// funzione comparatrice di stringhe
static int string_comparator(const void* a, const void* b) { return strcmp(*(const char**)a, *(const char**)b); }   

// funzione che ordina un array di stringhe
void sort(char **array, int n) { qsort(array, n, sizeof(const char*), string_comparator);} 

// funzione che stampa in base ai flag una directory
void print_line(char *path, int l_flag, int mod, int isParam)
{
    countPrint++;
    // se il file esiste
    if(path_type(path) != -1)
    {
        // se è un parametro allora stampa il path per intero, altrimenti il basename
        char *basename_path = isParam == 1 ? path : basename(path) ;
        // se l_flag non è attivo  printa il path 
        if (l_flag == 0) printf("%s", basename_path);
        else
        {
            // stampa "permessi \t hlink \t dim \t nome"
            if( mod == 0 )
            {
                print_permission(path); 
                printf("\t%d\t%d\t%s", get_hlink_count(path), get_file_dim(path), basename_path);
            }
            // stampa "permessi \t nome"
            else { print_permission(path); printf("\t%s", basename_path);  }
            // prendi il path del file a cui punta
            char target[256];
            int r = readlink(path, target, 256);
            target[r] = 0;
            // se è un link printa " --> target"
            if(path_type(path) == 2)
                printf(" -> %s", target);
        }
        printf("\n");
    }
}

// funzione che restituisce il valore "totale" del comando ls -l data una lista di file
int getTotale(int num_files, char *dir_files[])
{
    int totale = 0;
    for(int i=0; i<num_files; i++)
    {
        struct stat st;
        int tmp = 0;
        if( lstat(dir_files[i], &st) == 0 && !S_ISLNK(st.st_mode))
        {
            tmp = st.st_size/st.st_blksize + (st.st_size % st.st_blksize != 0);
            tmp *= st.st_blksize;
            totale += tmp;
        } 
    }
    // prende la var d'ambiente BLOCKSIZE
    char *env = getenv("BLOCKSIZE");
    int blocksize = 1024;
    // se la var esiste prendi il valore numerico, altrimenti lascia il val di default = 1024
    if (env != NULL) blocksize = atoi(env);
    // dividi il totale per blocksize
    totale /= blocksize;
    return totale;
}

// funzione che printa ricorsivamente gli elementi di una directory
void print_dir_recursive(char *dir, int l_flag, int mod)
{
    // se non è il primo print metti lo \n
    if (countPrint > 0)
        printf("\n%s:\n", dir);
    // altrimenti non metterlo e conta il print
    else
        {printf("%s:\n", dir); countPrint++;}
    // prendi il numero dei file nella directory
    int num_files = count_files(dir);
    char *dir_files[num_files];
    // metti in dir_files tutti i files nella directory dir
    list_files(dir, dir_files, num_files);
    // calcola il totale
    int totale = getTotale(num_files, dir_files);
    // se c'e' l_flag stampa totale
    if(l_flag) printf("total %d\n", totale);
    // printa
    for(int i=0; i<num_files; i++) print_line(dir_files[i], l_flag, mod, 0);
    // chiama ricorsivamente su tutte le directory in dir
    for(int i=0; i<num_files; i++)
        if (path_type(dir_files[i]) == 0) 
            print_dir_recursive(dir_files[i], l_flag, mod);
    // fai il free
    for(int i=0; i<num_files; i++) free(dir_files[i]);
}

// funzione che printa gli elementi di una directory
void print_dir(char *dir, int l_flag, int mod, int isParam)
{
    // se è un parametro printa il nome della directory
    if (isParam && countPrint>0) { printf("\n%s:\n", dir); countPrint++;}
    if (isParam && countPrint == 0) {printf("%s:\n", dir); countPrint++;}
    // conta quanti file ci sono
    int num_files = count_files(dir);
    char *dir_files[num_files];
    // mettili in dir_files
    list_files(dir, dir_files, num_files);
    // calcola il totale e printalo se l_flag è attivo
    int totale = getTotale(num_files, dir_files);
    if(l_flag) printf("total %d\n", totale);
    // printa e libera la memoria
    for(int i=0; i<num_files; i++) print_line(dir_files[i], l_flag, mod, 0);
    for(int i=0; i<num_files; i++) free(dir_files[i]);
}

// funzione che scegli se fare un ls ricorsivo o meno
void ls_dir(char *root, int l_flag, int mod, int R_flag, int isParam)
{
    if(R_flag == 1) print_dir_recursive(root, l_flag, mod);
    else print_dir(root, l_flag, mod, isParam);
}

// --- END FUNCTION DECLARATION ---


int main(int argc, char **argv)
{
    setlocale(LC_ALL, "C");
    strcpy(fname, argv[0]);
    // --- INIZIO PARSE PARAMETRI ---
    int option;
    int d_flag = 0, R_flag = 0, l_flag = 0, mod = 0;
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
                if (!string_is_number(optarg)) { print_usage(); exit(20); }
                //altrimenti convertilo
                mod = atoi(optarg);
                break;
            // -l senza argomento
            case ':':
                print_usage(); exit(20); break;
            // parametro inesistente
            case '?': print_usage(); exit(20); break;
        }
    }
    
    int paramLength = argc-optind, numDir = 0, numFilesAndLinks = 0;
    char *param[paramLength];
    for(int i=0; i<paramLength; i++)
    {
        // array con tutti i parametri in [files]
        param[i] = argv[i+optind];
        
        // calcolo le lunghezze dei 2 array
        switch(path_type(param[i]))
        {
            // se è una directory
            case 0: numDir++; break;
            // se è un file
            case 1: numFilesAndLinks++; break;
            // se è un link simbolico
            case 2: numFilesAndLinks++; break;
            // se non esiste printa su stderr
            case -1: 
                fprintf(stderr, "%s: cannot access \'%s\': No such file or directory\n",fname, param[i]); 
                countNotFile++;
                break;
        }
    }

    char *dirs[numDir];
    char *filesAndLinks[numFilesAndLinks];
    int dirsIndex = 0, filesIndex = 0, linksIndex = 0, filesAndLinksIndex = 0;
    
    // creo l'array con i path delle dir, dei file e link
    for(int i=0; i<paramLength; i++)
    {
        switch(path_type(param[i]))
        {
            case 0: dirs[dirsIndex++] = param[i]; break;
            case 1: filesAndLinks[filesAndLinksIndex++] = param[i]; break;
            case 2: filesAndLinks[filesAndLinksIndex++] = param[i]; break;
        }
    }

    // li ordino
    sort(dirs, dirsIndex);
    sort(filesAndLinks, filesAndLinksIndex);
    sort(param, paramLength);
    
    // --- FINE PARSE PARAMETRI ---
    
    // se il d_flag e' attivo
    if(d_flag)
    {
        // e ci sono parametri listali
        if(paramLength>0)
            for(int i=0; i<paramLength; i++) print_line(param[i], l_flag, mod, 1);
        // se non ci sono parametri printa la directory corrente
        else
            print_line(".", l_flag, mod, 1);
    }
    else
    {
        // se ci sono parametri
        if (paramLength > 0)
        {
            // printa i files e i link
            for(int i=0; i<numFilesAndLinks; i++)
                print_line(filesAndLinks[i], l_flag, mod, 1);
            // poi vedi se stampare il contenuto delle directory in modo ricorsivo o meno
            for(int i=0; i<numDir; i++)
                ls_dir(dirs[i], l_flag, mod, R_flag, 1);
        }
        // se non ci sono parametri stampa il contenuto, ricorsivo o meno su "."
        else
            ls_dir(".", l_flag, mod, R_flag, 0);
    }
    // restituisci i file non esistenti passati come parametro
    return countNotFile;
}

