/**
* Programme qui permet de gérer des groupes de musiciens et des musiciens grâce
* à l'allocation dynamique et aux classes.
* \file   td4.cpp
* \author Anthony Gingras et Vincent Tran
* \date   21 octobre 2026
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
#include <functional>

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

ListeGroupes::ListeGroupes() :
	capacite_(0),
	nElements_(0),
	elements_(nullptr)
{
}


ListeGroupes::ListeGroupes(int capaciteInitiale) :
	capacite_(capaciteInitiale),
	nElements_(0),
	elements_(capaciteInitiale > 0 ? new Groupe * [capaciteInitiale] : nullptr)
{
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

Groupe* ListeGroupes::trouverGroupe(const std::function<bool(const Groupe*)>& critere) const
{
	for (Groupe* groupe : getGroupes()) {
		if (critere(groupe))
			return groupe;
	}
	return nullptr;
}


span<Groupe* const> ListeGroupes::getGroupes() const
{
	return span<Groupe* const>(elements_, nElements_);
}

Groupe::Groupe(const Groupe* autre) :
	nom(autre->nom),
	genre(autre->genre),
	anneeFormation(autre->anneeFormation),
	membres(autre->membres.getTaille())
{
	for (auto&& membre : autre->membres.getElements())
		membres.ajouter(membre);
}

Groupe::Groupe(std::string nouveauNom, std::string nouveauGenre, int NouvelleAnneeFormation, int nmembres) :
	nom(nouveauNom),
	genre(nouveauGenre),
	anneeFormation(NouvelleAnneeFormation),
	membres(nmembres) 
{
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
void ListeGroupes::retirer(const Groupe* groupe)
{
	for (int i : range(nElements_)) {
		if (elements_[i] == groupe) {
			for (int j : range(i, nElements_ - 1))
				elements_[j] = elements_[j + 1];
			nElements_--;
			break;
		}

	}
}
//TODO:fait Une fonction pour trouver un Musicien par son nom dans une ListeGroupes, qui retourne un pointeur vers le musicien, ou nullptr si le musicien n'est pas trouvé.  Devrait utiliser span.
shared_ptr<Musicien> ListeGroupes::trouverMusicien(const string& nom) const
{
	for (const Groupe* groupe : getGroupes()) {
		for (const auto& musicien : groupe->membres.getElements()) {
			if (musicien->nom == nom) 
				return musicien;
		}
	}
	return nullptr;
}


//TODO:fait Compléter les fonctions pour lire le fichier et créer/allouer une ListeGroupes.  La ListeGroupes devra être passée entre les fonctions, pour vérifier l'existence d'un Musicien avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
shared_ptr<Musicien> lireMusicien(istream& fichier, const ListeGroupes& liste)
{
	Musicien musicien = {};
	musicien.nom = lireString(fichier);
	musicien.pays = lireString(fichier);
	musicien.anneeNaissance = lireUintTailleVariable(fichier);
	shared_ptr<Musicien> musicienExistant = liste.trouverMusicien(musicien.nom);

	if (musicienExistant != nullptr) 
		return musicienExistant;

	cout << "Le musicien " << musicien.nom << " a été créé." << endl;
	return shared_ptr<Musicien>(new Musicien(musicien)); 
}

Groupe* lireGroupe(istream& fichier, const ListeGroupes& liste)
{
	string nom = lireString(fichier);
	string genre = lireString(fichier);
	int anneeFormation = lireUintTailleVariable(fichier);
	int nMembres = lireUintTailleVariable(fichier);

	Groupe* groupe = new Groupe{ nom, genre, anneeFormation, nMembres };

	cout << groupe->nom << endl;

	for (int i : range(nMembres)) {
		shared_ptr<Musicien> musicien = lireMusicien(fichier, liste);
		groupe->membres.ajouter(musicien);
	}

	return groupe; 
}

ListeGroupes creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = lireUintTailleVariable(fichier);

	ListeGroupes listeGroupes(nElements);

	for (int i : range(nElements)) {
		listeGroupes.ajouter(lireGroupe(fichier, listeGroupes));
	}
	return listeGroupes; 
}


//TODO: Une fonction pour détruire un groupe (relâcher toute la mémoire associée à ce groupe, et les membres qui ne jouent plus dans aucun groupes de la collection).  Noter qu'il faut enleve le groupe détruit des groupes dans lesquels jouent les membres.  Pour fins de débogage, affichez les noms des membres lors de leur destruction.
void detruireGroupe(Groupe* groupe)
{
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

void afficherMusicien(ostream& o, const Musicien& musicien)
{
	o << "  " << musicien.nom << ", " << musicien.pays << ", " << musicien.anneeNaissance << endl;
}
//TODO: Une fonction pour afficher un groupe avec tous ses membres (en utilisant la fonction afficherMusicien ci-dessus).


ostream& operator<< (ostream& o, const Groupe& groupe)
{
	o << groupe.nom << " (" << groupe.genre << ", " << groupe.anneeFormation << ")" << endl;

	for (const auto& musicien : groupe.membres.getElements()) {
		afficherMusicien(o, *musicien);
	}
	return o;
}


void afficherListeGroupes(const ListeGroupes& listeGroupes)
{
	static const string ligneDeSeparation = "\n───────────────────────────\n";
	cout << ligneDeSeparation;
	span<Groupe* const> groupes = listeGroupes.getGroupes();
	for (const Groupe* groupe : groupes) {
		cout << (*groupe);
		cout << ligneDeSeparation;
	}
}

//void afficherGroupesMusicien(const ListeGroupes& listeGroupes, const string& nomMusicien)
//{
	//TODO: Utiliser votre fonction pour trouver le musicien (au lieu de le mettre à nullptr).
	//shared_ptr<Musicien> musicien = listeGroupes.trouverMusicien(nomMusicien);
	//if (musicien == nullptr)
		//cout << "Aucun musicien de ce nom" << endl;
//	else
	//	afficherListeGroupes(musicien->joueDans);
//}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.


	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";
	static const int anneeFormationRadiohead = 1985;
	static const int anneeNaissanceDaveGrohl = 1969;

	ListeGroupes listeGroupes = creerListe("groupes.bin");

	cout << ligneDeSeparation << "Le premier groupe de la liste est:" << endl;
	cout << (*listeGroupes.getGroupes()[0]);

	cout << ligneDeSeparation << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes);
	
	
	//cout << ligneDeSeparation << "Liste des groupes où Dave Grohl joue sont:" << endl;
	//afficherGroupesMusicien(listeGroupes, "Dave Grohl");
	Groupe* resultat = listeGroupes.trouverGroupe([](const Groupe* g) {return g->anneeFormation == anneeFormationRadiohead;});
	cout << (*resultat);

	Groupe* nirvahead = new Groupe(listeGroupes.trouverGroupe([](const Groupe* g) { return g->nom == "Nirvana"; }));
	listeGroupes.ajouter(nirvahead);
	nirvahead->nom = "Nirvahead";
	shared_ptr<Musicien> colinGreenwood = listeGroupes.trouverMusicien("Colin Greenwood");
	nirvahead->membres[1] = colinGreenwood;

	shared_ptr<Musicien> daveGrohl = listeGroupes.trouverMusicien("Dave Grohl");
	daveGrohl->anneeNaissance = anneeNaissanceDaveGrohl;

	cout << ligneDeSeparation << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);

	Groupe* nirvana = listeGroupes.getGroupes()[0];
	listeGroupes.retirer(nirvana);
	detruireGroupe(nirvana);
	
	Groupe groupeInexistant ;
	listeGroupes.retirer(&groupeInexistant);

	static const int nEntiers = 2;
	static const int premierEntier = 10;
	static const int deuxiemeEntier = 20;
	static const int entierRemplace = 30;
	static const int entierModifie = 40;

	Liste<int> listeInts(nEntiers);
	listeInts.ajouter(make_shared<int>(premierEntier));
	listeInts.ajouter(make_shared<int>(deuxiemeEntier));

	Liste listeInts2 = listeInts;

	listeInts[0] = make_shared<int>(entierRemplace);
	*listeInts[1] = entierModifie;

	for (const Liste<int>* liste : { &listeInts, &listeInts2 }) {
		for (const shared_ptr<int>& entier : liste->getElements())
			cout << *entier << endl;
	}
	detruireListeGroupes(listeGroupes);
}