#include "LevelA.h"
#include "Utility.h"
#define LOG(argument) std::cout << argument << '\n'


#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LEVEL_DATA[] =
{
    1,   1, 2,  3,  13,  3,  6,  7,  8,  9, 10, 11, 12, 12,
    35, 35, 35, 16, 13,  3, 19,  8,  8,  8,  8, 24, 24, 24,
    52, 52, 52, 52, 13,  3, 52, 52, 52, 37, 37, 37, 37, 37,
    52, 52, 52, 20, 33, 42, 23, 52, 49, 50, 51, 51, 51, 51,
    22, 22, 22, 45, 14, 14, 28, 22, 22, 22, 22, 22, 22, 22,
    35, 35, 35, 27, 14, 14, 15, 35, 35, 35, 35, 35, 35, 35,
    78, 79, 80, 26, 56, 15, 16, 85, 86, 87, 88, 77, 77, 77,
    91, 92, 93, 94, 13,  3, 97, 98, 99, 100, 101, 102, 103, 103
};

LevelA::~LevelA()
{
    delete [] m_state.enemies;
    delete [] m_state.objects;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeChunk(m_state.click_sfx);
    Mix_FreeChunk(m_state.transition_sfx);
    Mix_FreeChunk(m_state.ans_correct_sfx);
    Mix_FreeChunk(m_state.ans_incorrect_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelA::draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void LevelA::initialise()
{
//    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
//    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Title_Image.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 13, 8);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(0.3f, -4.3f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->m_speed = 2.0f;
    m_state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/player.png");
    
    // Walking
    m_state.player->m_walking[m_state.player->LEFT]  = new int[4] { 5, 6, 7, 8     };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[4] { 9, 10, 11,12   };
    m_state.player->m_walking[m_state.player->UP]    = new int[4] { 13, 14, 15, 16 };
    m_state.player->m_walking[m_state.player->DOWN]  = new int[4] { 1, 2, 3, 4     };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start George looking right
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
     Object stuff */
    GLuint object_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/final_treasure.png"); // saturation 28.74
    
    m_state.objects = new Entity[OBJECT_COUNT];
    m_state.objects[0].set_entity_type(OBJECT);
    m_state.objects[0].m_texture_id = object_texture_id;
    m_state.objects[0].set_position(glm::vec3(4.49f, -4.4f, 0.0f));
    m_state.objects[0].set_movement(glm::vec3(0.0f));
    m_state.objects[0].set_speed(1.0f);
    m_state.objects[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    
    
    /**
     Textbox stuff */
//    GLuint textbox_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final 2/SDLProject/assets/textbox2.png"); // saturation 28.74
//    
//    m_state.textbox = new Entity[TEXTBOX_COUNT];
//    m_state.textbox[0].set_entity_type(TEXTBOX);
//    m_state.textbox[0].m_texture_id = textbox_texture_id;
//    m_state.textbox[0].set_position(glm::vec3(4.55f, -6.47f, 0.0f));
//    m_state.textbox[0].set_movement(glm::vec3(0.0f));
//    m_state.textbox[0].set_speed(1.0f);
//    m_state.textbox[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    
    

    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_state.bgm = Mix_LoadMUS("/Users/chelsea/Desktop/Final 2/SDLProject/assets/bgm(games).mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    m_state.jump_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/jump.wav");
    m_state.click_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/game_click.wav");
    m_state.transition_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/route_transition.wav");
    m_state.ans_correct_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Answer_Correct.wav");
    m_state.ans_incorrect_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/Answer_Incorrect.wav");
//    m_state.typing_click_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final 2/SDLProject/assets/typing_click.wav");



}

void LevelA::update(float delta_time)
{
//    for (int i = 0; i < ENEMY_COUNT; ++i){
////        m_state.player->check_collision(&(m_state.enemies[i]));
////        m_state.enemies[i].check_collision_x(m_state.map);
//        if (m_state.enemies[i].get_ai_type() == JUMPER &&  m_state.enemies[i].m_collided_bottom == true) { m_state.enemies[i].ai_jumper(m_state.player);
//        }
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);
    m_state.objects[0].update(delta_time, m_state.player, m_state.player, 1, m_state.map);
    
//    m_state.textbox[0].update(delta_time, m_state.player, NULL, NULL, m_state.map);
//    m_state.enemies[0].update(delta_time, m_state.player, m_state.player, 1, m_state.map);
       
//    }
//    for (int i = 0; i < ENEMY_COUNT; i++)
//    {
//        m_state.enemies[i].update(delta_time, m_state.player, NULL, NULL, m_state.map);
//    }
//    
}


void LevelA::render(ShaderProgram *program)
{    
    m_state.map->render(program);
    m_state.player->render(program);
//    m_state.objects[0].render(program);
//    m_state.textbox[0].render(program);
//    for (int i = 0; i < ENEMY_COUNT; ++i) m_state.enemies[i].render(program);
    
}
