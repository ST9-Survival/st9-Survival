#pragma once
#include "Utils/Log.h"
#include "Utils/Utils.h"




class Menu
{
public:
	Menu(const Menu&) = delete;
	//test
	static Menu* get_instance();
	static void delete_instance();
	void show_menu();
	void button_events();
	void story();

private:
	//Menu(): m_window(sf::VideoMode(1920, 1080), "Game") {}
	Menu();
	void draw_button_labels(int);
	sf::RenderWindow m_window;
	std::vector<std::string> m_name_button;
	//std::vector<std::vector<std::vector<Utils::Cell>>> m_map;
	inline static Menu* s_instance = nullptr;
	std::pair<sf::FloatRect, bool> m_buttons[4];
	sf::Font m_font;
};

