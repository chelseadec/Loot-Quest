#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity()
{
    m_position     = glm::vec3(0.0f);
    m_velocity     = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);
    
    m_movement = glm::vec3(0.0f);
    
    m_speed = 0;
    m_model_matrix = glm::mat4(1.0f);
}

Entity::~Entity()
{
    delete [] m_animation_up;
    delete [] m_animation_down;
    delete [] m_animation_left;
    delete [] m_animation_right;
    delete [] m_walking;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float) (index % m_animation_cols) / (float) m_animation_cols;
    float v_coord = (float) (index / m_animation_cols) / (float) m_animation_rows;
    
    // Step 2: Calculate its UV size
    float width = 1.0f / (float) m_animation_cols;
    float height = 1.0f / (float) m_animation_rows;
    
    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    
    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };
    
    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::ai_activate(Entity *player)
{
    switch (m_ai_type)
    {
        case WALKER:
            ai_walker(player);
            break;
            
        case GUARD:
            ai_guard(player);
            break;
            
        case JUMPER:
            ai_jumper(player);
            break;
            
        case STANDING:
            ai_standing(player);
            break;
            
        default:
            break;
    }
}

void Entity::ai_jumper(Entity *player)
{
    if (m_collided_bottom == true) {
        m_is_jumping = true;
    }
    ai_walker(player);
}

void Entity::ai_walker(Entity *player)
{
    if (m_ai_state != SQUARE) {
        if (walk_vertical) {
            if (go_forward){
                m_animation_indices =  m_walking[3];
                m_movement = glm::vec3(0.0f, -1.0f, 0.0f);
                if (m_position.y < -3.15f ) {
                    go_forward = !go_forward;
                    m_movement = glm::vec3(0.0f, 1.0f, 0.0f);}
            }
            else {
                m_animation_indices =  m_walking[2];
                m_movement = glm::vec3(0.0f, 1.0f, 0.0f);
                if (m_position.y > -1.15f ) {
                    go_forward = !go_forward;
                    m_movement = glm::vec3(0.0f, -1.0f, 0.0f);}
            }
        }
        else {
            if (go_forward){
                m_animation_indices =  m_walking[0];
                m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
                if (m_position.x < 4.2f ) {
                    go_forward = !go_forward;
                    m_movement = glm::vec3(1.0f, 0.0f, 0.0f);}
                }
            else {
                m_animation_indices =  m_walking[1];
                m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
                if (m_position.x > 7.2f ) {
                    go_forward = !go_forward;
                    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);}
            }
        }
    }
    
    else { // pace in a square shape
        
        
        
        if (left_edge){
            LOG("TUREEEE");
            LOG("X");
            LOG(m_position.x);
            LOG("Y");
            LOG(m_position.y);
            m_animation_indices =  m_walking[2];
            m_movement = glm::vec3(0.0f, 1.0f, 0.0f);
            if (m_position.x == 11.9f && m_position.y == -4.2f) {
                LOG("IM HHHHHHHHHHH");
                left_edge = false;
                top_edge = true;
            }
        }
        if (top_edge){
            m_animation_indices =  m_walking[1];
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            if (m_position.x == 13.9f && m_position.y == -4.2f){
                top_edge = false;
                right_edge = true;
            }
        }
        if (right_edge){
            m_animation_indices =  m_walking[3];
            m_movement = glm::vec3(0.0f, -1.0f, 0.0f);
            if (m_position.x == 13.9f && m_position.y == -6.2f){
                right_edge = false;
                bottom_edge = true;
            }
        }
        if (bottom_edge){
            m_animation_indices =  m_walking[0];
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            if (m_position.x == 11.9f && m_position.y == -6.9f){
                bottom_edge = false;
                left_edge = true;
            }
        }
    }
}

