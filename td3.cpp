/**
* Programme qui permet de gérer des groupes de musiciens et des musiciens grâce
* à l'allocation dynamique et aux classes.
* \file   td2.cpp
* \author Anthony Gingras et Vincent Tran
* \date   26 septembre 2026
* Créé le 15 septembre 2026
*/

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de groupes musicaux en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <span>

#include "cppitertools/range.hpp"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;

#pragma endregion//}

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
	T valeur{};
	fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
	return valeur;
}
#define erreurFataleAssert(message) assert(false&&(message)),terminate()
static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
	uint8_t entete = lireType<uint8_t>(fichier);
	switch (entete) {
	case enteteTailleVariableDeBase + 0: return lireType<uint8_t>(fichier);
	case enteteTailleVariableDeBase + 1: return lireType<uint16_t>(fichier);
		//case enteteTailleVariableDeBase + 2: return lireType<uint32_t>(fichier); aucune variable devrait être aussi grande et donc cette ligne semble inutile.
	default:
		cerr << "À la position de fichier " << streamoff(fichier.tellg()) - 1 << ": "; erreurFataleAssert("Tentative de lire un entier de taille variable alors que le fichier contient autre chose à cet emplacement.");
	}
}

string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUintTailleVariable(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}

ListeGroupes::ListeGroupes() 
{
	capacite_ = 0;
	nElements_ = 0;
	elements_ = nullptr;
}

ListeGroupes::ListeGroupes(int capaciteInitiale)
{
	capacite_ = capaciteInitiale;
	nElements_ = 0;
	if (capaciteInitiale > 0) elements_ = new Groupe * [capacite_];
	else elements_ = nullptr;
}

void ListeGroupes::libererMemoire() 
{
	delete[] elements_;
	elements_ = nullptr;
	capacite_ = 0;
	nElements_ = 0;
}

int ListeGroupes::getTaille() const 
{
	return nElements_;
}

int ListeGroupes::getCapacite() const 
{
	return capacite_;
}

span<Groupe* const> ListeGroupes::getGroupes() const 
{
	return span<Groupe* const>(elements_, nElements_);
}

//TODO:fait Une fonction pour ajouter un Groupe à une ListeGroupes, le groupe existant déjà; on veut uniquement ajouter le pointeur vers le groupe existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Groupe ni Musicien, elle doit copier uniquement des pointeurs.
void ListeGroupes::ajouter(Groupe* pointeurGroupe)
{
	if (nElements_ >= capacite_) {
		int nouvelleCapacite = max(1, capacite_ * 2);

		Groupe** nouveauGroupe = new Groupe * [nouvelleCapacite];
		for (int i : range(nElements_)) {
			nouveauGroupe[i] = elements_[i];
		}
		delete[] elements_;
		elements_ = nouveauGroupe;
		capacite_ = nouvelleCapacite;
	}
	elements_[nElements_] = pointeurGroupe;
	nElements_++;
}
//TODO:fait Une fonction pour enlever un Groupe d'une ListeGroupes (enlever le pointeur) sans effacer le groupe; la fonction prenant en paramètre un pointeur vers le groupe à enlever.  L'ordre des groupes dans la liste n'a pas à être conservé.
void ListeGroupes::retirer(const Groupe* pointeurGroupe)
{
	for (int i : range(nElements_)) {
		if (elements_[i] == pointeurGroupe){
			for (int j : range(i, nElements_ - 1))
			{
				elements_[j] = elements_[j + 1];
			}
			nElements_--;
			break;
		}

	}
}
//TODO:fait Une fonction pour trouver un Musicien par son nom dans une ListeGroupes, qui retourne un pointeur vers le musicien, ou nullptr si le musicien n'est pas trouvé.  Devrait utiliser span.
Musicien* ListeGroupes::trouverMusicien(const string& nom) const
{
	span<Groupe*> groupes(elements_, nElements_);
	for (const Groupe* groupe : groupes) {
		span <Musicien* const> membres(groupe->membres.elements, groupe->membres.nElements);
		for (Musicien* musicien : membres) {
			if (musicien->nom == nom) return musicien;
		}
	}
	return nullptr;
}
//TODO:fait Compléter les fonctions pour lire le fichier et créer/allouer une ListeGroupes.  La ListeGroupes devra être passée entre les fonctions, pour vérifier l'existence d'un Musicien avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
Musicien* lireMusicien(istream& fichier, const ListeGroupes& liste) 
{
	Musicien musicien = {};
	musicien.nom = lireString(fichier);
	musicien.pays = lireString(fichier);
	musicien.anneeNaissance = lireUintTailleVariable(fichier);
	Musicien* musicienExistant = liste.trouverMusicien(musicien.nom);

	if (musicienExistant != nullptr) return musicienExistant;

	Musicien* nouveauMusicien = new Musicien(musicien);
	cout << "Le musicien " << musicien.nom << " à été créé." << endl;
	return nouveauMusicien; //TODO:fait Retourner un pointeur soit vers un musicien existant ou un nouveau musicien ayant les bonnes informations, selon si le musicien existait déjà.  Pour fins de débogage, affichez les noms des membres crées; vous ne devriez pas voir le même nom de musicien affiché deux fois pour la création.
}

