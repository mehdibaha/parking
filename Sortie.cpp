/*************************************************************************
								    Sortie
		    Tâche permettant à une voiture de sortir du parking
                             -------------------
    début                : 16/03/2016
    copyright            : 2016 (C) par Ruben
    e-mail               : ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <Sortie> (fichier Sortie.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
using namespace std;
#include <cstdlib>			// exit
#include <errno.h>			// pour connaitre la valeur de errno
#include <signal.h>			// signaux, sigaction
#include <sys/msg.h>		// boite aux lettres
#include <sys/shm.h>		// mémoire partagée
#include <sys/wait.h>		// waitpid
#include <sys/sem.h>		// sémaphore
#include <list>				// pour stocker les PIDs des tâches filles
#include <ctime>			// pour connaitre l'heure d'arrivée

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
static ListeFils listeFils;	// Les fils encore vivants

static int parkID;			// Identifiant de la MP représentant l'état du parking
static int nbPlacesID;		// Identifiant de la MP le nombre de places du parking occupées
static int semaphoreID;		// Identifiant du sémaphore général
static int* reqsID;			// Tableau des identifiants de MP utilisés par les tâches d'entrées
							// pour signaler que le parking est plein mais qu'une voiture veut rentrer
							
static pid_t* entreesPID;	// Tableau des PIDs des tâches d'entrée

static struct placeParking* parking;			// Zone de la MP <parkID>
static int* nbPlaces;							// Zone de la MP <nbPlacesID>
static struct requeteEntree* requeteEGB;		// Zone d'une des MPs de <reqsID> (pour Gaston Berger)
static struct requeteEntree* requeteEBP_profs;	// Zone d'une des MPs de <reqsID> (pour Blaise Pascal côté profs)
static struct requeteEntree* requeteEBP_autres;	// Zone d'une des MPs de <reqsID> (pour Blaise Pascal côté autres)

//------------------------------------------------------ Fonctions privées
static void fin ( int noSignal )
// Mode d'emploi :
//		Handler du signal SIGUSR2
//
//		<noSignal>	: le signal reçu
//
//		- Fait en sorte d'ignorer le signal SIGCHLD
//		- Détache toutes les MPs auquelles la tâche était attachée
//		- Met fin à tous les processus fils de la tâche par l'envoie de SIGUSR2
//
// Contrat :
//		- Ne doit pas être appelé explicitement
//
// Algorithme :
//		Destruction de la tâche par détachement des MPs
//		Pour chaque fils encore en vie
//			on signale à ce fils de mourir,
//			et on attend que ce soit fait
//		On met fin à la tâche avec le code 0
{
	if(noSignal != SIGUSR2)
	{
		return;
	}
	sigaction( SIGCHLD, NULL, NULL );
	shmdt( nbPlaces );
	shmdt( parking );
	shmdt( requeteEBP_autres );
	shmdt( requeteEBP_profs );
	shmdt( requeteEGB );
	
	for ( ConstListeFilsIterator itr = listeFils.begin( ); itr != listeFils.end(); itr++ )
	{
		kill( *itr, SIGUSR2 );
		waitpid( *itr, NULL, 0 );
	}
	
	exit(0);
	
} //----- fin de fin


static void mortFils ( int noSignal )
// Mode d'emploi :
//		Handler du signal SIGCHLD
//
//		<noSignal>	: le signal reçu
//
//		- Récupère le PIDs et le code de sortie du fils mort,
//		  et supprime ce dernier de la liste des fils
//		- Met à jour l'affichage des places de parking
//		- Met à jour l'affichage de la zone sortie (infos sur la voiture sortie)
//		- Décrémente le nombre de voitures présentes dans le parking
//		- Si le parking était plein, parcours les requêtes potentiellement déposées
//		  par les entrées, et envoie le signal SIGUSR1 à l'entrée la plus prioritaire
//
// Contrat :
//		- Ne doit pas être appelé explicitement
//
// Algorithme :
//		Récupère les informations liées à la mort du fils
//		Parcours de la liste des fils pour supprimer le fils en question
//		(NB: std::list::remove pose problème, donc effacement à la main)
//		Accès aux MPs pour écriture via mutex
//		Si le parking était plein
//			On defnie de quoi conserver une trace de la requete la plus prioritaire
//			On parcourt les MPs de requetes :
//			Si la requete contient une requete plus prioritaire que celle existante
//				On met à jour la requete la plus prioritaire
//			Si au moins une des requêtes était non nulle
//				On envoie SIGUSR1 à la requête la plus prioritaire
//		NB: la priorité est telle que décrite dans Sortie.h, pour la fonction Sortie
{
	if(noSignal != SIGCHLD)
	{
		return;
	}
	// Prises des informations liées à la mort du fils
	time_t heureDepart = time( NULL );
	int statut;
	pid_t pidFils = waitpid( -1, &statut, WNOHANG );
	int numPlace = WEXITSTATUS(statut);
	
	// On recherche le fils mort de la liste des fils
	ListeFilsIterator itr = listeFils.begin( );
	while ( itr != listeFils.end( ) && *itr != pidFils )
	{
		itr++;
	}
		
	// Si ce fils existait bel et bien, on le supprime et on fait les traitements associés
	// (prévient d'éventuels bugs)
	if( itr != listeFils.end( ) )
	{
		listeFils.erase( itr );
		
		// Init sembuf
		struct sembuf semOp;
		semOp.sem_num = SEM_PARKING;
		semOp.sem_op = -1;
		semOp.sem_flg = 0;
		
		// Mise à jour de l'affichage de l'état des places de parking
		Effacer(ConvertPlaceToZone(numPlace));
		
		// Mise à jour de l'affichage de la sortie
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			AfficherSortie( parking[numPlace-1].usager, parking[numPlace-1].numVoiture, parking[numPlace-1].heureArrive, heureDepart );
			// NB : on dispose d'une ressource et on en demande une autre via AfficherSortie,
			//		mais cela ne devrait pas mener à un interblocage.
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );

		// Mise à jour des places de parking
		semOp.sem_op = -1;
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			parking[numPlace-1].usager = AUCUN;
			parking[numPlace-1].numVoiture = 0;
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
				
		// Variables pour envoi potentiel de signal
		bool envoyerSignal = false;
		
		// Mise à jour du nombre de voitures
		semOp.sem_op = -1;
		semOp.sem_num = SEM_COMPTEUR;
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
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
			pid_t entreeADebloquer = 0;
						
			semOp.sem_op = -1;
			semOp.sem_num = SEM_REQUETE_GB;
			while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
				bestUsager = requeteEGB->usager;
				meilleureHeure = requeteEGB->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
						
			semOp.sem_op = -1;
			semOp.sem_num = SEM_REQUETE_BP_PROFS;
			while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
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
				if ( heure < meilleureHeure )
				{
					meilleureHeure = heure;
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_PROFS];
				}
				else
				{
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_GB];
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
			semOp.sem_num = SEM_REQUETE_BP_AUTRES;
			while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
				usager = requeteEBP_autres->usager;
				heure = requeteEBP_autres->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
			if( bestUsager == PROF  || ( bestUsager == AUCUN && usager == AUCUN ) || usager == AUCUN )
			{
				// Ne rien faire
			}
			else if( bestUsager == AUTRE && usager == AUTRE )
			{
				if ( heure < meilleureHeure )
				{
					meilleureHeure = heure;
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_AUTRES];
				}
				else
				{
					// On garde le précédent
				}
			}
			else 
			{
				entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_AUTRES];
			}
						
			if( entreeADebloquer )
			{
				kill( entreeADebloquer, SIGUSR1 );
			}
			
		}
	}
	
} //----- fin de fin

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie( int parkingID, int balID, int nombrePlacesOccupeesID, int* requetesID, int semID, pid_t* entreesID )
// Algorithme :
//		INITIALISATION
//			Mise à jour des variables globales statiques
//			Attachement aux MPs
//			Masquage des signaux SIGUSR2 puis SIGCHLD
//		MOTEUR
//			Dans une boucle infinie
//				Attendre qu'un message de type sortie arrive dans la boite aux lettres <balID>
//				Lancer une tache fille qui va faire sortir la voiture associée au message reçu
//				Ajouter le PID de la tâche ainsi créer à la liste des fils encore vivants
{
	// INITIALISATION
	// Mise à jour des variables globales
	parkID = parkingID;
	nbPlacesID = nombrePlacesOccupeesID;
	semaphoreID = semID;
	reqsID = requetesID;
	entreesPID = entreesID;
	
	// Attachement aux mps
	requeteEGB = (requeteEntree*) shmat( requetesID[REQ_GB], NULL, SHM_R );
	requeteEBP_profs = (requeteEntree*) shmat( requetesID[REQ_BP_PROFS], NULL, SHM_R );
	requeteEBP_autres = (requeteEntree*) shmat( requetesID[REQ_BP_AUTRES], NULL, SHM_R );
	parking = (placeParking*) shmat( parkingID, NULL, SHM_R|SHM_W );
	nbPlaces = (int*) shmat( nombrePlacesOccupeesID, NULL, SHM_R|SHM_W );
	
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
	
	// MOTEUR
	for( ;; )
	{
		// Attendre devant la boite aux lettres
		struct voiture message;
		while( msgrcv( balID, (void*) &message, sizeof(struct voiture)-sizeof(long), MSG_TYPE_SORTIE, 0 ) == -1 && errno == EINTR );
		
		// Lancer la tache qui va faire sortir la voiture		
		pid_t voiturier = SortirVoiture( message.numPlace );
		if( voiturier != -1 )
		{
			listeFils.push_back(voiturier);
		}
	}
	
	// DESTRUCTION
	// Via handler de SIGUSR2
	
} //----- fin de Sortie





























