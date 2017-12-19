#include "libsincro.h"


/*****************************************************************************************
**           Controlla se un file (in path assoluto) è un named pipe                    **
*****************************************************************************************/
int
ispipe (const char *filename)
{

  struct stat info;

  return ((stat (filename, &info) != -1) && (S_ISFIFO (info.st_mode)));

}

/*****************************************************************************************/





/**************************************************************************
**  legge una stinga da tastiera, con lunghezza massima di 255 carateri  **
**************************************************************************/
char *
myscanf ()
{

  int i = 0;
  char key;
  char *temp = (char *) malloc (255 * sizeof (char));
 
  if (temp == NULL) 
        TRACE(("\n Errore: memoria insufficiente"));   

  while (((key = getchar ()) != 10) && (i < 255))
    temp[i++] = key;

  temp[i] = 0;

  return temp;
}

/*****************************************************************************************/



/******************************************************************************************
**          Legge un numero da tastiera (ritorna zero con input alfanumerico)            **
******************************************************************************************/
int
askInt ()
{

/* ritorna il cast a int dell'output di myscanf */

return (atoi(myscanf()));

}

/*****************************************************************************************/








/*****************************************************************************************/

   /******************************** SINCRODIR ***********************************/



/*
 * err è la variabile che controlla l'esito delle operazioni su file
 * e directory durante la sincronizzazione
 */ 

int err = 1;





/*****************************************************************************************/
void
fatalError ()
{
  err = 0;
}

/*****************************************************************************************/


/*****************************************************************************************/
int
isdirectory (const char *filename)
{
  struct stat info;

  int ret;

  WHENERROR(stat (filename, &info));

  if (S_ISDIR (info.st_mode))

    {
      ret = 1;
    }

  else

    {
      ret = 0;
    }

  return ret;
}

/*****************************************************************************************/


/*****************************************************************************************/
char *
concatena3 (const char *str1, const char *str2, const char *str3)
{

  char *conc =
    (char *) malloc (strlen (str1) + strlen (str2) + strlen (str3) + 1);

  if (conc == NULL) TRACE(("\n Errore memoria insufficiente"));
 
  strcpy (conc, str1);
  strcat (conc, str2);
  strcat (conc, str3);
 


  /* visto che srtcat e strcpy restituiscono puntatori che possono essere riutilizzati...

  strcat(strcat(strcpy(conc , str1), str2), str3);
  
  ma è errato restituire un indirizzo di una variabile locale */


return conc;


}

/*****************************************************************************************/

/*****************************************************************************************/
void
copyFile (char *file, char *file1)
{


  int fd, fd1, letti;


  char buffer[buffsize];


  /* apertura del primo file in lettura */
   WHENERROR (fd = open (file, O_RDONLY));

  /* creazione o sovrascrittura del secondo file */
  WHENERROR (fd1 = open (file1, O_WRONLY | O_CREAT | O_TRUNC, 0664));

  /* ciclo di copia */
  while ((letti = read (fd, buffer, buffsize)) > 0)
    WHENERROR (write (fd1, buffer, letti));

  /* controllo in uscita dalla copia */
  WHENERROR (letti);

  /* chiusura dei file */
  WHENERROR (close (fd));
  WHENERROR (close (fd1));

  return;
}

/*****************************************************************************************/


/*****************************************************************************************/
/*copia la directory richiamando se stessa ricorsivamente per ogni sottodirectory*/
void
copyDirectory (char *dir, char *dir1)
{

  if (mkdir (dir1, 0777) != 0) /* return -1 on error */

    {
      printf("cercando di creare la directory %s", dir1);

      fatalError ();
    }

  OPdir (dir, dir1);
}


/*****************************************************************************************/

/* "bassa manovalanza"... :)  */

/*
 * OPdir...che sta per "operazione su directory" esegue la vara
 * sincronizzazione, in modo ricorsivo 
 */

void
OPdir (char *dir, char *dir1)
{


  DIR *directory, *directory1;
  struct dirent *fileinfo;


  if ((directory = opendir (dir)) == NULL)

    {
      printf("cercando di aprire1 %s\n", dir);
      fatalError ();
    }

  if ((directory1 = opendir (dir1)) == NULL)

    {
      printf("cercando di aprire2 %s\n", dir1);
      fatalError ();
    }

  /* salta le entry . e .. */
  fileinfo = readdir (directory);
  fileinfo = readdir (directory);

  while ((fileinfo = readdir (directory)) != NULL)
    if (!(isdirectory (concatena3 (dir, "/", fileinfo->d_name))))

      {

	copyFile (concatena3 (dir, "/", fileinfo->d_name),
		  concatena3 (dir1, "/", fileinfo->d_name));
      }

    else

      {
	copyDirectory (concatena3 (dir, "/", fileinfo->d_name),
		       concatena3 (dir1, "/", fileinfo->d_name));
      }
}

