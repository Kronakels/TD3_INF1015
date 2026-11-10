/**
 * Programme qui permet de gérer des groupes de musiciens et des musiciens grâce
 * à l'allocation dynamique et aux classes.
 * \file   td4.cpp
 * \author Anthony Gingras et Vincent Tran
 * \date   4 novembre 2026
 * Créé le 15 septembre 2026
 */

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp" // Structures de données pour la collection de groupes musicaux en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <span>
#include <functional>
#include <iomanip>
#include <typeinfo>
#include <forward_list>
#include "cppitertools/range.hpp"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"		   // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;

#pragma endregion //}

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
	T valeur{};
	fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
	return valeur;
}
#define erreurFataleAssert(message) assert(false && (message)), terminate()
static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
	uint8_t entete = lireType<uint8_t>(fichier);
	switch (entete)
	{
	case enteteTailleVariableDeBase + 0:
		return lireType<uint8_t>(fichier);
	case enteteTailleVariableDeBase + 1:
		return lireType<uint16_t>(fichier);
		// case enteteTailleVariableDeBase + 2: return lireType<uint32_t>(fichier); aucune variable devrait être aussi grande et donc cette ligne semble inutile.
	default:
		cerr << "À la position de fichier " << streamoff(fichier.tellg()) - 1 << ": ";
		erreurFataleAssert("Tentative de lire un entier de taille variable alors que le fichier contient autre chose à cet emplacement.");
	}
}

string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUintTailleVariable(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion //}

ListeGroupes::ListeGroupes() : capacite_(0),
nElements_(0),
elements_(nullptr)
{
}

ListeGroupes::ListeGroupes(int capaciteInitiale) : capacite_(capaciteInitiale),
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
	for (Groupe* groupe : getGroupes())
	{
		if (critere(groupe))
			return groupe;
	}
	return nullptr;
}

span<Groupe* const> ListeGroupes::getGroupes() const
{
	return span<Groupe* const>(elements_, nElements_);
}

Groupe::Groupe(const Groupe* autre) : nom(autre->nom),
genre(autre->genre),
anneeFormation(autre->anneeFormation),
membres(autre->membres.getTaille())
{
	for (auto&& membre : autre->membres.getElements())
		membres.ajouter(membre);
}

Groupe::Groupe(std::string nouveauNom, std::string nouveauGenre, int nouvelleAnneeFormation, int nMembres) : nom(nouveauNom),
genre(nouveauGenre),
anneeFormation(nouvelleAnneeFormation),
membres(nMembres)
{
}
// TODO:fait Une fonction pour ajouter un Groupe à une ListeGroupes, le groupe existant déjà; on veut uniquement ajouter le pointeur vers le groupe existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Groupe ni Musicien, elle doit copier uniquement des pointeurs.
void ListeGroupes::ajouter(Groupe* pointeurGroupe)
{
	if (nElements_ >= capacite_)
	{
		int nouvelleCapacite = max(1, capacite_ * 2);

		Groupe** nouveauGroupe = new Groupe * [nouvelleCapacite];
		for (int i : range(nElements_))
		{
			nouveauGroupe[i] = elements_[i];
		}
		delete[] elements_;
		elements_ = nouveauGroupe;
		capacite_ = nouvelleCapacite;
	}
	elements_[nElements_] = pointeurGroupe;
	nElements_++;
}
// TODO:fait Une fonction pour enlever un Groupe d'une ListeGroupes (enlever le pointeur) sans effacer le groupe; la fonction prenant en paramètre un pointeur vers le groupe à enlever.  L'ordre des groupes dans la liste n'a pas à être conservé.
void ListeGroupes::retirer(const Groupe* groupe)
{
	for (int i : range(nElements_))
	{
		if (elements_[i] == groupe)
		{
			for (int j : range(i, nElements_ - 1))
				elements_[j] = elements_[j + 1];
			nElements_--;
			break;
		}
	}
}
// TODO:fait Une fonction pour trouver un Musicien par son nom dans une ListeGroupes, qui retourne un pointeur vers le musicien, ou nullptr si le musicien n'est pas trouvé.  Devrait utiliser span.
shared_ptr<Musicien> ListeGroupes::trouverMusicien(const string& nom) const
{
	for (const Groupe* groupe : getGroupes())
	{
		for (const auto& musicien : groupe->membres.getElements())
		{
			if (musicien->nom_ == nom)
				return musicien;
		}
	}
	return nullptr;
}


