#include "Scene.h"

class LevelF : public Scene {
public:
    int ENEMY_COUNT = 0;
    
    ~LevelF();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};