/*****************************************************************************************/
/*controlla la variabile errno all'apertura del file in caso si sia verificato un errore*/
int
existEntry (char *name)
{
  struct stat info;
 
  int tmp = 1;

  if (stat (name, &info) == -1)

    {
      if (errno == ENOENT) /* cioè se la entry non esiste... */
	{
	  tmp = 0;
	}
    }
  else
    {
      tmp = 1;
    }

  return tmp;
}

/*****************************************************************************************/





/*****************************************************************************************/
/*confronta il campo stat.m_time dei due file*/
int
matchDate (char *file, char *file1)
{


  struct stat info, info1;

  time_t time_file, time_file1;

  int tmp = 0;

  WHENERROR (stat (file, &info));
  WHENERROR (stat (file1, &info1));

  time_file = info.st_mtime;
  time_file1 = info1.st_mtime;

  /* confronto fra le date di ultima modifica */
  if (time_file > time_file1)
    {
      tmp = 1;
    }

  else if (time_file < time_file1)
    {
      tmp = 2;
    }

  else if (time_file == time_file1)
    {
      tmp = 0;
    }

  return tmp;
}

/*****************************************************************************************/




/*****************************************************************************************/
/* funzione che viene chiamata nel caso che la entry sia un file */

void
syncronizeFile (char *file, char *file1)
{


  int i;

  /* confronta le date di file1 e file2 */
  i = matchDate (file, file1);

  switch (i)

    {
    case 0: /* le date sono uguali... non fare niente */
      break;
    case 1: /* la prima è più recente */
      copyFile (file, file1);
      break;
    case 2: /* la seconda è più recente */
      copyFile (file1, file);
    default: /* ERRORE !!! */
      fatalError ();
    }
}

/*****************************************************************************************/



/*****************************************************************************************/
int
examineDir (char *dir, char *dir1)
{
  int tmp, tmp1, ret;

  tmp = existEntry (dir);
  tmp1 = existEntry (dir1);

  if (tmp == 1)

    {
      if (tmp1 == 1)
	ret = BOTHEXIST;

      else
	ret = FIRSTEXIST;
    }

  else

    {
      if (tmp1 == 0)

	{
	  ret = NONEEXIST;
	}

      else

	{
	  ret = LASTEXIST;
	}
    }

  return ret;
}

/*****************************************************************************************/



/*****************************************************************************************/
int
searchEntry (char *filename, char *directory)
{

  DIR *tmpdir;
  struct dirent *tmpfile;
  int tmp = 0;

  if ((tmpdir = opendir (directory)) == NULL) /* Apro la directory... aperta? */
    fatalError ();


  while ((tmpfile = readdir (tmpdir)) != NULL)
    {
      if (filename == tmpfile->d_name)
	{
	  tmp = 1;
	}
    }

  return tmp;

}

/*****************************************************************************************/



/*****************************************************************************************/
int
syncronizeDir (char *dir, char *dir1)
{

  int tmp;

  /* esamina le directory... */
  tmp = examineDir (dir, dir1);

 
  switch (tmp)

    {
    case BOTHEXIST: /* esistono entrambe */

      {
	OPdir (dir, dir1); /* sincronizzale */
      }
      break;
    case FIRSTEXIST: /* esiste solo la prima */
      copyDirectory (dir, dir1); /* copia interamente la prima */
      break;
    case LASTEXIST: /* esiste solo la senconda */
      copyDirectory (dir1, dir); /* copia interamente la seconda */
      break;
    default: /* non esistono... ERORRE!!! */
      fatalError ();
    }

  return err; /* ritorna il codice di errore della sincronizzazione */
}

/*****************************************************************************************/


/*  SINCRO è la funzione da chimare nel main per effettuare le sincronizzazioni tra directory */

int
sincro (char *d1, char *d2)
{
  int esito;			/* esito della sincroniz... */

  /* la creazione di una directory richiede l'indicazione dei bit di autorizzazione
   * (vedi man creat) con valori ottali, per rendere più portabili le applicazioni
   * sono nate delle macro che teoricamente in futuro se venissero usate da tutti i
   * programmi consentirebbero anche modifiche allo schema dei permessi senza troppi
   * problemi per le applicazioni le macro sono definite in <sys/stat.h> */

  mode_t oldUmask;

  /* oldUmask = umask (0000); */

  oldUmask = umask (S_IRWXU);

  esito = syncronizeDir (d1, d2);

  oldUmask = umask (oldUmask);

  return esito;
}


