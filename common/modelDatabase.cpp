
#include "main.h"

mat4f ModelData::normalizingTransform() const
{
    const mat4f center = mat4f::translation(-vec3f(box.getCenter()));
    const mat4f upright = mat4f::face(up, -vec3f::eY);
    const mat4f scale = mat4f::scale(1.0f / box.getExtent().length());

    const OrientedBoundingBox3f tOBox = scale * upright * center * OrientedBoundingBox3f(box);
    bbox3f tBox = bbox3f(tOBox);

    const mat4f zOffset = mat4f::translation(vec3f::eZ * (0.005f + tBox.getExtentZ() / 2.0f));

    return zOffset * scale * upright * center;
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
    const string modelName = util::split(partsA[0], ".")[1];
    if (dirList.count(modelName) == 0) return;

    const string path = constants::shapeNetRoot + categoryName + "/" + modelName + "/model.obj";
    //if (!util::fileExists(path)) return;
    

    const auto partsB = util::split(line, ",\"");
    int upStartIndex = -1;
    for (int i = 0; i < partsB.size(); i++)
    {
        if (util::contains(partsB[i], "\\,"))
        {
            upStartIndex = i;
            break;
        }
    }

    if (upStartIndex == -1 || upStartIndex + 1 >= partsB.size())
        return;

    vector<string> upParts = util::split(util::remove(partsB[upStartIndex + 0], "\""), "\\,");
    const auto frontParts =  util::split(util::remove(partsB[upStartIndex + 1], "\""), "\\,");

    if (upParts.size() != 3 || frontParts.size() != 3) return;


    ModelData *data = new ModelData;
    models[modelName] = data;
    modelList.push_back(data);

    data->modelName = modelName;
    data->categoryName = categoryName;
    data->path = path;

    data->up.x = convert::toFloat(upParts[0]);
    data->up.y = convert::toFloat(upParts[1]);
    data->up.z = convert::toFloat(upParts[2]);

    data->front.x = convert::toFloat(frontParts[0]);
    data->front.y = convert::toFloat(frontParts[1]);
    data->front.z = convert::toFloat(frontParts[2]);
}

void ModelCategory::addArchitectureModel(const string &architectureName)
{
    ModelData *data = new ModelData;
    models[architectureName] = data;
    modelList.push_back(data);

    data->path = constants::architecturePath + architectureName + ".obj";
    data->categoryName = "architecture";
    data->modelName = architectureName;
}

void ModelDatabase::init()
{
    for (const string &csvFile : Directory::enumerateFiles(constants::shapeNetRoot, ".csv"))
    {
        const string categoryName = util::removeExtensions(csvFile);
        ModelCategory &category = categories[categoryName];
        category.categoryName = categoryName;

        for (auto &d : Directory::enumerateDirectories(constants::shapeNetRoot + category.categoryName))
            category.dirList.insert(d);

        categoryList.push_back(categoryName);
        
        auto lines = util::getFileLines(constants::shapeNetRoot + csvFile, 3);
        for (int lineIndex = 1; lineIndex < lines.size(); lineIndex++)
        {
            category.addCSVModel(lines[lineIndex]);
        }

        cout << "Loading " << categoryName << " models=" << category.modelList.size() << endl;
    }

    ModelCategory &architecture = categories["architecture"];
    architecture.categoryName = "architecture";
    architecture.addArchitectureModel("floor");
    architecture.addArchitectureModel("wallA");
    architecture.addArchitectureModel("wallB");
}
