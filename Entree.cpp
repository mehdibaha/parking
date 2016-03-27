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
typedef struct voiture Voiture;
typedef struct requeteEntree RequeteEntree;

//---------------------------------------------------- Variables statiques
// TODO :	les variables statiques, ce sont des globaux privés.
//			Faut les mettre dans la rubrique prévue.
static int semaphoreID;
static int compteurId;
static int parkID;
static int boiteID;

static int* nbPlacesOccupees;

static struct placeParking* parking;

struct requeteEntree* requeteEGB;
struct requeteEntree* requeteEBP_profs;
struct requeteEntree* requeteEGB_autres;

static map<pid_t,Voiture> voitureMap;

//------------------------------------------------------ Fonctions privées
// TODO :	Lorsque Entree est coincée parcequ'il n'y a plus de places dans le parking,
//			c'est sortie qui la débloque en lui envoyant un signal SIGUSR1.
//			Il faut donc handle ce signal
//			Attention il va y avoir des variables à remettre à jour (par exemple la MP de requete)


static void fin ( int noSignal )
// Mode d'emploi :
//
{
    sigaction( SIGCHLD, NULL, NULL );
    shmdt( nbPlacesOccupees );
    shmdt( parking );
    shmdt( requeteEGB_autres );
    shmdt( requeteEBP_profs );
    shmdt( requeteEGB );

    for ( auto itr = voitureMap.begin( ); itr != listeFils.end(); itr++ )
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
        semOp.semNum = SEM_PARKING;
        semOp.sem_op = -1;
        semOp.sem_flags = NULL;

        // Mise à jour des places de parking
        semOp.sem_op = -1;
        semop( semaphoreID, &semOp, 1 );
        parking[numPlace].heureArrive = heureEntree;
        parking[numPlace].numVoiture = v.numVoiture ;
        parking[numPlace].usager = v.usager ;
        nbPlacesOccupees++;
        semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );


        // Mise à jour de l'affichage de l'entrée
        semOp.sem_op = -1;
        semop( semaphoreID, &semOp, 1 );
        AfficherPlace(v.usager, v.numVoiture, v.heureArrive, heureEntree);
        semOp.sem_op = 1;
        semop( semaphoreID, &semOp, 1 );

        voitureMap.erase( itr );
    }

} //----- fin de mortFils

static void moteur( long type )
// Mode d'emploi :
//
{
    Voiture message;

    while( msgrcv( boiteID, (void*) &message, sizeof(struct message)-sizeof(long), type ) == -1 && errno == EINTR );

    // Lancer la tâche qui va faire rentrer la voiture
    if (nbPlacesOccupees < NB_PLACES_PARKING)
    {
        RequeteEntree requete;
        requete.numVoiture = voitureMap.size(); // CHOIX DE NUMEROTATION
        requete.usager = message.typeUsager;
        requete.heureArrive = time(NULL);

        TypeBarriere typeBarriere;
        if (type == 1)
        {
            typeBarriere = ENTREE_GASTON_BERGER;
        }
        else if (type == 2)
        {
            if (message.typeUsager == PROF)
                typeBarriere = PROF_BLAISE_PASCAL;
            else
                typeBarriere = AUTRE_BLAISE_PASCAL;
        }

        DessinerVoitureBarriere(typeBarriere, message.typeUsager);

        AfficherRequete(typebarriere, message.typeUsager, requete.heureArrive);

        pid_t pidCourant = GarerVoiture(typeBarriere);
        if (pidCourant != -1)
        {
            voitureMap.insert(make_pair(pidCourant, message));
        }
    }
} //----- fin de moteur

static void init( )
// Mode d'emploi :
//
{
    // Attachement aux mps
    parking = (placeParking*) shmat( parkID, NULL, NULL );
    nbPlacesOccupees = (int*) shmat( nombrePlacesOccupeesID, NULL, NULL );
    requeteEGB = (requeteEntree*) shmat( requetesID[REQ_GB], NULL, NULL );
    requeteEBP_profs = (requeteEntree*) shmat( requetesID[REQ_BP_PROFS], NULL, NULL );
    requeteEGB_autres = (requeteEntree*) shmat( requetesID[REQ_BP_AUTRES], NULL, NULL );

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
void Entree( int balID, int parkingID, int compteurVoituresID, int nombrePlacesOccupeesID,
                int* requeteID, int semID, int numSemRequete, long type )
{
    semaphoreID = semID;
    parkID = parkingID;
    boiteID = balID;

    init( );

    moteur( type );

	// TODO :	phase moteur
	//			On est planté devant la boite aux lettres : on attend qu'une voiture arrive
	//			Si c'est le cas, on vérifie le nombre de places dispos
	//			Si il y a de la place, on l'envoit se garer et on garde une trace du PID fils pour arret forcé de l'appli,
	//			(et on pose une requete ? à vérifier, j'ai un doute d'un seul coup)
	//			Sinon, on dépose une requête et on attend SIGUSR1 (dodo)
	//			Ensuite on se replante devant la boite.

} //----- fin de Entree

