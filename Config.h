/*************************************************************************
                                    config
	Configuration de l'application : constantes et structures de données
                             -------------------
    début                : 16/03/2016
    copyright            : 2016 (C) par Ruben
    e-mail               : ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Interface du module <config> (fichier config.h) ---------
#if ! defined ( CONFIG_H )
#define CONFIG_H

//------------------------------------------------------------------------
// Rôle du module <config>
// Config contient toutes les constantes utilisées par l'application,
// ainsi que les différentes structures de données qui seront utilisées.
// Ce module permet également de convertir certaines valeurs en type de
// zone directement utilisable par le module graphique.
//------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées
#include "Outils.h"

//------------------------------------------------------------- Constantes
const unsigned int NB_PLACES_PARKING = 8;
const unsigned int NB_ENTREES = 3;
const unsigned int NB_REQUETES = 3;
const unsigned int NB_SEGMENTS_A_PROTEGER = 7;
const unsigned int NB_VOITURES_MAX = 999;

const unsigned int NUM_PID_ENTREE_GB = 0;
const unsigned int NUM_PID_ENTREE_BP_PROFS = 1;
const unsigned int NUM_PID_ENTREE_BP_AUTRES = 2;

const unsigned int REQ_GB = 0;
const unsigned int REQ_BP_PROFS =  1;
const unsigned int REQ_BP_AUTRES = 2;

const unsigned int MSG_TYPE_ENTREE_GB = 1;
const unsigned int MSG_TYPE_ENTREE_BP_PROFS = 2;
const unsigned int MSG_TYPE_ENTREE_BP_AUTRES = 3;
const unsigned int MSG_TYPE_SORTIE = 4;

const unsigned int SEM_PARKING = 0;
const unsigned int SEM_COMPTEUR = 1;
const unsigned int SEM_NB_PLACES_OCCUPEES = 2;
const unsigned int SEM_REQUETE_GB = 3;
const unsigned int SEM_REQUETE_BP_PROFS = 4;
const unsigned int SEM_REQUETE_BP_AUTRES = 5;
const unsigned int SEM_IMMATRICULATION = 6;

#define DROITS_ACCES 0777

//------------------------------------------------------------------ Types
struct voiture
// C'est la structure des messages échangés par boites aux lettres
// INFORMATIONS sur le type : (voir constantes)
//	1:	Entrée GB
//	2:	Entrée BP Profs
//	3:	Entrée BP Autres
//	4:	Sortie
{
	long type;					// Type de message
	unsigned int numPlace;		// Numéro de la place de parking
	unsigned int numVoiture;	// Numéro d'immatriculation de la voiture
	enum TypeUsager usager;		// Prof ou autre
	time_t heureArrive;			// Heure d'arrivé à la porte
};

struct placeParking
// C'est la structure qui nous permet de conserver une trace de l'état
// actuel du parking
{
	unsigned int numVoiture;	// Numéro d'immatriculation de la voiture
	enum TypeUsager usager;		// Prof ou autre
	time_t heureArrive;			// Heure d'arrivé dans le parking
};

struct requeteEntree
// C'est la structure qui permet aux portes d'entrées de faire des
// requêtes, i.e. signaler que le parking est plein mais qu'une voiture
// est prête à rentrer
{
	unsigned int numVoiture;	// Numéro d'immatriculation de la voiture
	enum TypeUsager usager;		// Prof ou autre
	time_t heureArrive;			// Heure d'arrivé à la porte
};

union semun
// Lors de la commande make, un message d'erreur apparait nous disant queue
// cette structure n'est pas connue, alors que nous incluons bien sys/types.h,
// sys/sem.h et sys/ipc.h ...
// Nous avons donc pris le parti de la redéfinir nous même
{
	 int val;                  // Valeur pour SETVAL
	 struct semid_ds *buf;     // Buffer pour IPC_STAT, IPC_SET
	 unsigned short *array;    // Tableau pour GETALL, SETALL
	 struct seminfo *__buf;    // Buffer pour IPC_INFO
};

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
TypeZone ConvertPlaceToZone( int numPlace );
// Mode d'emploi :
//		<numPlace>	: le numéro de la place de parking à convertir en une
//					  zone d'affichage
//
//		- Permet de convertir un numéro de place en une zone utilisable
//		  par le module graphique
//		- Retourne le type de la zone directement utilisable par le module
//		  graphique
//
// Contrat :
//		- Le numéro de place doit être cohérent : entre 1 et NB_PLACES_PARKING,
//		  les deux compris
//		- Si ce n'est pas le cas, le retour pourra être n'importe quoi

TypeZone ConvertNumSemToZone( int numSem );
// Mode d'emploi :
//		<numSem>	: l'indice du sémaphore élémentaire identifiant l'entrée à convertir
//					  en une zone utilisable par le module graphique
//
//		- Permet de convertir un indice de sémaphore élémentaire en une zone utilisable
//		  par le module graphique
//		- Retourne le type de la zone directement utilisable par le module
//		  graphique
//
// Contrat :
//		- L'indice du sémaphore élémentaire doit être cohérent : SEM_REQUETE_GB, SEM_REQUETE_BP_PROFS ou
//		  SEM_REQUETE_BP_AUTRES
//		- Si ce n'est pas le cas, le retour pourra être n'importe quoi

#endif // CONFIG_H