Instrumentiste::Instrumentiste(const Musicien& musicien, std::istream& fichier) : Musicien(musicien)
{
	int nInstruments = 0;
	fichier >> nInstruments;
	for (int i : range(nInstruments))
	{
		string nouvelInstrument = "";
		fichier >> quoted(nouvelInstrument);
		instruments_.push_back(nouvelInstrument);
	}
}


Chanteur::Chanteur(const Musicien& musicien, std::istream& fichier) : Musicien(musicien)
{
	fichier >> quoted(styleVocal_);
}


ChanteurInstrumentiste::ChanteurInstrumentiste(const Musicien& musicien, std::istream& fichier) :
	Musicien(musicien),
	Chanteur(musicien, fichier),
	Instrumentiste(musicien, fichier)
{
}


std::ostream& operator<<(std::ostream& os, const Affichable& obj)
{
	return obj.afficher(os);
}


std::ostream& Musicien::afficher(std::ostream& os) const
{
	os << "  " << nom_ << ", " << pays_ << ", " << anneeNaissance_ << endl;
	return os;
}


std::ostream& Instrumentiste::afficherInterne(std::ostream& os) const
{
	os << "  Instruments :";
	for (auto&& instrument : instruments_)
	{
		os << " " << instrument;
	}
	os << endl;
	return os;
}


std::ostream& Instrumentiste::afficher(std::ostream& os) const
{
	Musicien::afficher(os);
	afficherInterne(os);
	return os;
}


std::ostream& Chanteur::afficherInterne(std::ostream& os) const
{
	os << "  Style vocal : " << styleVocal_ << endl;
	return os;
}


std::ostream& Chanteur::afficher(std::ostream& os) const
{
	Musicien::afficher(os);
	afficherInterne(os);
	return os;
}


std::ostream& ChanteurInstrumentiste::afficher(std::ostream& os) const
{
	Musicien::afficher(os);
	Chanteur::afficherInterne(os);
	Instrumentiste::afficherInterne(os);
	return os;
}


Musicien::~Musicien()
{
	cout << "Destruction du musicien : " << nom_ << endl;
}


Chanteur::~Chanteur()
{
	cout << "Destruction du chanteur." << endl;
}


Instrumentiste::~Instrumentiste()
{
	cout << "Destruction de l'instrumentiste." << endl;
}


ChanteurInstrumentiste::~ChanteurInstrumentiste()
{
	cout << "Destruction du chanteur-instrumentiste." << endl;
}


shared_ptr<Musicien> lireMusicien(istream& fichier, istream& fichierTypesMusiciens, const ListeGroupes& liste)
{
	Musicien musicien = {};
	musicien.nom_ = lireString(fichier);
	musicien.pays_ = lireString(fichier);
	musicien.anneeNaissance_ = static_cast<int>(lireUintTailleVariable(fichier));
	shared_ptr<Musicien> musicienExistant = liste.trouverMusicien(musicien.nom_);

	if (musicienExistant != nullptr)
		return musicienExistant;

	cout << "Le musicien " << musicien.nom_ << " a été créé." << endl;


	string typeMusicien = "";
	string musicienNom = "";
	fichierTypesMusiciens >> quoted(musicienNom) >> typeMusicien;
	if (musicienNom == musicien.nom_)
	{
		if (typeMusicien == "chant")
		{
			musicien.vectorTousMusiciens_.push_back(make_shared<Chanteur>(musicien, fichierTypesMusiciens));
			return make_shared<Chanteur>(musicien, fichierTypesMusiciens);
		}
		else if (typeMusicien == "instrument")
		{
			musicien.vectorTousMusiciens_.push_back(make_shared<Instrumentiste>(musicien, fichierTypesMusiciens));
			return make_shared<Instrumentiste>(musicien, fichierTypesMusiciens);
		}
		else if (typeMusicien == "chant+instrument")
		{
			musicien.vectorTousMusiciens_.push_back(make_shared<ChanteurInstrumentiste>(musicien, fichierTypesMusiciens));
			return make_shared<ChanteurInstrumentiste>(musicien, fichierTypesMusiciens);
		}
	}
	erreurFataleAssert("Type de musicien inconnu ou nom inattendu dans le fichier de types.");

	musicien.afficherVectorTousMusiciens(musicien.vectorTousMusiciens_);
}

