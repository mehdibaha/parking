/*************************************************************************
                           <Mere>  -  description
                             -------------------
    début                : XXX
    copyright            : (C) XXX par XXX
    e-mail               : XXX
*************************************************************************/

//---------- Réalisation de la tâche <Mere> (fichier <Mere>.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <sys/wait.h>

//------------------------------------------------------ Include personnel
#include "Mere.h"
#include "Outils.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques

//------------------------------------------------------ Fonctions privées
//static type nom ( liste de paramètres )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
//{
//} //----- fin de nom

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
int main ( int argc, char ** argv )
// Algorithme :
//
{
    enum TypeTerminal xt = XTERM;
    InitialiserApplication ( xt );
    sleep(10);
    TerminerApplication ( );
} //----- fin de Main

