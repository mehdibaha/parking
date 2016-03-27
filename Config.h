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
#define NB_SEGMENTS_A_PROTEGER 7
#define NB_VOITURES_MAX 999

#define NUM_PID_ENTREE_GB 0
#define NUM_PID_ENTREE_BP_PROFS 1
#define NUM_PID_ENTREE_BP_AUTRES 2

#define REQ_GB 0
#define REQ_BP_PROFS 1
#define REQ_BP_AUTRES 2

#define MSG_TYPE_ENTREE_GB 1
#define MSG_TYPE_ENTREE_BP 2
#define MSG_TYPE_SORTIE 3

#define SEM_PARKING 0
#define SEM_COMPTEUR 1
#define SEM_NB_PLACES_OCCUPEES 2
#define SEM_REQUETE_GB 3
#define SEM_REQUETE_BP_PROFS 4
#define SEM_REQUETE_BP_AUTRES 5
#define SEM_IMMATRICULATION 6

//------------------------------------------------------------------ Types
struct voiture
// INFORMATIONS sur le type :
//	1:	Entrée GB
//	2:	Entrée BP
//	3:	Sortie
{
	long type;
	unsigned int numVoiture;
	enum TypeUsager usager;
	time_t heureArrive;
};

struct placeParking
{
	unsigned int numPlace;
	unsigned int numVoiture;
	enum TypeUsager usager;
	time_t heureArrive;
};

struct requeteEntree
{
	unsigned int numVoiture;
	enum TypeUsager usager;
	time_t heureArrive;
};

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
// type Nom ( liste de paramètres );
// Mode d'emploi :
//
// Contrat :
//

#endif // CONFIG_H

