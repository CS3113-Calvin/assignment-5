#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_LEFT_EDGE 0.0f
#define LEVEL2_BOTTOM_EDGE -37.0f
#define LEVEL3_TOP_EDGE 1.0f
#define VIEW_SCALE 0.7f  // scale camera view

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>

#include <ctime>
#include <vector>

#include "Entity.h"
#include "Level_A.h"
#include "Level_B.h"
#include "Level_C.h"
#include "Level_Menu.h"
#include "Map.h"
#include "Scene.h"
#include "ShaderProgram.h"
#include "Utility.h"
#include "cmath"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH  = 1600,
          WINDOW_HEIGHT = 900;

const float BG_RED     = 23.0f / 255,
            BG_BLUE    = 28.0f / 255,
            BG_GREEN   = 57.0f / 255,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

enum GameStatus { RUNNING,
                  WIN,
                  LOSE };

// ————— GLOBAL VARIABLES ————— //
Scene*     g_current_scene;
LevelMenu* g_level_menu;
LevelA*    g_level_a;
LevelB*    g_level_b;
LevelC*    g_level_c;
int        g_player_lives = 3;
Entity*    g_player;
GLuint     g_heart_spritesheet;
Entity*    g_hearts;  // rendering hearts on screen
GLuint     g_fontsheet_texture_id;
GameStatus g_game_status = RUNNING;
// Entity*      g_enemies;

SDL_Window* g_display_window;
bool        g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4     g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator    = 0.0f;

void switch_to_scene(Scene* scene) {
    g_current_scene = scene;
    g_current_scene->initialize(g_player, g_fontsheet_texture_id);
}