/*********************** FINE FUNZIONI DI SINCRONIZZAZIONE ********************************/






/***********************     FUNZIONI DI GESTIONE FILE     ********************************/

/*****************************************************************************************/
char *
tablename (void)
{

  /* restituisce il nome della tabella, che e' sempre nella home directory
   * dell'utente e si chiama .syncrobase.db
   * La stringa ritornata viene allocata dinamicamente con malloc,
   * quindi va liberata con free() quando non e' piu' necessaria */

  char *home, *name;

  /* La funzione getenv cerca nell array envp[] una stringa che mecci
   * con la stringa puntata da name  */

  home = getenv ("HOME");

  name = malloc (strlen (home) + strlen ("/" FILENAME) + 1);

  if (name == NULL)
    {
      TRACE (("ERRORE CRITICO : Memoria insufficiente"));
    }

  strcpy (name, home);

  /* Alla home directory si aggiunge "/" ed il nome della tabella */
  strcat (name, "/" FILENAME);

  return name;
}

/*****************************************************************************************/


/*****************************************************************************************/
datum
makedatum (const char *dir1, const char *dir2)
{

  /* Partendo da due stringhe questa funzione crea una struttura
   * di tipo datum correttamente inizializzata */

  datum retval;

  retval.dsize = strlen (dir1) + strlen (dir2) + 2;

  /* Occorono due byte in piu' perche ogni stringa in C e' terminata
   * con codice ASCII 0 */

  retval.dptr = malloc (retval.dsize);

  if (retval.dptr == NULL)
    {
      TRACE (("ERRORE CRITICO : Memoria insufficiente"));
    }

  strcpy (retval.dptr, dir1);
  strcpy (retval.dptr + strlen (dir1) + 1, dir2);

  /* La seconda stringa e' accodata alla prima saltando il terminatore
   * che e' il solito codice ASCII 0 */

  return retval;
}

/*****************************************************************************************/


/*****************************************************************************************/
void
add (const char *dir1, const char *dir2)
{

  /* Questa funzione aggiunge un nuovo record al database se questo non esiste */

  GDBM_FILE table;
  char *file;
  datum key;
  int tmp;

  file = tablename ();

  table = gdbm_open (file, 4096, GDBM_WRCREAT, 0700, NULL);

  free (file);

  if (table == NULL)
    {
      TRACE (("Impossibile aprire in scrittura o creare il database"));
    }

  key = makedatum (dir1, dir2);

  tmp = gdbm_store (table, key, key, GDBM_INSERT);

  /* In questo caso il controllo del codice di ritorno della gdbm_store non serve perche'
   * nel caso in cui sia diverso da 0 allora nel database esiste gia' il riferimento alla
   * sincronizzazione regolare richiesta.
   * Altrimenti il campo e' stato inserito regolarmente. */

  free (key.dptr);

  gdbm_close (table);

}

/*****************************************************************************************/



/*****************************************************************************************/
int
find (const char *dir1, const char *dir2)
{

  /* Restituisce 1 se nel database esistono le due stringhe passate per argomento
   * 0 altrimenti */

  GDBM_FILE table;
  char *file;
  datum key, value;
  int i;

  file = tablename ();

  table = gdbm_open (file, 4096, GDBM_READER, 0, NULL);

  free (file);

  if (table == NULL)
    {
      TRACE (("ERRORE CRITICO: Impossibile aprire in lettura il database"));
    }

  key = makedatum (dir1, dir2);
  value = gdbm_fetch (table, key);

  gdbm_close (table);

  if (value.dptr == NULL)
    i = 0;
  else
    i = 1;

  free (key.dptr);

  return i;
}

/*****************************************************************************************/


/*****************************************************************************************/
void
delate (const char *dir1, const char *dir2)
{

  GDBM_FILE table;
  char *file;
  datum key;
  int tmp;

  file = tablename ();

  table = gdbm_open (file, 4096, GDBM_WRITER, 0, NULL);

  free (file);

  if (table == NULL)
    {
      TRACE (("ERRORE CRITICO: Impossibile aprire in lettura il database"));
    }

  key = makedatum (dir1, dir2);

  tmp = gdbm_delete (table, key);

  gdbm_close (table);

  return;

}

/*****************************************************************************************/
