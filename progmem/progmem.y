/*
    Progmem: fichier Bison pour l'analyse syntaxique des instructions
             du compilateur qui genere le bytecode pour le cours inf1995

    Jerome Collin
    Juin 2005
*/

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// deux procedures standards avec lex/yacc
int yylex (void);
void yyerror (char const *);

extern int ligne;  // pour identifier la ligne qui cause l'erreur

// pour initialiser avant l'analyse lexicale
extern int prologLexical ( const char *fichier );

FILE *fichierSortie;  // fichier binaire produit
int erreurs = 0; // pour rapporter le nombre d'erreur de compilation
int verbose = 0; // pour donner plus d'information sur la compilation
int nOctets = 2; // nombre d'octets du fichier binaire

%}

// jeton du langage
// voir www.cours.polymtl.ca/inf1995/tp/tp9 pour le description du langage
%token DBT
%token FIN
%token ATT
%token DAL
%token DET
%token SGO
%token SAR
%token MAR
%token MAV
%token MRE
%token TRD
%token TRG
%token DBC
%token FBC
%token DONNEE
%token POINTVIRGULE
%token MAUVAISJETON

// retour soit d'entier ou de chaines de caracteres pour les regles yacc
%union {
   int typeInt;
   char *typeString;
}

// types possibles pour les regles
%type <typeString> DONNEE
%type <typeInt> mnemonique1 mnemonique2 valeur

%expect 0
%verbose

%start instructions

%%

// regles YACC

instructions :
           /* aucune instruction */
          | instructions instruction POINTVIRGULE
          | instructions error       POINTVIRGULE
          ;

instruction :
             mnemonique1
              {
                if ( verbose > 0 ) {
                   fprintf ( stdout, " - %#.2X - OX00 - ligne %d\n", $1, ligne);
                }
                // inscription dans le fichier binaire
                fprintf ( fichierSortie, "%c%c", (unsigned char)$1, 0);
                nOctets += 2;
              }
           | mnemonique2 valeur
              {
                if ( verbose > 0 ) {
                   fprintf ( stdout, " - %#.2X - %#.2X - ligne %d\n", $1, $2, ligne);
                }
                // inscription dans le fichier binaire
                fprintf ( fichierSortie, "%c%c", (unsigned char)$1,
                                                 (unsigned char)$2 );
                nOctets += 2;
              }
          ;

// sans operande significatif
mnemonique1 : 
            DBT { $$ = 0x01; }
          | FIN { $$ = 0xFF; }
          | SAR { $$ = 0x09; }
          | MAR { $$ = 0x61; }
          | TRD { $$ = 0x64; }
          | TRG { $$ = 0x65; }
          | DBC { $$ = 0xC0; }
          | FBC { $$ = 0xC1; }
          ;

// avec operande significatif
mnemonique2 :
            ATT { $$ = 0x02; }
          | DAL { $$ = 0x44; }
          | DET { $$ = 0x45; }
          | SGO { $$ = 0x48; }
          | MAV { $$ = 0x62; }
          | MRE { $$ = 0x63; }
          | DBC { $$ = 0xC0; }
          ;

valeur : DONNEE {
                   char *inutil;
                   long int entier = strtol ( $1, &inutil, 10 );
                   if ( entier < 0 || entier > 255 ) {
                      yyerror ("donnee invalide");
                   }
                   $$ = (int)entier;
                }
          ;

%%

void
yyerror (char const *s) {
   erreurs++;
   fprintf ( stderr, "Erreur: ligne %d, %s\n", ligne, s );
}

void afficherAide() {
   fprintf (stderr, "\nprogmem : -v -o <fichier> <fichier>\n\n");
   fprintf (stderr, "  -v --verbose : affichage des codes a l'ecran\n");
   fprintf (stderr, "  -o --output <fichier> : fichier de sortie binaire\n");
   fprintf (stderr, "  <fichier> : fichier a compiler\n\n");
   exit (EXIT_FAILURE);
}

int main ( int argc, char *argv[] ) {
   int fichierEntreeOK = 0;
   int fichierSortieOK = 0;
   int nomFichierSortie = 0;


   // analyze de la ligne de commande
   int i = 1;
   while ( i < argc ) {
      if ( strcmp (argv[i], "-o") == 0 ||
           strcmp (argv[i], "--output") == 0 ) {
         i++;
         if ( i < argc && strlen( argv[i] ) < 99 ) {
            fichierSortie = fopen (argv[i], "w");
            if ( fichierSortie == NULL ) {
               fprintf (stderr, "Erreur: incapable d'ouvrir le fichier" );
               fprintf (stderr, " binaire de sortie\n");
               afficherAide();
            }
            fichierSortieOK = 1;
            nomFichierSortie = i;

            // 16 bits de longueur de fichier binaire.
            // A ajuster en fin de compilation. Voir plus bas.
            fprintf ( fichierSortie, "%c%c", 0, 0 );
         }
         else {
            fprintf (stderr, "Erreur: arguments incorrects\n");
            afficherAide();
         }
      }
      else if ( strcmp (argv[i], "-v") == 0 ||
         strcmp (argv[i], "--verbose") == 0 ) {
         verbose = 1;
      }
      else {
         if ( i == argc - 1 ) {
            if ( prologLexical(argv[i]) == 0 ) {
               afficherAide();
            }
            fichierEntreeOK = 1;
         }
         else {
            afficherAide();
         }
      }
      i++;
   }

   if ( fichierEntreeOK == 0 || fichierSortieOK == 0 ) {
      fprintf (stderr,
             "\nErreur: fichier de sortie et/ou d'entree non specifie(s)\n");
      exit (EXIT_FAILURE);
   }

   // Afficher les resultats de traduction a l'usager, si desire
   if ( verbose > 0 ) {
      fprintf (stderr, "\n   CMD   DONNEE  LIGNE\n");
      fprintf (stderr, "   ---   ------  -----\n");
   }

   // Faire l'analyse lexical et syntaxique du fichier a compiler
   if ( yyparse() || erreurs > 0 ) {
      // retourner un code d'erreur (1)
      fprintf (stderr, "\n*** compilation terminee sans succes ***\n\n");
      remove ( argv[nomFichierSortie] );  // ne rien produire - minimiser les problemes
      exit (EXIT_FAILURE);
   }

   // ajuster la longueur du fichier binaire en debut de fichier binaire
   if ( fseek (fichierSortie, 0, SEEK_SET) == 0 ) {
      unsigned char poidsFaible = (unsigned char) nOctets;
      unsigned char poidsFort   = (unsigned char) (nOctets >> 8);
      fprintf ( fichierSortie, "%c%c", poidsFort, poidsFaible );
   }
   else {
      // retourner un code d'erreur (1)
      fprintf (stderr, "\n*** incapable de determiner la longueur du fichier binaire ***\n\n");
      remove ( argv[nomFichierSortie] );
      exit (EXIT_FAILURE);
   }

   // Donner le compte du nombre d'octets dans le fichier binaire
   if ( verbose > 0 ) {
      fprintf (stderr, "\n   Nombre d'octets dans le fichier binaire produit: %d\n\n", nOctets);
   }

   // retourner le code unix standard pour un succes (utile pour
   // usage dans un Makefile par exemple)
   exit (EXIT_SUCCESS);
}


