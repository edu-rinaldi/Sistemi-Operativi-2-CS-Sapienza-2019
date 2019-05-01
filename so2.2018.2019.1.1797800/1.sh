#!/bin/bash

#function declaration
print_error()
{
    local exitcode=$1
    local error_message=$2
    >&2 echo $error_message; exit $exitcode
}

get_date_res=""
get_date()
{
    local str1=$1
    local DATE_REGEX="([0-9]{4})(0[1-9]|1[0-2])([0-2][0-9]|3[01])([01][0-9]|2[0-3])([0-5][0-9])"
    get_date_res=$( echo "$str1" | egrep -o $DATE_REGEX)
    return 0
}

#script file's basename
SCRIPT_BASENAME=$(basename $0)
#init flag
E_FLAG=0
B_FLAG=0
B_DIR=""
#regex
DATE_REGEX=".*_([0-9]{4})(0[1-9]|1[0-2])([0-2][0-9]|3[01])([01][0-9]|2[0-3])([0-5][0-9])_.*(\.(jpg|JPG|txt|TXT)$)"

#error messages
USAGE_MESSAGE="Uso: $SCRIPT_BASENAME [opzioni] directory"

#MAIN

while getopts ":eb:" c
do
    case $c in
        # -e flag attivo
        e) E_FLAG=1;;
        # -b flag attivo
        b) B_FLAG=1; B_DIR=$OPTARG;;
        # parametro inesistente
        \?) print_error 10 "$USAGE_MESSAGE";;
        # -b senza argomento
        :) print_error 10 "$USAGE_MESSAGE";;
    esac
done

# -e e -b entrambi settati --> exit 10
if [ $E_FLAG == 1 ] && [ $B_FLAG == 1 ]; then print_error 10 "$USAGE_MESSAGE"; fi;

# -b errors
B_NO_PERMISSIONS="L'argomento $B_DIR non e' valido in quanto non ha i permessi richiesti"
B_PARAM_NOT_VALID="L'argomento $B_DIR non e' valido"

#prendo directory
shift $(($OPTIND - 1))
directory="$@"
# -d errors
D_NOT_EXIST="L'argomento $directory non e' valido in quanto non esiste"
D_REGULAR_FILE="L'argomento $directory non e' valido in quanto non e' una directory"
D_NO_PERMISSIONS="L'argomento $directory non e' valido in quanto non ha i permessi richiesti"

# non viene passato directory --> exit 10
if [ -z $directory ]; then print_error 10 "$USAGE_MESSAGE"; fi;

# se d non esiste --> exit 100
if [ ! -e $directory ]; then print_error 100 "$D_NOT_EXIST"; fi;

# se d esiste ed è un file regolare --> exit 100
if [ -f $directory ]; then print_error 100 "$D_REGULAR_FILE"; fi;

# se d non ha i permessi di lettura e esecuzione --> exit 100
if [ ! -r $directory ] || [ ! -x $directory ]; then print_error 100 "$D_NO_PERMISSIONS"; fi;

# se il flag di b è 1
if [ $B_FLAG == 1 ]
    then
        # vedi se il parametro è una directory
        if [ -d $B_DIR ]
            then
                #non ha i permessi? --> errore 200
                B_DIR_PERMS="$(stat -c "%a %n" $B_DIR )"
                if [ "$B_DIR_PERMS" != "700" ]; then print_error 200 "$B_NO_PERMISSIONS"; fi;
            else
                #se non è una directory esistente, allora creala con i giusti permessi
                mkdir $B_DIR && chmod 700 $B_DIR
        fi 
fi

# --- INIZIO CALCOLO INSIEME F ---

#Insieme F finale inizializzato a vuoto
f_final=""

#calcolo f'
f_primo=""
#Per ogni file che matcha la regex della data
for f in $(find $directory  | egrep $DATE_REGEX)
    do
        #se è un link simbolico
        if [ -L "$f" ]
            then
                #prendi il file a cui punta e la sua data
                get_date $(readlink "$f")
                symlink_date=$get_date_res
                get_date $(basename "$f")
                f_date=$get_date_res
                #se le date sono uguali
                if [ "$symlink_date" == "$f_date" ]; 
                    then 
                        #aggiungi il file a f'
                        f_primo+=" $f "
                fi
        fi
    done

for s in $f_primo
do
    if [[ $f_final != *"$s"* ]]; then f_final+="$s "; fi;
done

#calcolo f"
inodes=""

for g in $(find $directory | egrep $DATE_REGEX)
do
    if [[ $f_final != *"$g"* && "$(stat --format %h $g)" > "1"  && $inodes != *"$(stat --format %i $g)"* ]]; then
        inodes+="$(stat --format %i $g) "
        #prendi la data
        get_date $g
        for f in $(find $directory -samefile $g | egrep ".*$get_date_res.*(\.(jpg|JPG|txt|TXT)$)")
        do
            #se esiste almeno un file con stessa data ed è un HL, ma non è se stesso
            if [[ "$f" != "$g" && ! -z "$f" ]]; then 
                #prendi tutti i file con quell'inode e prendi quello con max path
                inode=$(stat --format %i $f)
                file=$( find $directory -inum $inode | LC_ALL=C sort -r | head -1 )
                #aggiungilo all'insieme finale
                f_final+="$file "
                break 
                fi
        done
    fi
done


#calcolo f terzo
controllati="$f_final"
f_terzo="$f_final"
for f in $(find $directory | egrep $DATE_REGEX)
do
    if [[ $controllati != *"$f"* ]]; then
        get_date "$f"
        file_uguali="$f"
        fl="0"
        for g in $(find $directory | egrep ".*$get_date_res.*(\.(jpg|JPG|txt|TXT)$)" )
        do
            if [[ $f_terzo != *"$g"* && "$f" != "$g" ]] && cmp -s $f $g ; then
                fl="1"
                file_uguali="$file_uguali $g"
            fi
        done
        controllati="$controllati $file_uguali"
        if [ $fl == "1" ]; then
            f_terzo="$f_terzo $(echo "$file_uguali" | tr ' ' '\n' | LC_ALL=C sort -r | tr '\n' ' ' | cut -d " " -f2- )"
        fi
    fi
done


for s in $f_terzo
do
    if [[ $f_final != *"$s"* ]]; then f_final+="$s "; fi;
done


fs=$(echo "$f_final" | tr ' ' '\n' | LC_ALL=C sort | tr '\n' '|')
fs=${fs#"|"}
fs=${fs%"|"}
f_final=$(echo "$f_final" | tr ' ' '\n' | LC_ALL=C sort | tr '\n' ' ')
if [ ! -z $fs ]; then echo $fs; fi;
if [ $E_FLAG == 1 ]
        then
            exit 0           
fi

if [ $B_FLAG == 1 ] && [ $B_DIR != "" ]
    then
        for file in $f_final
        do
            newdir=$(dirname ${file/${directory}/${B_DIR}})
            mkdir -p $newdir
            mv "$file" "$newdir"
        done
        exit 0
fi

rm $f_final

for f in $(find $path -regextype posix-extended -type l -regex $DATE_REGEX)
do
        #se è un link simbolico
        if [[ ! -a "$f" ]]; then
            unlink $f
        fi
done
