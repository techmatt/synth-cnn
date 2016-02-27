
#include "main.h"

void MaterialDatabase::init()
{
    cout << "Loading material database..." << endl;
    auto lines = util::getFileLines(constants::synthCNNRoot + "data/shapes.csv");
    for (int i = 1; i < lines.size(); i++)
    {
        const string &line = lines[i];
        
        auto parts = util::split(line, ',', true);

        if (parts[2].size() == 0 ||
            parts[3].size() == 0 ||
            parts[4].size() == 0)
            continue;

        MaterialEntry *newMaterial = new MaterialEntry;
        materials.push_back(newMaterial);

        newMaterial->substance = parts[2];
        newMaterial->objectName = parts[3];
        newMaterial->albedo.x = convert::toFloat(parts[4]);
        newMaterial->albedo.y = convert::toFloat(parts[5]);
        newMaterial->albedo.z = convert::toFloat(parts[6]);
        newMaterial->c = convert::toFloat(parts[8]);
        newMaterial->d = convert::toFloat(parts[9]);

        substances.insert(newMaterial->substance);
        objectNames.insert(newMaterial->objectName);
    }
    cout << materials.size() << " materials loaded" << endl;
}
