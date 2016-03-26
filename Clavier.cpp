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
//TODO :	va y avoir besoin de faire des globaux pour stocker ce que prend
//			Clavier() en parametre, pour que Commande() puisse les utiliser

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
			// TODO :	déposer un message dans la boite associée
			// NB :	une seule boite mais plusieurs files, suffit de mettre la bonne valeur de
			//		long type (voir config .h, dans les #define)
		case 'A':
		case 'a':
			voiture = creerVoiture(TypeUsager::AUTRE, valeur);
			// TODO :	déposer un message dans la boite associée
		case 'S':
		case 's':
			voiture = creerVoiture(TypeUsager::AUCUN, valeur);
			// TODO :	déposer un message dans la boite associée
		default:
			break;
	}
} //----- fin de Commande

