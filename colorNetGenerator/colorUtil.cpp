
#include "main.h"

void ColorConverter::init()
{
    const string baseDir = R"(C:\Code\synth-cnn\colorNetGenerator\)";

    auto readGrid = [](const string &filename, const vec3i &dimensions) {
        ifstream file(filename);
        Grid3<vec3f> result(dimensions);
        for (int x = 0; x < dimensions.x; x++)
            for (int y = 0; y < dimensions.y; y++)
                for (int z = 0; z < dimensions.z; z++)
                {
                    float v0, v1, v2;
                    file >> v0 >> v1 >> v2;
                    result(x, y, z) = vec3f(v0, v1, v2);
                }
        return result;
    };

    cout << "Loading colors..." << endl;
    if (!util::fileExists(baseDir + "RGBToLAB.dat"))
    {
        cout << "Generating serialized files..." << endl;
        
        RGBToLABTable = readGrid(baseDir + "RGBToLAB.txt", vec3i(128, 128, 128));
        LABToRGBTable = readGrid(baseDir + "LABToRGB.txt", vec3i(101, 185, 202));

        util::serializeToFilePrimitive(baseDir + "RGBToLAB.dat", RGBToLABTable);
        util::serializeToFilePrimitive(baseDir + "LABToRGB.dat", LABToRGBTable);
    }

    util::deserializeFromFilePrimitive(baseDir + "RGBToLAB.dat", RGBToLABTable);
    util::deserializeFromFilePrimitive(baseDir + "LABToRGB.dat", LABToRGBTable);

    cout << "done" << endl;
}

vec3f ColorConverter::RGBToLAB(const vec3f &v) const
{
    const int x = math::clamp((int)math::round(v.x * 128.0f), 0, 127);
    const int y = math::clamp((int)math::round(v.y * 128.0f), 0, 127);
    const int z = math::clamp((int)math::round(v.z * 128.0f), 0, 127);
    return RGBToLABTable(x, y, z);
}

//Min: 0, -86, -107
//Max: 100, 98, 94

vec3f ColorConverter::LABToRGB(const vec3f &v) const
{
    const int x = math::clamp((int)math::round(v.x + 0.0f), 0, 100);
    const int y = math::clamp((int)math::round(v.y + 86.0f), 0, 184);
    const int z = math::clamp((int)math::round(v.z + 107.0f), 0, 201);
    return LABToRGBTable(x, y, z) / 255.0f;
}