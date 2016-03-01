
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

bool Scene::valid(AppState &state) const
{
    for (auto &instance : objects)
    {
        instance.model->loadModel(*state.graphics);
        if (instance.model->meshes.size() == 0)
            return false;
    }
    return true;
}

void Scene::render(AppState &state, const Cameraf &camera) const
{
    for (auto &instance : objects)
    {
        instance.render(state, camera);
    }
}

void Scene::saveMitsuba(AppState &state, const string &filename, const Cameraf &camera) const
{
    auto r = [](float min, float max) { return util::randomUniform(min, max); };

    auto baseLines = util::getFileLines(constants::synthCNNRoot + "data/mitsubaTemplate.txt");
    const auto shapeLines = util::getFileLines(constants::synthCNNRoot + "data/shapeTemplate.txt");
    const auto wardLines = util::getFileLines(constants::synthCNNRoot + "data/wardTemplate.txt");
    
    auto replaceAll = [](vector<string> &lines, const string &value, const string &replacement)
    {
        for (string &line : lines)
        {
            if (util::contains(line, value))
                line = util::replace(line, value, replacement);
        }
    };

    auto replaceLines = [](const vector<string> &lines, const string &value, const vector<string> &replacement)
    {
        vector<string> result;
        for (const string &line : lines)
        {
            if (!util::contains(line, value))
                result.push_back(line);
            else
                for (auto &s : replacement)
                    result.push_back(s);
        }
        return result;
    };

    replaceAll(baseLines, "#LOOKAT#", (camera.getEye() + camera.getLook()).toString(", "));
    replaceAll(baseLines, "#ORIGIN#", camera.getEye().toString(", "));

    for (int env = 0; env <= 0; env++)
    {
        const string &exrFilename = util::randomElement(state.environmentDatabase.environments);
        const string envPrefix = "#ENV_" + to_string(env) + "_";
        
        replaceAll(baseLines, envPrefix + "EXR#", exrFilename);
        replaceAll(baseLines, envPrefix + "ROTATE#", to_string(util::randomUniform(0.0f, 360.0f)));
        replaceAll(baseLines, envPrefix + "SCALE#", to_string(util::randomUniform(1.0f, 5.0f)));
    }
    
    for (auto &instance : iterate(objects))
    {
        const ModelData &model = *instance.value.model;
        const string objDir = constants::synthCNNRoot + "meshes/" + model.categoryName + "/";
        util::makeDirectory(objDir);

        const vec3f diffuseOverride(r(0.1f, 0.9f), r(0.1f, 0.9f), r(0.1f, 0.8f));
        const vec3f specularOverride(r(0.0f, 0.2f), r(0.0f, 0.2f), r(0.0f, 0.2f));

        const auto &randomMaterial = *util::randomElement(state.materialDatabase.materials);

        for (auto &m : iterate(model.meshes))
        {
            const string &objFilename = objDir + model.modelName + "_" + to_string(m.index) + ".obj";
            if (!util::fileExists(objFilename))
            {
                auto meshData = m.value.mesh.getTriMesh().getMeshData();
                meshData.clearAttributes();

                if (meshData.m_FaceIndicesVertices.size() == 0)
                    continue;
                    
                MeshIOf::saveToOBJ(objFilename, meshData);
            }

            auto localShapeLines = shapeLines;

            localShapeLines = replaceLines(localShapeLines, "#BSDF#", wardLines);

            replaceAll(localShapeLines, "#FILENAME#", objFilename);
            replaceAll(localShapeLines, "#MATRIX#", instance.value.transform.toString(", "));
            replaceAll(localShapeLines, "#ID#", "o" + to_string(instance.index) + "_m" + to_string(m.index));
            replaceAll(localShapeLines, "#FLIPNORMALS#", "false");
            replaceAll(localShapeLines, "#FACENORMALS#", "true");
            
            replaceAll(localShapeLines, "#ROUGHNESS#", to_string(randomMaterial.roughness()));
            
            //replaceAll(localShapeLines, "#DIFFUSE#", randomMaterial.diffuse().toString(", "));
            //replaceAll(localShapeLines, "#SPECULAR#", randomMaterial.specular().toString(", "));
            
            replaceAll(localShapeLines, "#DIFFUSE#", diffuseOverride.toString(", "));
            replaceAll(localShapeLines, "#SPECULAR#", specularOverride.toString(", "));

            for (auto &s : localShapeLines)
            {
                baseLines.push_back(s);
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
