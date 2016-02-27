
struct MaterialEntry
{
    string substance;
    string objectName;
    vec3f albedo;
    float c, d;
};

struct MaterialDatabase
{
    void init();

    vector<const MaterialEntry*> materials;
    set<string> substances;
    set<string> objectNames;
};