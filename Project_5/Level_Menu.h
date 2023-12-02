#include "Scene.h"

class LevelMenu : public Scene {
   public:
    // ————— CONSTRUCTOR ————— //
    ~LevelMenu();

    // ————— METHODS ————— //
    void initialize(Entity* player) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
