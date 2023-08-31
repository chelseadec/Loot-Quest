#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LOG(argument) std::cout << argument << '\n'


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <list>
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <iostream>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "MainMenu.hpp"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.hpp"
#include "LevelD.hpp"
#include "IntroScene.hpp"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480,
          FONTBANK_SIZE = 16;

const float BG_RED     = 0.0f,
            BG_GREEN   = 0.0f,
            BG_BLUE    = 0.0f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl",
           FONT_FILEPATH[] = "/Users/chelsea/Desktop/Final 2/SDLProject/assets/font1.png";

const float MILLISECONDS_IN_SECOND = 1000.0;


// ––––– GLOBAL VARIABLES ––––– //
int g_frame_counter;
int g_death_count = 0;
std::string val = "Hello";

Scene  *g_current_scene;

Level0 *g_level0;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;
LevelD *g_levelD;
LevelE *g_levelE; // intro(welcome) Scene

Effects *g_effects;
Scene   *g_levels[6];

SDL_Window* g_display_window;
bool g_game_is_running = true,
     is_game_running = true,
     final_lvl_completed = false,
     display_text = false,
     display_instructions = false,
     forest = false,
     cave = false,
     meadow = false,
     g_usr_typing = true,
     g_correct_ans1 = false,
     g_correct_ans2 = false,
     g_correct_ans3 = false;



int count = 0,
    g_incorrect_ans_count = 0,
    g_correct_ans_count = 0;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;



float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

std::string g_usr_response;
std::string g_filtered_response;
std::string g_answer_1 = "an egg";
std::string g_answer_2 = "wind";
std::string g_answer_3 = "footsteps";

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}


void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
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

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("LOOT QUEST",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_program.SetProjectionMatrix(g_projection_matrix);
    g_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_program.programID);
    
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_level0  = new Level0();
    g_levelA  = new LevelA();
    g_levelB  = new LevelB();
    g_levelC  = new LevelC();
    g_levelD  = new LevelD();
    g_levelE  = new LevelE();
    
    g_levels[0] = g_level0;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    g_levels[4] = g_levelD;
    g_levels[5] = g_levelE;

   
    // Start at level 0
    switch_to_scene(g_levels[0]);
    g_effects = new Effects(g_projection_matrix, g_view_matrix);
    g_effects->start(FADEOUT, 0.005f);
    
    g_frame_counter = 0;
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
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
                        // Jump
                        if (g_current_scene->m_state.player->m_collided_bottom)
                        {
                            g_current_scene->m_state.player->m_is_jumping = true;
                            Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                        }
                        break;
                    case SDLK_RETURN:
                
                        if (g_current_scene == g_levels[0]){
                            Mix_PlayChannel(-1, g_current_scene->m_state.click_sfx, 0);
                            switch_to_scene(g_levels[5]);
                        }
                        else if (g_current_scene == g_levels[5]) {
                            Mix_PlayChannel(-1, g_current_scene->m_state.click_sfx, 0);
                            ++count;
                            if (count == 4){
                                switch_to_scene(g_levels[1]);
                            }
                            if (count == 10 && (g_incorrect_ans_count == 0)){
                                switch_to_scene(g_levels[3]);
                                g_usr_response.erase();
                            }
                            if (count == 16 && (g_incorrect_ans_count == 0)){
                                switch_to_scene(g_levels[4]);
                                g_usr_response.erase();
                            }
                            if (count == 24 && (g_incorrect_ans_count == 0)) {
                                switch_to_scene(g_levels[5]);
                            }
                        }
                        break;
                        
                    case SDLK_BACKSPACE: // Need to fix this; only pops once
                        if (g_current_scene == g_levels[5]){
                            g_usr_response.pop_back();
                            g_filtered_response.pop_back();
                        }
                        break;
                    
                    default:
                        break;
                }
            case SDL_TEXTINPUT:
                g_usr_typing = true;
                Mix_PlayChannel(-1, g_current_scene->m_state.typing_click_sfx, 0);
//                if (g_current_scene == g_levels[5]){
                    g_usr_response.push_back(event.text.text[0]);
                    
//                }
                break;
               
