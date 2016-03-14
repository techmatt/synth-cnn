
class NetworkProcessor
{
public:
    void init();
    void evaluateRandomImages(const ImageDatabase &database, const DatasetSplit &split, int count, const string &outFilename);
    vector<float> evaluateImage(const ColorImageR8G8B8A8 &image);
    
    static ColorImageR8G8B8A8 cropImage(const ColorImageR8G8B8A8 &image, int dim);

private:
    Grid3f meanValues;
    Netf net;
};
