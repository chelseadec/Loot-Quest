#include "Scene.h"

class LevelE : public Scene {
public:
    int ENEMY_COUNT = 0;
    
    ~LevelE();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};

