#!/usr/bin/awk -f

# Inizializzo script
BEGIN {
    argomenti = ARGV[1]
    for (i = 2 ; i<ARGC ; ++i) argomenti=argomenti" "ARGV[i]
    print "Eseguito con argomenti "argomenti > "/dev/stdout"
    print "Eseguito con argomenti "argomenti > "/dev/stderr"
    fileRE = "\\(\\.[^ ]*\\.(tex|cls|sty|bbl|aux)"
    imgRE = "File: \\./[^ ]+\\.(jpg|png|pdf)"
    # handling:  non vengono dati almeno 2 argomenti
    if (ARGC < 3)
    {
        print "Errore: dare almeno 2 file di input" > "/dev/stderr"
        exit(0)
    }
}

BEGINFILE { 
    ++numFileOpened
    # se è il file I1 setta un flag a true
    if (FILENAME ~ "(\\.txt)$" && numFileOpened == 1) 
    {
        i1flag = 1
        strip_comments = 0
        also_figs = 0
        only_figs = 0
    }
    # se il file di log è il primo esci con errore 0
    if (FILENAME ~ "(\\.log)$" && numFileOpened == 1) exit(0)
    # se il file non è incluso nel file di log
    if (numFileOpened>2 && !(FILENAME in inclusi) && strip_comments == 1) 
    {
        print "Errore: il file "FILENAME" non risulta incluso" > "/dev/stderr"
        fileNonInclusi++
        nextfile
    }

}


ENDFILE {
    # se il file è il primo e di testo
    if (numFileOpened == 1) 
    {
        # dai errore se c'è una config non valida
        if (also_figs == 0 && only_figs == 1) {print "Errore di configurazione: only_figs=1 e also_figs=0" > "/dev/stderr" ; exit(0);}
        i1flag = 0
    }
    # se il file è quello di log
    if (FILENAME ~ "(\\.log)$") 
        for (i=0; i<length(files); i++)
            {
                # se è un file .tex,pdf,jpg,png stampalo
                if (files[i] ~ "(\\.(tex|pdf|png|jpg))") print files[i]
                # aggiungilo al "set" di file inclusi
                inclusi[files[i]] = 0
            }
    if (numFileOpened>2 && strip_comments == 1)
    {
        for(i=0;i<length(allrows); i++)
        {
            if (allrows[i] in lineeDaEliminare) continue;
            # se è da modificare
            if (allrows[i] in lineeDaModificare)
            {
                # modificalo
                match(allrows[i], /[^\\]\%/)
                row = substr(allrows[i],1, RSTART)
                #scrivi su file
                print row > FILENAME
                continue;
            }
            #scrivilo su file se non è un commento
            print allrows[i] > FILENAME
        }
    }
    delete allrows
    allrowsIndex = 0
    delete lineeDaEliminare
    delete lineeDaModificare
}

#flag iniziali
/strip_comments=1/ { if (i1flag == 1) strip_comments = 1 }
/also_figs=1/ { if (i1flag == 1) also_figs = 1 }
/only_figs=1/ { if (i1flag == 1) only_figs = 1 }

# --- Prende tutte le righe ---
{
    # se è un file di log prendi tutti i file interessati
    if ( FILENAME ~ "(\\.log)$" )
    {
        # accoda ogni riga a una stringa
        stringa=stringa""$0
        # appena matcha il regex fileRE
        if (stringa ~ fileRE && only_figs == 0)
        {
            # fai il match e prendi indice di inizio e lunghezza
            while (match(stringa, fileRE))
            {
                # prendi la sottostringa --> file
                file = substr(stringa, RSTART+1, RLENGTH)
                # se finisce con ) o spazio togli quel carattere
                if (file ~ "(\\)| )$") file = substr(file, 0, length(file)-1);
                # togli dalla stringa il file matchato
                sub(fileRE, "", stringa)
                # e aggiungilo a un array di tutti i files
                files[filesArraySize++] = file
            }
            
        }
        if (also_figs == 1 || only_figs == 1)
        {
            # matcha l'img
            if (stringa ~ imgRE)
            {
                # fai il match e prendi indicie di inizio e lunghezza
                match(stringa, imgRE)
                # prendi la sottostringa --> img
                img = substr(stringa, RSTART+6, RLENGTH-6)
                # se finisce con ) togli quel carattere
                if (img ~ "\\)$") img = substr(img, 0, length(img)-1);
                # togli dalla stringa il file matchato
                sub(imgRE, "", stringa)
                # e aggiungilo a un array di tutte le immagini
                files[filesArraySize++] = img
            }
        }
    }
    allrows[allrowsIndex++] = $0
}

/([^\\]\%|^\%)/ {
    # se è ad inizio riga o è preceduto da spazi
    if (($0 ~ "^%") || ($0 ~ "^( +%)") || ($0 ~ "^([ 	]+%)"))
        lineeDaEliminare[$0] = 0;
    else lineeDaModificare[$0] = 0
}

END {
    exit(fileNonInclusi)
}