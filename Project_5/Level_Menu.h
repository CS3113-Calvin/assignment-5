#include "Scene.h"

class LevelMenu : public Scene {
   public:
    // ————— CONSTRUCTOR ————— //
    ~LevelMenu();

    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
