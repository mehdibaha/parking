/*************************************************************************
                           config  -  description
                             -------------------
    début                : 
    copyright            : (C)  par 
    e-mail               : 
*************************************************************************/

//---------- Interface du module <config> (fichier config.h) ---------
#if ! defined ( CONFIG_H )
#define CONFIG_H

//------------------------------------------------------------------------
// Rôle du module <config>
//
//
//------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées
#include <ctime>
#include "Outils.h"

//------------------------------------------------------------- Constantes
#define NB_PLACES_PARKING 8
#define NB_ENTREES 3
#define NB_REQUETES 3
#define NB_SEGMENTS_A_PROTEGER 6

#define REQ_GB 0
#define REQ_BP_PROFS 1
#define REQ_BP_AUTRES 2

#define MSG_TYPE_ENTREE_GB 0
#define MSG_TYPE_ENTREE_BP 1
#define MSG_TYPE_SORTIE 2

#define SEM_PARKING 0
#define SEM_COMPTEUR 1
#define SEM_NB_PLACES_OCCUPEES 2
#define SEM_REQUETE_GB 3
#define SEM_REQUETE_BP_PROFS 4
#define SEM_REQUETE_BP_AUTRES 5

//------------------------------------------------------------------ Types
struct voiture
// INFORMATIONS sur le type :
//	0:	Entrée GB
//	1:	Entrée BP
//	2:	Sortie
{
	long type;
	int numVoiture;
	enum TypeUsager usager;
	time_t heure;
};

struct placeParking
{
	int numPlace;
	int numVoiture;
};

struct requeteEntree
{
	int numVoiture;
	enum TypeUsager usager;
	time_t heure;
};

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
// type Nom ( liste de paramètres );
// Mode d'emploi :
//
// Contrat :
//

#endif // CONFIG_H

