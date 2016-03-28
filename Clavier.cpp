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
#include <sys/msg.h>

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
static int boiteID;

//------------------------------------------------------ Fonctions privées
static Voiture creerVoiture ( TypeUsager typeUsager, long valeur, bool entree )
// Mode d'emploi :
// Construit et initialise une structure de type Voiture
// TODO : expliquer la semantique differente de valeur, et p-e renommer
{
	Voiture voiture;
	voiture.usager = typeUsager;
	voiture.heureArrive = time(NULL);
	if(entree)
	{
		compteurVoiture++;
		voiture.numPlace = 0;
		voiture.numVoiture = compteurVoiture % (NB_VOITURES_MAX + 1);
	}
	else
	{
		voiture.numPlace = valeur;
		voiture.numVoiture = 0;
	}
	return voiture;
} //----- fin de creerVoiture

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Clavier ( int balID )
{
	boiteID = balID;
	
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
			voiture = creerVoiture(TypeUsager::PROF, valeur, true);
			voiture.type = valeur == 1 ? MSG_TYPE_ENTREE_BP_PROFS : MSG_TYPE_ENTREE_GB;
			msgsnd( boiteID, &voiture, sizeof(Voiture)-sizeof(long), NULL );
			break;
			// NB :	une seule boite mais plusieurs files, suffit de mettre la bonne valeur de
			//		long type (voir config .h, dans les #define)
		case 'A':
		case 'a':
			voiture = creerVoiture(TypeUsager::AUTRE, valeur, true);
			voiture.type = valeur == 1 ? MSG_TYPE_ENTREE_BP_AUTRES : MSG_TYPE_ENTREE_GB;
			msgsnd( boiteID, &voiture, sizeof(Voiture)-sizeof(long), NULL );
			break;
		case 'S':
		case 's':
			voiture = creerVoiture(TypeUsager::AUTRE, valeur, false);
			voiture.type = MSG_TYPE_SORTIE;
			msgsnd( boiteID, &voiture, sizeof(Voiture)-sizeof(long), NULL );
			break;
		default:
			break;
	}
} //----- fin de Commande

