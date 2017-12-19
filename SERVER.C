#include "libsincro.h"

int pid;
int fd;
int operation;
int time;
int status;
int ok = 1;

char *dir1;
char *dir2;

char message[MAXBUFFER];


typedef struct
{
  char *op;
  char *pid;
  char *dir1;
  char *dir2;
  char *time;
}
packet;

packet msg;


extern int errno;
struct sigaction action;

static void
waitsig (const int sig)
{

  if ((sig == SIGQUIT) || (sig == SIGTERM))

    if (existEntry ("SINCRO.pipe"))
      {
	/* se la pipe esiste ancora la cancella */
	unlink ("SINCRO.pipe");
	/* e manda una SIGKILL a tutti i processi dello stesso gruppo */
	kill (0, SIGKILL);
      }

  /*
   *  eseguo una wait solo quando arriva un SIGCHLD e quindi sono sicuro che
   *  un processo � terminato.
   */

  if (sig == SIGCHLD)
    wait (&status);
}
/*****************************************************************************************/
static void
listen ()
{
 /*
  * imposta i gestori dei segnali SIGQUIT, SIGTERM e SIGCHLD 
  */
	if (sigemptyset (&action.sa_mask) != 0) TRACE("ERRORE: sigemptyset ha fallito...");
	
  action.sa_flags = 0;

  SIGERROR ((sigaction (SIGQUIT, &action, NULL)), "SIGQUIT");
  SIGERROR ((sigaction (SIGTERM, &action, NULL)), "SIGTERM");
  SIGERROR ((sigaction (SIGCHLD, &action, NULL)), "SIGCHLD");
}
/*****************************************************************************************/
char *
strdiv (const char *in)
{
 
  /* dichiara un puntatore a char */
  char *s;
  s = malloc (strlen ((in) + 1));

  /* controlla se l'allocazione dinamica della memoria ha fallito */
  if (s == NULL)
    TRACE (("\nErrore memoria insufficiente..."));

  /* copia "in" in "s" */
  strcpy (s, in);

  /* divide i vari componenti del message ricevuto separati dal token ":" */
  msg.op = strsep (&s, ":");
  msg.pid = strsep (&s, ":");
  msg.dir1 = strsep (&s, ":");
  msg.dir2 = strsep (&s, ":");
  msg.time = strsep (&s, ":");

  /* dealloca la memoria allocata con malloc... free non ritorna nulla... */
  free (s);

  return msg.op;

}


/**********************************************************************************************/



void
setsig (const int sig, const int clpid)
{
  /* manda un message al processi con pid "sig" */
  int temp = kill (clpid, sig);
  (void) temp;
  return;
}


/**********************************************************************************************/




/**********************************************************************************************/


int
main (int argc, char **argv)
{
  int pid, status, esito, tmp;

  char *opzione;

  (void) argc;
  (void) argv;
  (void) status;

  esito = 0;

  unlink (tablename ());
  add ("INIZIALIZZAZIONE", "DATABASE");
  delate ("INIZIALIZZAZIONE", "DATABASE");

  action.sa_handler = waitsig;

  listen ();

  WHENFORK (pid = fork ());

  if (pid == 0)
    {

      WHENFORK (pid = fork ());

      if (pid == 0)
	{


	  if (!existEntry ("SINCRO.pipe"))
	    {

	      if (!ispipe ("SINCRO.pipe"))
		{

		  if ((mkfifo ("SINCRO.pipe", 0644)) < 0)
		    {
		      if (errno == EROFS)
			{
			  printf ("\n Errore: file-system read-only...\n");
			  exit (-1);
			}

		      else if (errno == ENOSPC)
			{
			  printf
			    ("\n ERRORE: tabella descrittori file piena \n");
			  exit (-1);
			}
		      else
			{
			  printf
			    ("\n ERRORE: la chimata mkfifo � fallita...");
			  exit (-1);
			}
		    }
		}
	    }


	  fd = open (("SINCRO.pipe"), O_RDONLY);	/*si mette in attesa dell'arrivo di messaggi */

	  for (;;)
	    {
	      while (read (fd, message, MAXBUFFER) > 0)
		{
		  WHENFORK (pid = fork ());

		  if (pid == 0)
		    /*siamo nel pronipote il quale girando in background esegue il codice del gestore */
		    {
		      opzione = strdiv (message);
		      tmp = (int) atoi (opzione);
		      switch (tmp) /* che operazione � stata richiesta? */
			{
			case 1: /* sincronizzazione semplice */
			  {
			    if ((esito = sincro (msg.dir1, msg.dir2)))
			      esito = (sincro (msg.dir2, msg.dir1));
			    if (esito)
			      {
                                /* se la sincronizzazione non � fallita 
                                 * manda una SIGUSR1 al pid del richiedente
                                 */
				setsig (10, atoi (msg.pid));
			      }
			    else
			      {
                                /* altrimenti.... manda SIGUSR2 */
				setsig (12, atoi (msg.pid));
			      }
			    exit (0);
			  }
			  break;
			case 2: /* sincronizzazione regolare */
			  {
                            /* se nel database non c'� gi� una voce relativa
                             * alla sincronizzazione regolare richiesta
                             * viene aggiunta
                             */
			    if (!find (msg.dir1, msg.dir2))
			      add (msg.dir1, msg.dir2);
                            /* sono a quando nel database c'� la voce relativa alla
                             * sincronizzazione....
                             */
			    while (find (msg.dir1, msg.dir2))
			      {
				if ((esito = sincro (msg.dir1, msg.dir2)))
				  esito = sincro (msg.dir2, msg.dir1);
                                /* se la sincronizzazione � avvenuta con successo
                                 * dormi per msg.time secondi...
                                 * altrimenti rimuovi la voce dal database
                                 */
				if (esito)
				  sleep (atoi (msg.time));
				else
				  delate (msg.dir1, msg.dir2);
			      }
			    exit (0);
			  }
			  break;
			case 3: /* terminazione di sincronizzazione */
			  {              
                            /* se trovi nel database la voce richiesta... eliminala! */
                 	    if (find (msg.dir1, msg.dir2))
			      {
				delate (msg.dir1, msg.dir2);
			      }
			    exit (0);
			  }
			  break;
			default:
			  break;
			}
		    }		/*  fine del gestore */
		}
	    }
	}
    }

  /* il padre muore subito ... */
  return 0; 
}
