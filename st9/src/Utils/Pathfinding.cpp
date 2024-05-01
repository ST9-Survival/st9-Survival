#include "Pathfinding.h"

#include <iostream>

#include "entities/Player/Player.h"

#include "Utils.h"

namespace Utils {
	Pathfinding* Pathfinding::s_instance = nullptr;

	void Pathfinding::Init(std::shared_ptr<Player> i_player, std::vector<std::vector<std::vector<Utils::Cell>>>& i_map)
	{
		LOG_INFO("sizeof Cell {}", sizeof(Cell));
		LOG_INFO("sizeof cell_map {}", sizeof(cell) * i_map.size() * i_map[0].size() * i_map[0][0].size());
		LOG_INFO("sizeof cell {}", sizeof(cell));
		LOG_INFO("sizeof i_map {}", sizeof(Cell) * i_map.size() * i_map[0].size() * i_map[0][0].size());
		if (!s_instance)
			s_instance = new Pathfinding(i_player, i_map);
	}

	void Pathfinding::Delete()
	{
		delete s_instance;
		s_instance = nullptr;
	}

	Pathfinding* Pathfinding::get_instance()
	{
		if (!s_instance)
			throw std::exception("Pathfinding::Init muss vor Pathfinding::get_instance() gerufen werden");
		return s_instance;
	}

	std::vector<glm::vec3> Pathfinding::find_path(const glm::vec3& start, Priority p) const
	{
		/*
		if(!m_player->is_alive() && p == Priority::player)
		{
			p = Priority::nothing;
		}
		*/
		switch (p)
		{
		case Priority::nothing:
			return make_path(start, m_nothing_cellmap);
			break;
		case Priority::tower:
			return make_path(start, m_tower_cellmap);
			break;

		case Priority::player:
			const glm::vec3 dest = m_player->get_pos();
			//LOG_DEBUG("player: x:{} y:{} z:{}", dest.x, dest.y, dest.z);
			if (vec3_almost_equal(dest / 135.0f, start / 135.0f, 1.0f))
			{
				//LOG_INFO("used brensenham");
				return bresenham(dest, start);
			}
			//LOG_INFO("used backtracking");

			return make_path(start, m_player_cellmap);
			break;


		}

		return {};

	}

	Pathfinding::Pathfinding(const std::shared_ptr<Player>& i_player, std::vector<std::vector<std::vector<Utils::Cell>>>& i_map)
		: m_player(i_player), m_map(i_map)
	{

	}

	Pathfinding::~Pathfinding() = default;

	std::vector<glm::vec3> Pathfinding::make_path(const glm::vec3& start,
		const std::vector<std::vector<std::vector<cell>>>& cellmap) const
	{
		// Runde den Startpunkt auf ganze Zahlen ab und teile ihn durch 135
		const glm::ivec3 rounded_start = round(glm::vec3{ start.x / 135.0f, start.y / 135.0f, 0 });

		std::vector<glm::vec3> bewegungsablauf; // Speichert den Bewegungsablauf
		if (!is_valid(rounded_start))
		{
			LOG_CRITICAL("Ungueltiger Startpunkt"); // Logge kritische Meldung, falls Startpunkt ungueltig ist
			return {};
		}
		const cell* u = &cellmap[rounded_start.z][rounded_start.y][rounded_start.x]; // Zeiger auf die aktuelle Zelle

		if (u == nullptr)
		{
			LOG_CRITICAL("Ungueltige Zelle an Startposition"); // Logge kritische Meldung, falls die Zelle an der Startposition ungueltig ist
			return {};
		}

		// Erzeuge Bewegungsablauf durch Backtracking
		if (u->parent != nullptr)
		{
			// Falls die aktuelle Zelle einen Elternknoten hat
			for (auto pos : bresenham(start, u->parent->pos * 135))
			{
				bewegungsablauf.push_back(pos); // Fuege Punkte entlang der Bresenham-Linie zum Bewegungsablauf hinzu
			}
			u = u->parent; // Aktualisiere die aktuelle Zelle auf den Elternknoten
		}
		else
		{
			// Falls die aktuelle Zelle keinen Elternknoten hat, erzeuge Bewegungsablauf direkt zum aktuellen Punkt
			for (auto pos : bresenham(start, u->pos * 135))
			{
				bewegungsablauf.push_back(pos); // Fuege Punkte entlang der Bresenham-Linie zum Bewegungsablauf hinzu
			}
			std::ranges::reverse(bewegungsablauf); // Kehre den Bewegungsablauf um, da er in umgekehrter Reihenfolge erstellt wurde
			return bewegungsablauf; // Rueckgabe des Bewegungsablaufs
		}

		// Fortsetzung des Backtrackings, bis der Startknoten erreicht ist
		while (u->parent != nullptr)
		{
			if (u == nullptr)
			{
				LOG_CRITICAL("Speicherfehler aufgetreten"); // Logge kritische Meldung bei Speicherfehlern
				__debugbreak(); // Unterbreche das Programm fuer Debugzwecke
			}
			for (auto pos : bresenham(u->pos * 135, u->parent->pos * 135))
			{
				bewegungsablauf.push_back(pos); // Fuege Punkte entlang der Bresenham-Linie zum Bewegungsablauf hinzu
			}
			u = u->parent; // Aktualisiere die aktuelle Zelle auf den Elternknoten
		}

		std::ranges::reverse(bewegungsablauf); // Kehre den Bewegungsablauf um, da er in umgekehrter Reihenfolge erstellt wurde
		return bewegungsablauf; // Rueckgabe des Bewegungsablaufs
	}


