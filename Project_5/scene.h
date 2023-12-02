#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>

#include "Entity.h"
#include "Map.h"
#include "ShaderProgram.h"
#include "Utility.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

/**
    Notice that the game's state is now part of the Scene class, not the main file.
*/
struct GameState {
    // ————— GAME OBJECTS ————— //
    Map    *map;
    Entity *player;
    Entity *enemies;
    Entity *collectables;
    // GLuint  fontsheet_texture_id;

    // ————— AUDIO ————— //
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;

    // ————— POINTERS TO OTHER SCENES ————— //
    int next_scene_id;
};

class Scene {
   public:
    // ————— ATTRIBUTES ————— //
    int m_number_of_enemies = 1;

    GameState m_state;

    // ————— METHODS ————— //
    virtual void initialize(Entity *player)     = 0;
    virtual void update(float delta_time)       = 0;
    virtual void render(ShaderProgram *program) = 0;

    // ————— GETTERS ————— //
    GameState const get_state() const { return m_state; }
    int const       get_number_of_enemies() const { return m_number_of_enemies; }
};
