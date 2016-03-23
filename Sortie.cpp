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

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie( int parkingID, int nombrePlacesOccupeesID, int* requetesID, int nbRequetes )
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
	
	// MOTEUR
	
	// DESTRUCTION
	
} //----- fin de Nom





























