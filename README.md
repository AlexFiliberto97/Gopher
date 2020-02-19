# Gopher Server
Scopo:

Sviluppo di un server, in grado di funzionare in modalità multi processo oppure multi thread, che attraverso il protocollo Gopher (descritto nel RFC 1436), permetta di accedere (in download) a file ed elenchi di file

Requisiti vincolanti nell'implementazione:
Il server deve offrire le stesse funzionalità sotto Linux e Windows.
Il server viene contattato per default sulla porta TCP 7070 (invece della 70).
La scelta sulla porta su cui rimane in attesa il server deve essere
modificabile tramite opzione da linea comandi e/o lettura di un file di configurazione.
Il server deve rileggere il file di configurazione in caso di invio di un segnale (sighup) per la versione Linux o
di un evento di console per la versione Windows.
In caso di cambio del numero di porta, il server chiude il socket in attesa sulla porta precedente e riapre un nuovo socket sulla nuova porta ma eventuali connessioni attive al momento
della richiesta vengono completate sulla porta precedente.
Per ogni richiesta deve essere creato un nuovo processo oppure un nuovo thread a seconda
della modalità utilizzata.
La scelta della modalità (multi-thread o multi-processo) deve poter essere effettuata all'atto della
partenza (tramite opzione da linea comandi e/o lettura di un file di configurazione).
Il processo o thread principale attende con una select() il completamento di una richiesta di connessione.
I file devono essere acceduti in modalità esclusiva anche se in sola lettura; è richiesto quindi un meccanismo di lock per l'accesso ai file.
Per determinare il tipo di file (necessario per definire il primo carattere dell'output inviato dal server)
è possibile utilizzare il comando file da Linux (invocato utilizzando la funzione popen()).
Sotto Windows è possibile utilizzare l'estensione del file associandola al tipo di file secondo le convenzioni standard
(exe corrisponde a programma binario eseguibile; jpg, png, ... corrispondono a immagini, etc.)
Prima dell'invio il file deve essere mappato in memoria.
L'effettivo invio del file avviene sempre da parte di un thread creato appositamente (anche nel caso di utilizzo della modalità multiprocesso)
che accede al file mappato in memoria.
Ogni operazione di invio di un file deve essere registrata su un file di log da un processo separato che riceve l'informazione sull'operazione svolta da una pipe (di tipo semplice).
Il processo va in lettura sulla pipe quando viene risvegliato da un evento (caso Windows) o con un meccanismo di condition variable (caso Linux).
Le informazioni da registrare sono:
nome file
dimensione file
indirizzo IP e numero di porta del client che ha effettuato la richiesta
La lista dei file da rendere disponibile deve essere costruita utilizzando le primitive specifiche della piattaforma per la lettura delle directory, senza fare uso dei comandi di sistema.
Il server deve funzionare in modalità daemon sotto Unix/Linux.
Il server deve essere interrogabile tramite il comando curl. Non è richiesto lo sviluppo di un client.

Al progetto andrà allegata una breve (4-8 pagine) relazione che descriva (e motivi) le scelte progettuali.

Il progetto può essere svolto da una o due persone.
