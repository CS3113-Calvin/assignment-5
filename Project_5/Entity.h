#pragma once

#include "Map.h"

enum EntityType { PLATFORM,
                  PLAYER,
                  ENEMY };
enum AIType { WALKER,
              GUARD,
              PATROLLER,
              JUMPER };
enum AIState { WALKING,
               IDLE,
               ATTACKING };

class Entity {
   private:
    bool m_is_active = true;
    bool m_is_alive  = true;

    // ––––– ANIMATION ––––– //
    int* m_animation_walk     = NULL;  // move to the right
    int* m_animation_idle     = NULL;
    int* m_animation_attack_1 = NULL;
    int* m_animation_attack_2 = NULL;
    int* m_animation_jump     = NULL;
    int* m_animation_fall     = NULL;
    int* m_animation_hit      = NULL;
    int* m_animation_die      = NULL;

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    // ————— TRANSFORMATIONS ————— //
    float     m_speed;
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;
    float     m_scale = 1.0f;
    int       m_flip  = 0;  // 0 = no flip, 1 = horizontal flip, 2 = vertical flip, 3 = both flips

    // ————— ENEMY AI ————— //
    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;

    float m_width  = 1;
    float m_height = 1;

    float invulnerability_time = 1.0f * (2.0f / 3.0f);

   public:
    // patrol bounds
    float m_min_x = 0.0f;
    float m_max_x = 0.0f;

    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;
    static const int WALK              = 0,
                     IDLE              = 1,
                     ATTACK_1          = 2,
                     ATTACK_2          = 3,
                     JUMP              = 4,
                     FALL              = 5,
                     HIT               = 6,
                     DIE               = 7;

    // ————— ANIMATION ————— //
    int** m_animations = new int* [8] {
        m_animation_walk,
            m_animation_idle,
            m_animation_attack_1,
            m_animation_attack_2,
            m_animation_jump,
            m_animation_fall,
            m_animation_hit,
            m_animation_die
    };

    int m_animation_frames = 0,
        m_animation_index  = 0,
        m_animation_cols   = 0,
        m_animation_rows   = 0;

    int*  m_animation_indices = NULL;
    float m_animation_time    = 0.0f;

    // ––––– PHYSICS (JUMPING) ––––– //
    bool  m_is_jumping    = false;
    float m_jumping_power = 0;

    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;

    GLuint m_texture_id;

    // Hit status
    bool  m_hit                  = false;                 // Has the player been hit by an enemy?
    float m_invulnerability_time = invulnerability_time;  // How long until the player can be hit again?

    // ————— METHODS ————— //
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map);  // Now, update should check for both objects in the game AND the map
    void render(ShaderProgram* program);

    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    // Overloading our methods to check for only the map
    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);

    void idle() {
        m_movement.x        = 0.0f;
        m_movement.y        = 0.0f;
        m_animation_indices = m_animations[IDLE];
    }

    void attack_1() {
        m_movement.x        = 0.0f;
        m_movement.y        = 0.0f;
        m_animation_indices = m_animations[ATTACK_1];
    }

    void attack_2() {
        m_movement.x        = 0.0f;
        m_movement.y        = 0.0f;
        m_animation_indices = m_animations[ATTACK_2];
    }

    void jump() {
        m_movement.y        = 1.0f;
        m_animation_indices = m_animations[JUMP];
    }

    void fall() {
        m_movement.y        = -1.0f;
        m_animation_indices = m_animations[FALL];
    }

    void hit() {
        m_movement.x        = 0.0f;
        m_movement.y        = 0.0f;
        m_animation_indices = m_animations[HIT];
    }

    void die() {
        m_movement.x        = 0.0f;
        m_movement.y        = 0.0f;
        m_animation_indices = m_animations[DIE];
    }

    void move_left() {
        m_movement.x        = -1.0f;
        m_flip              = 1;
        m_animation_indices = m_animations[WALK];
    };
    void move_right() {
        m_movement.x        = 1.0f;
        m_flip              = 0;
        m_animation_indices = m_animations[WALK];
    };
    void move_up() {
        m_movement.y        = 1.0f;
        m_animation_indices = m_animations[WALK];
    };
    void move_down() {
        m_movement.y        = -1.0f;
        m_animation_indices = m_animations[WALK];
    };

    void ai_activate(Entity* player);
    void ai_walk();
    void ai_guard(Entity* player);
    void ai_jump();
    void ai_patrol(float min_x, float max_x);

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // ————— GETTERS ————— //
    EntityType const get_entity_type() const { return m_entity_type; };
    AIType const     get_ai_type() const { return m_ai_type; };
    AIState const    get_ai_state() const { return m_ai_state; };
    glm::vec3 const  get_position() const { return m_position; };
    glm::vec3 const  get_movement() const { return m_movement; };
    glm::vec3 const  get_velocity() const { return m_velocity; };
    glm::vec3 const  get_acceleration() const { return m_acceleration; };
    float const      get_jumping_power() const { return m_jumping_power; };
    float const      get_speed() const { return m_speed; };
    int const        get_width() const { return m_width; };
    int const        get_height() const { return m_height; };
    float const      get_scale() const { return m_scale; };
    bool const       get_is_alive() const { return m_is_alive; };

    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_speed(float new_speed) { m_speed = new_speed; };
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
    void const set_scale(float new_scale) { m_scale = new_scale; };
    void const set_is_alive(bool new_is_alive) { m_is_alive = new_is_alive; };
};
