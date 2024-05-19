#include "Game.h"
#include "healthbar.h"
#include <complex>
#include <SFML/Graphics.hpp>
#include "Utils/Utils.h"
#include <imgui.h>
#include <SFML/Audio.hpp>

#include "Camera.h"
#include "imgui-SFML.h"
#include "entities/Player/Player.h"
#include "entities/EnemyManager.h"
#include "MainBuilding.h"
#include "BuildSystem.h"
#include "imgui_internal.h"
#include "Projektil.h" //k�nnen wir sp�ter l�schen, ist nur zum debuggen hier
#include "Sounds.h"
#include "Tower.h"

constexpr int BACKGROUND_HEIGHT = 135;
constexpr int BACKGROUND_WIDTH = 135;

constexpr int height = 22;
constexpr int width = 41;

std::vector<std::vector<std::array<uint8_t, 2>>> erstelle_map()
{
	std::vector<std::vector<std::array<uint8_t, 2>>> karte;
	std::array<uint8_t, 2> single_cell = { 0,0 };
	for (int i = 0; i < width; i++)
	{
		std::vector<std::array<uint8_t, 2>> inner_map;
		for (int j = 0; j < height; j++)
		{
			single_cell[0] = Utils::Random::UInt(0, 3);
			inner_map.emplace_back(single_cell);

		}
		karte.emplace_back(inner_map);
	}
	return karte;
}

Game::Game(sf::RenderWindow& window) :m_window(window)
{
	//window.setFramerateLimit(2);
	m_background_textures.resize(4);
	m_geld = 5000000;
	if (!m_background_textures[0].loadFromFile("Resources/images/Background1.jpg")) { LOG_ERROR("texture konnte nicht geladen werden"); }
	if (!m_background_textures[1].loadFromFile("Resources/images/Background2.jpg")) { LOG_ERROR("texture konnte nicht geladen werden"); }
	if (!m_background_textures[2].loadFromFile("Resources/images/Background3.jpg")) { LOG_ERROR("texture konnte nicht geladen werden"); }
	if (!m_background_textures[3].loadFromFile("Resources/images/Background4.jpg")) { LOG_ERROR("texture konnte nicht geladen werden"); }

	m_background_sprites.resize(4);

	m_background_sprites[0].setTexture(m_background_textures[0]);
	m_background_sprites[1].setTexture(m_background_textures[1]);
	m_background_sprites[2].setTexture(m_background_textures[2]);
	m_background_sprites[3].setTexture(m_background_textures[3]);


	m_building_textures.resize(4);

	if (!m_building_textures[0].loadFromFile("Resources/images/1111.png")) { LOG_ERROR("texture konnte nicht geladen werden"); }
	if (!m_building_textures[1].loadFromFile("Resources/images/Top.png")) { LOG_ERROR("texture konnte nicht geladen werden"); }
	if (!m_building_textures[2].loadFromFile("Resources/images/buttom.png")) { LOG_ERROR("texture konnte nicht geladen werden"); }
	if (!m_building_textures[3].loadFromFile("Resources/images/Top.png")) { LOG_ERROR("texture konnte nicht geladen werden"); }

	m_map = std::vector(1, std::vector(height, std::vector(width, Utils::Cell::NOTHING)));
	m_EntityMap = std::vector(1, std::vector(height, std::vector<std::shared_ptr<Entity>>(width)));

	texture.create(window.getSize().x, window.getSize().y);

	LOG_DEBUG("m_map size : {}  ; [0] size: {} ; [0][0] size :{}", m_map.size(), m_map[0].size(), m_map[0][0].size());
}

