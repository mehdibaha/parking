/*************************************************************************
                           <Clavier>  -  description
                             -------------------
    début                : 2016
    copyright            : 2015-2016 (C) par Mehdi Baha, Ruben Pericas-Moya
    e-mail               : el-mehdi.baha@insa-lyon.fr, ruben.pericas-moya@insa-lyon.fr
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
// Voir Config.h pour utiliser le paramètre valeur
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
{
	Voiture voiture;
    switch( code )
	{
		case 'E':
		case 'e':
			// Fin de l'application
			exit( 0 );
		case 'P':
		case 'p':
			// Arrivée d'un prof
			voiture = creerVoiture(TypeUsager::PROF, valeur, true);
			voiture.type = valeur == 1 ? MSG_TYPE_ENTREE_BP_PROFS : MSG_TYPE_ENTREE_GB;
			msgsnd( boiteID, &voiture, sizeof(Voiture)-sizeof(long), NULL );
			break;
		case 'A':
		case 'a':
			// Arrivée d'un "autre"
			voiture = creerVoiture(TypeUsager::AUTRE, valeur, true);
			voiture.type = valeur == 1 ? MSG_TYPE_ENTREE_BP_AUTRES : MSG_TYPE_ENTREE_GB;
			msgsnd( boiteID, &voiture, sizeof(Voiture)-sizeof(long), NULL );
			break;
		case 'S':
		case 's':
			// Sortie d'une voiture quelquonque
			voiture = creerVoiture(TypeUsager::AUTRE, valeur, false);
			voiture.type = MSG_TYPE_SORTIE;
			msgsnd( boiteID, &voiture, sizeof(Voiture)-sizeof(long), NULL );
			break;
		default:
			break;
	}
} //----- fin de Commande

