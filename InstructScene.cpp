#include "InstructScene.hpp"
#include "Utility.h"
#define LOG(argument) std::cout << argument << '\n'


#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LevelF_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

LevelF::~LevelF()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
//    Mix_FreeChunk(m_state.click_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelF::initialise()
{
//    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/tileset_4.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LevelF_DATA, map_texture_id, 1.0f, 13, 8);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(4.9f, -8.9f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->m_speed = 2.0f;
    m_state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/player.png");
    
    // Walking
    m_state.player->m_walking[m_state.player->LEFT]  = new int[4] { 5, 6, 7, 8     };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[4] { 9, 10, 11,12   };
    m_state.player->m_walking[m_state.player->UP]    = new int[4] { 13, 14, 15, 16 };
    m_state.player->m_walking[m_state.player->DOWN]  = new int[4] { 1, 2, 3,  4    };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->DOWN];  // start George looking left
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
    GLuint enemy_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/ghost.png");
    
    m_state.enemies = new Entity[ENEMY_COUNT];
    m_state.enemies[0].set_entity_type(ENEMY);
    m_state.enemies[0].set_ai_type(GUARD);
    m_state.enemies[0].set_ai_state(IDLE);
    m_state.enemies[0].m_texture_id = enemy_texture_id;
    m_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_state.enemies[0].set_speed(1.0f);
    m_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_state.bgm = Mix_LoadMUS("/Users/chelsea/Desktop/Final 2/SDLProject/assets/bgm(games).mp3");

    Mix_PlayMusic(m_state.bgm, -1);
    

    m_state.jump_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/jump.wav");
    m_state.click_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/game_click.wav");
    m_state.ans_correct_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Answer_Correct.wav");
    m_state.ans_incorrect_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Answer_Incorrect.wav");
   
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);

}

void LevelF::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);
}

void LevelF::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
}
