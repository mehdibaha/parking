/*************************************************************************
                           <Mere>  -  description
                             -------------------
    début                : XXX
    copyright            : (C) XXX par XXX
    e-mail               : XXX
*************************************************************************/

//---------- Réalisation de la tâche <Mere> (fichier Mere.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <sys/unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <cstdlib>

//------------------------------------------------------ Include personnel
#include "Mere.h"
#include "Clavier.h"
#include "Entree.h"
#include "Sortie.h"
#include "Outils.h"
#include "Heure.h"
#include "Config.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques

//------------------------------------------------------ Fonctions privées
static void cleanFin ( int noSignal )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
{
	
} //----- fin de cleanFin


//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
int main ( int argc, char ** argv )
// Algorithme :
//
{
	// INITIALISATION
	// Masquage signal
	struct sigaction sigintAction;
	sigintAction.sa_handler = cleanFin;
	sigemptyset( &sigintAction.sa_mask );
	sigintAction.sa_flags = 0;
	sigaction( SIGINT, &sigintAction, NULL );
	
	// Boites aux lettres
	int balID = msgget( ftok(argv[0], 'm'), IPC_CREAT | DROITS_ACCES );
		// NB :	une seule file de message.
		//		Le clavier deposera seulement un message de type différent pour le comportement à simuler.
		//		Les tâches d'entrées/sortie n'attendront qu'un seul type de message.
	
	// Mémoire partagée
	int parkingID = shmget( ftok(argv[0], 'p'), NB_PLACES_PARKING*sizeof(struct placeParking), IPC_CREAT | DROITS_ACCES );
	int nombrePlacesOccupeesID = shmget( IPC_PRIVATE, sizeof(unsigned int), IPC_CREAT | DROITS_ACCES );
	int immatriculationID = shmget( IPC_PRIVATE, sizeof(unsigned int), IPC_CREAT | DROITS_ACCES );
	int requetesID[NB_REQUETES];
	for( int i = 0; i<NB_REQUETES; i++ )
	{
		requetesID[i] = shmget( IPC_PRIVATE, sizeof(struct requeteEntree), IPC_CREAT | DROITS_ACCES );
	}
	
	int* nbPlaces = (int*) shmat( nombrePlacesOccupeesID, NULL, SHM_R|SHM_W );
	*nbPlaces = 0;
	shmdt( nbPlaces );
	
	// TODO : init les segments de MP qui ont besoin de l'être
	
	// Sémaphores
	int semID = semget( IPC_PRIVATE, NB_SEGMENTS_A_PROTEGER, IPC_CREAT | DROITS_ACCES );
	union semun args;
	unsigned short val[NB_SEGMENTS_A_PROTEGER];
	for(unsigned int i =0; i< NB_SEGMENTS_A_PROTEGER; i++)
	{
		val[i] = 1;
	}
	args.array = val;
	semctl( semID, NB_SEGMENTS_A_PROTEGER, SETALL, args );
	
	// Mise en place de l'environnement fourni
	InitialiserApplication ( XTERM );
	
	// Creation des taches filles
    pid_t clavier;
    pid_t entreeGB;
    pid_t entreeProfsBP;
    pid_t entreeAutresBP;
    pid_t sortie;
    pid_t temps;
	
	temps = ActiverHeure( );
	
    if( ( entreeGB = fork( ) ) == 0 )
    {
        Entree( balID, parkingID, immatriculationID, nombrePlacesOccupeesID, requetesID[REQ_GB], semID, SEM_REQUETE_GB, MSG_TYPE_ENTREE_GB );
    }
	else if( ( entreeProfsBP = fork( ) ) == 0 )
	{
		Entree( balID, parkingID, immatriculationID, nombrePlacesOccupeesID, requetesID[REQ_BP_PROFS], semID, SEM_REQUETE_BP_PROFS, MSG_TYPE_ENTREE_BP_PROFS );
	}
	else if( ( entreeAutresBP = fork( ) ) == 0 )
	{
		Entree( balID, parkingID, immatriculationID, nombrePlacesOccupeesID, requetesID[REQ_BP_AUTRES], semID, SEM_REQUETE_BP_AUTRES, MSG_TYPE_ENTREE_BP_AUTRES );
	}
	else if( ( sortie = fork( ) ) == 0 )
	{
		pid_t entreesID[NB_ENTREES];
		entreesID[NUM_PID_ENTREE_GB] = entreeGB;
		entreesID[NUM_PID_ENTREE_BP_PROFS] = entreeProfsBP;
		entreesID[NUM_PID_ENTREE_BP_AUTRES] = entreeAutresBP;
		
		Sortie( parkingID, balID, nombrePlacesOccupeesID, requetesID, semID, entreesID );
	}
    else if( ( clavier = fork( ) ) == 0 )
	{
		Clavier( balID );
	}
	else
    {
		// MOTEUR : attendre que l'utilisateur décide de fermer l'application
        waitpid( clavier, NULL, 0 );
		
		// DESTRUCTION
		// TODO :	faire une fonction de destruction
		kill( sortie, SIGUSR2 );
		waitpid( sortie, NULL, 0 );
		kill( entreeAutresBP, SIGUSR2 );
		waitpid( entreeAutresBP, NULL, 0 );
		kill( entreeProfsBP, SIGUSR2 );
		waitpid( entreeProfsBP, NULL, 0 );
		kill( entreeGB, SIGUSR2 );
		waitpid( entreeGB, NULL, 0 );
		kill( temps, SIGUSR2 );
		waitpid( temps, NULL, 0 );
		
		msgctl( balID, IPC_RMID, NULL );
		shmctl( parkingID, IPC_RMID, NULL );
		shmctl( nombrePlacesOccupeesID, IPC_RMID, NULL );
		shmctl( immatriculationID, IPC_RMID, NULL );
		for( int i = 0; i<NB_REQUETES; i++ )
		{
			shmctl( requetesID[i], IPC_RMID, NULL );
		}
		semctl( semID, IPC_RMID, NULL );
		
		TerminerApplication( );
		
		exit( 0 );
    }
	
	return 0;
} //----- fin de Main

