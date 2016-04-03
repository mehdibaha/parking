/*************************************************************************
                                      Mere
				La tâche principale de la simulation de parking
                             -------------------
    début                : 16/03/2016
    copyright            : (C) par Ruben
    e-mail               : ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Interface de la tâche <Mere> (fichier Mere.h) -------

#if !defined ( MERE_H )
#define MERE_H

//------------------------------------------------------------------------
// Rôle de la tâche <Mere>
// La tâche sortie est la tâche principale de notre application. C'est
// elle qui va gérer la simulation globale du fonctionnement du parking.
// Elle va créer toutes les ressources nécessaires au bon fonctionnement
// de l'application (mémoires partagées, sémaphores, boites aux lettres),
// et lancer les autres tâches.
// Elle attendra ensuite que quelqu'un veuille mettre fin à l'application
//(par le biais d'une tâche fille) puis s'assurera que toutes les autres
// tâches se terminent.
// Elle finira par nettoyer les IPCs puis mettra un terme à l'application
// en se terminant, laissant ainsi le sytsème tel qu'il était à l'origine.
//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
int main ( int argc, char ** argv );
// Mode d'emploi :
//		Fonction principale, point d'entrée de l'application
//
//		<argc>	: le nombre de paramètres passés au lancement de l'application
//		<argv>	: pointeurs vers un tableau de char*, chaque case contenant
//				  les paramètres passés sous forme de string au sens C
//
//		- Initialise les IPCs nécessaires au bon fonctionnement de l'application
//		- Met en place l'environnement graphique
//		- Lance les autres tâches : les entrées, la sortie, le temps et
//		  la gestion clavier
//		- Attend la fin de l'application, donnée par le suicide de la tâche
//		  de gestion clavier
//		- Met fin proprement aux tâches filles, libère les IPcs et termine
//		  l'environnement graphique, avant de se terminer par le code 0
//
// Contrat :
//		- Aucun des paramètres passés par le lanceur de l'application ne sera
//		  pris en compte : seul le paramètre implicite qu'est le chemin vers
//		  l'exécutable de l'application le sera
//

#endif // MERE_H

