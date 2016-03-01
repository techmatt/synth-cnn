
#include "main.h"

void EnvironmentDatabase::init()
{
    environments = Directory::enumerateFilesWithPath(constants::synthCNNRoot + "envmaps/", ".exr");

    for (auto &s : environments)
    {
        const string PNGName = util::replace(s, ".exr", ".png");
        if (!util::fileExists(PNGName))
        {
            util::runCommand(string("mtsutil tonemap \"") + s + "\"");
        }
    }
}
