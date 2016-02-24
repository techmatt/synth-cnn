
struct ModelInstance
{
    ModelInstance()
    {
        while (debugColor.length() < 0.5f)
        {
            debugColor = vec3f(util::randomUniformf(), util::randomUniformf(), util::randomUniformf());
        }
    }

    const ModelData *model;
    mat4f transform;

    vec3f anchor;
    float scale;

    vec3f debugColor;
};

struct Scene
{
    void addArchitecture(AppState &state, const string &architectureName, float scale);
    
    void addRandomModel(AppState &state, float scale);

    void render(AppState &state);

    vector<ModelInstance> models;
    int centeralModelIndex;
};

struct SceneGenerator
{
    Scene makeRandomScene(AppState &state);
};