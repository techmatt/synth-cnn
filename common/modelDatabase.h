
struct ModelDataMesh
{
    bbox3f box;
    D3D11TriMesh mesh;
};

struct ModelData
{
    void loadModel(GraphicsDevice &graphics) const;

    mat4f normalizingTransform() const;

    vec3f up;
    vec3f front;

    string path;
    
    mutable bbox3f box;
    mutable vector<ModelDataMesh> meshes;
};

struct ModelCategory
{
    const ModelData& getModel(const string &s) const
    {
        MLIB_ASSERT_STR(models.count(s) > 0, "category not found: " + s);
        return *models.find(s)->second;
    }

    void addCSVModel(const string &line);
    void addArchitectureModel(const string &filename);

    string name;

    map<string, ModelData*> models;
    vector<ModelData*> modelList;
};

struct ModelDatabase
{
    void init();

    const ModelCategory& getCategory(const string &s) const
    {
        MLIB_ASSERT_STR(categories.count(s) > 0, "category not found: " + s);
        return categories.find(s)->second;
    }

    map<string, ModelCategory> categories;
    vector<string> categoryList;
};