	void Pathfinding::dijkstra(const std::vector<glm::ivec3>& start_points, std::vector<std::vector<std::vector<cell>>>& cellmap)
	{

		// Initialisierung eines Vektors, der alle Zellen enthaelt
		// Eine PriorityQueue waere eigentlich besser, aber es gibt Probleme damit.
		// Ein Vektor wird verwendet, und Sortierung fuehrt zum gleichen Ergebnis wie mit einer PriorityQueue.
		std::vector<cell*> q_vector;

		// Ueberpruefen, ob die Zellkarte die richtige Groesse hat, und gegebenenfalls neu initialisieren
		if (cellmap.size() != m_map.size() || cellmap[0].size() != m_map[0].size() || cellmap[0][0].size() != m_map[0][0].size())
		{
			cellmap =
				std::vector(m_map.size(),
					std::vector(m_map[0].size(),
						std::vector(m_map[0][0].size(),
							cell{ {0,0,0},DBL_MAX,nullptr })));
			// Alle Positionen werden in q_vector geladen (koennte noch optimiert werden)
			for (int i = 0; i < m_map.size(); i++) // z
			{
				for (int j = 0; j < m_map[i].size(); j++) // y
				{
					for (int k = 0; k < m_map[i][j].size(); k++) // x
					{
						cellmap[i][j][k].pos = { k, j, i }; // x y z 
						q_vector.push_back(&cellmap[i][j][k]); // Zelle zu q_vector hinzufuegen
					}
				}
			}
		}

		// Zellkarte ist bereits korrekt dimensioniert.
		else
		{
			// alle Positionen in q_vector laden (koennte noch optimiert werden)
			for (int i = 0; i < m_map.size(); i++) // z
			{
				for (int j = 0; j < m_map[i].size(); j++) // y
				{
					for (int k = 0; k < m_map[i][j].size(); k++) // x
					{
						cellmap[i][j][k].pos = { k, j, i }; // x y z 
						cellmap[i][j][k].dist = DBL_MAX;
						cellmap[i][j][k].parent = nullptr;
						q_vector.push_back(&cellmap[i][j][k]); // hinzufuegen der zelle zu q_vector
					}
				}
			}
		}


		{// Setze die Distanz der Startpunkte auf 0 und deren Eltern auf nullptr
			int amount_valid = start_points.size();
			for (const auto start : start_points)
			{
				if (!is_valid(start))
				{
					amount_valid--;
				}
				{
					cellmap[start.z][start.y][start.x].dist = 0; // Distanz am Startpunkt auf 0 setzen
					cellmap[start.z][start.y][start.x].parent = nullptr; // Elternknoten auf nullptr setzen
				}

			}
			if (amount_valid <= 0)
			{
				LOG_ERROR("keine gueltigen start punkte verfuegbar");
				return;
			}
		}

		// Vergleichsfunktion fuer Zellen, um sie absteigend nach Distanz zu sortieren
		auto comp = [](const cell* c1, const cell* c2)->bool
			{
				return (c1->dist) > (c2->dist);
			};

		// Haupt-Dijkstra-Algorithmus
		while (!q_vector.empty())
		{
			std::ranges::sort(q_vector, comp); // Sortiere den q_Vector nach Distanz absteigend.

			cell* current = q_vector.back(); // Nehme das letzte Element aus dem q_Vector.
			q_vector.pop_back(); // Loesche das letzte Element aus dem q_Vector.

			for (cell* v : get_neighbours(current, cellmap)) // Hole die Nachbarn von current.
			{
				// Berechne die Distanz zwischen current und v.
				const double dist = current->dist + get_dist(current, v);

				// Vergleiche die berechnete Distanz mit der in v gespeicherten Distanz.
				if (dist < v->dist)
				{
					// Aktualisiere die Distanz und den Parent von v.
					v->dist = dist;
					v->parent = current;
				}
			}
		}

	}

