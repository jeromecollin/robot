
/*
    serieViaUSB: programme qui peut etre considere comme un terminal
                 extremement primitif et qui vise l'echange de quelques
                 octets avec une carte a microcontroleur par port RS232
                 Unique comminication possible : 2400 baud, 8 bits,
                 aucun bit de parite et un seul d'arret.

    Matthew Khouzam
    Jerome Collin
    Michaël Ferris
    Mathieu Marengère-Gosselin
    Fevrier 2007
    Modifications, mars 2011

*/

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits>
#include <time.h>
#include <usb.h>        /* acces a libusb, voir http://libusb.sourceforge.net/ */
#include <usbcmd.h>
#include <ctype.h>

enum ModesAffichage{BYTE, HEX, DEC, BIN};

// vrai s'il y a communication de la carte vers le PC.
int lecture = false;

// vrai s'il y a communication du PC vers la carte.
int ecriture = false;

// mode d'affichage est à BYTE par défaut.
ModesAffichage modeAffichage = BYTE;

// Nombre de caractères avant de faire un saut
int nbSauts = 0;

// transmettre uniquement nBytes
int nBytes = INT_MAX;

FILE *fpFichier = NULL;
char fichier[1024] = "";
int utiliseFichier = false;

// pour la conversion char-bin. 
// Tiré de http://cboard.cprogramming.com/c-programming/42817-convert-char-binary.html
char* charToBin ( unsigned char c )
{
    static char bin[CHAR_BIT + 1] = {0};
    int i;

    for ( i = CHAR_BIT - 1; i >= 0; i-- )
    {
        bin[i] = (c % 2) + '0';
        c /= 2;
    }

    return bin;
}

// pour l'interface USB vers la carte
usb_dev_handle *gestionUSB;

