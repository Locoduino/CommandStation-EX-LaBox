#ifndef version_labox_h
#define version_labox_h

#define VERSION_LABOX_CAN "0.6.3"
// 0.4    - 08/12/23
// 0.5    - 09/12/23 : Ajout du retour d'information au programme de test. C'est la mesure de courant qui a ete choisie
//                     pour cela lignes 83 a 97 du programme.
// 0.5.1  - 09/12/23 : Fix oubli break  case 0xFE:
//                     TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::ON : POWERMODE::OFF);
//                     break;
// 0.5.2  - 09/12/23
// 0.5.3  - 10/12/23
// 0.5.4  - 10/12/23 : Add POWERMODE::OVERLOAD
// 0.5.5  - 11/12/23 : Correction inversion :
//                     case 0xFE:
//                     TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::OFF : POWERMODE::ON);  
// 0.6.0  - 11/12/23 : Adoption d'un nouveau format de messages totalement incompatible avec les anciens
// 0.6.1  - 11/12/23 : Ajout du retour d'informations
//                     Ajout de commandes dont la POM  case 0xF7:
//                       WRITE CV on MAIN <w CAB CV VALUE>
// 0.6.2  - 12/12/23 : Ajout d'une méthode pour acquisition de hash (supprimé ensuite)
//                     La declaration des broches du CAN et de la vitesse ont ete deplcees lignes 184 à 186 de config.h
// 0.6.3  - 21/02/24 : Modification des identifiants de messages CAN

#endif
