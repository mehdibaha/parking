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
#include <signal.h>
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
static struct requeteEntree* requeteEBP_autres;

// Debug
static ofstream log;

//------------------------------------------------------ Fonctions privées
static void fin ( int noSignal )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
{
	//log << "On a recu le signal de fin" << endl;
	//log.close();
	
	sigaction( SIGCHLD, NULL, NULL );
	shmdt( nbPlaces );
	shmdt( parking );
	shmdt( requeteEBP_autres );
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
	//log << "Un fils est mort" << endl;
	// Prises des informations liées à la mort du fils
	int statut;
	pid_t pidFils = waitpid( -1, &statut, WNOHANG );
	//log << "Il avait le PID : " << pidFils << endl;
	int numPlace = WEXITSTATUS(statut);
	//log << "Il a sortie une voiture de la place " << numPlace << endl;
	time_t heureDepart = time( NULL );
	
	// On recherche le fils mort de la liste des fils
	ListeFilsIterator itr = listeFils.begin( );
	while ( itr != listeFils.end( ) && *itr != pidFils )
	{
		itr++;
	}
	
	//log << "On a trouvé dans la liste des fils le PID : " << *itr << endl;
	
	// Si ce fils existait bel et bien, on le supprime et on fait les traitements associés
	if( itr != listeFils.end( ) )
	{
		//log << "Il existait" << endl;
		listeFils.erase( itr );
		
		// Init sembuf
		struct sembuf semOp;
		semOp.sem_num = SEM_PARKING;
		semOp.sem_op = -1;
		semOp.sem_flg = NULL;
		
		//log << "Sembuf init... Demande affichage sortie (via valeurs de parking)" << endl;
		//log << "Le semaphore vaut : " << semctl(semaphoreID, SEM_PARKING, GETVAL, NULL) << endl;
		Effacer(ConvertPlaceToZone(numPlace));
		
		// Mise à jour de l'affichage de la sortie
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			//log << "Autorisation donnée par le sémaphore" << endl;
			AfficherSortie( parking[numPlace-1].usager, parking[numPlace-1].numVoiture, parking[numPlace-1].heureArrive, heureDepart );
			// NB : on dispose d'une ressource et on en demande une autre via AfficherSortie,
			//		mais cela ne devrait pas mener à un interblocage.
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		//log << "Done. Maj affichage parking demandée" << endl;

        // Mise à jour de l'affichage du parking
        //Afficher(ConvertZone(numPlace), "LEAVING");
		
		//log << "Done. Maj parking demandée" << endl;
		
		// Mise à jour des places de parking
		semOp.sem_op = -1;
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			//log << "Autorisation donnée par le sémaphore" << endl;
			parking[numPlace-1].usager = AUCUN;
			parking[numPlace-1].numVoiture = 0;
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		//log << "Done. Maj nombre place demandée" << endl;
		
		// Variables pour envoi potentiel de signal
		bool envoyerSignal = false;
		
		// Mise à jour du nombre de voitures
		semOp.sem_op = -1;
		semOp.sem_num = SEM_COMPTEUR;
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			//log << "Autorisation donnée par le sémaphore" << endl;
			if(--(*nbPlaces) == NB_PLACES_PARKING-1)
			{
				envoyerSignal = true;
			}
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
			
		//log << "Done. Doit-on envoyer un signal ? ";
		
		// Envoie d'un signal pour débloquer les entrées si nécessaire
		if( envoyerSignal )
		{
			//log << "Oui" << endl;
			time_t heure = 0;
			time_t meilleureHeure = 0;
			enum TypeUsager usager = AUCUN;
			enum TypeUsager bestUsager = AUCUN;
			pid_t entreeADebloquer = 0;
			
			//log << "Verif requete GB... (" << entreesPID[NUM_PID_ENTREE_GB] << ")" << endl;
			
			semOp.sem_op = -1;
			semOp.sem_num = SEM_REQUETE_GB;
			while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
				bestUsager = requeteEGB->usager;
				meilleureHeure = requeteEGB->heureArrive;
				semOp.sem_op = 1;
			semop( semaphoreID, &semOp, 1 );
			
			//log << "Done. Verif requete BP profs... (" << entreesPID[NUM_PID_ENTREE_BP_PROFS] << ")" << endl;
			
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
				//log << "On choisit BPProfs car prof prio" << endl;
			}
			else if( bestUsager == PROF && usager == PROF )
			{
				if ( heure < meilleureHeure )
				{
					meilleureHeure = heure;
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_PROFS];
					//log << "On choisit BPProfs car prof avant l'autre" << endl;
				}
				else
				{
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_GB];
					//log << "On garde GB car prof avant l'autre" << endl;
				}
			}
			else if( bestUsager == AUCUN && usager == AUCUN)
			{
				// Ne rien faire
				//log << "On ne choisit personne" << endl;
			}
			else
			{
				entreeADebloquer = entreesPID[NUM_PID_ENTREE_GB];
				//log << "On choisit GB" << endl;
			}
			
			//log << "entreeADebloquer vaut " << entreeADebloquer << endl;
			//log << "Done. Verif requete BP autres... (" << entreesPID[NUM_PID_ENTREE_BP_AUTRES] << ")" << endl;
			
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
				//log << "On choisit le precedent" << endl;
			}
			else if( bestUsager == AUTRE && usager == AUTRE )
			{
				if ( heure < meilleureHeure )
				{
					meilleureHeure = heure;
					entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_AUTRES];
					//log << "On choisit BPAutres car + en avance" << endl;
				}
				else
				{
					// On garde le précédent
					//log << "On choisit le precedent car + en avance" << endl;
				}
			}
			else 
			{
				entreeADebloquer = entreesPID[NUM_PID_ENTREE_BP_AUTRES];
				//log << "On choisit le precedent car + en avance" << endl;
			}
			
			//log << "Done." << endl;
			//log << "Il faut débloquer l'entrée " << entreeADebloquer << endl;
			
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
//
{
	//log.open("sortie.//log");
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
	requeteEBP_autres = (requeteEntree*) shmat( requetesID[REQ_BP_AUTRES], NULL, NULL );
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
	
	//log << "INIT reussie" << endl;
	// MOTEUR
	for( ;; )
	{
		//log << "Attente qu'une voiture sorte (INFINITE LOOP)..." << endl;
		// Attendre devant la boite aux lettres
		struct voiture message;
		while( msgrcv( balID, (void*) &message, sizeof(struct voiture)-sizeof(long), MSG_TYPE_SORTIE, NULL ) == -1 && errno == EINTR );
		
		//log << "Un voiture veut sortir ! Elle était à la place : " << message.numPlace << endl;
		// Lancer la tache qui va faire sortir la voiture
		unsigned int i;
		unsigned int numPlace = 0;
		
		pid_t voiturier = SortirVoiture( message.numPlace );
		if( voiturier != -1 )
		{
			listeFils.push_back(voiturier);
		}
	}
	
	// DESTRUCTION
	// Via handler de SIGUSR2
	
} //----- fin de Nom





























