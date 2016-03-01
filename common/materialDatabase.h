
struct MaterialEntry
{
    float roughness() const
    {
        return 1.0f - d;
    }
    vec3f diffuse() const
    {
        return albedo;
    }
    vec3f specular() const
    {
        vec3f result;
        for (int i = 0; i < 3; i++)
        {
            const float a = (1.0f - albedo[i]) * 0.5f;
            result[i] = powf((c + powf(a, 1.0f / 3.0f)), 3.0f) - a;
        }
        return result;
    }

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