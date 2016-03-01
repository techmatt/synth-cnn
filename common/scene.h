
struct ModelInstance
{
    ModelInstance()
    {
        while (debugColor.length() < 0.5f)
        {
            debugColor = vec3f(util::randomUniformf(), util::randomUniformf(), util::randomUniformf());
        }
    }

    void render(AppState &state, const Cameraf &camera) const;

    const ModelData *model;
    mat4f transform;

    vec3f anchor;
    float scale;

    vec3f debugColor;

    OBB3f worldBBox;
};

struct MitsubaOptions
{
    float minReflectance;
    float maxReflectance;
    bool randomizeMaterialGroups;
};

struct Scene
{
    void addArchitecture(AppState &state, const string &architectureName, float scale);
    
    void addRandomModel(AppState &state, float scale);

    void render(AppState &state, const Cameraf &camera) const;

    void saveMitsuba(AppState &state, const string &filename, const Cameraf &camera) const;

    bool valid(AppState &state) const;

    vector<ModelInstance> objects;
    int mainObjectIndex;
};

struct SceneGenerator
{
    Scene makeRandomScene(AppState &state);
};
