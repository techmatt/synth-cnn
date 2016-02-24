
#include "main.h"

void Scene::addArchitecture(AppState &state, const string &architectureName, float scale)
{
    ModelInstance instance;

    instance.anchor = vec3f::origin;
    instance.model = &state.modelDatabase.getCategory("architecture").getModel(architectureName);

    instance.transform = mat4f::scale(scale);

    models.push_back(instance);
}

void Scene::addRandomModel(AppState &state, float scale)
{
    const string &categoryName = util::randomElement(state.modelDatabase.categoryList);
    const ModelCategory &category = state.modelDatabase.getCategory(categoryName);
    const ModelData &model = *util::randomElement(category.modelList);

    model.loadModel(*state.graphics);

    ModelInstance instance;

    instance.model = &model;
    instance.anchor = vec3f(util::randomUniform(1.0f, 5.0f), util::randomUniform(1.0f, 5.0f), 0.0f);
    instance.scale = scale;

    instance.transform = mat4f::translation(instance.anchor) * mat4f::rotationZ(util::randomUniform(0.0f, 360.0f)) * mat4f::scale(instance.scale) * model.normalizingTransform();

    models.push_back(instance);
}
void Scene::render(AppState &state)
{
    for (auto &instance : models)
    {
        instance.model->loadModel(*state.graphics);

        for (auto &m : instance.model->meshes)
        {
            state.assets.renderMesh(m.mesh, state.camera.getCameraPerspective() * instance.transform, instance.debugColor);
        }
    }
}

Scene SceneGenerator::makeRandomScene(AppState &state)
{
    Scene result;

    result.addArchitecture(state, "floor", 5.0f);
    result.addArchitecture(state, "wallA", 5.0f);
    result.addArchitecture(state, "wallB", 5.0f);

    result.addRandomModel(state, util::randomUniform(0.5f, 1.5f));

    return result;
}
