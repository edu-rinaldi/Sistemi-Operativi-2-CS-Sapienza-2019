#!/bin/bash

print_error()
{
    local exitcode=$1
    local error_message=$2
    >&2 echo $error_message; exit $exitcode
}

# true se il comando esiste
command_exist()
{
    if type "$1" &> /dev/null ; then return 0; fi;
    return 1
}

# true se il processo esiste
process_exist()
{
    if [[ ! -z $(ps --no-headers $1) ]] ; then return 0; fi;
    return 1
}

#Script launch directory
SCRIPT_DIR=$0

ERROR_MSG="Usage: $SCRIPT_DIR bytes walltime sampling commands files"

# num di bytes
b="0"
# walltime
w="0"
# intervallo di campionamento
c="0"
# lista di commands
comandi=()
command=""
# lista di N files in f
fFiles=()
# lista di N files in g
gFiles=()

re='^[0-9]+$'
# controlla se b non è un numero
if ! [[ $1 =~ $re ]] ; then
    print_error 15 "$ERROR_MSG"
fi

# assegna b
b="$1"
shift


# controlla se w non è un numero
if ! [[ $1 =~ $re ]] ; then
    print_error 15 "$ERROR_MSG"
fi

# assegna w
w="$1"
shift


# controlla se c non è un numero
if ! [[ $1 =~ $re ]] ; then
    print_error 15 "$ERROR_MSG"
fi

# assegna c
c="$1"
shift

#finchè non contieni ;;;
str=""
if [ -z "$1" ]; then print_error 15 "$ERROR_MSG"; fi;
while [[ "$1" != *';;;'* ]]; do
    if [ -z "$1" ]; then print_error 30 "$ERROR_MSG"; fi;
    str+="$1 "
    shift
done
tmp=$1
str+="${tmp//;;;/}"
shift

tmp=()
IFS=';;' read -r -a tmp <<< "$str"
for i in ${!tmp[@]}; do
    if [[ ! -z ${tmp[$i]} ]]; then
        comandi+=( "$(echo ${tmp[$i]} | xargs )" )
    fi
done



N=${#comandi[@]}

#finchè non finiscono i files
while [ ! -z "$1" ]; do
    if [[ "${#fFiles[@]}" < "$N" ]]; then
        fFiles+=( "$1" )
    else
        gFiles+=( "$1" )
    fi
    shift

done


# se num di fFiles != N != gFiles --> error 30
if ! [[ "${#fFiles[@]}" == "$N" && "${#gFiles[@]}" == "$N" ]]; then
    print_error 30 "$ERROR_MSG"
fi

processiLanciati=""
numProcessiLanciati=0
#per ogni comando in comandi
for i in ${!comandi[@]}; do
    comando="${comandi[$i]}"
    nomeComando="$(printf $comando)"
    # se un comando esiste
    if command_exist "$nomeComando" ; then 
        #lancialo
        bash -c "$comando" &>${fFiles[$i]} 2>${gFiles[$i]} &
        #segnati il suo pid tra i processi lanciati
        processiLanciati+="$! "
        ((numProcessiLanciati++))
    fi
done

# scrivi sul file descriptor 3
processiLanciati=$(echo $processiLanciati | xargs )
>&3 echo $processiLanciati
while [[ ! -e "done.txt" && $numProcessiLanciati > "0" ]]; do
    #controlla ogni processo in processiLanciati
    for pid in $processiLanciati ; do
        # vedi se il processo è gia terminato
        if ! process_exist "$pid" ; then
            processiLanciati=${processiLanciati//$pid/}
            ((numProcessiLanciati--))
        else
            # prendi immagine processo
            pSize=$( expr $(ps -o vsz= "$pid") \* 1024 )
            # prendi elapsed time
            pW=$(echo $(ps --no-headers -eo pid,etime | egrep $pid) | cut -d " " -f 2 | cut -d ":" -f 1)
            # se pSize > b oppure elapsed time > walltime
            if [[ $b -gt "0" && $pSize -gt $b ]] || [[ $w > -gt  && $pW -gt $w ]]; then
                #killa il processo
                kill -SIGINT $pid
                processiLanciati=${processiLanciati//$pid/}
                ((numProcessiLanciati--))
            fi
        fi      
    done

    #fallo ogni c secondi
    sleep $c
done

if [[ -e "done.txt" ]]; then
    >&1 echo "File done.txt trovato"; exit 0 
fi

if [ $numProcessiLanciati -eq "0" ]; then
    >&1 echo "Tutti i processi sono terminati"; exit 1
fi