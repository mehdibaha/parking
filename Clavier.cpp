/*************************************************************************
                           <Clavier>  -  description
                             -------------------
    début                : XXX
    copyright            : (C) XXX par XXX
    e-mail               : XXX
*************************************************************************/

//---------- Réalisation de la tâche <Clavier> (fichier Clavier.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <stdlib.h>
#include <ctime>

//------------------------------------------------------ Include personnel
#include "Clavier.h"
#include "Config.h"
#include "Menu.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types
typedef struct voiture Voiture;
typedef enum TypeUsager TypeUsager;

//---------------------------------------------------- Variables statiques
static int compteurVoiture = 0;

//------------------------------------------------------ Fonctions privées
static Voiture creerVoiture ( TypeUsager typeUsager, long typeEntree )
// Mode d'emploi :
// Construit et initialise une structure de type Voiture
{
	Voiture voiture;
	voiture.usager = typeUsager;
	voiture.heureArrive = time(NULL);
	voiture.type = typeEntree;
	compteurVoiture++;
	voiture.numVoiture = compteurVoiture % (NB_VOITURES_MAX + 1);
	return voiture;
} //----- fin de creerVoiture

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Clavier ( int balID )
{
    for (;;)
    {
        Menu ( );
    }
}

void Commande ( char code, unsigned int valeur )
// Algorithme :
//
{
	Voiture voiture;
    switch( code )
	{
		case 'E':
		case 'e':
			exit( 0 );
		case 'P':
		case 'p':
			voiture = creerVoiture(TypeUsager::PROF, valeur);
		case 'A':
		case 'a':
			voiture = creerVoiture(TypeUsager::AUTRE, valeur);
		case 'S':
		case 's':
			voiture = creerVoiture(TypeUsager::AUCUN, valeur);
		default:
			break;
	}
} //----- fin de Commande

