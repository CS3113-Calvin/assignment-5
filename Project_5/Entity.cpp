/**
 * Author: Calvin Tian
 * Assignment: Rise of the AI
 * Date due: 2023-11-18, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>

#include "Entity.h"
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

Entity::Entity() {
    // ––––– PHYSICS ––––– //
    m_position     = glm::vec3(0.0f);
    m_velocity     = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // ––––– TRANSLATION ––––– //
    m_movement     = glm::vec3(0.0f);
    m_speed        = 0;
    m_model_matrix = glm::mat4(1.0f);
}

Entity::~Entity() {
    delete[] m_animation_attack_1;
    delete[] m_animations;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index) {
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width  = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    // float* tex_coords;
    // if (m_flip == 0) {
    //     tex_coords = (float[12]){
    //         u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
    //         u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord};
    // } else if (m_flip == 1) {
    //     tex_coords = (float[12]){
    //         u_coord + width, v_coord + height, u_coord, v_coord + height, u_coord, v_coord,
    //         u_coord + width, v_coord + height, u_coord, v_coord, u_coord + width, v_coord};
    // } else if (m_flip == 2) {
    //     tex_coords = (float[12]){
    //         u_coord, v_coord, u_coord + width, v_coord, u_coord + width, v_coord + height,
    //         u_coord, v_coord, u_coord + width, v_coord + height, u_coord, v_coord + height};
    // } else if (m_flip == 3) {
    //     tex_coords = (float[12]){
    //         u_coord + width, v_coord, u_coord, v_coord, u_coord, v_coord + height,
    //         u_coord + width, v_coord, u_coord, v_coord + height, u_coord + width, v_coord + height};
    // } else {
    //     std::cout << "Invalid flip value" << std::endl;
    //     // float tex_coords[] = {
    //     //     u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
    //     //     u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord};
    // }
    float tex_coords[4][12] = {
        {u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
         u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord},
        {u_coord + width, v_coord + height, u_coord, v_coord + height, u_coord, v_coord,
         u_coord + width, v_coord + height, u_coord, v_coord, u_coord + width, v_coord},
        {u_coord, v_coord, u_coord + width, v_coord, u_coord + width, v_coord + height,
         u_coord, v_coord, u_coord + width, v_coord + height, u_coord, v_coord + height},
        {u_coord + width, v_coord, u_coord, v_coord, u_coord, v_coord + height,
         u_coord + width, v_coord, u_coord, v_coord + height, u_coord + width, v_coord + height},
    };

    float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords[m_flip]);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::ai_activate(Entity* player) {
    switch (m_ai_type) {
        case WALKER:
            ai_walk();
            break;

        case GUARD:
            ai_guard(player);
            break;

        case JUMPER:
            ai_jump();
            break;

        case PATROLLER:
            ai_patrol(m_min_x, m_max_x);
            break;

        default:
            break;
    }
}

void Entity::ai_walk() {
    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
}

void Entity::ai_guard(Entity* player) {
    switch (m_ai_state) {
        case IDLE:
            if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = WALKING;
            break;

        case WALKING:
            if (m_position.x > player->get_position().x) {  // move left
                m_movement          = glm::vec3(-1.0f, 0.0f, 0.0f);
                m_animation_indices = m_animations[WALK];
                m_flip              = 1;
            } else {
                m_movement          = glm::vec3(1.0f, 0.0f, 0.0f);  // move right
                m_animation_indices = m_animations[WALK];
                m_flip              = 0;
            }
            break;

        case ATTACKING:
            break;

        default:
            break;
    }
}

void Entity::ai_jump() {
    if (m_collided_bottom) {
        m_velocity.y = 5.0f;
    }
}

void Entity::ai_patrol(float min_x, float max_x) {
    if (m_position.x < min_x) {  // now move right
        m_movement          = glm::vec3(1.0f, 0.0f, 0.0f);
        m_animation_indices = m_animations[WALK];
        m_flip              = 0;
    } else if (m_position.x > max_x) {  // now move left
        m_movement          = glm::vec3(-1.0f, 0.0f, 0.0f);
        m_animation_indices = m_animations[WALK];
        m_flip              = 1;
    }
}

void Entity::update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map) {
    if (!m_is_active) return;

    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;

    if (m_animation_indices != NULL) {
        if (glm::length(m_movement) != 0) {
            m_animation_time        += delta_time;
            float frames_per_second  = (float)1 / SECONDS_PER_FRAME;

            if (m_animation_time >= frames_per_second) {
                m_animation_time = 0.0f;
                m_animation_index++;

                if (m_animation_index >= m_animation_frames) {
                    m_animation_index = 0;
                }
            }
        }
    }

    // If player is on ladder, allow them to move up and down
    if (map->get_is_on_ladder(m_position)) {
        // print movement
        std::cout << "ladder movement: " << m_movement.x << ", " << m_movement.y << std::endl;
        if (m_movement.y > 0) {
            m_velocity.y = m_movement.y * m_speed;
        } else if (m_movement.y < 0) {
            m_velocity.y = m_movement.y * m_speed;
        } else {
            m_velocity.y = 0;
        }
    } else if (map->get_is_in_water(m_position)) {
        // print movement
        std::cout << "water movement: " << m_movement.x << ", " << m_movement.y << std::endl;
        if (m_movement.y > 0) {
            m_velocity.y = m_movement.y * m_speed * 0.5f;
        } else if (m_movement.y < 0) {
            m_velocity.y = m_movement.y * m_speed * 0.5f;
        } else {
            if (!m_is_jumping) {  // drag player down when not jumping
                m_velocity += m_acceleration * 0.1f * delta_time;
            }
        }
    } else {
        // m_velocity.y = 0;
        m_velocity += m_acceleration * delta_time;
    }

    m_velocity.x = m_movement.x * m_speed;
    // m_velocity.y = m_movement.y * m_speed;

    // We make two calls to our check_collision methods, one for the collidable objects and one for
    // the map.
    m_position.y += m_velocity.y * delta_time;
    check_collision_y(map);
    check_collision_y(objects, object_count);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(map);
    check_collision_x(objects, object_count);

    if (m_entity_type == ENEMY) ai_activate(player);
    // if (m_ai_type == JUMPER) {
    //     // print collision status
    //     std::cout << "collided_bottom: " << std::boolalpha << m_collided_bottom << std::endl;
    //     std::cout << "collided_top: " << std::boolalpha << m_collided_top << std::endl;
    //     std::cout << "collided_left: " << std::boolalpha << m_collided_left << std::endl;
    //     std::cout << "collided_right: " << std::boolalpha << m_collided_right << std::endl;
    // }

    if (m_is_jumping) {
        m_is_jumping = false;

        m_velocity.y += m_jumping_power;
    }

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    // scale
    m_model_matrix = glm::scale(m_model_matrix, glm::vec3(m_scale, m_scale, 1.0f));
    // m_model_matrix = glm::scale(m_model_matrix, glm::vec3(m_width, m_height, 1.0f));
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity)) {
            if (m_entity_type == PLAYER) {
                std::cout << "velocity" << m_velocity.x << ", " << m_velocity.y << std::endl;
            }
            float y_distance                   = fabs(m_position.y - collidable_entity->get_position().y);
            float y_overlap                    = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->get_height() / 2.0f));
            float y_collidable_entity_velocity = collidable_entity->get_velocity().y;
            if (m_velocity.y < 0 || y_collidable_entity_velocity > 0) {
                // m_position.y += y_overlap;
                m_velocity.y      = 0;
                m_collided_bottom = true;
                std::cout << "collided_bottom with enemy" << std::endl;
                collidable_entity->m_is_active = false;  // turn off enemy
                collidable_entity->set_is_alive(false);  // turn off enemy
            } else if (m_velocity.y > 0 || y_collidable_entity_velocity < 0) {
                // m_position.y -= y_overlap;
                m_velocity.y   = 0;
                m_collided_top = true;
                std::cout << "collided_top with enemy" << std::endl;
                m_is_alive = false;  // lose game
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity)) {
            float x_distance                   = fabs(m_position.x - collidable_entity->get_position().x);
            float x_overlap                    = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->get_width() / 2.0f));
            float x_collidable_entity_velocity = collidable_entity->get_velocity().x;
            if (m_velocity.x > 0 || x_collidable_entity_velocity < 0) {
                // m_position.x -= x_overlap;
                m_velocity.x     = 0;
                m_collided_right = true;
                std::cout << "collided_right with enemy" << std::endl;
                m_is_alive = false;  // lose game
            } else if (m_velocity.x < 0 || x_collidable_entity_velocity > 0) {
                // m_position.x += x_overlap;
                m_velocity.x    = 0;
                m_collided_left = true;
                std::cout << "collided_left with enemy" << std::endl;
                m_is_alive = false;  // lose game
            }
        }
    }
}

void const Entity::check_collision_y(Map* map) {
    // Probes for tiles above
    glm::vec3 top       = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left  = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom       = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left  = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y   -= penetration_y;
        m_velocity.y    = 0;
        m_collided_top  = true;
    } else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y   -= penetration_y;
        m_velocity.y    = 0;
        m_collided_top  = true;
    } else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y   -= penetration_y;
        m_velocity.y    = 0;
        m_collided_top  = true;
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0) {
        m_position.y      += penetration_y;
        m_velocity.y       = 0;
        m_collided_bottom  = true;
    } else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0) {
        m_position.y      += penetration_y;
        m_velocity.y       = 0;
        m_collided_bottom  = true;
    } else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0) {
        m_position.y      += penetration_y;
        m_velocity.y       = 0;
        m_collided_bottom  = true;
    }
}

void const Entity::check_collision_x(Map* map) {
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left  = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0) {
        m_position.x    += penetration_x;
        m_velocity.x     = 0;
        m_collided_left  = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0) {
        m_position.x     -= penetration_x;
        m_velocity.x      = 0;
        m_collided_right  = true;
    }
}

void Entity::render(ShaderProgram* program) {
    if (!m_is_active) return;
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL) {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[]   = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    float tex_coords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const {
    if (other == this) return false;
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}
