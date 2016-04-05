/*************************************************************************
                           <Clavier>  -  description
                             -------------------
    début                : 2016
    copyright            : 2015-2016 (C) par Mehdi Baha, Ruben Pericas-Moya
    e-mail               : el-mehdi.baha@insa-lyon.fr, ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Interface de la tâche <Clavier> (fichier Clavier.h) -------

#if !defined ( CLAVIER_H )
#define CLAVIER_H

//------------------------------------------------------------------------
// Rôle de la tâche <Clavier>
// Permet la gestion des commandes tapées sur le clavier
//
//------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées

//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void Clavier( int balID );

void Commande(char code, unsigned int valeur);

#endif // XXX_H
