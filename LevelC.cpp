#include "LevelC.hpp"
#include "Utility.h"
#define LOG(argument) std::cout << argument << '\n'


#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LEVELC_DATA[] =
{
    1,   1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 24, 24,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 37, 37,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 64,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 77,
    78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 90,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 103
};

LevelC::~LevelC()
{
    delete [] m_state.enemies;
    delete [] m_state.objects;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeChunk(m_state.click_sfx);
    Mix_FreeChunk(m_state.transition_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelC::initialise()
{
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/forest.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 13, 8);

  
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(6.0f, -7.6f, 1.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->m_speed = 2.0f;
    m_state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/player.png");
    
    // Walking
    m_state.player->m_walking[m_state.player->LEFT]  = new int[4] { 5, 6, 7, 8     };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[4] { 9, 10, 11,12   };
    m_state.player->m_walking[m_state.player->UP]    = new int[4] { 13, 14, 15, 16 };
    m_state.player->m_walking[m_state.player->DOWN]  = new int[4] { 1, 2, 3,  4    };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->UP];  // start George looking down
    m_state.player->m_animation_frames = 3;
    m_state.player->m_animation_index  = 0;
    m_state.player->m_animation_time   = 0.0f;
    m_state.player->m_animation_cols   = 4;
    m_state.player->m_animation_rows   = 4;
    m_state.player->set_height(0.8f);
    m_state.player->set_width(0.8f);
    
    // Jumping
    m_state.player->m_jumping_power = 5.0f;
    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/challenger_sprite_forest.png");
    
    m_state.enemies = new Entity[ENEMY_COUNT];
    m_state.enemies[0].set_entity_type(ENEMY);
    m_state.enemies[0].set_ai_type(WALKER);
    m_state.enemies[0].m_texture_id = enemy_texture_id;
    m_state.enemies[0].set_position(glm::vec3(9.5f, -1.15f, -1.0f));
    m_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_state.enemies[0].set_speed(1.0f);
    m_state.enemies[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Walking
    m_state.enemies[0].m_walking[m_state.enemies[0].LEFT]  = new int[4] { 5, 6, 7, 8     };
    m_state.enemies[0].m_walking[m_state.enemies[0].RIGHT] = new int[4] { 9, 10, 11,12   };
    m_state.enemies[0].m_walking[m_state.enemies[0].UP]    = new int[4] { 13, 14, 15, 16 };
    m_state.enemies[0].m_walking[m_state.enemies[0].DOWN]  = new int[4] { 1, 2, 3,  4    };

    m_state.enemies[0].m_animation_indices =  m_state.enemies[0].m_walking[m_state.enemies[0].DOWN];  // start Challenger 1 looking down
    m_state.enemies[0].m_animation_frames = 3;
    m_state.enemies[0].m_animation_index  = 0;
    m_state.enemies[0].m_animation_time   = 0.0f;
    m_state.enemies[0].m_animation_cols   = 4;
    m_state.enemies[0].m_animation_rows   = 4;
    m_state.enemies[0].set_height(0.8f);
    m_state.enemies[0].set_width(0.8f);
    
    // Jumping
    m_state.enemies[0].m_jumping_power = 5.0f;
    
    
    /**
     Object stuff */
    GLuint object_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/image3.png");
    
    m_state.objects = new Entity[OBJECT_COUNT];
    m_state.objects[0].set_entity_type(OBJECT);
    m_state.objects[0].m_texture_id = object_texture_id;
    m_state.objects[0].set_position(glm::vec3(2.55f, -1.40f, 0.0f));
    m_state.objects[0].set_movement(glm::vec3(0.0f));
    m_state.objects[0].set_speed(1.0f);
    m_state.objects[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_state.bgm = Mix_LoadMUS("/Users/chelsea/Desktop/Final 2/SDLProject/assets/bgm(games).mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    LOG("IM HERRRREEE");
    
    m_state.jump_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/jump.wav");
    m_state.click_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/game_click.wav");
    m_state.transition_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/route_transition.wav");
    m_state.ans_correct_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Answer_Correct.wav");
    m_state.ans_incorrect_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Answer_Incorrect.wav");
//    m_state.typing_click_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/typing_click.wav");

}

void LevelC::update(float delta_time)
{
//    for (int i = 0; i < ENEMY_COUNT; ++i){
//        if (m_state.enemies[i].get_ai_type() == JUMPER &&  m_state.enemies[i].m_collided_bottom == true) { m_state.enemies[i].ai_jumper(m_state.player);
//        }
        m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);
        m_state.enemies[0].update(delta_time, m_state.player,  m_state.player, 1, m_state.map);
        m_state.objects[0].update(delta_time, m_state.player,  m_state.player, 1, m_state.map);
       
//    }
   
}

void LevelC::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    m_state.enemies[0].render(program);
    m_state.objects[0].render(program);
}
