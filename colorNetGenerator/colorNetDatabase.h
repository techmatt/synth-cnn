
const float LScale = 0.01f;

struct TestSplit
{
    TestSplit() {}
    TestSplit(const set<char> &_acceptedCharacters)
    {
        acceptedCharacters = _acceptedCharacters;
    }
    bool check(const string &filename) const
    {
        const char c = util::getFilenameFromPath(filename)[0];
        return acceptedCharacters.count(c) > 0;
    }

    set<char> acceptedCharacters;
};

struct ColorNetEntry
{
    Grid2uc LChannel;            // 256 x 256
    Grid3uc clusterDistribution; // 64 x 64 x 32
};

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator<<(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, const ColorNetEntry &data) {
    s.writePrimitive(data.LChannel);
    s.writePrimitive(data.clusterDistribution);
    return s;
}

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator>>(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, ColorNetEntry &data) {
    s.readPrimitive(data.LChannel);
    s.readPrimitive(data.clusterDistribution);
    return s;
}

struct ClusterInfo
{
    vec3f LAB;
    float thresholdA;
    float thresholdB;
};

struct ColorNetDatabase
{
    ColorImageR8G8B8A8 randomImage() const;

    vec3f findClosestCentroid(const vec3f &rgb) const
    {
        return clusters[findClosestCentroidIndex(rgb)].LAB;
    }

    int findClosestCentroidIndex(const vec3f &rgb) const
    {
        const vec3f LAB = converter.RGBToLAB(rgb);
        vec3f squashedLAB = LAB;
        squashedLAB.x *= LScale;

        int result = -1;
        float bestDistSq = numeric_limits<float>::max();
        for (const auto &c : iterate(clusters))
        {
            vec3f squashedC = c.value.LAB;
            squashedC.x *= LScale;

            const float distSq = vec3f::distSq(squashedC, squashedLAB);
            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                result = c.index;
            }
        }
        return result;
    }

    void init();

    void testRandomImages(const string &directory, int imageCount) const;

    void clusterColors(const string &filenameBase, int imageCount, int samplesPerImage, int clusterCount);

    

    void createDatabase(const string &directory, int sampleCount);

    ColorImageR8G8B8A8 quantizeImage(const ColorImageR8G8B8A8 &image, bool useL) const;
    ColorImageR8G8B8A8 extractChannel(const ColorImageR8G8B8A8 &image, int channelIndex) const;
    static float computeClusterDist(const vec3f &LAB0, const vec3f &LAB1);
    Grid2uc computeDensityMap(const ColorImageR8G8B8A8 &image, const ClusterInfo &cluster) const;
    
    ColorConverter converter;
    vector<string> allImageFilenames;
    vector<ClusterInfo> clusters;
};
