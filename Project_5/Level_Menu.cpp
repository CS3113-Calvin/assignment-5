#include "Level_Menu.h"

#include "Utility.h"

LevelMenu::~LevelMenu() {
    std::cout << "LevelMenu::~LevelMenu()" << std::endl;
}

void LevelMenu::initialize(Entity* player, GLuint fontsheet_texture_id) {
    std::cout << "LevelMenu::initialize()" << std::endl;
    // GLuint map_texture_id = Utility::load_texture("assets/images/Cavernas_by_Adam_Saltsman.png");
    // GLuint map_texture_id = Utility::load_texture("assets/images/tileset.png");
    // m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 12, 32);

    m_state.fontsheet_texture_id = Utility::load_texture("assets/fonts/font1.png");
}

void LevelMenu::update(float delta_time) {
    std::cout << "LevelMenu::update()" << std::endl;
}

void LevelMenu::render(ShaderProgram *program) {
    std::cout << "LevelMenu::render()" << std::endl;
    // Can keep background as just a color
    // Draw "The Caverns Below" text
    Utility::draw_text(program, m_state.fontsheet_texture_id, "The Caverns Below", 0.5f, -0.25f, glm::vec3(-2.5f, 0.0f, 0.0f));
    // Draw "Press Enter to Start" text
    Utility::draw_text(program, m_state.fontsheet_texture_id, "Press Enter to Start", 0.25f, -0.125f, glm::vec3(-1.7f, -0.7f, 0.0f));
}
