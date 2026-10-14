#pragma once
// Structures mémoires pour une collection de groupes musicaux.

#include <string>
#include <span>
#include <memory>

struct Groupe; struct Musicien; // Permet d'utiliser les types alors qu'ils seront définis après.

class ListeGroupes 
{
public:
	ListeGroupes();
	ListeGroupes(int capaciteInitiale);
	void ajouter(Groupe* groupe);
	void retirer(const Groupe* groupe);
	void libererMemoire();
	Musicien* trouverMusicien(const std::string& nom) const;
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
	void ajouter(Musicien* musicien);
	std::span<Musicien* const> getMusiciens() const;
private:
	int capacite_, nElements_;
	std::unique_ptr<Musicien* []> elements_;
};

struct Groupe
{
	std::string nom, genre; // Nom du groupe et genre de musique qu'il fait principalement.
	int anneeFormation; // Année où le groupe a été formé initialement.
	ListeMusiciens membres;
};

struct Musicien
{
	std::string nom, pays; int anneeNaissance;
	ListeGroupes joueDans;
};