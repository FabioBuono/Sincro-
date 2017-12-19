
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <gdbm.h>
#include <errno.h>


#define buffsize 80


/*
 * define di debug: tutte le define usano le macro standard per il debug __LINE__ e __FILE__
 * che indicano rispettivamente la riga e il file conivolti nell'errore 
 */


/* WHENERROR serve a stampare errori che non richiedono la chiusura dell'applicazione */
#define WHENERROR(c) do{ if((c)<0){ printf("Errore in [ %s : %d]\n",__FILE__,__LINE__); }} while(0)

/* SIGERROR serve a stampare errori critici relativi alla gestione dei segnali */
#define SIGERROR(c, a) do{ if((c)<0){ printf("Errore in [%s : %d] gestendo ",__FILE__,__LINE__); printf(a);\
kill(0,SIGKILL);}}while(0)

#define WHENFORK(c) do{ if((c)<0){ printf("Errore in [%s : %d] tentando una fork ",__FILE__,__LINE__);\
kill(0,SIGKILL);}}while(0)

/* TRACE si comporta come SIGERROR, ma relativi ad errori critici generici */
#define TRACE(a) do{printf ("TRACE : %s - %u\n", __FILE__, __LINE__); printf (a); fflush (stdout);\
kill(0, SIGKILL);}while(0)


/************** define per le funzioni di sincronizzazione *************/


#define MAXBUFFER  4096         /* diensione del bufer di I/O */


/*
 * macro per la funzione existEntry
 */

#define BOTHEXIST 0
#define FIRSTEXIST 1
#define LASTEXIST 2
#define NONEEXIST 3



/**************   define per le funzioni di gestione file   *************/


/* FILENAME indica il nome del file usato come base fisica del database */

#define FILENAME ".syncrobase.db"




/*****************************************************************************************
**           Controlla se un file (in path assoluto) è un named pipe                    **
*****************************************************************************************/
int
ispipe (const char *);
/*****************************************************************************************/





/*****************************************************************************************
*                          legge una stinga da tastiera                                 **
******************************************************************************************/

char *
myscanf (void);
/*****************************************************************************************/



/******************************************************************************************
**          Legge un numero da tastiera (ritorna zero con input alfanumerico)            **
******************************************************************************************/
int
askInt (void);
/*****************************************************************************************/


/*****************************************************************************************/
void
fatalError (void);
/*****************************************************************************************/


/*****************************************************************************************/
int
isdirectory (const char *);
/*****************************************************************************************/


/*****************************************************************************************/
char *
concatena3 (const char *, const char *, const char *);
/*****************************************************************************************/

/*****************************************************************************************/
void
copyFile (char *, char *);
/*****************************************************************************************/


/*****************************************************************************************/
/*copia la directory richiamando se stessa ricorsivamente per ogni sottodirectory*/
void
copyDirectory (char *, char *);

/*****************************************************************************************/


/*copia la directory richiamando se stessa ricorsivamente per ogni sottodirectory*/
void
OPdir (char *, char *);
/*****************************************************************************************/
/*controlla la variabile errno all'apertura del file in caso si sia verificato un errore*/
int
existEntry (char *);
/*****************************************************************************************/





/*****************************************************************************************/
/*confronta il campo stat.m_time dei due file*/
int
matchDate (char *, char *);
/*****************************************************************************************/




/*****************************************************************************************/
/* funzione che viene chiamata nel caso che la entry sia un file */
void
syncronizeFile (char *, char *);
/*****************************************************************************************/



/*****************************************************************************************/
int
examineDir (char *, char *);
/*****************************************************************************************/



/*****************************************************************************************/
int
searchEntry (char *, char *);
/*****************************************************************************************/



/*****************************************************************************************/
int
syncronizeDir (char *, char *);

/*****************************************************************************************/
int sincro (char *, char *);
/*********************** FINE FUNZIONI DI SINCRONIZZAZIONE ********************************/






/***********************     FUNZIONI DI GESTIONE FILE     ********************************/

/*****************************************************************************************/
char * tablename(void);
/*****************************************************************************************/


/*****************************************************************************************/
datum makedatum (const char * , const char * );
/*****************************************************************************************/


/*****************************************************************************************/
void add(const char *,const char *);
/*****************************************************************************************/



/*****************************************************************************************/
int find(const char *, const char *);
/*****************************************************************************************/


/*****************************************************************************************/
void delate(const char *, const char *);
/*****************************************************************************************/




/**************************************************************************************************/


