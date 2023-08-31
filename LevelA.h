#include "Scene.h"

class LevelA : public Scene {
public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 0,
        OBJECT_COUNT = 1,
        TEXTBOX_COUNT = 1,
        FONTBANK_SIZE = 16,
        CLICK_COUNT = 0;
    
    bool levelA_next = true;
    
    // ————— CONSTRUCTOR ————— //
    ~LevelA();
    
    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position);

};
