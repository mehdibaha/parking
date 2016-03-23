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
#include <ctime>

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
static int* nbPlacesOccupees;
static pid_t* entreesPID;
struct requeteEntree* requeteEGB;
struct requeteEntree* requeteEBP_profs;
struct requeteEntree* requeteEGB_autres;

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
	time_t heureDepart = time( NULL );
	
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
		
		// Mise à jour de l'affichage de la sortie
		semop( semaphoreID, &semOp, 1 );
			AfficherSortie( parking[numPlace].usager, parking[numPlace].numVoiture, parking[numPlace].heureArrive, heureDepart );
			// NB : on dispose d'une ressource et on en demande une autre via AfficherSortie,
			//		mais cela ne devrait pas mener à un interblocage.
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		// Mise à jour des places de parking
		semOp.sem_op = -1;
		semop( semaphoreID, &semOp, 1 );
			parking[numPlace].usager = AUCUN;
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		// Mise à jour du nombre de voitures
		bool envoyerSignal = false;
		pid_t entreeADebloquer;
		
		semOp.sem_op = -1;
		semOp.semNum = SEM_COMPTEUR;
		semop( semaphoreID, &semOp, 1 );
			if(--nbPlacesOccupees == 7)
			{
				envoyerSignal = true;
			}
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
			
		// Envoie d'un signal pour débloquer les entrées si nécessaire
		if( envoyerSignal )
		{
			time_t heure = 0;
			time_t meilleureHeure = 0;
			enum TypeUsager usager = AUCUN;
			enum TypeUsager bestUsager = AUCUN;
			
			semOp.sem_op = -1;
			semOp.semNum = NUM_ID_ENTREE_GB;
				bestUsager = requeteEGB->usager
				meilleureHeure = requeteEGB->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
			
			semOp.sem_op = -1;
			semOp.semNum = NUM_PID_ENTREE_BP_PROFS;
				usager = requeteEBP_profs->usager
				heure = requeteEBP_profs->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
			if( bestUsager != PROF && usager == PROF )
			{
				meilleureHeure = heure;
				bestUsager = usager;
				entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_PROFS];
			}
			else if( bestUsager == PROF && usager == PROF )
			{
				if ( heure > meilleureHeure )
				{
					meilleureHeure = heure;
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_PROFS];
				}					
			}
			else if( bestUsager == AUCUN && usager == AUCUN)
			{
				// Ne rien faire
			}
			else
			{
				entreeADebloquer = entreesPID[NUM_PID_ENTREE_GB];
			}
			
			semOp.sem_op = -1;
			semOp.semNum = NUM_PID_ENTREE_BP_AUTRES;
				usager = requeteEBP_profs->usager
				heure = requeteEBP_profs->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
			if( bestUsager == PROF  || ( bestUsager == AUCUN && usager == AUCUN ) || usager == AUCUN )
			{
				// Ne rien faire
			}
			else if( bestUsager == AUTRE && usager == AUTRE )
			{
				if ( heure > meilleureHeure )
				{
					meilleureHeure = heure;
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_AUTRES];
				}
			}
			else 
			{
				entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_AUTRES];
			}
			
			kill( entreeADebloquer, SIGUSR1, NULL );
		}
		
	}
	
} //----- fin de fin

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie( int parkingID, int balID, int nombrePlacesOccupeesID, int* requetesID, int semID, pid_t* entreesID )
// Algorithme :
//
{
	// INITIALISATION
	// Mise à jour des variables globales
	semaphoreID = semID;
	parkID = parkingID;
	entreesPID = entreesID;
	
	// Attachement aux mps
	requeteEGB = (requeteEntree*) shmat( requetesID[REQ_GB], NULL, NULL );
	requeteEBP_profs = (requeteEntree*) shmat( requetesID[REQ_BP_PROFS], NULL, NULL );
	requeteEGB_autres = (requeteEntree*) shmat( requetesID[REQ_BP_AUTRES], NULL, NULL );
	parking = (placeParking*) shmat( parkingID, NULL, NULL );
	nbPlacesOccupees = (int*) shmat( nombrePlacesOccupeesID, NULL, NULL );
	
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
		if( parking[i].numVoiture == message.numVoiture )
		{
			numPlace = parking[i].numPlace;
		}
	}
	pid_t voiturier = SortirVoiture( numPlace );
	if( voiturier != -1 )
	{
		listeFils.push_back(voiturier);
	}
	
	// DESTRUCTION
	shmdt(nbPlacesOccupees);
	shmdt(parking);
	shmdt(requeteEGB_autres);
	shmdt(requeteEBP_profs);
	shmdt(requeteEGB);
	//TODO : demasquer sigchld
	
} //----- fin de Nom





