Groupe* lireGroupe(istream& fichier, const ListeGroupes& liste)
{
	Groupe groupe = {};
	groupe.nom = lireString(fichier);
	groupe.genre = lireString(fichier);
	groupe.anneeFormation = lireUintTailleVariable(fichier);
	groupe.membres.nElements = 0;  //NOTE: Vous avez le droit d'allouer d'un coup le tableau pour les membres, sans faire de réallocation comme pour ListeGroupes.  Vous pouvez aussi copier-coller les fonctions d'allocation de ListeGroupes ci-dessus dans des nouvelles fonctions et faire un remplacement de Groupe par Musicien, pour réutiliser cette réallocation.
	groupe.membres.capacite = lireUintTailleVariable(fichier);
	groupe.membres.elements = new Musicien * [groupe.membres.capacite];

	cout << groupe.nom << endl;

	for (int i : range(groupe.membres.capacite)) {
		Musicien* musicien = lireMusicien(fichier, liste); //TODO: Placer le musicien au bon endroit dans les membres du groupe.
		groupe.membres.elements[i] = musicien;
		groupe.membres.nElements++;
	}
	Groupe* nouveauGroupe = new Groupe(groupe);
	for (int i : range(nouveauGroupe->membres.nElements)) {
		Musicien* musicien = nouveauGroupe->membres.elements[i];
		musicien->joueDans.ajouter(nouveauGroupe);
	}
	//TODO: Ajouter le groupe à la liste des groupes dans lesquels le musicien joue.

	return nouveauGroupe; //TODO: Retourner le pointeur vers le nouveau groupe.
}

ListeGroupes creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = lireUintTailleVariable(fichier);

	//TODO: Créer une liste de groupes vide.
	ListeGroupes listeGroupes(nElements);

	for (int i : range(nElements)) {
		listeGroupes.ajouter(lireGroupe(fichier, listeGroupes));
	}
	return listeGroupes; //TODO: Retourner la liste de groupes.
}


//TODO: Une fonction pour détruire un groupe (relâcher toute la mémoire associée à ce groupe, et les membres qui ne jouent plus dans aucun groupes de la collection).  Noter qu'il faut enleve le groupe détruit des groupes dans lesquels jouent les membres.  Pour fins de débogage, affichez les noms des membres lors de leur destruction.
void detruireGroupe(Groupe* groupe)
{
	Musicien** listeMusicien = groupe->membres.elements;
	int nMusiciens = groupe->membres.nElements;
	span<Musicien*> tableauMusiciens(listeMusicien, nMusiciens);
	for (Musicien* ptrMusicien : tableauMusiciens){
		ListeGroupes& groupesJoueDans = ptrMusicien->joueDans;
		groupesJoueDans.retirer(groupe);
		if (groupesJoueDans.getTaille() == 0) {
			groupesJoueDans.libererMemoire();
			cout << (*ptrMusicien).nom << " a été supprimé." << endl;
			delete ptrMusicien;

		}
	}
	delete[] listeMusicien;
	delete groupe;
}
//TODO: Une fonction pour détruire une ListeGroupes et tous les groupes qu'elle contient.
void detruireListeGroupes(ListeGroupes& listeGroupes)
{
	for (Groupe* groupe : listeGroupes.getGroupes()) {
		detruireGroupe(groupe);
	}
	listeGroupes.libererMemoire();
}

