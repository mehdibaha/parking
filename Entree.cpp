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

//------------------------------------------------------ Include personnel
#include "Entree.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types
typedef std::list<pid_t> ListeFils;
typedef std::list::iterator ListeFilsIterator;
typedef std::list::const_iterator ConstListeFilsIterator;

//---------------------------------------------------- Variables statiques
static int semaphoreID;
static int compteurId;
static int parkId;

static int* nbPlacesOccupees;

static pid_t* entreesPID;

static struct placeParking* parking;

struct requeteEntree* requeteEGB;
struct requeteEntree* requeteEBP_profs;
struct requeteEntree* requeteEGB_autres;

static std::list listeFils;

//------------------------------------------------------ Fonctions privées
static void fin ( int noSignal )
// Mode d'emploi :
//
{
    sigaction( SIGCHLD, NULL, NULL );
    shmdt( nbPlacesOccupees );
    shmdt( parking );

    ListeFilsIterator itr;
    for ( itr = listeFils.begin( ); itr != listeFils.end(); itr++ )
    {
        kill( SIGUSR2, *itr, NULL );
        waitpid( *itr, NULL, 0 );
    }

    exit(0);
} //----- fin de fin

static void mortFils ( int noSignal )
// Mode d'emploi :
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

        // Mise à jour de l'affichage de l'entrée
        semop( semaphoreID, &semOp, 1 );
        // TODO Afficher entrée ?
        // NB : on dispose d'une ressource et on en demande une autre via AfficherSortie,
        //		mais cela ne devrait pas mener à un interblocage.
        semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );

        // Mise à jour des places de parking
        semOp.sem_op = -1;
        semop( semaphoreID, &semOp, 1 );
        // TODO Assigner voiture à parking[numPlace] ?
        semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );

        // Mise à jour du nombre de voitures
        bool envoyerSignal = false;
        pid_t entreeADebloquer;

        semOp.sem_op = -1;
        semOp.semNum = SEM_COMPTEUR;
        semop( semaphoreID, &semOp, 1 );
        if(nbPlacesOccupees < NB_PLACES_PARKING)
        {
            // TODO L'incrementaton se fait toute seule ?
            envoyerSignal = true;
        }
        semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );
    }

} //----- fin de fin

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Entree( int parkingID, int compteurVoituresID, int nombrePlacesOccupeesID,
                int* requeteID, int semID, int numSemRequete, long type )
{
    semaphoreID = semID;
    parkId = parkingId;

    // Attachement aux mps
    parking = (placeParking*) shmat( parkingID, NULL, NULL );
    nbPlacesOccupees = (int*) shmat( nombrePlacesOccupeesID, NULL, NULL );

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



    Voiture voiture;

} //----- fin de Entree

