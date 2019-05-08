#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

void print_usage(char n[]);
int string_is_number(char s[]);

int main(int argc, char **argv)
{
    
    // --- INIZIO PARSE PARAMETRI ---
    int option;
    int d_flag = 0, R_flag = 0, l_flag = 0, mod = 0;
    
    while ((option = getopt(argc, argv, ":dRl:-:")) != -1 )
    {
        switch(option)
        {
            
            // -d
            case 'd': 
                d_flag = 1; 
                break;
            
            // -R
            case 'R':
                R_flag = 1;
                break;
            
            // -l
            case 'l':
                
                //setta il flag a true
                l_flag = 1;

                // se non è un numero --> exit 20
                if (!string_is_number(optarg)) 
                {
                    print_usage(argv[0]);
                    exit(20);
                }
                
                //altrimenti convertilo
                mod = atoi(optarg);
                break;
            
            // -l senza argomento
            case ':':
                print_usage(argv[0]);
                exit(20);
                break;
            // parametro inesistente
            case '?':
                print_usage(argv[0]);
                exit(20);
                break;
        }
    }

    printf("Flag d settato a %d\nFlag R settato a %d\nFlag l settato a %d con valore %d\n",d_flag, R_flag,l_flag, mod);
    
    // --- FINE PARSE PARAMETRI ---

    

    return 0;
}

//funzione del print usage
void print_usage(char n[]) { printf("Usage %s [-dR] [-l mod] [files]\n", n);}
//funzione che restituisce true se la stringa contiene SOLO numeri
int string_is_number(char s[]) 
{
    int slen = strlen(s);
    for(int i=0; i<slen; i++)
        if (!isnumber(s[i])) return 0;
    return 1;
}