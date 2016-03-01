
#include "main.h"

void MaterialDatabase::init()
{
    cout << "Loading material database..." << endl;
    auto lines = util::getFileLines(constants::synthCNNRoot + "data/shapes.csv", 3);
    for (int i = 1; i < lines.size(); i++)
    {
        const string &line = lines[i];
        
        const auto parts = util::split(line, ',', true);

        if (parts.size() <= 7 ||
            parts[2].size() == 0 ||
            parts[3].size() == 0 ||
            parts[4].size() == 0)
            continue;

        vec3f albedo;
        albedo.x = convert::toFloat(parts[4]);
        albedo.y = convert::toFloat(parts[5]);
        albedo.z = convert::toFloat(parts[6]);
        if (albedo.lengthSq() <= 0.5f * 0.5f)
            continue;

        MaterialEntry *newMaterial = new MaterialEntry;
        materials.push_back(newMaterial);

        newMaterial->substance = parts[2];
        newMaterial->objectName = parts[3];
        newMaterial->albedo = albedo;
        newMaterial->c = convert::toFloat(parts[8]);
        newMaterial->d = convert::toFloat(parts[9]);

        substances.insert(newMaterial->substance);
        objectNames.insert(newMaterial->objectName);
    }
    cout << materials.size() << " materials loaded" << endl;

    cout << "substances:" << endl;
    for (auto &s : substances)
        cout << s << endl;

    cout << "object names:" << endl;
    for (auto &s : objectNames)
        cout << s << endl;
}