	std::vector<glm::vec3> Pathfinding::bresenham(const glm::vec3& dest, const glm::vec3& start) const
	{
		const glm::ivec3 temp_dest = round(dest);
		const glm::ivec3 temp_start = round(start);

		bool swapped = false;// Wird auf true gesetzt, wenn x1 > x2, um die Routenrichtung umzukehren
		std::vector<glm::vec3> route;// Vektor zur Speicherung der Routenpunkte
		route.reserve(135);


		// Berechnung der Bresenham-Linie zwischen Start- und Zielpunkt
		{
			int x1 = temp_start.x;
			int y1 = temp_start.y;
			int x2 = temp_dest.x;
			int y2 = temp_dest.y;

			// Ueberpruefen und ggf. vertauschen der Start- und Zielkoordinaten
			if (x1 > x2) 
			{
				swapped = true;
				std::swap(x1, x2);
				std::swap(y1, y2);
			}

			const int dx = abs(x2 - x1);// Differenz in x-Richtung
			const int dy = abs(y2 - y1);// Differenz in y-Richtung
			const int sx = (x1 < x2) ? 1 : -1;// Schrittweite in x-Richtung
			const int sy = (y1 < y2) ? 1 : -1;// Schrittweite in y-Richtung

			int err = dx - dy;// Fehlerterm fuer die Bresenham-Berechnung

			// Bresenham-Algorithmus zur Generierung der Linie
			while (true) 
			{

				route.emplace_back(x1, y1, 0.0f); // Aktuellen Punkt zur Route hinzufuegen

				if (x1 == x2 && y1 == y2) 
				{
					break;// Ziel erreicht, Schleife beenden
				}

				const int err2 = 2 * err;

				// Aktualisierung des Fehlers und der Koordinaten basierend auf dem Bresenham-Algorithmus
				if (err2 > -dy)
				{
					err -= dy;
					x1 += sx;
				}

				if (err2 < dx) 
				{
					err += dx;
					y1 += sy;
				}
			}
		}

		// Umkehren der Route, falls sie nicht vertauscht wurde
		if (!swapped)
		{
			std::ranges::reverse(route);
		}
		return route; // R�ckgabe der generierten Route
	}


	bool Pathfinding::is_valid(const glm::vec3& pos) const
	{
		if (!(pos.z < m_map.size() && pos.z >= 0))
			return false;
		if (!(pos.y < m_map[static_cast<int>(pos.z)].size() && pos.y >= 0))
			return false;
		if (!(pos.x < m_map[static_cast<int>(pos.z)][static_cast<int>(pos.y)].size() && pos.x >= 0))
			return false;
		return true;

	}

	void Pathfinding::calculate_paths()
	{
		//priority player
		{
			//ScopedTimer player("priority player");
			const glm::ivec3 start = glm::ivec3(m_player->get_pos().x / 135.0f, m_player->get_pos().y / 135.0f, 0);
			if (!is_valid(start))
			{
				LOG_ERROR("das haette nicht passieren sollen start: x:{} y:{} z:{}",
					start.x, start.y, start.z);
			}
			dijkstra({ start }, m_player_cellmap);
		}

		//priority nothing
		{
			std::vector<glm::ivec3> start_points;
			//for(auto towers: get_alltowers())
			//{
			//	start_points.push_back(tower.get_pos()/135.0f);
			//}
			start_points.push_back((glm::ivec3(m_player->get_pos().x / 135.0f, m_player->get_pos().y / 135.0f, 0)));
			dijkstra(start_points, m_nothing_cellmap);
		}

		//priority tower
		{
			std::vector<glm::ivec3> start_points;
			//for(auto towers: get_alltowers())
			//{
			//	start_points.push_back(tower.get_pos()/135.0f);
			//}
			if (start_points.empty() == true)
			{
				start_points.push_back((glm::ivec3(m_player->get_pos().x / 135.0f, m_player->get_pos().y / 135.0f, 0)));
			}
			dijkstra(start_points, m_tower_cellmap);
		}
	}

	static std::vector<glm::ivec3> dirs({ {0,0,-1},{0,0,1}, {0,-1,0},{0,1,0},{-1,0,0},{1,0,0} });
	std::vector<Pathfinding::cell*> Pathfinding::get_neighbours(const Pathfinding::cell* current, std::vector<std::vector<std::vector<Pathfinding::cell>>>& m_cellmap) const
	{
		std::vector<cell*> neighbours;
		LOG_TRACE("current pos: x: {} y: {} z:{}", current->pos.x, current->pos.y, current->pos.z);
		for (auto dir : dirs)
		{
			glm::vec3 pos = current->pos + dir;

			LOG_TRACE("pos: x: {} y: {} z:{}",pos.x, pos.y, pos.z);
			LOG_TRACE("is_valid pos {}", is_valid(pos));
			if (is_valid(pos))
				neighbours.push_back(&m_cellmap[static_cast<int>(pos.z)][static_cast<int>(pos.y)][static_cast<int>(pos.x)]);

		}
		return neighbours;
	}

	double Pathfinding::get_dist(cell* curr, const cell* dest) const
	{
		switch (m_map[static_cast<int>(dest->pos.z)][static_cast<int>(dest->pos.y)][static_cast<int>(dest->pos.x)])
		{
		case Cell::NOTHING: // fallthrough
		case Cell::STAIR:
			return 1;

		case Cell::DEFENSE:
			return 2;

		case Cell::WALL:
			return 10;
		case Cell::TURRET:

			break;
		}
		return 1;
	}
}