void initialize() {
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("The Caverns Below!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-16.0f * VIEW_SCALE, 16.0f * VIEW_SCALE, -9.0f * VIEW_SCALE, 9.0f * VIEW_SCALE, -1.0f, 1.0f);
    // g_projection_matrix = glm::ortho(-5.0f*2, 5.0f*2, -3.75f*2, 3.75f*2, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // Set up hearts texture
    g_heart_spritesheet = Utility::load_texture("assets/images/Hearts.png");

    // Set up fontsheet
    g_fontsheet_texture_id = Utility::load_texture("assets/fonts/font1.png");

    // Set up player
    g_player = new Entity(PLAYER);
    // g_player->set_entity_type();
    g_player->set_position(glm::vec3(36.0f, -15.0f, 0.0f));
    g_player->set_movement(glm::vec3(0.0f));
    g_player->set_speed(3.5f);
    g_player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_player->m_texture_id = Utility::load_texture("assets/images/DarkSamurai (64x64).png");

    // Walking
    g_player->m_animations[g_player->WALK]     = new int[8]{14, 15, 16, 17, 18, 19, 20};
    g_player->m_animations[g_player->IDLE]     = new int[8]{0, 1, 2, 3, 4, 5, 6, 7};
    g_player->m_animations[g_player->ATTACK_1] = new int[4]{28, 29, 30, 31};
    g_player->m_animations[g_player->ATTACK_2] = new int[4]{42, 43, 44, 45};
    g_player->m_animations[g_player->JUMP]     = new int[4]{56, 57, 58, 59};
    g_player->m_animations[g_player->FALL]     = new int[4]{70, 71, 72, 73};
    g_player->m_animations[g_player->HIT]      = new int[2]{84, 85};
    g_player->m_animations[g_player->DIE]      = new int[14]{98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111};

    // g_player->m_animation_indices = g_player->m_animations[g_player->WALK];  // start George looking left
    // g_player->move_left();
    g_player->set_curr_state(g_player->WALK_LEFT);
    g_player->set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
    // g_player->m_animation_frames  = 4;
    g_player->m_animation_index = 0;
    g_player->m_animation_time  = 0.0f;
    g_player->m_animation_cols  = 14;
    g_player->m_animation_rows  = 8;
    g_player->set_height(1.2f);
    g_player->set_width(0.6f);
    g_player->set_scale(2.8f);
    g_player->m_jumping_power = 8.0f;
    // g_player->m_lives         = g_player_lives;

    // ————— LEVEL SETUP ————— //
    g_level_menu = new LevelMenu();
    g_level_a    = new LevelA();
    g_level_b    = new LevelB();
    g_level_c    = new LevelC();
    // switch_to_scene(g_level_menu);  // todo: change this back to menu
    switch_to_scene(g_level_c);

    // Audio
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    Mix_Music* bgm = Mix_LoadMUS("assets/audio/Malicious.mp3");
    Mix_PlayMusic(bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "Game initialized!" << std::endl;
}

void process_input() {
    if (g_current_scene != g_level_menu) {
        g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));
    }

    // ————— KEY HOLD ————— //
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (g_current_scene != g_level_menu && g_game_status == RUNNING) {
        if (key_state[SDL_SCANCODE_LEFT] || key_state[SDL_SCANCODE_A]) {
            // g_current_scene->m_state.player->move_left();
            g_current_scene->m_state.player->set_curr_state(g_current_scene->m_state.player->WALK_LEFT);
            g_current_scene->m_state.player->set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
        } else if (key_state[SDL_SCANCODE_RIGHT] || key_state[SDL_SCANCODE_D]) {
            g_current_scene->m_state.player->set_curr_state(g_current_scene->m_state.player->WALK_RIGHT);
            g_current_scene->m_state.player->set_movement(glm::vec3(1.0f, 0.0f, 0.0f));
            // g_current_scene->m_state.player->move_right();
        } else if (key_state[SDL_SCANCODE_UP] || key_state[SDL_SCANCODE_W]) {
            g_current_scene->m_state.player->set_curr_state(g_current_scene->m_state.player->WALK_UP);
            g_current_scene->m_state.player->set_movement(glm::vec3(0.0f, 1.0f, 0.0f));
            // g_current_scene->m_state.player->move_up();
        } else if (key_state[SDL_SCANCODE_DOWN] || key_state[SDL_SCANCODE_S]) {
            g_current_scene->m_state.player->set_curr_state(g_current_scene->m_state.player->WALK_DOWN);
            g_current_scene->m_state.player->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
            // g_current_scene->m_state.player->move_down();
        } else {
            // g_current_scene->m_state.player->set_curr_state(g_current_scene->m_state.player->IDLE);
        }

        // ————— NORMALISATION ————— //
        if (glm::length(g_current_scene->m_state.player->get_movement()) > 1.0f) {
            g_current_scene->m_state.player->set_movement(glm::normalize(g_current_scene->m_state.player->get_movement()));
        }
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // ————— KEYSTROKES ————— //
        switch (event.type) {
                // ————— END GAME ————— //
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;

                    case SDLK_SPACE:
                        // case SDLK_UP:
                        // ————— JUMPING ————— //
                        if (g_current_scene != g_level_menu && g_game_status == RUNNING) {
                            if (g_current_scene->m_state.player->m_collided_bottom) {
                                g_current_scene->m_state.player->m_is_jumping = true;
                                Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                            }
                        }
                        break;

                    case SDLK_e:
                        // attack
                        if (g_current_scene != g_level_menu && g_game_status == RUNNING) {
                            // g_current_scene->m_state.player->attack_1();
                            std::cout << "\n\n\n\nAttacking\n\n\n\n"
                                      << std::endl;
                            g_current_scene->m_state.player->set_curr_state(g_current_scene->m_state.player->ATTACK_1);
                        }
                        break;

                    case SDLK_RETURN:
                        // ————— SWITCHING SCENES ————— //
                        if (g_current_scene == g_level_menu && g_game_status == RUNNING) {
                            switch_to_scene(g_level_a);
                        }
                        break;
                    default:
                        break;
                }

            default:
                break;
        }
    }
}

