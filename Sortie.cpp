/*************************************************************************
                           Sortie  -  description
                             -------------------
    début                :
    copyright            : (C) par
    e-mail               :
*************************************************************************/

//---------- Réalisation de la tâche <Sortie> (fichier Sortie.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système

//------------------------------------------------------ Include personnel
#include "Sortie.h"
#include "Config.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques

//------------------------------------------------------ Fonctions privées
static void fin ( int noSignal )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
{
	
} //----- fin de fin


static void mortFils ( int noSignal )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
{
	
} //----- fin de fin

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie( int parkingID, int balID, int nombrePlacesOccupeesID, int* requetesID, int nbRequetes )
// Algorithme :
//
{
	// INITIALISATION
	// Attachement aux mps
	struct requeteEntree* requeteEGB = shmat(requetesID[REQ_GB], NULL, NULL);
	struct requeteEntree* requeteEBP_profs = shmat(requetesID[REQ_BP_PROFS], NULL, NULL);
	struct requeteEntree* requeteEGB_autres = shmat(requetesID[REQ_BP_AUTRES], NULL, NULL);
	struct placeParking* parking = shmat(parkingID, NULL, NULL);
	int* nbPlacesOccupees = shmat(nombrePlacesOccupeesID, NULL, NULL);
	
	// Masquage signal
	struct sigaction sigusr2Action;
	sigusr2Action.sa_handler = fin;
	sigemptyset( &sigusr2Action.sa_mask );
	sigusr2Action.sa_flags = 0;
	sigaction( SIGUSR2, &sigusr2Action, NULL );
	
	struct sigaction sigchldAction;
	sigchldAction.sa_handler = cleanFin;
	sigemptyset( &sigchldAction.sa_mask );
	sigchldAction.sa_flags = 0;
	sigaction( SIGINT, &sigchldAction, NULL );
	
	// MOTEUR
	
	// DESTRUCTION
	shmdt(nbPlacesOccupees);
	shmdt(parking);
	shmdt(requeteEGB_autres);
	shmdt(requeteEBP_profs);
	shmdt(requeteEGB);
	
} //----- fin de Nom





























