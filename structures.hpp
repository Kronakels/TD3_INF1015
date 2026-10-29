/**
* Structures de données pour une collection de groupes musicaux.
* \file   structures.hpp
* \author Anthony Gingras et Vincent Tran
* \date   21 octobre 2026
* Créé le 15 septembre 2026
*/

#pragma once
// Structures mémoires pour une collection de groupes musicaux.

#include <string>
#include <span>
#include <memory>
#include <functional>
#include <vector>
#include <istream>

#include "cppitertools/range.hpp"

struct Groupe; class Musicien;   // Permet d'utiliser les types alors qu'ils seront définis après.
class ListeGroupes
{
public:
	ListeGroupes();
	ListeGroupes(int capaciteInitiale);
	void ajouter(Groupe* groupe);
	void retirer(const Groupe* groupe);
	void libererMemoire();
	std::shared_ptr<Musicien> trouverMusicien(const std::string& nom) const;
	Groupe* trouverGroupe(const std::function<bool(const Groupe*)>& critere) const;
	int getTaille() const;
	int getCapacite() const;
	std::span<Groupe* const> getGroupes() const;
private:
	int capacite_, nElements_;
	Groupe** elements_; // Pointeur vers un tableau de Groupe*, chaque Groupe* pointant vers un Groupe.
};

template <typename T>
class Liste
{
public:
	Liste() :
		capacite_(0), 
		nElements_(0), 
		elements_(nullptr)
	{
	}

	explicit Liste(int capacite):
		capacite_(capacite),
		nElements_(0),
		elements_(std::make_unique<std::shared_ptr<T>[]>(capacite))
	{
	}

	Liste(const Liste& autre) :
		capacite_(autre.capacite_),
		nElements_(autre.nElements_),
		elements_(std::make_unique<std::shared_ptr<T>[]>(autre.capacite_))
	{
		for (int i : iter::range(nElements_))
			elements_[i] = autre.elements_[i];
	}
	
	void ajouter(const std::shared_ptr<T>& element) {
		elements_[nElements_] = element;
		nElements_++;
	}

	int getTaille() const { return nElements_; }

	std::shared_ptr<T>& operator[](int index) 
	{
		return elements_[index];
	}

	std::span<const std::shared_ptr<T>> getElements() const 
	{
		return std::span<const std::shared_ptr<T>>(elements_.get(), nElements_);
	}
private:
	int capacite_, nElements_;
	std::unique_ptr<std::shared_ptr<T>[]> elements_;
};

class Musicien
{
public:
	friend std::shared_ptr<Musicien> lireMusicien(std::istream& fichier, std::istream& fichierTypes, const ListeGroupes& liste);
	friend std::shared_ptr<Musicien> ListeGroupes::trouverMusicien(const std::string& nom) const;
	friend void afficherMusicien(std::ostream& o, const Musicien& musicien);
	friend int main();
private:
	std::string nom, pays;
	int anneeNaissance = 0;
};

class Instrumentiste : public Musicien
{
public:
	Instrumentiste(const Musicien& musicien, std::istream& fichier);
private:
	std::vector<std::string> instruments_;
};

class Chanteur : public Musicien
{
public:
	Chanteur(const Musicien& musicien, std::istream& fichier);
private:
	std::string styleVocal_;
};

using ListeMusiciens = Liste<Musicien>;



struct Groupe
{
	Groupe(const Groupe*);
	Groupe() = default;
	Groupe(std::string nom, std::string genre, int anneeFormation, int nMembres);
	std::string nom, genre; // Nom du groupe et genre de musique qu'il fait principalement.
	int anneeFormation = 0; // Année où le groupe a été formé initialement.
	ListeMusiciens membres;
};
