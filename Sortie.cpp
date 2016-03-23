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
#include <cstdlib>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <list>

//------------------------------------------------------ Include personnel
#include "Sortie.h"
#include "Config.h"
#include "Outils.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types
typedef std::list<pid_t> ListeFils;
typedef std::list::iterator ListeFilsIterator;
typedef std::list::const_iterator ConstListeFilsIterator;

//---------------------------------------------------- Variables statiques
static std::list listeFils;
static int semaphoreID;
static struct placeParking* parking;
int* nbPlacesOccupees;

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
	// Prises des informations liées à la mort du fils
	int statut;
	pid_t pidFils = waitpid( -1, &statut, WNOHANG );
	int numPlace = WEXITSTATUS(statut);
	
	// On recherche le fils mort de la liste des fils
	ListeFilsIterator itr = listeFils.begin( );
	while ( itr != pidFils || itr != listeFils.end( ) )
	{
		itr++;
	}
	
	// Si ce fils existait bel et bien, on le supprime et on fait les traitements associés
	if( itr != listeFils.end( ) )
	{
		listeFils.erase( itr );
		
		// Init sembuf
		struct sembuf semOp;
		semOp.semNum = SEM_PARKING;
		semOp.sem_op = -1;
		semOp.sem_flags = NULL;
		
		// Mise à jour des places de parking
		semop( semaphoreID, &semOp, 1 );
		parking[numPlace].numVoiture = 0;
		semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
	}
	
} //----- fin de fin

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie( int parkingID, int balID, int nombrePlacesOccupeesID, int* requetesID, int semID )
// Algorithme :
//
{
	// INITIALISATION
	// Mise à jour des variables globales
	semaphoreID = semID;
	parkID = parkingID;
	
	// Attachement aux mps
	struct requeteEntree* requeteEGB = (requeteEntree*) shmat(requetesID[REQ_GB], NULL, NULL);
	struct requeteEntree* requeteEBP_profs = (requeteEntree*) shmat(requetesID[REQ_BP_PROFS], NULL, NULL);
	struct requeteEntree* requeteEGB_autres = (requeteEntree*) shmat(requetesID[REQ_BP_AUTRES], NULL, NULL);
	parking = (placeParking*) shmat(parkingID, NULL, NULL);
	nbPlacesOccupees = (int*) shmat(nombrePlacesOccupeesID, NULL, NULL);
	
	// Masquage signal
	struct sigaction sigusr2Action;
	sigusr2Action.sa_handler = fin;
	sigemptyset( &sigusr2Action.sa_mask );
	sigusr2Action.sa_flags = 0;
	sigaction( SIGUSR2, &sigusr2Action, NULL );
	
	struct sigaction sigchldAction;
	sigchldAction.sa_handler = mortFils;
	sigemptyset( &sigchldAction.sa_mask );
	sigchldAction.sa_flags = 0;
	sigaction( SIGINT, &sigchldAction, NULL );
	
	// MOTEUR
	// Attendre devant la boite aux lettres
	struct voiture message;
	while( msgrcv( balID, (void*) &message, sizeof(struct message)-sizeof(long), MSG_TYPE_SORTIE ) == -1 && errno == EINTR );
	
	// Lancer la tache qui va faire sortir la voiture
	unsigned int i;
	unsigned int numPlace = 0;
	for(i = 0; i< NB_PLACES_PARKING; i++)
	{
		// TODO : semaphore ?
		if(parking[i].numVoiture == message.numVoiture)
		{
			numPlace = parking[i].numPlace;
		}
	}
	pid_t voiturier = SortirVoiture( numPlace );
	if(voiturier != -1)
	{
		listeFils.push_back(voiturier);
	}
	
	// DESTRUCTION
	shmdt(nbPlacesOccupees);
	shmdt(parking);
	shmdt(requeteEGB_autres);
	shmdt(requeteEBP_profs);
	shmdt(requeteEGB);
	
} //----- fin de Nom





























