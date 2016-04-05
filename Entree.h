/*************************************************************************
                           Entree  -  description
                             -------------------
    début                : 2016
    copyright            : 2015-2016 (C) par Mehdi Baha, Ruben Pericas-Moya
    e-mail               : el-mehdi.baha@insa-lyon.fr, ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Interface de la tâche <Entree> (fichier Entree.h) -------
#if ! defined ( ENTREE_H )
#define ENTREE_H

//------------------------------------------------------------------------
// Rôle de la tâche <Entree>
// Permet la gestion des entrées des voitures
//
//------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées

//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Entree( int balID, int parkingID, int nombrePlacesOccupeesID,
				int requeteID, int semID, int numSemRequete, long type );

#endif // ENTREE_H