void Game::render_map(glm::vec3 player_pos, sf::RenderTarget& render_target)
{
	//Utils::ScopedTimer ttt("render_map funktion");
	player_pos = round(player_pos / 135.0f);

	constexpr int rendersizex = 11;
	constexpr int rendersizey = 7;

	for (int i = static_cast<int>(player_pos.x) - rendersizex; i < static_cast<int>(player_pos.x) + rendersizex; i++)
	{
		for (int j = static_cast<int>(player_pos.y) - rendersizey; j < static_cast<int>(player_pos.y) + rendersizey; j++)
		{
			if (Utils::is_valid({ i,j,0.0f }))
			{
				m_background_sprites[m_tiles[i][j][0]].setPosition(static_cast<float>(i) * BACKGROUND_WIDTH, static_cast<float>(j) * BACKGROUND_HEIGHT);
				render_target.draw(m_background_sprites[m_tiles[i][j][0]]);
				
				if(glm::vec2(i,j) != glm::vec2{20,10} && glm::vec2(i,j) != glm::vec2(20,10) && m_EntityMap[0][j][i])
					render_target.draw(*m_EntityMap[0][j][i]);
			}
		}
	}
}
void Game::render_tower(sf::RenderTarget& render_target)
{
	sf::Sprite drawable;

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			switch (m_map[0][j][i])
			{
			case Utils::Cell::WALL:
			{

				drawable.setTexture(m_building_textures[0]);
				drawable.setPosition(i * 135.0f
					, j * 135.0f);
				render_target.draw(drawable);
			}
			break;
			case Utils::Cell::TURRET: //fallthrough
			{
				//		drawable.setTexture(m_building_textures[2]);
				//		drawable.setPosition(i * 135.0f
				//			, j * 135.0f);
				//		m_window.draw(drawable);

				//		drawable.setTexture(m_building_textures[1]);
				//		drawable.setPosition(i * 135.0f
				//			, j * 135.0f);

				//		//drawable.setOrigin( 135.0f / 2.0f, 135.0f / 2.0f);
				//		//drawable.setRotation(45.0f);
				//		m_window.draw(drawable);
				//		//drawable.setRotation(0.0f);
				///*		drawable.setRotation(0.0f);
				//		drawable.setOrigin(0,0);*/


			}
			break;
			case Utils::Cell::NOTHING:
			case Utils::Cell::STAIR:
				break;
			}
		}
	}

}
void Game::run_game(int)
{
	m_sounds.add_group("player");
	m_sounds.add_group("music");
	m_sounds.load_buffer("resources/Sounds/Heilung.mp3", false, "player");
	m_sounds.load_buffer("resources/Sounds/Aufzeichnung(2).mp3", false, "player");
	m_sounds.load_buffer("resources/Sounds/hitmarker.ogg", false, "player");
	m_sounds.load_buffer("resources/Sounds/record.wav", true, "music");
	m_sounds.load_buffer("resources/Sounds/record-1.wav", true, "music");

	m_sounds.set_volume(0, -1);
	m_sounds.set_volume(0, 0);
	m_sounds.set_volume(0, 1);

	m_sounds.music(0);
	std::shared_ptr<Player> p = std::make_shared<Player>();

	Camera window_camera(&m_window, p.get());
	Camera texture_camera(&texture, p.get());
	sf::Clock deltaClock;
	Utils::Timer delta_timer;

	Utils::Pathfinding::Init(p, m_map);
	Utils::Pathfinding* pa = Utils::Pathfinding::get_instance();

	bool right_click = false;
	bool left_click = false;

	EnemyManager* ma = EnemyManager::get_instance();
	std::shared_ptr<MainBuilding> mb = std::make_shared<MainBuilding>();
	{
		const glm::vec2 cell_pos = mb->get_pos() / 135.0f;
		m_EntityMap[0][static_cast<int>(cell_pos.y)][static_cast<int>(cell_pos.x)] = mb;
		m_EntityMap[0][static_cast<int>(cell_pos.y) + 1][static_cast<int>(cell_pos.x)] = mb;
		m_map[0][static_cast<int>(cell_pos.y)][static_cast<int>(cell_pos.x)] = Utils::Cell::TURRET;
		m_map[0][static_cast<int>(cell_pos.y) + 1][static_cast<int>(cell_pos.x)] = Utils::Cell::TURRET;
	}
	healthbar hb{};
	BuildSystem* buildsystem = BuildSystem::get_instance();


	m_tiles = erstelle_map();

	//m_window.clear();
	//render_map(p->get_pos(), m_window);
	//m_window.display();

	Utils::Cell selected = Utils::Cell::NOTHING;

	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Tower>> towers;

	bool first_run = true;
	float lautstarke[3] = { 10.0f,10.0f,10.0f };
	bool paused = false;
	bool should_do_dockspace = true;
	bool player_alive = true;
	p->set_pos(mb->get_pos());
	EnemyManager::set_updated_tower(true);
	EnemyManager::set_player_moving(true);
	pa->calculate_paths(towers, mb);


	while (m_window.isOpen() && m_open)
	{

		EnemyManager::set_updated_tower(false);
		EnemyManager::set_player_moving(false);
		float deltatime = delta_timer.Elapsed();
		delta_timer.Reset();
		sf::Event event{};

		while (m_window.pollEvent(event)) //Hier werden alle moeglichen Events wie tasten und so weiter gehandled
		{
			ImGui::SFML::ProcessEvent(m_window, event);//Imgui Funktion die die Events handled fuer imgui

			switch (event.type)
			{
			case sf::Event::Resized:
				texture.create(event.size.width, event.size.height);
				break;
			case sf::Event::MouseButtonReleased://fallthrough
			case sf::Event::MouseButtonPressed:
			{
				bool down = event.type == sf::Event::MouseButtonPressed;
				if (event.mouseButton.button == sf::Mouse::Button::Left)
					left_click = down;
				if (event.mouseButton.button == sf::Mouse::Button::Right)
					right_click = down;
			}
			break;


			case sf::Event::KeyPressed: //TODO: nicht alles hier machen bitte und logik weiter unten machen
				if (event.key.code == sf::Keyboard::Key::Escape)
				{
					/*m_sounds.pause_all(false);
					m_open = showpausemenu();
					m_sounds.play_all();*/
					m_open = false;
				}

				break;
			case sf::Event::GainedFocus: //fallthrough
			case sf::Event::LostFocus:
				paused = event.type == sf::Event::LostFocus;
				break;
			case sf::Event::Closed:
				m_window.close();
				break;
			default:
				break;
			}
		}
		ImGui::SFML::Update(m_window, deltaClock.restart());//Imgui funktion damit alles geupdatet wird
		if (should_do_dockspace)
			ImGui::DockSpaceOverViewport();


		if (!paused)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
				switch (const int dir =(signed)Utils::Random::UInt(0, 3))
				{
				case 0://oben
					ma->add_enemy(glm::vec3(Utils::Random::UInt(0, width * BACKGROUND_WIDTH - BACKGROUND_WIDTH), 0, 0),
						static_cast<Utils::Priority>(Utils::Random::UInt(0, 2)));
					break;
				case 1://links
					ma->add_enemy(glm::vec3(0, Utils::Random::UInt(0, height * BACKGROUND_HEIGHT - BACKGROUND_WIDTH), 0),
						static_cast<Utils::Priority>(Utils::Random::UInt(0, 2)));
					break;
				case 2://unten
					ma->add_enemy(glm::vec3(Utils::Random::UInt(0, width * BACKGROUND_WIDTH - BACKGROUND_WIDTH), (height-1)*BACKGROUND_HEIGHT, 0),
						static_cast<Utils::Priority>(Utils::Random::UInt(0, 2)));
					break;
				case 3://rechts
					ma->add_enemy(glm::vec3((width-1)*BACKGROUND_WIDTH, Utils::Random::UInt(0, height * BACKGROUND_HEIGHT - BACKGROUND_WIDTH), 0),
						static_cast<Utils::Priority>(Utils::Random::UInt(0, 2)));
					break;
				default:
						break;
				}

			}
			//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))  // nur zum debuggen
			//{
			//	m_sounds.add_sound("player", 2);
			//	new Projectile(p->get_pos(), glm::vec3(p->get_movement_speed().x * 2.5, p->get_movement_speed().y * 2.5, 0), 180, 0.1, 5);
			//}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) //Depression.exe 
			{
				m_sounds.add_sound("player", 1);
				p->take_damage(1);

				hb.damage_input(1);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
			{
				m_sounds.add_sound("player", 0);
				p->take_damage(-1);

				hb.regeneration(1);

			}
		}

		if (paused)
		{
			m_sounds.pause_all(true);
		}
		else
			m_sounds.play_all();

		Utils::Cell temp_cell = buildsystem->display();

		if (should_do_dockspace)
		{
			ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

			ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Menu"))
				{
					if (ImGui::MenuItem("MenuItem")) {}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Examples"))
				{
					if (ImGui::MenuItem("MenuItem")) {}
					ImGui::EndMenu();
				}
				//if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
				if (ImGui::BeginMenu("Tools"))
				{
					if (ImGui::MenuItem("MenuItem"))
					{

					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}



			ImVec2 content_size = ImGui::GetContentRegionAvail();

			texture.create(static_cast<uint32_t>(content_size.x), static_cast<uint32_t>(content_size.y));
			texture_camera.set_RenderTarget(&texture);
		}
		window_camera.move_cam_to_player();
		texture_camera.move_cam_to_player();
		if (m_window.hasFocus())//Spiel logik sollte hier rein
		{
			sf::Vector2f temp;
			Utils::Timer logic_timer;
			Projectile::update_all(deltatime);



			p->update(deltatime);

			if (should_do_dockspace)
			{
				temp = texture.mapPixelToCoords(sf::Mouse::getPosition(m_window));
				ImVec2 imvec2 = ImGui::GetCursorScreenPos();
				temp = { temp.x - imvec2.x ,temp.y - imvec2.y };

			}
			else
			{
				temp = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
			}


			glm::vec3 mouse_pos = glm::vec3{ temp.x,temp.y,0.0f };



			(*buildsystem)(left_click, right_click, should_do_dockspace, m_map, entities, towers, mouse_pos,mb->get_pos());

			std::ranges::sort(towers,
			[&p](const std::shared_ptr<Tower>& tower1, const std::shared_ptr<Tower>& tower2)
			{
				const glm::ivec2 distance_player_tower1 = (p->get_pos() - tower1->get_pos());
				const glm::ivec2 distance_player_tower2 = (p->get_pos() - tower2->get_pos());
				const int manhattan_distance_player_to_tower1 = abs(distance_player_tower1.x) + abs(distance_player_tower1.y);
				const int manhattan_distance_player_to_tower2 = abs(distance_player_tower2.x) + abs(distance_player_tower2.y);
				return manhattan_distance_player_to_tower1 < manhattan_distance_player_to_tower2;
			});

			if (should_do_dockspace) {
				ImGui::PopItemWidth();
				ImGui::End();
			}
			p->shoot(deltatime, m_sounds, mouse_pos);

			// ReSharper disable once CppUseRangeAlgorithm
			std::for_each(/*std::execution::par,*/ towers.begin(), towers.end(),
			[this, &ma, &deltatime](const std::shared_ptr<Tower>& tower)
			{
				tower->fire(*ma, m_sounds, deltatime);
			});


			for (auto it = towers.begin(); it != towers.end();)
			{
				if ((*it)->get_hp() <= 0)
				{
					it->reset();
					it = towers.erase(it);
					EnemyManager::set_updated_tower(true);
				}
				else
					++it;
			}

			for (auto it = entities.begin(); it != entities.end();)
			{
				if ((*it)->get_hp() <= 0)
				{
					const glm::ivec3 cell_pos = (*it)->get_pos() / 135.0f;
					m_map[0][cell_pos.y][cell_pos.x] = Utils::Cell::NOTHING;
					m_EntityMap[0][cell_pos.y][cell_pos.x].reset();
					it = entities.erase(it);
					EnemyManager::set_updated_tower(true);
				}
				else
					++it;
			}

			if (first_run == true || EnemyManager::should_update() == true)
			{
				pa->calculate_paths(towers, mb);
			}
			ma->update(deltatime);

			m_sounds.cleanup();
			m_sounds.music(deltatime);


			if (!hb.alive())
			{

			}
			if (mb->get_hp() <= 0)
			{
				m_open = false;
			}
			ma->naive_enemy_killer();

			ImGui::Begin("DEBUG WINDOW");
			ImGui::TextWrapped("Game logic: MS: %f ", logic_timer.Elapsed() * 1000.0f);
			ImGui::End();
		}
		else if (should_do_dockspace)
		{
			ImGui::PopItemWidth();
			ImGui::End();
		}

		{//Debug Fenster
			ImGui::Begin("DEBUG WINDOW");
			ImGui::TextWrapped("MS: %f\nFPS: %2.2f", deltatime * 1000.0f, 1.0f / deltatime);
			ImGui::TextWrapped("amount of enemies: %llu", ma->get_enemies().size());
			ImGui::TextWrapped("geld %f", m_geld);
			if (ImGui::Button("should do docking"))
			{
				should_do_dockspace = !should_do_dockspace;
			}

			if (paused)
				ImGui::TextWrapped("paused");
			else
				ImGui::TextWrapped("not paused");

			ImGui::SliderFloat("Allgemein", lautstarke, 0, 100);
			ImGui::SliderFloat("Player", &lautstarke[1], 0, 100);
			ImGui::SliderFloat("Musik", &lautstarke[2], 0, 100);
			m_sounds.set_volume(lautstarke[0], -1);
			m_sounds.set_volume(lautstarke[1], 0);
			m_sounds.set_volume(lautstarke[2], 1);

			ImGui::End();
		}

		{//Rendern (Bitte keine als zu gro�e logik ab hier)

			//hier ist die render order
			m_window.clear();//das momentane fenster wird gecleared



			if (should_do_dockspace) {
				texture.clear();
				render_map(p->get_pos(), texture); //als erstes wird der Boden gerendert (weil der immer ganz unten sein sollte)
				texture.draw(*mb);

				//for (auto& tower : towers)
				//{
				//	texture.draw(*tower);
				//	tower->drawtower(texture);
				//}

				//render_tower(texture);
				ma->draw(texture);

				if (p->get_hp() > 0)
				{
					texture.draw(*p);
				}
				Projectile::draw_all_projectiles(texture);
				hb.draw_healthbar(texture, *p);
				texture.display();

				ImGui::Begin("Viewport");
				ImGui::Image(texture);
				ImGui::End();
			}
			else
			{
				render_map(p->get_pos(), m_window); //als erstes wird der Boden gerendert (weil der immer ganz unten sein sollte)
				m_window.draw(*mb);
				//for (auto& tower : towers)
				//{
				//	//texture.draw(*tower);
				//	tower->drawtower(m_window);
				//}
				render_tower(m_window);
				ma->draw(m_window);
				m_window.draw(*p);
				Projectile::draw_all_projectiles(m_window);
				hb.draw_healthbar(m_window, *p);
			}

			ImGui::SFML::Render(m_window); //zu guter letzt kommt imgui (die fenster wie Debug und so)

			m_window.display();
		}
		first_run = false;
	}

	m_sounds.clear_all();
	m_tiles.clear();
	m_open = true;
	EnemyManager::delete_instance();
	window_camera.move_to_default();
	texture_camera.move_to_default();
	Utils::Pathfinding::Delete();
	pa = nullptr;
	BuildSystem::delete_instance();
	buildsystem = nullptr;

	for (int i = 0; i < m_map.size(); i++)
	{
		for (int j = 0; j < m_map[i].size(); j++)
		{
			for (int k = 0; k < m_map[i][j].size(); k++)
			{
				m_map[i][j][k] = Utils::Cell::NOTHING;
				m_EntityMap[i][j][k].reset();
			}
		}
	}



}

void Game::add_geld(double m_geld)
{
	(*this).m_geld += m_geld; // sch�nster code des 21 jahunderts
}

void Game::erstelle_game(sf::RenderWindow& i_window)
{
	if (!s_game)
		s_game = new Game(i_window);
}

Game* Game::get_game()
{
	return s_game;
}

std::vector<std::vector<std::vector<std::shared_ptr<Entity>>>>& Game::getEntityMap()
{
	return (s_game->m_EntityMap);
}

void Game::set_map(const Utils::Cell& cell, int x, int y, int z) {
	if (
		z < m_map.size() && z >= 0 &&
		y < m_map[z].size() && y >= 0 &&
		x < m_map[z][y].size() && x >= 0
		)
		m_map[z][y][x] = cell;
	else
		LOG_ERROR("index [{}][{}][{}] is not valid", z, y, x);
}
std::vector<std::vector<std::vector<Utils::Cell>>>& Game::get_map() {
	return m_map;
}

