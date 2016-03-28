/*************************************************************************
                           Entree  -  description
                             -------------------
    début                : 
    copyright            : (C)  par 
    e-mail               : 
*************************************************************************/

//---------- Interface de la tâche <Entree> (fichier Entree.h) -------
#if ! defined ( ENTREE_H )
#define ENTREE_H

//------------------------------------------------------------------------
// Rôle de la tâche <Entree>
//
//
//------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées

//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Entree( int balID, int parkingID, int immatriculationID, int nombrePlacesOccupeesID,
				int requeteID, int semID, int numSemRequete, long type );
// TODO :	il va très certainement falloir passer en parametre le type de message à envoyer.
//			On ne peut pas le mettre en brut ici, car toutes les entrees vont partager le meme code.
// Mode d'emploi :
//
// Contrat :
//

#endif // ENTREE_H

