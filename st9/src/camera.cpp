#include "Camera.h"
#include "Player.h"

Camera::Camera(sf::RenderWindow* window)
	:m_player(nullptr), m_window(window)
{
	m_view = sf::View({ 0,0 }, { (float)m_window->getSize().x,(float)m_window->getSize().y });
}

Camera::Camera(sf::RenderWindow* window, Player* player)
	:m_player(player), m_window(window)
{
	m_window = window;
	m_player = player;
	m_view = sf::View({ 0,0 }, { (float)m_window->getSize().x,(float)m_window->getSize().y });
}

Camera::~Camera()
{
}

void Camera::set_player(Player* player)
{
	m_player = player;
}

Player* Camera::get_player()
{
	return m_player;
}

void Camera::move_cam_to_player()
{
	m_view.setCenter(m_player->get_pos().x + 50, m_player->get_pos().y + 50);
	m_view.setSize({(float)m_window->getSize().x,(float)m_window->getSize().y});
	m_window->setView(m_view);
}

void Camera::set_window_size(int x, int y)
{
	m_view.setSize((float)x, (float)y);
	m_window->setView(m_view);
}

void Camera::move_to_pos(int x, int y)
{
	m_view.setCenter(x,y);
	m_view.setSize({ (float)m_window->getSize().x,(float)m_window->getSize().y });
	m_window->setView(m_view);
}

void Camera::move_to_pos(sf::Vector2f v)
{
	m_view.setCenter(v);
	m_view.setSize({ (float)m_window->getSize().x,(float)m_window->getSize().y });
	m_window->setView(m_view);
}

void Camera::move_to_pos(sf::View v)
{
	m_view = v;
	m_view.setSize({ (float)m_window->getSize().x,(float)m_window->getSize().y });
	m_window->setView(m_view);
}