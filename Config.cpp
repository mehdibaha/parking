/*************************************************************************
                                    config
	Configuration de l'application : constantes et structures de données
	 Implémentation des fonctions de conversion pour le module graphique
                             -------------------
    début                : 16/03/2016
    copyright            : 2016 (C) par Ruben
    e-mail               : ruben.pericas-moya@insa-lyon.fr
*************************************************************************/

//---------- Réalisation du module <Config> (fichier Config.cpp) -----

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <ctime>

//------------------------------------------------------ Include personnel
#include "Config.h"

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
TypeZone ConvertPlaceToZone(int numPlace)
// Algorithme :
//		Switch basique :
//			Association de <numPlace> à la TypeZone correspondante.
{
    TypeZone typeZone;
    switch(numPlace)
    {
        case 1:
            typeZone = ETAT_P1;
            break;
        case 2:
            typeZone = ETAT_P2;
            break;
        case 3:
            typeZone = ETAT_P3;
            break;
        case 4:
            typeZone = ETAT_P4;
            break;
        case 5:
            typeZone = ETAT_P5;
            break;
        case 6:
            typeZone = ETAT_P6;
            break;
        case 7:
            typeZone = ETAT_P7;
            break;
        case 8:
            typeZone = ETAT_P8;
            break;
        default:
            break;
    }
    return typeZone;
} //----- fin de ConvertPlaceToZone

TypeZone ConvertNumSemToZone(int numSem)
// Algorithme :
//		Switch basique :
//			Association de <numSem> à la TypeZone correspondante.
{
	TypeZone typeZone;
    switch(numSem)
    {
        case SEM_REQUETE_BP_PROFS:
            typeZone = REQUETE_R1;
            break;
        case SEM_REQUETE_BP_AUTRES:
            typeZone = REQUETE_R2;
            break;
		case SEM_REQUETE_GB:
            typeZone = REQUETE_R3;
            break;
        default:
            break;
    }
    return typeZone;
} //----- fin de ConvertNumSemToZone

