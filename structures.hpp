#pragma once
// Structures mémoires pour une collection de groupes musicaux.

#include <string>
#include <span>
#include <memory>
#include <functional>

struct Groupe; struct Musicien; // Permet d'utiliser les types alors qu'ils seront définis après.

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

class ListeMusiciens
{
public:
	ListeMusiciens();
	explicit ListeMusiciens(int capacite);
	ListeMusiciens(const ListeMusiciens& autre);
	void ajouter(const std::shared_ptr<Musicien>& musicien);
	std::span<const std::shared_ptr<Musicien>> getMusiciens() const;
	std::shared_ptr<Musicien> operator[](int index) const;
private:
	int capacite_, nElements_;
	std::unique_ptr<std::shared_ptr<Musicien>[]> elements_;
};

struct Groupe
{
	std::string nom, genre; // Nom du groupe et genre de musique qu'il fait principalement.
	int anneeFormation = 0; // Année où le groupe a été formé initialement.
	ListeMusiciens membres;
};

struct Musicien
{
	std::string nom, pays;
	int anneeNaissance = 0;
	//ListeGroupes joueDans;
};