/*
    Progmem: fichier Flex pour l'analyse lexical des instructions
             du compilateur qui genere le bytecode pour le cours inf1995

    Jerome Collin
    Juin 2005
*/

%{

#include "progmem.tab.h"

int ligne = 1;
extern void yyerror ( char const *s);

%}

DIGIT    [0-9]
INTEGER  {DIGIT}+

%%

[ \t]+          /* ignorer les espaces */
[\n]       { ligne++; }
[\r]            /* ignorer les \r, tenir compte uniquement du /n */

                /* passer les lignes de commentaire */
\/\/[^\n]*
#[^\n]*
"%"[^\n]*

                /* reconnaitre les mots reserves */

DBT|dbt { return DBT; }
FIN|fin { return FIN; }
ATT|att { return ATT; }
DAL|dal { return DAL; }
DET|det { return DET; }
SGO|sgo { return SGO; }
SAR|sar { return SAR; }
MAR|mar { return MAR; }
MAV|mav { return MAV; }
MRE|mre { return MRE; }
TRD|trd { return TRD; }
TRG|trg { return TRG; }
DBC|dbc { return DBC; }
FBC|fbc { return FBC; }

                /* la donnee est toujours un entier */
{INTEGER}  {
             yylval.typeString = yytext;
             return DONNEE;
           }

;  { return POINTVIRGULE; }

               /* tout autre caratere est un probleme */
[a-zA-Z0-9]+  { return MAUVAISJETON; }

.  { yyerror ( "character non reconnu"); }

%%

// Un seul fichier a analyser - aucun support pour un genre de fichier
// "include" - ne rien faire
int yywrap () {
   return 1;
}

// Pour tout ce qui doit etre fait avant que l'analyse lexical ne debute
// Ouvrire le fichier source pour y lire les instructions
int prologLexical ( const char *fichier ) {
   yyin = fopen( fichier, "r" );
   if ( yyin == NULL ) {
       fprintf (stderr, "Erreur: incapable d'ouvrir le fichier source %s\n",
                                                              fichier );
       return 0; // code d'erreur
   }

   return 1; // succes
}


