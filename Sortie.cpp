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
using namespace std;
#include <cstdlib>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <list>
#include <ctime>

// Debug
#include <fstream>

//------------------------------------------------------ Include personnel
#include "Sortie.h"
#include "Config.h"
#include "Outils.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types
typedef std::list<pid_t> ListeFils;
typedef std::list<pid_t>::iterator ListeFilsIterator;
typedef std::list<pid_t>::const_iterator ConstListeFilsIterator;

//---------------------------------------------------- Variables statiques
static ListeFils listeFils;

static int parkID;
static int nbPlacesID;
static int semaphoreID;
static int* reqsID;
static pid_t* entreesPID;

static struct placeParking* parking;
static int* nbPlaces;
static struct requeteEntree* requeteEGB;
static struct requeteEntree* requeteEBP_profs;
static struct requeteEntree* requeteEGB_autres;

// Debug
ofstream log;

//------------------------------------------------------ Fonctions privées
static void fin ( int noSignal )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
{
	log << "On a recu le signal de fin" << endl;
	log.close();
	
	sigaction( SIGCHLD, NULL, NULL );
	shmdt( nbPlaces );
	shmdt( parking );
	shmdt( requeteEGB_autres );
	shmdt( requeteEBP_profs );
	shmdt( requeteEGB );
	
	ListeFilsIterator itr;
	for ( itr = listeFils.begin( ); itr != listeFils.end(); itr++ )
	{
		kill( *itr, SIGUSR2 );
		waitpid( *itr, NULL, 0 );
	}
	
	exit(0);
	
} //----- fin de fin


static void mortFils ( int noSignal )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
{
	log << "Un fils est mort" << endl;
	// Prises des informations liées à la mort du fils
	int statut;
	pid_t pidFils = waitpid( -1, &statut, WNOHANG );
	int numPlace = WEXITSTATUS(statut);
	time_t heureDepart = time( NULL );
	
	// On recherche le fils mort de la liste des fils
	ListeFilsIterator itr = listeFils.begin( );
	while ( itr != listeFils.end( ) || *itr != pidFils )
	{
		itr++;
	}
	
	// Si ce fils existait bel et bien, on le supprime et on fait les traitements associés
	if( itr != listeFils.end( ) )
	{
		listeFils.erase( itr );
		
		// Init sembuf
		struct sembuf semOp;
		semOp.sem_num = SEM_PARKING;
		semOp.sem_op = -1;
		semOp.sem_flg = NULL;
		
		// Mise à jour de l'affichage de la sortie
		semop( semaphoreID, &semOp, 1 );
			AfficherSortie( parking[numPlace-1].usager, parking[numPlace-1].numVoiture, parking[numPlace-1].heureArrive, heureDepart );
			// NB : on dispose d'une ressource et on en demande une autre via AfficherSortie,
			//		mais cela ne devrait pas mener à un interblocage.
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		// Mise à jour des places de parking
		semOp.sem_op = -1;
		semop( semaphoreID, &semOp, 1 );
			parking[numPlace-1].usager = AUCUN;
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		// Mise à jour du nombre de voitures
		bool envoyerSignal = false;
		pid_t entreeADebloquer;
		
		semOp.sem_op = -1;
		semOp.sem_num = SEM_COMPTEUR;
		semop( semaphoreID, &semOp, 1 );
			if(--(*nbPlaces) == NB_PLACES_PARKING-1)
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
			semOp.sem_num = NUM_PID_ENTREE_GB;
				bestUsager = requeteEGB->usager;
				meilleureHeure = requeteEGB->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
			
			semOp.sem_op = -1;
			semOp.sem_num = NUM_PID_ENTREE_BP_PROFS;
				usager = requeteEBP_profs->usager;
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
			semOp.sem_num = NUM_PID_ENTREE_BP_AUTRES;
				usager = requeteEBP_profs->usager;
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
			
			kill( entreeADebloquer, SIGUSR1 );
		}
	}
	
} //----- fin de fin

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie( int parkingID, int balID, int nombrePlacesOccupeesID, int* requetesID, int semID, pid_t* entreesID )
// Algorithme :
//
{
	log.open("sortie.log");
	// INITIALISATION
	// Mise à jour des variables globales
	parkID = parkingID;
	nbPlacesID = nombrePlacesOccupeesID;
	semaphoreID = semID;
	reqsID = requetesID;
	entreesPID = entreesID;
	
	// Attachement aux mps
	requeteEGB = (requeteEntree*) shmat( requetesID[REQ_GB], NULL, NULL );
	requeteEBP_profs = (requeteEntree*) shmat( requetesID[REQ_BP_PROFS], NULL, NULL );
	requeteEGB_autres = (requeteEntree*) shmat( requetesID[REQ_BP_AUTRES], NULL, NULL );
	parking = (placeParking*) shmat( parkingID, NULL, NULL );
	nbPlaces = (int*) shmat( nombrePlacesOccupeesID, NULL, NULL );
	
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
	sigaction( SIGCHLD, &sigchldAction, NULL );
	
	log << "INIT reussie" << endl;
	// MOTEUR
	for( ;; )
	{
		log << "Attente qu'une voiture sorte (INFINITE LOOP)..." << endl;
		// Attendre devant la boite aux lettres
		struct voiture message;
		while( msgrcv( balID, (void*) &message, sizeof(struct voiture)-sizeof(long), MSG_TYPE_SORTIE, NULL ) == -1 && errno == EINTR );
		
		log << "Un voiture veut sortir ! C'est la numero : " << message.numVoiture << endl;
		// Lancer la tache qui va faire sortir la voiture
		unsigned int i;
		unsigned int numPlace = 0;
		for(i = 0; i< NB_PLACES_PARKING; i++)
		{
			// TODO : semaphore ?
			if( parking[i].numVoiture == message.numVoiture )
			{
				numPlace = i+1;
				log << "On la connait ! Elle etait a la place " << numPLace << endl;
				break;
			}
		}
		pid_t voiturier = SortirVoiture( numPlace );
		if( voiturier != -1 )
		{
			listeFils.push_back(voiturier);
		}
	}
	
	// DESTRUCTION
	// Via handler de SIGUSR2
	
} //----- fin de Nom





























