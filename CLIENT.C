#include "libsincro.h"


char message[2048];		/* message to send over the pipe */

struct sigaction action;


/****************************************************************************************
**             Compose the message and send it over the pipe                           **
*****************************************************************************************/
void
sendMessage (const int opzione, int pid, char *dir1, char *dir2, int time)
{
  int fd;			
  WHENERROR (sprintf (message, "%d:%d:%s:%s:%d", opzione, pid, dir1, dir2, time)) ;
  WHENERROR(fd = open ("SINCRO.pipe", O_WRONLY | O_NONBLOCK));  
  WHENERROR(write (fd, message, 2048));  
  WHENERROR(close (fd));
  return;
}
/*****************************************************************************************/
void
simpleSincro ()
{

  int pid;

  char *dir1;
  char *dir2;

  printf ("\nInserire il path della prima directory : ");
  dir1 = myscanf ();


  printf ("\nInserire il path della seconda directory : ");
  dir2 = myscanf ();


  printf ("\n spedizione in corso..... \n");

  sendMessage (1, pid = getpid (), dir1, dir2, 0);

}
/*****************************************************************************************/
void
regSincro ()
{
  int pid;
  int time;
  char *dir1;
  char *dir2;

  printf ("\nInserire il path della prima directory : ");
  dir1 = myscanf ();
  printf ("\nInserire il path della seconda directory : ");
  dir2 = myscanf ();
  printf ("\nInserire l'intervallo di sincronizzazione in secondi : ");
  time = askInt ();
  sendMessage (2, pid = getpid (), dir1, dir2, time);
}
/*****************************************************************************************/
void
termSincro ()
{
  int pid;
  char *dir1;
  char *dir2;
  printf ("\nInserire il path della prima directory : ");
  dir1 = myscanf ();
  printf ("\nInserire il path della seconda directory : ");
  dir2 = myscanf ();

  sendMessage (3, pid = getpid (), dir1, dir2, 0);
}
/*****************************************************************************************/
static void
waitsig (const int sig)
{
  if (sig == SIGUSR1)
    {
       printf ("\n SINCRO: sincronizzazione avvenuta. ");
    }
  else if (sig == SIGUSR2)
   {
    printf ("\n SINCRO: sincronizzazione fallita. ");
   }
  else if (sig == SIGPIPE)
  {
      TRACE(("  Broken pipe: il gestore potrebbe non essere pi� in esecuzione...  "));
  }
}
/*****************************************************************************************/
static void
listen ()
{
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;

  SIGERROR (sigaction (SIGUSR1, &action, NULL), "SIGUSR1");
  SIGERROR (sigaction (SIGUSR2, &action, NULL), "SIGUSR2");
  SIGERROR (sigaction (SIGPIPE, &action, NULL), "SIGPIPE");

}
/*****************************************************************************************/
int
main (int argc, char **argv)
{
  char opzione;
  int b = 0;
  (void) argc;
  (void) argv;

  action.sa_handler = waitsig;

  if (!ispipe ("SINCRO.pipe"))
      TRACE(("\n Errore: gestore non in esecuzione..."));

  listen ();

      printf("\n\nBenvenuti in Sincro.\n\n");
      printf("1. Sincronizzazione semplice.\n");
      printf("2. Sincronizzazione regolare.\n");
      printf("3. Terminazione Sincronizzazione regolare. \n");
      printf("4. Stampa menu \n");
      printf("5. Esci.\n\n");

   do 
     {
      printf ("> ");
      opzione = askInt ();
      switch (opzione)
	{
	case 1:
	  {
	    simpleSincro ();	/* sincronizzazione semplice */
	  }
	  break;
	case 2:
	  {
	    regSincro ();	/* sincronizzazione regolare  */
	  }
	  break;
	case 3:
	  {
	    termSincro ();	/* terminazione di sincronizzazione */
	  }
	  break;
	case 4:
	  {
	    printf("\n1. Sincronizzazione semplice \n");
            printf("2. Sincronizzazine regolare \n");
            printf("3. Terminazione di sincronizzazione \n");
            printf("4. Stampa men� \n");
            printf("5. Esci \n\n");
	  }
	  break;
       case 5: 
         {
          b = 1;
         }
        break;
       default: printf("\n Errore: digita un numero compreso tra 1 e 5 \n\n"); break;
	}
    }
  while (b != 1);
  return (0);
}
/*****************************************************************************************/
