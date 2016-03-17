
struct FaceEntry
{
    string baseFilename;
    vector< vector<float> > featureSet;
};

class FaceProcessor
{
public:
    void init();
    vector<float> process(const ColorImageR8G8B8A8 &image);

    void process(const string &inputImage, const string &outputFilename);
    void processAll(const string &inputImageFolder, const string &outputCroppedFolder, const string &outputFeatureFolder);

    //void dumpDistanceMatrix(const string &folder, const string &filename);
    
    void loadFeatures(const string &featureFolder);

    void evaluateAll(const string &baseDir);
    void evaluateMatches(const string &baseFilename, const string &baseDir);

    void convertJPGToPNG();
    
private:
    static float L2Dist(const vector<float> &a, const vector<float> &b)
    {
        float distSq = 0.0f;
        for (auto &v : iterate(a))
        {
            distSq += math::square(v.value - b[v.index]);
        }
        return sqrtf(distSq);
    }
    static float compareFaces(const FaceEntry &a, const FaceEntry &b)
    {
        float minDist = numeric_limits<float>::max();
        for (auto &fA : a.featureSet)
        {
            for (auto &fB : b.featureSet)
            {
                minDist = min(minDist, L2Dist(fA, fB));
            }
        }
        return minDist;
    }

    float compareFaces()
    {

    }

    Netf net;
    vec3f meanValue;

    map<string, FaceEntry> allFaces;
};
