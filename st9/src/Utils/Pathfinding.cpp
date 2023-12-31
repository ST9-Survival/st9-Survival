#include "Pathfinding.h"

#include "../Player.h"

namespace Utils {
	Pathfinding* Pathfinding::s_instance = nullptr;


	void Pathfinding::Init(Player* i_player, std::vector<std::vector<std::vector<Utils::Cell>>>& i_map)
	{
		s_instance = new Pathfinding(i_player,i_map);
	}


	Pathfinding* Pathfinding::get_instance()
	{
		if (!s_instance)
			throw std::exception("Pathfinding::Init muss vor Pathfinding::get_instance() gerufen werden");
		return s_instance;
	}

	glm::vec3 Pathfinding::find_nearest(const glm::vec3& start, Priority i_priority)
	{
		switch (i_priority)
		{
		case Priority::nothing:
			glm::vec3 pos1 = glm::abs(m_player->get_pos() - start);
			glm::vec3 pos2 = glm::abs(glm::vec3{ 1,1,1 } - start);
			glm::vec3 min_dist = 2 ? glm::vec3(1) : glm::vec3(1);


			return min_dist;
			break;
		case Priority::player:
			return m_player->get_pos();
			break;
		case Priority::tower: break;
		}
		return { -1,-1,-1 };
	}

	void Pathfinding::find_path(const glm::vec3& dest, const glm::vec3& start)
	{
		dijkstra(dest, start);
	}


	Pathfinding::Pathfinding(Player* i_player, std::vector<std::vector<std::vector<Utils::Cell>>>& i_map ): m_player(i_player), m_map(i_map)
	{
		for (int i = 0; i < m_map.size(); i++)
		{
			std::vector<std::vector<cell>> t_vec_vec_c;

			for (int j = 0; j < m_map[i].size(); j++)
			{
				std::vector<cell> t_vec_c;
				for (int k = 0; k < m_map[i][j].size(); k++)
				{
					t_vec_c.push_back({ {k,j,i },DBL_MAX,nullptr });
				}
				t_vec_vec_c.push_back(t_vec_c);
			}
			m_cellmap.push_back(t_vec_vec_c);
		}
	}

	Pathfinding::~Pathfinding() = default;

	void Pathfinding::dijkstra(const glm::vec3& dest, const glm::vec3& start)
	{
		std::vector<cell*> q_vector;
		for(int i = 0; i < m_map.size();i++)
		{
			for(int j = 0; j < m_map[i].size();j++)
			{
				for(int k = 0; k < m_map[i][j].size(); k++)
				{

					m_cellmap[i][j][k].dist = DBL_MAX;
					m_cellmap[i][j][k].parent = nullptr;
					q_vector.push_back(&m_cellmap[i][j][k]);
						
				}
			}
		}
		m_cellmap[start.z][start.y][start.x].dist = 0;
		auto comp = [](const cell* c1,const cell* c2)->bool
		{
			return c1 > c2;
		};
		while (!q_vector.empty())
		{
			std::sort(q_vector.begin(), q_vector.end(), comp);
			cell* u = q_vector.back();
			q_vector.pop_back();
			for(cell* v : get_neighbours(u,q_vector))
			{
				const double dist = u->dist + get_dist(u,v);
				if (dist < m_cellmap[v->pos.z][v->pos.y][v->pos.x].dist)
				{
					m_cellmap[v->pos.z][v->pos.y][v->pos.x].dist = dist;
					m_cellmap[v->pos.z][v->pos.y][v->pos.x].parent = &m_cellmap[u->pos.z][u->pos.y][u->pos.x];
				}
			}
		}

	}




	bool Pathfinding::is_valid(glm::vec3 pos) const
	{
		if (
			pos.z < m_map.size() && pos.z >= 0 &&
			pos.y < m_map[pos.z].size() && pos.y >= 0 &&
			pos.x < m_map[pos.z][pos.y].size() && pos.x >= 0
			)
			return true;
		return false;
		
	}
	
	static std::vector<glm::vec3> dirs({ {0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0} });

	std::vector<Pathfinding::cell*> Pathfinding::get_neighbours(const cell* current, const std::vector<cell*>& q_vector)
	{
		std::vector<cell*> neighbours;
		for(auto dir : dirs)
		{
			glm::vec3 pos = current->pos + dir;
			if (is_valid(pos))
				neighbours.push_back(&m_cellmap[pos.z][pos.y][pos.x]);

		}
		return neighbours;
	}

	double Pathfinding::get_dist(cell* curr, const cell* dest)
	{
		switch (m_map[dest->pos.z][dest->pos.y][dest->pos.x])
		{
		case Cell::NOTHING:
			break;
		case Cell::WALL: 
			break;
		case Cell::DEFENSE: 
			break;
		case Cell::STAIR: 
			break;
		default:
				return 1;
		}
		return 0;
	}
}
