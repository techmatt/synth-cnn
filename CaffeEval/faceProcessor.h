
struct FaceEntry
{
    string filename;
    vector<float> features;
};

class FaceProcessor
{
public:
    void init();
    vector<float> process(const ColorImageR8G8B8A8 &image);
    void processAll(const string &folder);
    void dumpDistanceMatrix(const string &folder, const string &filename);
    void process(const string &inputImage, const string &outputFilename);
    
private:
    Netf net;
    vec3f meanValue;
};