template <typename U>
void Musicien::afficherVectorTousMusiciens(U vectorTousMusiciens)
{
	for (auto musicien: vectorTousMusiciens) {
		cout << musicien;
	}
}
Groupe* lireGroupe(istream& fichier, istream& fichierTypesMusiciens, const ListeGroupes& liste)
{
	string nom = lireString(fichier);
	string genre = lireString(fichier);
	int anneeFormation = static_cast<int>(lireUintTailleVariable(fichier));
	int nMembres = static_cast<int>(lireUintTailleVariable(fichier));

	Groupe* groupe = new Groupe{ nom, genre, anneeFormation, nMembres };

	cout << groupe->nom << endl;

	for (int i : range(nMembres))
	{
		shared_ptr<Musicien> musicien = lireMusicien(fichier, fichierTypesMusiciens, liste);
		groupe->membres.ajouter(musicien);
	}

	return groupe;
}

ListeGroupes creerListe(string nomFichier, istream& fichierTypesMusiciens)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = static_cast<int>(lireUintTailleVariable(fichier));

	ListeGroupes listeGroupes(nElements);

	for (int i : range(nElements))
	{
		listeGroupes.ajouter(lireGroupe(fichier, fichierTypesMusiciens, listeGroupes));
	}
	return listeGroupes;
}

// TODO: Une fonction pour détruire un groupe (relâcher toute la mémoire associée à ce groupe, et les membres qui ne jouent plus dans aucun groupes de la collection).  Noter qu'il faut enleve le groupe détruit des groupes dans lesquels jouent les membres.  Pour fins de débogage, affichez les noms des membres lors de leur destruction.
void detruireGroupe(Groupe* groupe)
{
	delete groupe;
}
// TODO: Une fonction pour détruire une ListeGroupes et tous les groupes qu'elle contient.
void detruireListeGroupes(ListeGroupes& listeGroupes)
{
	for (Groupe* groupe : listeGroupes.getGroupes())
	{
		detruireGroupe(groupe);
	}
	listeGroupes.libererMemoire();
}

// TODO: Une fonction pour afficher un groupe avec tous ses membres (en utilisant la fonction afficherMusicien ci-dessus).

ostream& operator<<(ostream& o, const Groupe& groupe)
{
	o << groupe.nom << " (" << groupe.genre << ", " << groupe.anneeFormation << ")" << endl;

	for (const auto& musicien : groupe.membres.getElements())
	{
		o << *musicien;
	}
	return o;
}

void afficherListeGroupes(const ListeGroupes& listeGroupes)
{
	static const string ligneDeSeparation = "\n───────────────────────────\n";
	cout << ligneDeSeparation;
	span<Groupe* const> groupes = listeGroupes.getGroupes();
	for (const Groupe* groupe : groupes)
	{
		cout << (*groupe);
		cout << ligneDeSeparation;
	}
}

void verifierDestruction(const ListeGroupes& listeGroupes)
{
	cout << "\n--- Test de destruction ---" << endl;
	{
		Groupe* premierGroupe = listeGroupes.getGroupes()[0];
		shared_ptr<Musicien> premierMusicien = premierGroupe->membres[0];

		unique_ptr<Musicien> copieMusicien;
		auto* chanteurInstrumentiste = dynamic_cast<ChanteurInstrumentiste*>(premierMusicien.get());

		if (chanteurInstrumentiste != nullptr)
		{
			copieMusicien = make_unique<ChanteurInstrumentiste>(*chanteurInstrumentiste);
		}
		else
		{
			copieMusicien = make_unique<Musicien>(*premierMusicien);
		}
	}
	cout << "Fin du test de destruction." << endl;

}

int main()
{
	bibliotheque_cours::activerCouleursAnsi(); // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";
	

	ifstream fichierTypesMusiciens("types_musiciens.txt");

	ListeGroupes listeGroupes = creerListe("groupes.bin", fichierTypesMusiciens);



	detruireListeGroupes(listeGroupes);
}