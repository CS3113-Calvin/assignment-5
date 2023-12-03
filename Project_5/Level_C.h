#include "Scene.h"

class LevelC : public Scene {
   public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 1;

    // ————— CONSTRUCTOR ————— //
    ~LevelC();

    // ————— METHODS ————— //
    void initialize(Entity* player, GLuint g_fontsheet_texture_id) override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