void update() {
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks      = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP) {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);

        if (g_current_scene->m_state.player->m_hit) {
            g_player_lives--;
            g_current_scene->m_state.player->m_hit = false;
        }

        if (g_player_lives <= 0) {
            g_game_status = LOSE;
        }

        if (g_current_scene->m_state.player->m_got_flag) {
            g_game_status = WIN;
        }

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    // Update scene if player crosses bounds
    if (g_current_scene == g_level_a) {
        // left edge
        if (g_current_scene->m_state.player->get_position().x < LEVEL1_LEFT_EDGE + g_current_scene->m_state.player->get_width() / 2.0f) {
            switch_to_scene(g_level_b);
        }
    } else if (g_current_scene == g_level_b) {
        // bottom edge
        if (g_current_scene->m_state.player->get_position().y < LEVEL2_BOTTOM_EDGE + g_current_scene->m_state.player->get_height() / 2.0f) {
            switch_to_scene(g_level_c);
        }
    } else if (g_current_scene == g_level_c) {
        // top edge
        // if (g_current_scene->m_state.player->get_position().y > LEVEL3_TOP_EDGE) {
        //     switch_to_scene(g_level_a);
        // }
    }

    // ————— PLAYER CAMERA ————— //
    g_view_matrix = glm::mat4(1.0f);

    if (g_current_scene != g_level_menu) {
        // std::cout << "Player position x: " << g_current_scene->m_state.player->get_position().x << std::endl;
        // std::cout << "Player position y: " << g_current_scene->m_state.player->get_position().y << std::endl;
        float x_clamp = glm::clamp(g_current_scene->m_state.player->get_position().x, 0 + (16.0f * VIEW_SCALE), 37.0f - (16.0f * VIEW_SCALE));
        float y_clamp = glm::clamp(g_current_scene->m_state.player->get_position().y, -37.0f + (9.0f * VIEW_SCALE), 0 - (9.0f * VIEW_SCALE));

        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-x_clamp, -y_clamp, 0));
    }
}

void render() {
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);

    float x_clamp = glm::clamp(g_current_scene->m_state.player->get_position().x, 0 + (16.0f * VIEW_SCALE), 37.0f - (16.0f * VIEW_SCALE));
    float y_clamp = glm::clamp(g_current_scene->m_state.player->get_position().y, -37.0f + (9.0f * VIEW_SCALE), 0 - (9.0f * VIEW_SCALE));
    if (g_current_scene != g_level_menu) {
        // render hearts

        Utility::draw_text(&g_shader_program, g_fontsheet_texture_id, "Current lives: " + std::to_string(g_player_lives), 0.5f, -0.25f, glm::vec3(x_clamp - (15.0f * VIEW_SCALE), y_clamp + (8.0f * VIEW_SCALE), 0.0f));
        // Utility::draw_text(&g_shader_program, g_fontsheet_texture_id, std::to_string(g_player_lives), 0.5f, -0.25f, glm::vec3(x_clamp - 7.0f, y_clamp + 5.5f, 0.0f));
    }

    // render win/lose text
    if (g_game_status == WIN) {
        // glm::vec3 position = g_current_scene->m_state.player->get_position();
        Utility::draw_text(&g_shader_program, g_fontsheet_texture_id, "You win!", 1.5f, -0.7f, glm::vec3(x_clamp - 3.0f, y_clamp + 4.0f, 0.0f));
        std::cout << "You win!\n";
    } else if (g_game_status == LOSE) {
        // glm::vec3 position = g_current_scene->m_state.player->get_position();
        Utility::draw_text(&g_shader_program, g_fontsheet_texture_id, "You lose!", 1.5f, -0.7f, glm::vec3(x_clamp - 3.5f, y_clamp + 4.0f, 0.0f));
        std::cout << "You lose!\n";
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    SDL_Quit();

    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_level_a;
    delete g_level_b;
    delete g_level_c;
    delete g_level_menu;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[]) {
    initialize();

    while (g_game_is_running) {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
