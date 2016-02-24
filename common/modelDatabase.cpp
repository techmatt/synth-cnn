
#include "main.h"

mat4f ModelData::normalizingTransform() const
{
    const mat4f translate = mat4f::translation(-vec3f(box.getCenter().x, box.getCenter().y, box.getMin().z));
    const mat4f scale = mat4f::scale(1.0f / box.getExtent().length());
    const mat4f zOffset = mat4f::translation(0.0f, 0.0f, 0.005f);

    return zOffset * scale * translate;
}

void ModelData::loadModel(GraphicsDevice &graphics) const
{
    if (meshes.size() > 0)
        return;

    MeshDataf meshData;
    MeshIOf::loadFromOBJ(path, meshData);

    vector< pair<MeshDataf, Materialf> > meshParts;

    if (meshData.m_MaterialFile.size() == 0)
    {
        meshParts.push_back(make_pair(meshData, Materialf()));
    }
    else
    {
        meshParts = meshData.splitByMaterial();
    }

    meshes.resize(meshParts.size());

    box = bbox3f();
    for (auto &m : iterate(meshParts))
    {
        TriMeshf triMesh(m.value.first);
        triMesh.setColor(vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        triMesh.computeNormals();

        for (auto &v : triMesh.getVertices())
        {
            float s = 0.0f;
            s = max(s, abs(v.normal | vec3f(1.0f, 0.0f, 0.0f)));
            s = max(s, abs(v.normal | vec3f(0.0f, 1.0f, 0.0f)));
            s = max(s, abs(v.normal | vec3f(0.0f, 0.0f, 1.0f)));
            s = max(s, abs(v.normal | vec3f(1.0f, 1.0f, 1.0f).getNormalized()));
            v.color = vec4f(s, s, s, 1.0f);
        }

        meshes[m.index].mesh = D3D11TriMesh(graphics, triMesh);
        meshes[m.index].box = triMesh.computeBoundingBox();
        box.include(meshes[m.index].box);
    }
}

void ModelCategory::addCSVModel(const string &line)
{
    //3dw.a4678e6798e768c3b6a66ea321171690,"02842573,04012084,02691156","biplane,propeller plane,airplane,aeroplane,plane","0.0\,0.0\,1.0","0.0\,1.0\,0.0",Old-school Yellow Biplane,

    const auto partsA = util::split(line, ',');
    const string &modelName = util::remove(partsA[0], "3dw.");

    const string path = constants::shapeNetRoot + name + "/" + modelName + "/model.obj";
    //if (!util::fileExists(path))
    if (dirList.count(modelName) == 0)
    {
        return;
    }

    ModelData *data = new ModelData;
    models[modelName] = data;
    modelList.push_back(data);

    data->path = path;
}

void ModelCategory::addArchitectureModel(const string &architectureName)
{
    ModelData *data = new ModelData;
    models[architectureName] = data;
    modelList.push_back(data);

    data->path = constants::architecturePath + architectureName + ".obj";
}

void ModelDatabase::init()
{
    for (const string &csvFile : Directory::enumerateFiles(constants::shapeNetRoot, ".csv"))
    {
        const string categoryName = util::removeExtensions(csvFile);
        cout << "Loading " << categoryName << endl;

        ModelCategory &category = categories[categoryName];
        category.name = categoryName;

        for (auto &d : Directory::enumerateDirectories(constants::shapeNetRoot + category.name))
            category.dirList.insert(d);

        categoryList.push_back(categoryName);
        
        auto lines = util::getFileLines(constants::shapeNetRoot + csvFile, 3);
        for (int lineIndex = 1; lineIndex < lines.size(); lineIndex++)
        {
            category.addCSVModel(lines[lineIndex]);
        }
    }

    ModelCategory &architecture = categories["architecture"];
    architecture.name = "architecture";
    architecture.addArchitectureModel("floor");
    architecture.addArchitectureModel("wallA");
    architecture.addArchitectureModel("wallB");
}
