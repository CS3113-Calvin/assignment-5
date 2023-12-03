#include "Scene.h"

// GLuint g_fontsheet_texture_id;

class LevelMenu : public Scene {
   public:
    // ————— CONSTRUCTOR ————— //
    ~LevelMenu();

    // ————— METHODS ————— //
    void initialize(Entity* player, GLuint g_fontsheet_texture_id) override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
