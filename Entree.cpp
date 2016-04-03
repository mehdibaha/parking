/*************************************************************************
                           Entree  -  description
                             -------------------
    début                :
    copyright            : (C) par
    e-mail               :
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
static int immatID;
static int nbPlacesID;
static int reqID;
static int semaphoreID;
static int numSemReq;

static struct placeParking* parking;
static int* immatriculation;
static int* nbPlaces;
struct requeteEntree* req;

static map<pid_t,Voiture> voitureMap;

static int signalRecu = 0;
static Voiture msg;
static long typeBar;

// Debug
static ofstream log;

//------------------------------------------------------ Fonctions privées
static void garer(Voiture& message)
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
	// Bah oui, sinon, on la redessine
		
	//log << "Voiture dessinée" << endl;
	// TODO : heu, lecture d'une MP ligne en dessous... mutex ?
	if (*nbPlaces < NB_PLACES_PARKING)
	{
		//log << "Il y a de la place" << endl;
		
		// Init sembuf
		struct sembuf semOp;
		semOp.sem_op = -1;
		semOp.sem_num = SEM_NB_PLACES_OCCUPEES;
		semOp.sem_flg = NULL;
		// Mise à jour du nombre de places occupées
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			(*nbPlaces)++;
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );

		pid_t pidCourant = GarerVoiture(typeBarriere);
		if (pidCourant != -1)
		{
			voitureMap.insert(make_pair(pidCourant, message));
		}
		
		sleep(1);	// Pour éviter les collisions à l'entrée !
		
		//log << "On a crée une tache pour garer la voiture : " << pidCourant << endl;
	}
	else
	{
		//log << "Il n'y avait pas de place"  << endl;
		msg = message;
		// Init sembuf
		struct sembuf semOp;
		semOp.sem_op = -1;
		semOp.sem_num = numSemReq;
		semOp.sem_flg = NULL;
		// Mise à jour de la requete
		while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			req->numVoiture = msg.numVoiture;
			req->usager = msg.usager;
			req->heureArrive = time(NULL);
			semOp.sem_op = 1;
		semop( semaphoreID, &semOp, 1 );
		
		AfficherRequete(typeBarriere, req->usager, req->heureArrive);
		
		//log << "Requete mise à jour, attente de SIGUSR1..." << endl;
		do
		{
			pause();	// On s'endort jusqu'à ce qu'on recoive un signal
		} while( signalRecu != SIGUSR1 );
		//log << "On est sorti de la pause !" << endl;
		signalRecu = 0;
		// NB :		do-while pour éviter de faire quelque chose si on a recu autre chose que sigusr1
		
	}
}
// TODO :	Lorsque Entree est coincée parcequ'il n'y a plus de places dans le parking,
//			c'est sortie qui la débloque en lui envoyant un signal SIGUSR1.
//			Il faut donc handle ce signal
//			Attention il va y avoir des variables à remettre à jour (par exemple la MP de requete)
static void placeLibre( int noSignal )
// Mode d'emploi :
//
{
	//log << "ON A RECU SIGUR1" << endl;
	// Init sembuf
	struct sembuf semOp;
	semOp.sem_op = -1;
	semOp.sem_num = numSemReq;
	semOp.sem_flg = NULL;
	// Mise à jour de la requete
	while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
		req->usager = AUCUN;
		semOp.sem_op = 1;
	semop( semaphoreID, &semOp, 1 );
	
	// Effacement ecran requete
	Effacer( ConvertNumSemToZone( numSemReq ) );
	
	signalRecu = noSignal;
	garer(msg);
}


static void fin ( int noSignal )
// Mode d'emploi :
//
{
	//log << "On a recu le signal de fin" << endl;
	//log.close();
    sigaction( SIGCHLD, NULL, NULL );
	sigaction( SIGUSR1, NULL, NULL );
	sigaction( SIGUSR2, NULL, NULL );
	
	shmdt( parking );
    shmdt( nbPlaces );
    shmdt( req );
    shmdt( immatriculation );

    for ( auto itr = voitureMap.begin( ); itr != voitureMap.end(); itr++ )
    {
        kill( itr->first, SIGUSR2 );
        waitpid( itr->first, NULL, 0 );
    }

    exit(0);
    // TODO FAIT ?
} //----- fin de fin

static void mortFils ( int noSignal )
// Mode d'emploi :
// TODO : Récupérer immatriculation et tout
// TODO :	la mort d'un fils, ça veut dire quoi ? ca veut dire qu'une voiture s'est effectivement garée.
//			Il faut donc :
//				-> Mettre à jour l'affichage
//				-> Mettre à jour le tableaux des places de parking
//				-> Mettre à jour le nombre de voitures présentes
//				(ordre à vérifier)
//			Pas besoin d'envoyer un signal
// TODO FAIT ?
{
	//log << "Un fils est mort : ";
    // Prises des informations liées à la mort du fils
    int statut;
    pid_t pidFils = waitpid( -1, &statut, WNOHANG );
	//log << pidFils << endl;
    int numPlace = WEXITSTATUS(statut);
    time_t heureEntree = time( NULL );

    // On recherche le fils mort de la liste des fils
    auto itr = voitureMap.find(pidFils);
    // Si ce fils existait bel et bien, on le supprime et on fait les traitements associés
    if( itr != voitureMap.end( ) )
    {
		//log << "Ce fils était connu" << endl;
		//log << "Il a garé la voiture à la place " << numPlace << endl;
        Voiture v = itr->second;
		
		//log << "Cette voiture :" << endl;
		//log << "-> Est arrivée à " << heureEntree << endl;
		//log << "-> A le numéro " << v.numVoiture << endl;
		//log << "-> Appartient à un usager de type " << v.usager << endl;
		
        // Init sembuf
        struct sembuf semOp;
        semOp.sem_num = SEM_PARKING;
        semOp.sem_op = -1;
        semOp.sem_flg = NULL;
		
		//log << "SemBuf init... Demande maj parking" << endl;
		//log << "Le sémaphore a une valeur de " << semctl(semaphoreID, SEM_PARKING, GETVAL, NULL) << endl;
        // Mise à jour des places de parking
        while( semop( semaphoreID, &semOp, 1 ) == -1 && errno == EINTR );
			parking[numPlace-1].heureArrive = heureEntree;
			parking[numPlace-1].numVoiture = v.numVoiture;
			parking[numPlace-1].usager = v.usager;
			semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );

		//log << "Done. Maj affichage entree" << endl;
		// Mise à jour de l'affichage de l'entrée
		AfficherPlace( numPlace, v.usager, v.numVoiture, v.heureArrive );

		//log << "Done. Maj affichage parking" << endl;
		// Mise à jour de l'affichage du parking
		//Afficher(ConvertZone(numPlace), "ENTERING");

        voitureMap.erase( itr );
		
		//log << "Fin du handler se SIGCHLD..." << endl;
		//log << "Au passage, errno vaut : " << errno << endl;
    }

} //----- fin de mortFils

static void moteur( long type )
// Mode d'emploi :
//
{
	//log << "Phase MOTEUR..." << endl;
    Voiture message;
	//log << "Struct voiture cree" << endl;

	for( ;; )
	{
		//log << "In INIFINITE LOOP - waiting for a car to arrive" << endl;
		while( msgrcv( boiteID, (void*) &message, sizeof(struct voiture)-sizeof(long), type, NULL ) == -1 && errno == EINTR );

		//log << "Une voiture est arrivée" << endl;
		//log << "Il y a " << *nbPlaces << " places occupees" << endl;
		
		// Lancer la tâche qui va faire rentrer la voiture
		garer(message);
		
	}
	//log << "On est sorti de la boucle infinie !! :o"  << endl;
} //----- fin de moteur

static void init( )
// Mode d'emploi :
//
{
    // Attachement aux mps
    parking = (placeParking*) shmat( parkID, NULL, NULL );
    nbPlaces = (int*) shmat( nbPlacesID, NULL, NULL );
    req = (requeteEntree*) shmat( reqID, NULL, NULL );
	immatriculation = (int*) shmat( immatID, NULL, NULL );
	
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
} //----- fin de moteur

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Entree( int balID, int parkingID, int immatriculationID, int nombrePlacesOccupeesID,
                int requeteID, int semID, int numSemRequete, long type )
{
	//log.open("entree.//log");
	
	// Init globaux privés
	boiteID = balID;
	parkID = parkingID;
	immatID = immatriculationID;
	nbPlacesID = nombrePlacesOccupeesID;
	reqID = requeteID;
    semaphoreID = semID;
	numSemReq = numSemRequete;
	typeBar = type;
	
	//log << "INIT globaux OK" << endl;
	
	
	
	// Phase d'INITIALISATION
    init( );
	
	//log << "Phase INIT OK" << endl;

	// Phase MOTEUR
    moteur( type );

	// TODO :	phase moteur
	//			On est planté devant la boite aux lettres : on attend qu'une voiture arrive
	//			Si c'est le cas, on vérifie le nombre de places dispos
	//			Si il y a de la place, on l'envoit se garer et on garde une trace du PID fils pour arret forcé de l'appli,
	//			(et on pose une requete ? à vérifier, j'ai un doute d'un seul coup)
	//			Sinon, on dépose une requête et on attend SIGUSR1 (dodo)
	//			Ensuite on se replante devant la boite.

} //----- fin de Entree

