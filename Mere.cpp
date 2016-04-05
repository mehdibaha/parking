/*************************************************************************
                                      Mere
				La tâche principale de la simulation de parking
                             -------------------
    début                : 16/03/2016
    copyright            : (C) par Ruben
    e-mail               : ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <Mere> (fichier Mere.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <sys/unistd.h>		// sleep, fork
#include <sys/wait.h>		// waitpid
#include <sys/msg.h>		// boite aux lettres
#include <sys/shm.h>		// mémoire partagée
#include <sys/sem.h>		// sémaphore
#include <sys/types.h>		// pid_t
#include <sys/ipc.h>		// flags pour IPCs
#include <signal.h>			// signaux, sigaction
#include <cstdlib>			// exit

//------------------------------------------------------ Include personnel
#include "Mere.h"
#include "Clavier.h"
#include "Entree.h"
#include "Sortie.h"
#include "Outils.h"
#include "Heure.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
int main ( int argc, char ** argv )
// Algorithme :
//		INITIALISATION
//			Création boite aux lettres (1 seule, multifiles)
//			Création des mémoires partagées
//			Initialisation du nombre de places occupées à 03/2016
//			Création d'un sémaphore général (1 seul, constitué de plusieurs élémentaires)
//			Initialisation de tous les sémaphores élémentaires à 1 (mutex)
//			Mise en place de l'environnement graphique
//			Lancement de toutes les autres tâches, dans l'ordre suivant :
//				* Heure
//				* Entrées (Gaston Berger, Blaise Pascal profs, Blaise Pascal autres)
//				* Sortie
//				* Clavier
//		MOTEUR
//			Attente de la fin de la tâche Clavier
//		DESTRUCTION
//			Pour chaque tâche parmi celles créées, hormis Clavier, dans l'ordre inverse de création
//				Envoyer SIGUSR2 à la tâche
//				Attendre la fin de la tâche
//			Destruction de tous les IPCs créés dans la phase d'initialisation
//			Auto-destruction avec le code de retour 0
{
	// INITIALISATION	
	// Boites aux lettres
	int balID = msgget( ftok(argv[argc - 1], 'm'), IPC_CREAT | DROITS_ACCES );
		// NB :	une seule file de message.
		//		Le clavier deposera seulement un message de type différent pour le comportement à simuler.
		//		Les tâches d'entrées/sortie n'attendront qu'un seul type de message.
	
	// Mémoire partagée
	int parkingID = shmget( ftok(argv[argc - 1], 'p'), NB_PLACES_PARKING*sizeof(struct placeParking), IPC_CREAT | DROITS_ACCES );
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
		semctl( semID, IPC_RMID, 0 );
		
		TerminerApplication( );
		
		exit( 0 );
    }
	
	return 0;
} //----- fin de Main