void afficherMusicien(const Musicien& musicien)
{
	cout << "  " << musicien.nom << ", " << musicien.pays << ", " << musicien.anneeNaissance << endl;
}

//TODO: Une fonction pour afficher un groupe avec tous ses membres (en utilisant la fonction afficherMusicien ci-dessus).

void afficherGroupe(const Groupe& groupe) {
	cout << groupe.nom << " (" << groupe.genre << ", " << groupe.anneeFormation << ")" << endl;
	span<Musicien* const> membres(groupe.membres.elements, groupe.membres.nElements);
	for (const Musicien* musicien : membres) {
		afficherMusicien(*musicien);
	}
}

void afficherListeGroupes(const ListeGroupes& listeGroupes)
{
	//TODO: Utiliser des caractères Unicode pour définir la ligne de séparation (différente des autres lignes de séparations dans ce progamme).
	static const string ligneDeSeparation = "\n───────────────────────────\n";
	cout << ligneDeSeparation;
	//TODO: Changer le for pour utiliser un span.
	span<Groupe* const> groupes = listeGroupes.getGroupes();
	for (const Groupe* groupe : groupes) {
		//TODO: Afficher le groupe.
		afficherGroupe(*groupe);
		cout << ligneDeSeparation;
	}
}

void afficherGroupesMusicien(const ListeGroupes& listeGroupes, const string& nomMusicien)
{
	//TODO: Utiliser votre fonction pour trouver le musicien (au lieu de le mettre à nullptr).
	const Musicien* musicien = listeGroupes.trouverMusicien(nomMusicien);
	if (musicien == nullptr)
		cout << "Aucun musicien de ce nom" << endl;
	else
		afficherListeGroupes(musicien->joueDans);
}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.


	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	//TODO: Chaque TODO dans cette fonction devrait se faire en 1 ou 2 lignes, en appelant les fonctions écrites.

	//TODO: La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.  Devrait afficher les noms des 22 musiciens sans doublons (par l'affichage pour fins de débogage dans votre fonction lireMusicien).
	ListeGroupes listeGroupes = creerListe("groupes.bin");

	cout << ligneDeSeparation << "Le premier groupe de la liste est:" << endl;
	afficherGroupe(*listeGroupes.getGroupes()[0]);
	//TODO: Afficher le premier groupe de la liste.  Devrait être Nirvana.

	cout << ligneDeSeparation << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes);
	//TODO: Afficher la liste des groupes.  Il devrait y en avoir 7.

	Musicien* daveGrohl = listeGroupes.trouverMusicien("Dave Grohl");
	daveGrohl->anneeNaissance = 1969;
	//TODO: Modifier l'année de naissance de Dave Grohl pour être 1969 (a une valeur qui n'a pas de sens dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des groupes et des membres dans les listes, il faut y aller par son nom.

	cout << ligneDeSeparation << "Liste des groupes où Dave Grohl joue sont:" << endl;
	afficherGroupesMusicien(listeGroupes, "Dave Grohl");
	//TODO: Afficher la liste des groupe où Dave Grohl joue.  Il devrait y avoir Nirvana et Foo Fighters.

	Groupe* nirvana = listeGroupes.getGroupes()[0];
	listeGroupes.retirer(nirvana);
	detruireGroupe(nirvana);
	//TODO: Détruire et enlever le premier groupe de la liste (Nirvana).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les membres Kurt Cobain et Krist Novoselic, mais pas Dave Grohl puisqu'il joue aussi dans Foo Fighters.

	cout << ligneDeSeparation << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);
	//TODO: Afficher la liste des groupes.
	afficherGroupesMusicien(listeGroupes, "Elvis Gratton");
	Groupe groupeInexistant = {}; //purement pour faire appel à chaque ligne
	listeGroupes.retirer(&groupeInexistant);

	//TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.
	detruireListeGroupes(listeGroupes);
	//TODO: Détruire tout avant de terminer le programme.  La bibliothèque de verification_allocation devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
}