//            case SDLK_BACKSPACE:
//                LOG("TRUUUUUUUEEEEEE");
//                if (g_current_scene == g_levels[5]){
//                    LOG("INNNNN HEEEEEEEEEEEE");
//                    if (!g_usr_response.empty()) {
//                        g_usr_response.pop_back();
//                        LOG("CUURRRRR RESPONNNNSSSEE");
//                        LOG(g_usr_response);
//                    }
//                }
//                break;
                
            default:
                break;
        }
    }
    


    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_current_scene->m_state.player->m_movement.x = -1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_current_scene->m_state.player->m_movement.x = 1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
    }
    
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_current_scene->m_state.player->m_movement.y = -1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->DOWN];
    }
    else if (key_state[SDL_SCANCODE_UP])
    {
        g_current_scene->m_state.player->m_movement.y = 1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->UP];
    }

    
    if (glm::length(g_current_scene->m_state.player->m_movement) > 1.0f)
    {
        g_current_scene->m_state.player->m_movement = glm::normalize(g_current_scene->m_state.player->m_movement);
    }
}

std::string filter_input(std::string& val) {
    std::string new_str="";
    for (std::string::iterator it = val.begin(); it != val.end(); ++it){
        std::list<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v','w', 'x', 'y', 'z', ' '};
        for (auto i = alphabet.begin(); i != alphabet.end(); ++i){
            if (*it == *i) {
                new_str.push_back(*i);
            }
        }
    }
    return new_str;
    
}

void update()
{
    
    g_frame_counter++;
    
    LOG("COUNT");
    LOG(count);
    if (g_current_scene == g_levels[5] && (count ==  8) ) {
        if (filter_input(g_usr_response) == g_answer_1) {
            g_correct_ans1 = true;
        }
    }
    else if (g_current_scene == g_levels[5] && (count ==  15) ) {
        if (filter_input(g_usr_response) == g_answer_2) {
            g_correct_ans2 = true;
        }
    }
    else if (g_current_scene == g_levels[5] && (count ==  20) ) {
        if (filter_input(g_usr_response) == g_answer_3) {
            g_correct_ans3 = true;
        }
    }
    
    
    LOG("NEW SIZE");
    LOG(filter_input(g_usr_response).size());
    
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);
        
        
//        if (g_is_colliding_bottom == false && g_current_scene->m_state.player->m_collided_bottom) g_effects->start(SHAKE, 1.0f);
        
        g_is_colliding_bottom = g_current_scene->m_state.player->m_collided_bottom;
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);
    
    if (g_current_scene->m_state.player->get_position().x > LEVEL1_LEFT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, 3.75, 0));
    } else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    }
    
//    LOG("X");
//    LOG(g_current_scene->m_state.player->get_position().x);
    // Allow player to swap between scenes
    if (g_current_scene == g_levels[1]) {
//        if (g_current_scene->m_state.player->get_position().x > 1.0f) {
//            display_instructions = true;
//            switch_to_scene(g_levels[5]);
//        }
        if (g_current_scene->m_state.player->get_position().x >= 8.4f) {
            Mix_PlayChannel(-1, g_current_scene->m_state.transition_sfx, 0);
            switch_to_scene(g_levelB);}
        
        else if (g_current_scene->m_state.player->get_position().y >= 0.5f) {
            Mix_PlayChannel(-1, g_current_scene->m_state.transition_sfx, 0);
            switch_to_scene(g_levelC);}
        
        else if (g_current_scene->m_state.player->get_position().y <= -7.7f) {
            Mix_PlayChannel(-1, g_current_scene->m_state.transition_sfx, 0);
            switch_to_scene(g_levelD);}
        
    }
    
    if (g_current_scene == g_levels[2]) { // meadow
//
        if (g_current_scene->m_state.player->get_position().y > 0.25f || (g_current_scene->m_state.player->get_position().x < -0.08)) {
            Mix_PlayChannel(-1, g_current_scene->m_state.transition_sfx, 0);
            switch_to_scene(g_levelA);
            g_current_scene->m_state.player->set_position(glm::vec3(8.3f, -4.4f, 0.0f));
        }
        
    }
    if (g_current_scene == g_levels[3]) { // forest
        
        if (g_current_scene->m_state.player->get_position().y < -7.6f) {
            Mix_PlayChannel(-1, g_current_scene->m_state.transition_sfx, 0);
            switch_to_scene(g_levelA);
            g_current_scene->m_state.player->set_position(glm::vec3(4.5f, -0.05f, 0.0f));
        }
        
    }
    
    if (g_current_scene == g_levels[4]) { // cave
        
        if (g_current_scene->m_state.player->get_position().y > 0.6f) {
            Mix_PlayChannel(-1, g_current_scene->m_state.transition_sfx, 0);
            switch_to_scene(g_levelA);
            g_current_scene->m_state.player->set_position(glm::vec3(4.5f, -7.4f, 0.0f));
        }
    }
    