void Entity::ai_guard(Entity *player)
{
    switch (m_ai_state) {
        case IDLE:
            if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = WALKING;
            break;
            
        case WALKING:
            if (m_position.x > player->get_position().x) {
                m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            } else {
                m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            break;
            
        case ATTACKING:
            break;
            
        default:
            break;
    }
}

void Entity::ai_standing(Entity *player){
    m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Entity::update(float delta_time, Entity *player, Entity *objects, int object_count, Map *map)
{
    if (!m_is_active) return;
 
    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;
    
    if (m_entity_type == ENEMY) {
        ai_activate(player);
    }
    
    
    if (m_animation_indices != NULL)
    {
        if (glm::length(m_movement) != 0)
        {
            m_animation_time += delta_time;
            float frames_per_second = (float) 1 / SECONDS_PER_FRAME;
            
            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;
                
                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
        }
    }
    
    // Our character moves from left to right, so they need an initial velocity
    m_velocity.x = m_movement.x * m_speed;
    m_velocity.y = m_movement.y * m_speed;
    
    // Now we add the rest of the gravity physics
    m_velocity += m_acceleration * delta_time;
    
    m_position.y += m_velocity.y * delta_time;
    check_collision_y(objects, object_count);
    
//    if (m_entity_type == PLAYER && objects->m_entity_type == ENEMY) {
//        if (m_collided_bottom == true) {
//            LOG("WAAOOOOOHHH");
//            objects->deactivate();
//        }
//        else if (m_collided_top == true) {
//            LOG("YURRRRRRRRRR2222");
//            player->deactivate();
//         }
//    }
   
//    if (m_entity_type == ENEMY && objects->m_entity_type == PLAYER) {
//        LOG("OKKKKKKKKKKKKKKK");
//        if (m_collided_bottom == true) {
//            LOG("HELOOOOOOOOOOOOOO");
////            objects->set_death_count(++(objects->m_death_count));
//            objects->deactivate();
//        }
//    }
//    check_collision_y(map);
    
    m_position.x += m_velocity.x * delta_time;
    check_collision_x(objects, object_count);
    

//    if (m_collided_left == true || m_collided_right == true) {
//        LOG("YURRRRRRRRRR");
//        player->deactivate();
//    }
    
//    check_collision_x(map);
    
    // Jump
    if (m_is_jumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        m_is_jumping = false;
        
        // STEP 2: The player now acquires an upward velocity
        m_velocity.y += m_jumping_power;
    }
    
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
}

void const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->get_position().y + 1);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0) {
                if (collidable_entities->m_entity_type == PLAYER){
                    m_animation_indices = m_walking[2];
                    m_ai_type = STANDING;
                    m_position.y   -= fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
                }
                else { m_position.y   -= y_overlap;}
               
                m_velocity.y    = 0;
                m_collided_top  = true;
               
            } else if (m_velocity.y < 0) {
                if (collidable_entities->m_entity_type == PLAYER){
                    m_animation_indices = m_walking[3];
                    m_ai_type = STANDING;
                    m_position.y   += fabs(y_distance - (m_height / 1.0f) - (collidable_entity->m_height / 1.0f));
                }
                else {m_position.y += y_overlap;}
                m_velocity.y       = 0;
                m_collided_bottom  = true;
                
                
//                if (m_ai_type == JUMPER && collidable_entity->get_entity_type() == PLATFORM) ai_jumper();
            }
        }
    }
}

void const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->get_position().x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->get_width() / 2.0f));
            if (m_velocity.x > 0) {
                m_position.x     -= x_overlap;
                m_velocity.x      = 0;
                m_collided_right  = true;
                if (collidable_entities->m_entity_type == PLAYER){
                    LOG("11111111111111");
                    m_animation_indices = m_walking[1];
                    m_ai_type = STANDING;
                }
            } else if (m_velocity.x < 0) {
                m_position.x    += x_overlap;
                m_velocity.x     = 0;
                m_collided_left  = true;
                m_animation_indices = m_walking[0];
                m_ai_type = STANDING;
            }
        }
    }
}

void const Entity::check_collision_y(Map *map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        
    }
}

void const Entity::check_collision_x(Map *map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
}
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}

void Entity::render(ShaderProgram *program)
{
    if (!m_is_active) return;
    
    program->SetModelMatrix(m_model_matrix);
    
    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }
    
    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool const Entity::check_collision(Entity *other) const
{
    // If we are checking with collisions with ourselves, this should be false
    if (other == this) return false;
    
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;
    
    float x_distance = fabs(m_position.x - other->get_position().x) - ((m_width  + other->get_width())  / 2.0f);
    float y_distance = fabs(m_position.y - other->get_position().y) - ((m_height + other->get_height()) / 2.0f);
    
    return x_distance < 0.0f && y_distance < 0.0f;
}
