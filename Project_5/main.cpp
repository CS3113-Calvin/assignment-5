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

// ————— GLOBAL VARIABLES ————— //
Scene*       g_current_scene;
LevelMenu*   g_level_menu;
LevelA*      g_level_a;
LevelB*      g_level_b;
LevelC*      g_level_c;
unsigned int g_player_lives = 3;
Entity*      g_player;

SDL_Window* g_display_window;
bool        g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4     g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator    = 0.0f;

void switch_to_scene(Scene* scene) {
    g_current_scene = scene;
    g_current_scene->initialize(g_player);
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

    // Set up player
    g_player = new Entity();
    g_player->set_entity_type(PLAYER);
    g_player->set_position(glm::vec3(36.0f, -15.0f, 0.0f));
    g_player->set_movement(glm::vec3(0.0f));
    g_player->set_speed(3.5f);
    g_player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_player->m_texture_id = Utility::load_texture("assets/images/DarkSamurai (64x64).png");

    // Walking
    g_player->m_animations[g_player->WALK] = new int[8]{14, 15, 16, 17, 18, 19, 20};
    // g_player->m_animations[g_player->LEFT] = new int[4] { 1, 5, 9, 13 };
    // g_player->m_animations[g_player->RIGHT] = new int[4]{3, 7, 11, 15};
    // g_player->m_animations[g_player->UP]    = new int[4]{2, 6, 10, 14};
    // g_player->m_animations[g_player->DOWN]  = new int[4]{0, 4, 8, 12};

    g_player->m_animations[g_player->IDLE]     = new int[8]{0, 1, 2, 3, 4, 5, 6, 7};
    g_player->m_animations[g_player->ATTACK_1] = new int[4]{28, 29, 30, 31};
    g_player->m_animations[g_player->ATTACK_2] = new int[4]{42, 43, 44, 45};
    g_player->m_animations[g_player->JUMP]     = new int[4]{56, 57, 58, 59};
    g_player->m_animations[g_player->FALL]      = new int[4]{70, 71, 72, 73};
    g_player->m_animations[g_player->HIT]      = new int[2]{84, 85};
    g_player->m_animations[g_player->DIE]      = new int[14]{98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111};

    g_player->m_animation_indices = g_player->m_animations[g_player->WALK];  // start George looking left
    g_player->m_animation_frames  = 4;
    g_player->m_animation_index   = 0;
    g_player->m_animation_time    = 0.0f;
    g_player->m_animation_cols    = 14;
    g_player->m_animation_rows    = 8;
    g_player->set_height(1.2f);
    g_player->set_width(0.6f);
    g_player->set_scale(2.8f);
    g_player->m_jumping_power = 8.0f;

    // ————— LEVEL SETUP ————— //
    g_level_menu = new LevelMenu();
    g_level_a    = new LevelA();
    g_level_b    = new LevelB();
    g_level_c    = new LevelC();
    // switch_to_scene(g_level_menu);  // todo: change this back to menu
    switch_to_scene(g_level_b);

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
                        if (g_current_scene != g_level_menu) {
                            if (g_current_scene->m_state.player->m_collided_bottom) {
                                g_current_scene->m_state.player->m_is_jumping = true;
                                Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                            }
                        }
                        break;

                    case SDLK_RETURN:
                        // ————— SWITCHING SCENES ————— //
                        if (g_current_scene == g_level_menu) {
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

    // ————— KEY HOLD ————— //
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (g_current_scene != g_level_menu) {
        if (key_state[SDL_SCANCODE_LEFT]) {
            g_current_scene->m_state.player->move_left();
            // g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_animations[g_current_scene->m_state.player->LEFT];
        } else if (key_state[SDL_SCANCODE_RIGHT]) {
            g_current_scene->m_state.player->move_right();
            // g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_animations[g_current_scene->m_state.player->RIGHT];
        } else if (key_state[SDL_SCANCODE_UP]) {
            g_current_scene->m_state.player->move_up();
            // g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_animations[g_current_scene->m_state.player->UP];
        } else if (key_state[SDL_SCANCODE_DOWN]) {
            g_current_scene->m_state.player->move_down();
            // g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_animations[g_current_scene->m_state.player->DOWN];
        }

        // ————— NORMALISATION ————— //
        if (glm::length(g_current_scene->m_state.player->get_movement()) > 1.0f) {
            g_current_scene->m_state.player->set_movement(glm::normalize(g_current_scene->m_state.player->get_movement()));
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

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    // Update scene if player crosses bounds
    if (g_current_scene == g_level_a) {
        // left edge
        if (g_current_scene->m_state.player->get_position().x < LEVEL1_LEFT_EDGE) {
            switch_to_scene(g_level_b);
        }
    } else if (g_current_scene == g_level_b) {
        // bottom edge
        if (g_current_scene->m_state.player->get_position().y < LEVEL2_BOTTOM_EDGE) {
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

    // float x_clamp = glm::clamp(g_current_scene->m_state.player->get_position().x, -36.0f, 36.0f);
    // float y_clamp = glm::clamp(g_current_scene->m_state.player->get_position().y, -36.0f, 36.0f);
    if (g_current_scene != g_level_menu) {
        std::cout << "Player position x: " << g_current_scene->m_state.player->get_position().x << std::endl;
        std::cout << "Player position y: " << g_current_scene->m_state.player->get_position().y << std::endl;
        float x_clamp = glm::clamp(g_current_scene->m_state.player->get_position().x, 0 + (16.0f * VIEW_SCALE), 37.0f - (16.0f * VIEW_SCALE));
        float y_clamp = glm::clamp(g_current_scene->m_state.player->get_position().y, -37.0f + (9.0f * VIEW_SCALE), 0 - (9.0f * VIEW_SCALE));

        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-x_clamp, -y_clamp, 0));
        // if (g_current_scene->m_state.player->get_position().x > LEVEL1_LEFT_EDGE) {
        //         std::cout << "Player position: " << g_current_scene->m_state.player->get_position().x << std::endl;
        //         g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, -g_current_scene->m_state.player->get_position().y, 0));
        //         // g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, 3.75, 0));
        //     } else {
        //         g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5*2, 3.75*2, 0));
        //     }
    }
}

void render() {
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);

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
