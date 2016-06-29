/*
 * Nom: quatrePortsEntree
 * Description: programme a placer sur le ATMega16/324/644
 * pour eviter de nuire au ATMega8 lorsque celui-ci devient
 * un programmeur.  Il suffit de placer les ports [a-d] en entree
 * Version: 1
 */

#define F_CPU 8000000L

#include <avr/io.h> 
#include <util/delay.h>

int main()
{
  DDRA = 0x00; // PORT A est en mode entree
  DDRB = 0x00; // PORT B est en mode entree
  DDRC = 0x00; // PORT C est en mode entree
  DDRD = 0x00; // PORT D est en mode entree

  while(1) {
		;
  }

  return 0; 
}
