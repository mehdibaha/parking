/*************************************************************************
                                    Sortie
		    Tâche permettant à une voiture de sortir du parking
                             -------------------
    début                : 16/03/2016
    copyright            : 2016 (C) par Ruben
    e-mail               : ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Interface de la tâche <Sortie> (fichier Sortie.h) -------
#if ! defined ( SORTIE_H )
#define SORTIE_H

//------------------------------------------------------------------------
// Rôle de la tâche <Sortie>
// La tâche sortie est celle qui va gérer la barrière de sortie dans notre
// application.
// Elle répondra à l'événement principal indépendant "une voiture veut
// sortir du parking".
// C'est elle qui va permettre aux voitures de quitter le parking,
// qui va mettre à jour l'affichage des places et qui signalera aux
// entrées qu'une place s'est libérée, ceci afin de gérer la priorité de
// garage aux professeurs puis aux utilisateurs arrivés le plus tôt à une
// porte d'entrée (dans le cas où le parking est plein puis qu'une place
// se libère).
//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Sortie(	int parkingID, int balID, int nbPlacesOccupeesID,
				int* requetesID, int semID, pid_t* entreesPID );
// Mode d'emploi :
//		Tâche permettant de gérer la sortie des voitures du parking
//
//		<parkingID>			: identifiant d'une mémoire partagée contenant
//							  l'état de toutes les places de parking (tableau)
//		<balID>				: identifiant de la boite aux lettres signalant
//							  qu'une voiture souhaite sortir du parking
//		<nbPlacesOccupeesID>: identifiant d'une mémoire partagée contenant
//							  le nombre de places de parking occupées
//		<requetesID>		: pointeur vers la première case d'un tableau
//							  contenant les identifiants des mémoires partagées
//							  utilisées par les entrées pour signaler qu'il n'y
//							  a plus de places disponibles mais qu'une voiture
//							  souhaite entrer dans le parking
//		<semID>				: identifiant du sémaphore général défini permettant
//							  l'accès aux ressources critiques
//		<entreesPID>		: pointeur vers la première case d'un tableau
//							  contenant les PIDs de tous les processus
//							  gérant les entrées du parking
//
//		- Attend indéfiniement qu'une voiture veuille sortir du parking. Lorsque
//		c'est le cas, lance une tâche fille qui fait sortir ladite voiture.
//		- Une fois cette voiture complétement sortie, met à jour le tableau (MP)
//		d'dentifiant <parkingID> ansi que la MP <nbPlacesOccupeesID>, 
//		affiche à l'écran le nouvel état du parking et affiche des informations
//		relative à la sortie (type d'usager, immatriculation, heures d'arrivée et
//		de départ, temps total et prix associé)
//		- Si le parking était plein et qu'une voiture vient de complétement sortir,
//		cette fonction consulte les requêtes des MPs d'identifiants <requetesID>,
//		et envoie si besoin le signal SIGUSR1 qui à l'entrée la plus prioritaire.
//		La priorité est gérée de la manière suivante :
//			* Usagers de type PROF plus prioritaire que ceux de type AUTRE
//			* Pour deux usagers les mêmes, celui arrivé en tout premier est prioritaire
//			* En cas d'égalité de type et de temps, c'est le premier dans l'odrdre du
//			  tableau <requetesID> qui est choisi
//		- Peut réagir au signal SIGUSR2. A la réception de ce signal, la tâche sortie
//		déclenche la fin de toutes ses tâches filles et déclenche sa fin propre.
//
//
// Contrat :
//		- les pointeurs ne doivent pas être nuls
//		- en cas d'arrêt autre que par celui de SIGUSR2, rien n'est garanti concernant
//		  le nettoyage des processus enfants de la tâche Sortie

#endif // SORTIE_H

