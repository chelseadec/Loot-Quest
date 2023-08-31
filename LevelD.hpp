#include "Scene.h"

class LevelD : public Scene {
public:
    int ENEMY_COUNT = 1,
        OBJECT_COUNT = 1;
    
    ~LevelD();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
