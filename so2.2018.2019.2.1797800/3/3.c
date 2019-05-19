#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
char *pname, *f_in, *f_out, *s;
int i1, i2;

void print_usage() {fprintf(stderr, "Usage: %s filein fileout awk_script i1 i2\n", pname); exit(10); }

int read_permission(char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && (st.st_mode & S_IRUSR || st.st_mode & S_IRGRP || st.st_mode & S_IROTH);
}

int write_permission(char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && (st.st_mode & S_IWUSR || st.st_mode & S_IWGRP || st.st_mode & S_IWOTH);
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

// funzione che resistuisce la dimensione in byte di un file/directory
int get_file_dim(char *path) { struct stat st; lstat(path, &st); return st.st_size; }

// ./3 f_in f_out s i1 i2
int main(int argc, char **argv)
{
    // debug flag
    const int DEBUG_MODE = 0;
    // nome di un file temporaneo che userò per gawk
    char *TMP_FNAME = "file_temporaneo";
    // nome programma
    pname = argv[0];
    // se ho meno di 5 argomenti oltre il nome del programma o i1 e i2 non sono interi>0 --> print usage
    if(argc < 6 || !check_integer(argv[4]) || !check_integer(argv[5])) print_usage();
    
    // prendi i parametri
    f_in = argv[1], f_out = argv[2], s = argv[3];
    i1 = atoi(argv[4]), i2 = atoi(argv[5]);

    // se siamo in debug mode printa i parametri
    print_parameter(DEBUG_MODE);

    // se non ho i permessi di lettura o il file non esiste --> exit 20
    if(!read_permission(f_in)) {fprintf(stderr, "Unable to open file %s because of e\n", f_in); exit(20);}
    
    // apri il file binario f_in
    FILE *fileBin1 = fopen(f_in, "rb");

    int primo_num, secondo_num;
    // leggi i primi 4 byte
    int n = fread(&primo_num, 1, 4, fileBin1);

    // leggi i secondi 4 byte
    n += fread(&secondo_num, 1, 4, fileBin1);

    // la lunghezza del prossimo blocco è n1
    int primo_testo_len = primo_num;
    // inizializza la stringa che conterra il testo
    char primo_testo[primo_testo_len+1];
    // setta l'ultimo byte a 0 per terminare la stringa
    primo_testo[primo_testo_len] = 0;
    // leggi il testo
    n += fread(primo_testo, 1, primo_num, fileBin1);

    // fai la stessa cosa con la parte da negare
    int ascii_negato_len = secondo_num;
    char ascii_negato[ascii_negato_len+1];
    ascii_negato[ascii_negato_len] = 0;
    n += fread(ascii_negato, 1, ascii_negato_len, fileBin1);
    
    // se il file non ha almeno 8+n1+n2 byte --> errore
    if(n<8+primo_num+secondo_num) 
    {
        fprintf(stderr, "Wrong format for input binary file %s\n", f_in); 
        fclose(fileBin1); 
        exit(30); 
    }

    // nega i bit con l'operatore ~
    for(int i=0; i<ascii_negato_len; i++)
        ascii_negato[i] = ~ascii_negato[i];


    // carattere "Buffer" per leggere il file
    char c;
    // numero byte letti
    int byteLetti;
    // il testo decodicato sara' grande dimensione del file, meno i primi 2 blocchi da 4 byte
    char testo[get_file_dim(f_in)-8];

    // inizializzo la lunghezza del testo
    int testo_len = 0;
    // faccio il "merge" dei due vettori, piu il resto del testo
    for(int i=0; i<primo_testo_len; i++)
        testo[testo_len++] = primo_testo[i];
    
    for(int i=0; i<ascii_negato_len; i++)
        testo[testo_len++] = ascii_negato[i];
    while(( byteLetti = fread(&c, 1, 1, fileBin1) ) >= 1 )
        testo[testo_len++] = c;
    // termino la stringa
    testo[testo_len] = 0;

    // apro un file temporaneo dove scrivere il testo da passare a gawk
    FILE *tmp = fopen(TMP_FNAME, "wb");
    // scrivo il testo
    fwrite(testo, testo_len, 1, tmp);
    // chiudo i due file
    fclose(fileBin1);
    fclose(tmp);
    // dichiaro i due file descriptor da utilizzare per le due pipes ( uno per l'output, l'altro per error)
    int file_descriptor[2];
    int fd_error[2];
    // inizializzo i due vettori con i file descriptor
    pipe(file_descriptor);
    pipe(fd_error);
    // creo subprocess
    pid_t pid = fork();

    // entro nel processo figlio
    if(pid == 0)
    {
        // chiudo i due file descriptor che non mi servono
        close(file_descriptor[0]);
        close(fd_error[0]);
        // apro STDOUT sul file descriptor scelto per l'out
        dup2(file_descriptor[1], STDOUT_FILENO);
        // apro STDERR sul file descriptor scelto per l'err
        dup2(fd_error[1], STDERR_FILENO);
        // preparo ed eseguo il programma
        char *argv[] = {(char *)"gawk", s, TMP_FNAME, NULL};
        execvp(argv[0], argv);
        // chiudo i fds e esco dal processo figlio
        close(file_descriptor[1]);
        close(fd_error[1]);
        exit(0);
    }
    // chiudo i fds che non mi servono
    close(file_descriptor[1]);
    close(fd_error[1]);
    // apro i due file descriptor
    FILE *out = fdopen(file_descriptor[0], "rb");
    FILE *err = fdopen(fd_error[0], "rb");

    // leggo in due buffer
    char buff[40000];
    char buff2[40000];
    int len_buff1 = 0, len_buff2 = 0;
    while(fread(&c, 1, 1, out) >= 1)
        buff[len_buff1++] = c;

    while(fread(&c, 1, 1, err) >= 1)
        buff2[len_buff2++] = c;
    
    // chiudo i fds
    fclose(out);
    fclose(err);

    // aspetto che il processo figlio finisca
    waitpid(pid, 0, 0);
    // tolgo il file tmp
    remove(TMP_FNAME);

    int len_testo_offuscato = len_buff1 + len_buff2;
    // faccio il merge dei due buffer in una sola stringa
    char testo_offuscato[len_testo_offuscato];
    for(int i=0; i<len_buff1; i++)
        testo_offuscato[i] = buff[i];
    for(int i=0; i<len_buff2; i++)
        testo_offuscato[i+len_buff1] = buff2[i];

    // se il testo da scrivere non è almeno i1+i2 byte --> exit con codice 80 
    int exit_80 = 0;
    if(len_testo_offuscato < i1+i2) {exit_80 = 1; i1 = 0; i2 = 0;}

    // offuscamento
    for(int i=i1; i<i1+i2; i++)
        testo_offuscato[i] = ~testo_offuscato[i];

    // apro il file binario su cui scrivere (fout)
    FILE *fileBin2 = fopen(f_out, "wb");

    // scrivo i primi due blocchi da 4  byte con i1 e i2 rispettivamente
    fwrite(&i1, 4, 1, fileBin2);
    fwrite(&i2, 4, 1, fileBin2);
    // scrivo il testo modificato da gawk e offuscato
    fwrite(testo_offuscato, len_testo_offuscato, 1, fileBin2);
    // chiudo il file
    fclose(fileBin2);
    return exit_80 ? 80 : 0;
}