//    if (g_current_scene == g_level0) {LOG("LEVEL 0");}
//    if (g_current_scene == g_levelA) {LOG("LEVEL A");}
//    if (g_current_scene == g_levelB) {LOG("LEVEL B");}
//    if (g_current_scene == g_levelC) {LOG("LEVEL C");}
//    if (g_current_scene == g_levelD) {LOG("LEVEL D");}
//    if (g_current_scene == g_levelE) {LOG("LEVEL E");}

    if (g_current_scene->m_state.enemies[0].get_ai_type() == STANDING){
//        LOG("IM HEEEEEERRRRRREEEEE");
        if (g_current_scene == g_levels[3]){ // Level C (Forest)
            forest = true;
        }
        else if (g_current_scene == g_levels[2]){ // Level B (Meadow)
            meadow = true;
        }
        else if (g_current_scene == g_levels[4]) {// Level D (Cave)
            cave = true;
        }
//            LOG("YESSSSSSSS");
        display_text = true;
        switch_to_scene(g_levels[5]);
        
    }
    
    g_view_matrix = glm::translate(g_view_matrix, g_effects->m_view_offset);
    
}


void render()
{
    GLuint font_texture_id = Utility::load_texture(FONT_FILEPATH);
    g_program.SetViewMatrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
    
//    bool appear = true;
    
//    std::string usrr_response;
//    std::cout << "Enter your response: " << std::endl;
//    std::cin >> usrr_response;
//    LOG("NEW RESPONNNNNSEEEE");
//    LOG(usrr_response);
    
    std::list<int> range = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    for (auto i = range.begin(); i != range.end(); ++i ) {
        if (g_current_scene == g_levels[0]) {
            draw_text(&g_program, font_texture_id, "Press Enter to Start", 0.30f, 0.0f, glm::vec3(2.25f, -3.75f, 0.0f));
            if ((g_frame_counter % 55) == *i) {
                draw_text(&g_program, font_texture_id, "LOOT QUEST", 0.5f, 0.05f, glm::vec3(2.4f, -2.75f, 0.0f));
            }
        }
    }
  
    if (g_current_scene == g_levels[5] && !display_text){
        // INTRODUCTION
        if (count == 0){
            draw_text(&g_program, font_texture_id, "Welcome, brave adventurer, ", 0.3f, 0.00f, glm::vec3(1.2f, -0.25f, 0.0f));
            draw_text(&g_program, font_texture_id, "to the world of Loot Quest! ", 0.3f, 0.00f, glm::vec3(1.0f, -0.75f, 0.0f));
        }
        else if (count == 1) {
            draw_text(&g_program, font_texture_id, "A realm where mysteries ", 0.3f, 0.00f, glm::vec3(1.5f, -1.5f, 0.0f));
            draw_text(&g_program, font_texture_id, "abound and riddles pave your ", 0.3f, 0.00f, glm::vec3(0.8f, -2.0f, 0.0f));
            draw_text(&g_program, font_texture_id, "path to treasure. ", 0.3f, 0.00f, glm::vec3(2.3f, -2.5f, 0.0f));
        }

        else if (count == 2) {
            draw_text(&g_program, font_texture_id, "I am your guide on this ", 0.3f, 0.00f, glm::vec3(1.55f, -2.25f, 0.0f));
            draw_text(&g_program, font_texture_id, "extraordinary journey. ", 0.3f, 0.00f, glm::vec3(1.75f, -2.75f, 0.0f));
        }

        else if (count == 3) {
            draw_text(&g_program, font_texture_id, "Prepare to embark on a ", 0.3f, 0.00f, glm::vec3(1.6f, -2.75f, 0.0f));
            draw_text(&g_program, font_texture_id, "quest unlike any other,  ", 0.3f, 0.00f, glm::vec3(1.6f, -3.25f, 0.0f));
            draw_text(&g_program, font_texture_id, "where wit and wisdom are ", 0.3f, 0.00f, glm::vec3(1.4f, -3.75f, 0.0f));
            draw_text(&g_program, font_texture_id, "your greatest allies.", 0.3f, 0.00f, glm::vec3(1.8f, -4.25f, 0.0f));
        }
        
        draw_text(&g_program, font_texture_id, "Press Enter to Continue...", 0.25f, 0.00f, glm::vec3(2.0f, -6.75f, 0.0f));
    }

    if (g_current_scene == g_levels[5] && display_text) {
        if (forest){
            // FIRST CHALLENGER (FOREST)
            if (count == 4){ // SET 5 == forest challenger
                draw_text(&g_program, font_texture_id, "Greetings, young seeker!", 0.3f, 0.00f, glm::vec3(1.5f, -2.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "I am the Sage of", 0.3f, 0.00f, glm::vec3(2.7f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "the Whispering Woods, ", 0.3f, 0.00f, glm::vec3(2.1f, -3.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "guardian of ancient ", 0.3f, 0.00f, glm::vec3(2.2f, -3.75f, 1.0f));
                draw_text(&g_program, font_texture_id, " knowledge.", 0.3f, 0.00f, glm::vec3(3.2f, -4.25f, 1.0f));
            }

            else if (count == 5) {

                draw_text(&g_program, font_texture_id, "To prove yourself worthy,", 0.3f, 0.00f, glm::vec3(1.5f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "answer my riddle:", 0.3f, 0.00f, glm::vec3(2.5f, -3.25f, 0.0f));

            }

            else if (count == 6) {
                draw_text(&g_program, font_texture_id, "I'm a vessel without hinges,", 0.3f, 0.00f, glm::vec3(0.8f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "lock, or lid. Yet within", 0.3f, 0.00f, glm::vec3(1.4f, -3.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "me, a golden treasure is hid.", 0.3f, 0.00f, glm::vec3(0.7f, -3.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "What am I?", 0.3f, 0.00f, glm::vec3(3.7f, -4.25f, 0.0f));
            
                //            draw_text(&g_program, font_texture_id, " ", 0.3f, 0.00f, glm::vec3(2.3f, -4.25f, 0.0f));
            }
            else if (count == 7){
                draw_text(&g_program, font_texture_id, "Enter your response:  ", 0.3f, 0.00f, glm::vec3(2.1f, -2.75f, 0.0f));
                
                float count = 0.0f;
                for (std::string::iterator it = g_usr_response.begin(); it != g_usr_response.end(); ++it){
                    std::list<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v','w', 'x', 'y', 'z', ' '};
                    for (auto i = alphabet.begin(); i != alphabet.end(); ++i){
                        if (*it == *i) {
        //                    g_filtered_response.push_back(*it);
                            LOG("FILLTERED!!!!!");
                            LOG(g_usr_response);
                            std::string letter(1, *it);
                            draw_text(&g_program, font_texture_id, letter, 0.3f, 0.00f, glm::vec3(4.2f + count, -3.25f, 0.0f));
                            count += 0.30;
                        }
                    }

                }

            }
            if (g_correct_ans1 && count == 8){
                Mix_PlayChannel(-1, g_current_scene->m_state.ans_correct_sfx, 0);
                draw_text(&g_program, font_texture_id, "CORRECT!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                ++g_correct_ans_count;
            }
            else if (!g_correct_ans1 && count == 8){
                LOG("INNNNNNN");
                Mix_PlayChannel(-1, g_current_scene->m_state.ans_incorrect_sfx, 0);
                draw_text(&g_program, font_texture_id, "INCORRECT!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                ++g_incorrect_ans_count;

            }
            else if (g_incorrect_ans_count > 0) {
                draw_text(&g_program, font_texture_id, "YOU LOSE!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
            }
        }
        // SECOND CHALLENGER (CAVE)
        if (cave){
            if (count == 10){ // SET 5 == forest challenger
                draw_text(&g_program, font_texture_id, "Ah, another soul dares ", 0.3f, 0.00f, glm::vec3(1.8f, -2.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "to tread upon my domain.", 0.3f, 0.00f, glm::vec3(1.5f, -2.75f, 0.0f));
            }

            else if (count == 11) {
                draw_text(&g_program, font_texture_id, "I am the Sorcerer of ", 0.3f, 0.00f, glm::vec3(2.0f, -2.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "the Crystal Caverns,", 0.3f, 0.00f, glm::vec3(2.1f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "master of illusions.", 0.3f, 0.00f, glm::vec3(2.1f, -3.25f, 0.0f));

            }
            else if (count == 12) {
                draw_text(&g_program, font_texture_id, " Solve my riddle: ", 0.3f, 0.00f, glm::vec3(2.3f, -3.25f, 0.0f));
            }

            else if (count == 13) {
                draw_text(&g_program, font_texture_id, "Voiceless it cries, ", 0.3f, 0.00f, glm::vec3(2.1f, -2.00f, 0.0f));
                draw_text(&g_program, font_texture_id, "wingless flutters; toothless", 0.3f, 0.00f, glm::vec3(1.0f, -2.50f, 0.0f));
                draw_text(&g_program, font_texture_id, "bites, mouthless mutters.", 0.3f, 0.00f, glm::vec3(1.4f, -3.00f, 0.0f));
                draw_text(&g_program, font_texture_id, "What is it?", 0.3f, 0.00f, glm::vec3(3.2f, -3.50f, 0.0f));
            }
            else if (count == 14){
                draw_text(&g_program, font_texture_id, "Enter your response:  ", 0.3f, 0.00f, glm::vec3(2.1f, -2.75f, 0.0f));
                
                float count = 0.0f;
                for (std::string::iterator it = g_usr_response.begin(); it != g_usr_response.end(); ++it){
                    std::list<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v','w', 'x', 'y', 'z', ' '};
                    for (auto i = alphabet.begin(); i != alphabet.end(); ++i){
                        if (*it == *i) {
        //                    g_filtered_response.push_back(*it);
                            LOG("FILLTERED!!!!!");
                            LOG(g_usr_response);
                            std::string letter(1, *it);
                            draw_text(&g_program, font_texture_id, letter, 0.3f, 0.00f, glm::vec3(4.2f + count, -3.25f, 0.0f));
                            count += 0.30;
                        }
                    }

                }

            }
            if (g_correct_ans2 && count == 15){
                Mix_PlayChannel(-1, g_current_scene->m_state.ans_correct_sfx, 0);
                draw_text(&g_program, font_texture_id, "CORRECT!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                ++g_correct_ans_count;
            }
            else if (!g_correct_ans2 && count == 15){
                LOG("INNNNNNN");
                Mix_PlayChannel(-1, g_current_scene->m_state.ans_incorrect_sfx, 0);
                draw_text(&g_program, font_texture_id, "INCORRECT!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                ++g_incorrect_ans_count;

            }
            else if (g_incorrect_ans_count > 0) {
                draw_text(&g_program, font_texture_id, "YOU LOSE!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
            }
        }
        // THRID CHALLENGER (MEADOW)
        if (meadow) {
            if (count == 16){ // SET 5 == forest challenger
                draw_text(&g_program, font_texture_id, "Halt! I am the Warrior", 0.3f, 0.00f, glm::vec3(1.6f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "of the Thunderpeak Mountains.", 0.3f, 0.00f, glm::vec3(0.7f, -3.25f, 0.0f));
            }

            else if (count == 17) {

                draw_text(&g_program, font_texture_id, "My blade defends the", 0.3f, 0.00f, glm::vec3(2.0f, -2.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "secrets of this land. ", 0.3f, 0.00f, glm::vec3(2.0f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "Answer my riddle: ", 0.3f, 0.00f, glm::vec3(2.4f, -3.25f, 0.0f));
            }

            else if (count == 18) {
                draw_text(&g_program, font_texture_id, "The more you take,", 0.3f, 0.00f, glm::vec3(2.4f, -2.25f, 0.0f));
                draw_text(&g_program, font_texture_id, "the more you leave behind.", 0.3f, 0.00f, glm::vec3(1.2f, -2.75f, 0.0f));
                draw_text(&g_program, font_texture_id, "What am I?", 0.3f, 0.00f, glm::vec3(3.5f, -3.25f, 0.0f));
            }
            else if (count == 19){
                draw_text(&g_program, font_texture_id, "Enter your response:  ", 0.3f, 0.00f, glm::vec3(2.1f, -2.75f, 0.0f));
                
                float count = 0.0f;
                for (std::string::iterator it = g_usr_response.begin(); it != g_usr_response.end(); ++it){
                    std::list<char> alphabet = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v','w', 'x', 'y', 'z', ' '};
                    for (auto i = alphabet.begin(); i != alphabet.end(); ++i){
                        if (*it == *i) {
        //                    g_filtered_response.push_back(*it);
                            LOG("FILLTERED!!!!!");
                            LOG(g_usr_response);
                            std::string letter(1, *it);
                            draw_text(&g_program, font_texture_id, letter, 0.3f, 0.00f, glm::vec3(4.2f + count, -3.25f, 0.0f));
                            count += 0.30;
                        }
                    }

                }

            }
            if (g_correct_ans3 && count == 20){
                Mix_PlayChannel(-1, g_current_scene->m_state.ans_correct_sfx, 0);
                draw_text(&g_program, font_texture_id, "CORRECT!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                ++g_correct_ans_count;
            }
            else if (!g_correct_ans3 && count == 20){
                LOG("INNNNNNN");
                Mix_PlayChannel(-1, g_current_scene->m_state.ans_incorrect_sfx, 0);
                draw_text(&g_program, font_texture_id, "INCORRECT!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                ++g_incorrect_ans_count;

            }
            if (count == 21) {
                if (!g_correct_ans3){
                    draw_text(&g_program, font_texture_id, "YOU LOSE!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                }
                else if (g_correct_ans3) {
                    draw_text(&g_program, font_texture_id, "YOU WIN!", 0.3f, 0.00f, glm::vec3(3.8f, -3.25f, 0.0f));
                }
            }
        }
        draw_text(&g_program, font_texture_id, "Press Enter to Continue...", 0.25f, 0.00f, glm::vec3(2.0f, -6.75f, 0.0f));
    }

    
    if (display_instructions){
        if (count == 4){ // SET 4 == forest challenger
            draw_text(&g_program, font_texture_id, "In the land of Loot Quest, ", 0.3f, 0.00f, glm::vec3(1.6f, -3.25f, 0.0f));
            draw_text(&g_program, font_texture_id, "three mighty challengers stand", 0.3f, 0.00f, glm::vec3(1.6f, -3.75f, 0.0f));
            draw_text(&g_program, font_texture_id, " in your way,", 0.3f, 0.00f, glm::vec3(1.4f, -4.25f, 0.0f));
        }
    
        else if (count == 5) {
            draw_text(&g_program, font_texture_id, "each guarding a cache ", 0.3f, 0.00f, glm::vec3(4.5f, -0.25f, 1.0f));
            draw_text(&g_program, font_texture_id, "of invaluable loot.", 0.3f, 0.00f, glm::vec3(4.5f, -0.75f, 1.0f));
        }
    
        else if (count == 6) {
            draw_text(&g_program, font_texture_id, "Your task is to unravel ", 0.3f, 0.00f, glm::vec3(4.5f, -0.25f, 0.0f));
            draw_text(&g_program, font_texture_id, "their enigmatic riddles,", 0.3f, 0.00f, glm::vec3(4.5f, -0.75f, 0.0f));
            draw_text(&g_program, font_texture_id, "proving your cunning", 0.3f, 0.00f, glm::vec3(4.5f, -1.25f, 0.0f));
            draw_text(&g_program, font_texture_id, "and intellect.", 0.3f, 0.00f, glm::vec3(4.5f, -1.75f, 0.0f));
        }

    }
    

    
 
    glUseProgram(g_program.programID);
    g_current_scene->render(&g_program);
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_level0;
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_levelD;
    delete g_levelE;
    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        
//        if (g_current_scene->m_state.next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id]);
//
        render();
    }
    
    shutdown();
    return 0;
}
