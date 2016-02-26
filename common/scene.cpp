
#include "main.h"

void Scene::addArchitecture(AppState &state, const string &architectureName, float scale)
{
    ModelInstance instance;

    instance.anchor = vec3f::origin;
    instance.model = &state.modelDatabase.getCategory("architecture").getModel(architectureName);

    instance.transform = mat4f::scale(scale);

    objects.push_back(instance);
}

void Scene::addRandomModel(AppState &state, float scale)
{
    const string &categoryName = util::randomElement(state.modelDatabase.categoryList);
    const ModelCategory &category = state.modelDatabase.getCategory(categoryName);
    const ModelData &model = *util::randomElement(category.modelList);

    model.loadModel(*state.graphics);

    ModelInstance instance;

    instance.model = &model;
    instance.anchor = vec3f(util::randomUniform(1.0f, 3.0f), util::randomUniform(1.0f, 3.0f), 0.0f);
    instance.scale = scale * 2.0f;

    instance.transform = mat4f::translation(instance.anchor) * mat4f::rotationZ(util::randomUniform(0.0f, 360.0f)) * mat4f::scale(instance.scale) * model.normalizingTransform();

    instance.worldBBox = instance.transform * OBB3f(instance.model->box);

    objects.push_back(instance);
}

void ModelInstance::render(AppState &state, const Cameraf &camera) const
{
    model->loadModel(*state.graphics);

    for (auto &m : model->meshes)
    {
        state.renderer.renderMesh(m.mesh, camera.getCameraPerspective() * transform, debugColor);
    }
}

void Scene::render(AppState &state, const Cameraf &camera) const
{
    for (auto &instance : objects)
    {
        instance.render(state, camera);
    }
}

void Scene::saveMitsuba(const string &filename, const Cameraf &camera) const
{
    auto baseLines = util::getFileLines(constants::synthCNNRoot + "data/mitsubaTemplate.txt");
    const auto shapeLines = util::getFileLines(constants::synthCNNRoot + "data/shapeTemplate.txt");
    
    auto replaceAll = [](vector<string> &lines, const string &value, const string &replacement)
    {
        for (string &line : lines)
        {
            if (util::contains(line, value))
                line = util::replace(line, value, replacement);
        }
    };

    replaceAll(baseLines, "#LOOKAT#", camera.getLook().toString(", "));
    replaceAll(baseLines, "#ORIGIN#", camera.getEye().toString(", "));
    
    for (auto &instance : iterate(objects))
    {
        const ModelData &model = *instance.value.model;
        const string objDir = constants::synthCNNRoot + "meshes/" + model.categoryName + "/";
        util::makeDirectory(objDir);

        for (int flip = 0; flip <= 0; flip++)
        {
            const string flipStr = flip ? "true" : "false";
            for (auto &m : iterate(model.meshes))
            {
                const string &objFilename = objDir + model.modelName + "_" + to_string(m.index) + ".obj";
                if (!util::fileExists(objFilename))
                {
                    auto meshData = m.value.mesh.getTriMesh().getMeshData();
                    meshData.clearAttributes();
                    
                    MeshIOf::saveToOBJ(objFilename, meshData);
                }
                auto localShapeLines = shapeLines;
                replaceAll(localShapeLines, "#FILENAME#", objFilename);
                replaceAll(localShapeLines, "#MATRIX#", instance.value.transform.toString(", "));
                replaceAll(localShapeLines, "#ID#", "o" + to_string(instance.index) + "_m" + to_string(m.index) + "_f" + flipStr);
                replaceAll(localShapeLines, "#FLIP#", flipStr);
                
                for (auto &s : localShapeLines)
                {
                    baseLines.push_back(s);
                }
            }
        }
    }

    baseLines.push_back("</scene>");

    util::saveLinesToFile(baseLines, filename);
}

Scene SceneGenerator::makeRandomScene(AppState &state)
{
    Scene result;

    result.addArchitecture(state, "floor", 5.0f);
    result.addArchitecture(state, "wallA", 5.0f);
    result.addArchitecture(state, "wallB", 5.0f);

    result.addRandomModel(state, util::randomUniform(0.75f, 1.0f));
    result.mainObjectIndex = (int)result.objects.size() - 1;

    return result;
}