void afficherAide ( void ) {
   fprintf (stderr, "\n" );
   fprintf (stderr, "usage: serieViaUSB [-l | -e] [-nb <n>] [-f <fichier>] [-h | -d | -b] [-s <n>]\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "description : programme permettant de recevoir et\n" );
   fprintf (stderr, "              d'envoyer des octets de facon serielle mais\n" );
   fprintf (stderr, "              indirectement via le cable USB\n" );
   fprintf (stderr, "              pour echange avec la carte microcontroleur\n" );
   fprintf (stderr, "              du cours inf1995.  Aucun protocole\n" );
   fprintf (stderr, "              special ne controle la signification des\n");
   fprintf (stderr, "              octets.\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-e --ecriture: pour envoie des donnees vers la\n" );
   fprintf (stderr, "               la carte.  Cette option demande\n" );
   fprintf (stderr, "               l'utilisation de l'option -f.\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-l --lecture: pour reception des donnees en provenance\n" );
   fprintf (stderr, "              de la carte\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-nb --nBytes <n>: terminer le programme directement apres\n" );
   fprintf (stderr, "              le transfert de n octets.  Sans cette option,\n" );
   fprintf (stderr, "              lit ou ecrit indefiniment.\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-f --fichier <fichier>:\n");
   fprintf (stderr, "              Prendre les donnees a envoyer vers la carte\n" );
   fprintf (stderr, "              dans le fichier specifie (implique l'option\n" );
   fprintf (stderr, "              -e) ou ecrire les donnees dans le fichier\n" );
   fprintf (stderr, "              lorsqu'elles proviennent de la carte\n" );
   fprintf (stderr, "              (implique l'option -l). stdout est utilise\n" );
   fprintf (stderr, "              avec l'option -e si l'option -f n'est pas\n" );
   fprintf (stderr, "              utilisee (cas par defaut).\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-h --hexadecimal: afficher les octets envoyés ou reçus\n" );
   fprintf (stderr, "                  dans une représentation hexadécimale.\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-d --decimal: afficher les octets envoyés ou reçus\n" );
   fprintf (stderr, "                  dans une représentation décimale.\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-b --binaire: afficher les octets envoyés ou reçus\n" );
   fprintf (stderr, "                  dans une représentation binaire.\n" );
   fprintf (stderr, "\n" );
   fprintf (stderr, "-s --saut <n>: effectue un retour à la ligne à chaque n\n" );
   fprintf (stderr, "                  caractère.\n" );
   fprintf (stderr, "\n" );
   fflush(0);
   exit (-1);
}

/* Fonctions pour gerer le USB */

/* fonction assez standard pour l'ouverture du port USB */
int usbOuvrir()
{
  struct usb_bus    *bus;
  struct usb_device *dev = 0;

  /* facon de faire standard pour trouver le device USB et l'ouvrir... */
  usb_init();
  usb_find_busses();
  usb_find_devices();
  for(bus=usb_busses; bus; bus=bus->next){
    for(dev=bus->devices; dev; dev=dev->next){
      if(dev->descriptor.idVendor == USBDEV_VENDOR &&
         dev->descriptor.idProduct == USBDEV_PRODUCT)
        break;
    }
    if(dev)
      break;
  }
  if( ! dev ) {
    fprintf (stderr, "Erreur: incapable de trouver le peripherique USB ");
    fprintf (stderr, "(vendor=0x%x product=0x%x)\n", USBDEV_VENDOR, USBDEV_PRODUCT);
    exit (-1);
  }

  // la documentation de libUSB donne des exemples de cette facon de faire
  gestionUSB = usb_open(dev);
  if ( ! gestionUSB ) {
    fprintf (stderr, "Erreur: incapable d'ouvrir le port USB vers le ");
    fprintf (stderr, "peripherique: %s\n", usb_strerror());
    usb_close (gestionUSB);
    exit (-1);
  }

  return 0;
}

int usbAjustementSerie (int baud, int bits, int parity)
{
  int nOctets;
  unsigned char cmd[4];
  char msg[4] = {0, 0, 0, 0};
  // le firmware attend ces parametres, et dans cet ordre
  cmd[0] = baud;
  cmd[1] = bits;
  cmd[2] = parity;
  cmd[3] = 0;

  // USBASP_FUNC_SETSERIOS ajuste les parametres USB (voir firmware)
  nOctets = usb_control_msg (gestionUSB,
             USB_TYPE_VENDOR | USB_RECIP_DEVICE | (1 << 7),
             USBASP_FUNC_SETSERIOS,
             (cmd[1] << 8) | cmd[0], (cmd[3] << 8) | cmd[2],
             msg, 4, 5000);

  if ( nOctets < 0 ) {
     fprintf(stderr, "Erreur: probleme de transmission USB: %s\n", usb_strerror());
     usb_close (gestionUSB);
     exit (-1);
  }

  // la configuration se confirme par un echo des parametres
  // il faut donc verifier que l'on a bien recu ce qu'on attend...
  if ( msg[0] == cmd[0] && msg[1] == cmd[1] &&
       msg[2] == cmd[2] && msg[3] == msg[3] ) {
     return 1;
  }

  return 0;
}

int analyseLigneDeCommande ( int argc, char *argv[] ) {

   // analyze de la ligne de commande
   int i = 1;
   while ( i < argc ) {
      // nombre d'octets: -nb | --nBytes <int>
      if ( strcmp (argv[i], "-nb") == 0 ||
           strcmp (argv[i], "--nBytes") == 0 ) {
         i++;
         if ( i < argc ) {
            nBytes = strtol ( argv[i], NULL, 10);
            if ( nBytes == LONG_MIN || nBytes == LONG_MAX ) {
               fprintf (stderr, "Erreur: nombre d'octets incorrect pour ");
               fprintf (stderr, "l'option -nb\n\n");
               afficherAide();
            }
         }
         else {
            fprintf (stderr, "Erreur: argument manquant pour -nb ou --nBytes\n\n");
            afficherAide();
         }
      }
      // fichier d'entree ou de sortie: -f | -fichier <string>
      else if ( strcmp (argv[i], "-f") == 0 ||
           strcmp (argv[i], "--fichier") == 0 ) {
         i++;
         utiliseFichier = true;
         if ( i < argc && strlen( argv[i] ) < 1023 ) {
            strcpy ( fichier, argv[i] );
         }
         else {
            fprintf (stderr, "Erreur: argument manquant pour -f ou --fichier\n\n");
            afficherAide();
         }
      }
      else if ( strcmp (argv[i], "-s") == 0 ||
           strcmp (argv[i], "--saut") == 0 ) {
         i++;
         if ( i < argc ) {
            nbSauts = strtol ( argv[i], NULL, 10);
         }
         else {
            fprintf (stderr, "Erreur: argument manquant pour -s ou --saut\n\n");
            afficherAide();
         }
      }
      // mode lecture: -l | --lecture
      else if ( strcmp (argv[i], "-l") == 0 ||
                strcmp (argv[i], "--lecture") == 0 ) {
         lecture = true;
      }
      else if ( strcmp (argv[i], "-e") == 0 ||
                strcmp (argv[i], "--ecriture") == 0 ) {
         ecriture = true;
      }
      else if ( strcmp (argv[i], "-h") == 0 ||
                strcmp (argv[i], "--hexadecimal") == 0 ) {
         modeAffichage = HEX;
      }
      else if ( strcmp (argv[i], "-d") == 0 ||
                strcmp (argv[i], "--decimal") == 0 ) {
         modeAffichage = DEC;
      }
      else if ( strcmp (argv[i], "-b") == 0 ||
                strcmp (argv[i], "--binaire") == 0 ) {
         modeAffichage = BIN;
      }
      else {
         afficherAide();
      }
      i++;
   }

   // determiner la lecture et/ou l'ecriture et ajuster
   // la lecture ou l'ecriture dans le fichier en consequence au besoin
   char modeFichier[4] = "";
   if ( lecture == ecriture ) {
      fprintf (stderr, "Erreur: exactement une option, -l ou -e ");
      fprintf (stderr, "doit etre specifiee\n");
      afficherAide();
   }
   else if ( lecture == true ) {
      strcpy (modeFichier, "w");
      fpFichier = stdout;
   }
   else if ( ecriture == true ) {
      strcpy (modeFichier, "r");
      fpFichier = stdin;
   }

   // Ouverture du fichier, si necessaire
   if ( utiliseFichier == true ) {
      fpFichier = fopen (fichier, modeFichier);
      if ( fpFichier == NULL ) {
         fprintf (stderr, "Erreur: probleme en essayant d'ouvrir le fichier" );
         fprintf (stderr, " %s\n", fichier);
         afficherAide();
      }
   }

   // le cas de l'ecriture est un peu special
   if ( ecriture == true ) {
      // l'ecriture doit implique un fichier en entree
      if ( utiliseFichier == false ) {
         fprintf (stderr, "Erreur: l'utilisation de l'option -e implique\n");
         fprintf (stderr, "        l'utilisation de l'option -f\n\n");
         afficherAide();
      }
      else {
         // on doit avoir le nombre precis d'octets a transmettre
         // lorsqu'on ecrit vers la carte, ca simplifie la communication
         struct stat buf;
         if ( stat(fichier, &buf) != 0 ) {
            fprintf (stderr, "Erreur: incapable d'optenir des informations ");
            fprintf (stderr, "sur le fichier %s\n", fichier);
            exit (-1);
         }
         if ( buf.st_size < nBytes ) {
            nBytes = buf.st_size;
         }
      } 
   }

   return 1; // succes
}

int main ( int argc, char *argv[] ) {

   // commencer par analyser les options sur la ligne de commande...
   if ( ! analyseLigneDeCommande ( argc, argv ) )
      return -1;

   /* OK, ouvrir tout ce qui est USB... */
   usbOuvrir();
   if ( usbAjustementSerie(USBASP_MODE_SETBAUD2400,
                           USBASP_MODE_UART8BIT, USBASP_MODE_PARITYN ) ) {
      fprintf (stderr, "OK: le peripherique USB est reconnu et la\n");
      fprintf (stderr, "    communication serie doit se faire a 2400 baud,\n");
      fprintf (stderr, "    8 bits et sans parite.\n" );
      fprintf (stderr, "--------------------------------------------\n");
   }
   else {
      fprintf(stderr, "Erreur: incapable d'ajuster la communication serie\n");
      exit (-1);
   }
   fflush(0);

   // lecture et/ou ecriture sans fin
   int i = 0;
   int j = 0;
   int k = 0;
   int rtn;
   int grandeurTampon = 8;
   unsigned char tampon[grandeurTampon];
   struct timespec tempSpec;
   tempSpec.tv_sec = 0;
   tempSpec.tv_nsec = 80000000; // 50 ms

   while ( i < nBytes ) {
     // mettre 0xFF dans tout le tampon
     grandeurTampon = 8;
     memset (tampon, 0xFF, grandeurTampon);

     // lecture - le PC recoit les donnees de la carte
     if ( lecture ) {
        rtn = usb_control_msg (gestionUSB,
                   USB_TYPE_VENDOR | USB_RECIP_DEVICE | (1 << 7),
                   USBASP_FUNC_READSER,
                   0, 0, (char *)tampon,
                   grandeurTampon, 5000);
        if ( rtn < 0 ){
          fprintf (stderr, "Erreur: problem de tansmission USB:" );
          fprintf (stderr, " %s\n", usb_strerror());
          usb_close (gestionUSB);
          exit(-1);
        }

        /* le premier octet donne le nombre d'octets vraiment transferes 
           (au maximum 7) */
        if ( tampon[0] < grandeurTampon ) {
           grandeurTampon = tampon[0] + 1;
        }

        j = 1;
        while ( j < grandeurTampon && i < nBytes ) {
          switch( modeAffichage )
          {
              case HEX:
                  fprintf(fpFichier, "%#04x ", tampon[j]);
                  break;

              case DEC:
                  fprintf(fpFichier, "%#04d ", tampon[j]);
                  break;

              case BIN:
                  fprintf(fpFichier, "%s ", charToBin(tampon[j]));
                  break;

              case BYTE:
              default:
                  fputc (tampon[j], fpFichier);
          }
        
          if( nbSauts ) {
              if( ( (i+1) % nbSauts) == 0 ) { fputc ('\n', fpFichier); }
          }

          i++;
          j++;
        }
      }
      // le PC envoie les donnees vers la carte
      if ( ecriture ) {
        // remplir le tampon - fpFichier ne peut etre stdin
        j = 1;
        while ( j < grandeurTampon && i < nBytes ) {
          tampon[j] = getc (fpFichier);
          i++;
          j++;
        }
        tampon[0] = j - 1;

        // on envoie vers l'USB  0 == envoie
        rtn = usb_control_msg (gestionUSB,
                   USB_TYPE_VENDOR | USB_RECIP_DEVICE | (0 << 7),
                   USBASP_FUNC_WRITESER,
                   0, 0, (char *)tampon, j, 5000);

        // echo a l'ecran, pas strictement necessaire mais interessant
        for ( k = 0; k < j; k++ )
        {
          switch( modeAffichage )
          {
              case HEX:
                  fprintf(stderr, "%#04x ", tampon[k]);
                  break;

              case DEC:
                  fprintf(stderr, "%#04d ", tampon[k]);
                  break;

              case BIN:
                  fprintf(stderr, "%s ", charToBin(tampon[k]));
                  break;

              case BYTE:
              default:
                  fputc (tampon[k], stderr);
           }

           if( nbSauts ) {
               if( ( (k+1) % nbSauts) == 0 ) { fputc ('\n', stderr); }
           }

        }

        // il faut que la carte ecrive les octets en memoire
        // et via l'interface serie qui est lente.  Donner une chance
        // a la carte de realiser ses operations en lui laissant du temps.
        // ce petit delai devrait etre suffisant pour qu'elle 
        // "n'echappe pas des octets" en cours de route...
        nanosleep (&tempSpec, NULL);

        if ( rtn < 0 ) {
          fprintf (stderr, "Erreur: problem de tansmission USB:" );
          fprintf (stderr, " %s\n", usb_strerror());
          usb_close (gestionUSB);
          exit(-1);
        }
      } /* while */

      fflush(0);
   }

   // rapport indiquant que la transmission est terminee
   fprintf (stderr, "\n--------------------------------------------\n");
   fprintf (stderr, "serieViaUSB : %d octets ont ete transmis ", nBytes );
   fprintf (stderr, "ou recus\n" );
   fflush(0);

   usb_close (gestionUSB);
   return 0;
}



