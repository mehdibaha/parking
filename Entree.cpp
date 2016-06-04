/*************************************************************************
                           Entree  -  description
                             -------------------
    début                : 2016
    copyright            : 2015-2016 (C) par Mehdi Baha, Ruben Pericas-Moya
    e-mail               : el-mehdi.baha@insa-lyon.fr, ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <Entree> (fichier Entree.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système

using namespace std;
#include <map>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <list>
#include <ctime>

// Debug
#include <fstream>

//------------------------------------------------------ Include personnel
#include "Entree.h"
#include "Config.h"
#include "Outils.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes


//------------------------------------------------------------------ Types
typedef struct voiture Voiture;
typedef struct requeteEntree RequeteEntree;

//---------------------------------------------------- Variables statiques
static int boiteID;
static int parkID;
static int nbPlacesID;
static int reqID;
static int semaphoreID;
static int numSemReq;

static struct placeParking* parking;
static unsigned int* nbPlaces;
struct requeteEntree* req;

static map<pid_t,Voiture> voitureMap;

static int signalRecu = 0;
static Voiture msg;
static long typeBar;

//------------------------------------------------------ Fonctions privées
static void garer(Voiture& message)
// Mode d'emploi :
// Permet de gérer l'arrivée d'une voiture de l'entrée à sa place,
// à partir du message d'entrée de la voiture
{
	TypeBarriere typeBarriere;
	switch(typeBar)
	{
		case MSG_TYPE_ENTREE_GB:
			typeBarriere = ENTREE_GASTON_BERGER;
			break;
		case MSG_TYPE_ENTREE_BP_PROFS:
			typeBarriere = PROF_BLAISE_PASCAL;
			break;
		case MSG_TYPE_ENTREE_BP_AUTRES:
			typeBarriere = AUTRE_BLAISE_PASCAL;
			break;
		default:
			typeBarriere = AUCUNE;
			break;
	}
	
	
	if( signalRecu != SIGUSR1 )
	{
		DessinerVoitureBarriere(typeBarriere, message.usager);
	}
		
	if (*nbPlaces < NB_PLACES_PARKING)
	{
        // Il y a de la place   	
        // Initialisation semaphore nbPlacesOccupees
		struct sembuf semOp;
		semOp.sem_op = -1;
		semOp.sem_num = SEM_NB_PLACES_OCCUPEES;
		semOp.sem_flg = 0;
		
        // Mise à jour du nombre de places occupées
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			(*nbPlaces)++;
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );

		pid_t pidCourant = GarerVoiture(typeBarriere);
		if (pidCourant != -1)
		{
            // Ajout de la voiture dans la structure
			voitureMap.insert(make_pair(pidCourant, message));
		}
		
		sleep(1);	// On évite les collisions
		
		// Créaton d'une tâche pour créer la voiture
	}
	else
	{
		// Il n'y a pas de place
		msg = message;
		// Initialisation semaphore pour les requêtes
		struct sembuf semOp;
		semOp.sem_op = -1;
		semOp.sem_num = numSemReq;
		semOp.sem_flg = 0;
		// Mise à jour de la requete
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			req->numVoiture = msg.numVoiture;
			req->usager = msg.usager;
			req->heureArrive = time(NULL);
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		AfficherRequete(typeBarriere, req->usager, req->heureArrive);
		
		// On temporise juqu'à l'arrivée d'un signal SIGUSR1
		do
		{
			pause();
		} while( signalRecu != SIGUSR1 );
		signalRecu = 0;
		// NB :		do-while pour éviter de faire quelque chose si on a recu autre chose que sigusr1
		
	}
} //----- fin de garer

static void placeLibre( int noSignal )
// Mode d'emploi :
// Gère le signal SIGUSR1
{
	if(noSignal != SIGUSR1)
	{
		return;
	}
	// SIGUSR1 reçu
	// Init sembuf
	struct sembuf semOp;
	semOp.sem_op = -1;
	semOp.sem_num = numSemReq;
	semOp.sem_flg = 0;
	// Mise à jour de la requete
	while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
		req->usager = AUCUN;
		semOp.sem_op = 1;
	semop( semaphoreID, &semOp, 1 );
	
	// Effacement écran requete
	Effacer( ConvertNumSemToZone( numSemReq ) );
	
	signalRecu = noSignal;
	garer(msg);
} //----- fin de placeLibre


static void fin ( int noSignal )
// Mode d'emploi :
// Met fin à la tâche principale
{
	if(noSignal != SIGUSR2)
	{
		return;
	}
	// Réception signal de fin
    sigaction( SIGCHLD, NULL, NULL );
	sigaction( SIGUSR1, NULL, NULL );
	sigaction( SIGUSR2, NULL, NULL );
	
	shmdt( parking );
    shmdt( nbPlaces );
    shmdt( req );

    // On kill toutes les tâches liées aux voitures présentes
    for ( auto itr = voitureMap.begin( ); itr != voitureMap.end(); itr++ )
    {
        kill( itr->first, SIGUSR2 );
        waitpid( itr->first, NULL, 0 );
    }

    exit(0);
} //----- fin de fin

static void mortFils ( int noSignal )
// Mode d'emploi :
// Gère l'arrivée d'une voiture (mort fils correspond à l'appel Mere de Garer)
{
	if(noSignal != SIGCHLD)
	{
		return;
	}
    // Prises des informations liées à la mort du fils
    int statut;
    pid_t pidFils = waitpid( -1, &statut, WNOHANG );
    int numPlace = WEXITSTATUS(statut);
    time_t heureEntree = time( NULL );

    // On recherche le fils mort de la liste des fils
    auto itr = voitureMap.find(pidFils);
    // Si ce fils existait bel et bien, on le supprime et on fait les traitements associés
    if( itr != voitureMap.end( ) )
    {
        Voiture v = itr->second;
		
        // Init sembuf
        struct sembuf semOp;
        semOp.sem_num = SEM_PARKING;
        semOp.sem_op = -1;
        semOp.sem_flg = 0;
		
        // Mise à jour des places de parking
        while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			parking[numPlace-1].heureArrive = heureEntree;
			parking[numPlace-1].numVoiture = v.numVoiture;
			parking[numPlace-1].usager = v.usager;
			semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );

		// Mise à jour de l'affichage de l'entrée
		AfficherPlace( numPlace, v.usager, v.numVoiture, v.heureArrive );

		// Mise à jour de l'affichage du parking
        voitureMap.erase( itr );
    }

} //----- fin de mortFils

static void moteur( long type )
// Mode d'emploi :
// Bloque jusqu'à l'arrivée d'un message d'entrée d'une voiture
{
    Voiture message;

	for( ;; )
	{
		// On attend qu'une voiture arrive
		while( msgrcv( boiteID, (void*) &message, sizeof(struct voiture)-sizeof(long), type, 0 ) == -1 && errno == EINTR );
		
		// Lancement la tâche qui va faire rentrer la voiture
		garer(message);
		
	}
} //----- fin de moteur

static void init( )
// Mode d'emploi :
// Initialisation de l'application
{
    // Attachement aux mémoires partagées
    parking = (placeParking*) shmat( parkID, NULL, 0 );
    nbPlaces = (unsigned int*) shmat( nbPlacesID, NULL, 0 );
    req = (requeteEntree*) shmat( reqID, NULL, 0 );
	
	// Armer SIGUSR1 sur placeLibre
	struct sigaction sigusr1Action;
    sigusr1Action.sa_handler = placeLibre;
    sigemptyset( &sigusr1Action.sa_mask );
    sigusr1Action.sa_flags = 0;
    sigaction( SIGUSR1, &sigusr1Action, NULL );
	
    // Armer SIGUSR2 sur fin
    struct sigaction sigusr2Action;
    sigusr2Action.sa_handler = fin;
    sigemptyset( &sigusr2Action.sa_mask );
    sigusr2Action.sa_flags = 0;
    sigaction( SIGUSR2, &sigusr2Action, NULL );

    // Armer SIGCHLD sur mortFils
    struct sigaction sigchldAction;
    sigchldAction.sa_handler = mortFils;
    sigemptyset( &sigchldAction.sa_mask );
    sigchldAction.sa_flags = 0;
    sigaction( SIGCHLD, &sigchldAction, NULL );
} //----- fin de init

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Entree( int balID, int parkingID, int nombrePlacesOccupeesID,
                int requeteID, int semID, int numSemRequete, long type )
{	
	// Initialisation des globaux privés
	boiteID = balID;
	parkID = parkingID;
	nbPlacesID = nombrePlacesOccupeesID;
	reqID = requeteID;
    semaphoreID = semID;
	numSemReq = numSemRequete;
	typeBar = type;
	
	// Phase d'INITIALISATION
    init( );

	// Phase MOTEUR
    moteur( type );
} //----- fin de Entree

