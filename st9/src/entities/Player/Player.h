#pragma once
#include "entities/entity/Entity.h"

class Sounds;

class Player : public Entity
{
public:
	Player();
	void update_player(float deltatime);
	void shoot(float deltatime, Sounds& i_sounds, glm::vec3 mouse_pos) const;
	glm::ivec3 get_movement_speed() const;
	void set_hp(double i_health);

	void do_damage_calc();

	void take_damage(const double damage) override;

protected:
	std::vector<std::vector<std::vector<sf::Texture>>> m_textures;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	glm::ivec3 cell_pos{};
	glm::ivec3 prev_cell_pos{};
	glm::ivec3 prev_pos;
	int m_geld;

	glm::ivec3 speed;
};
