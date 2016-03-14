
#include "main.h"

void NetworkProcessor::init()
{
    const string baseDir = constants::synthCNNRoot + "nets/";
    const string netFilename = baseDir + "flickr-net-eval.prototxt";
    const string modelFilename = baseDir + "flickr.caffemodel";
    const string meanFilename = constants::synthCNNRoot + "databases/synth-mean.binaryproto";

    meanValues = CaffeUtil::gridFromBinaryProto(meanFilename);
    
    net = Netf(new Net<float>(netFilename, caffe::TEST));
    net->CopyTrainedLayersFrom(modelFilename);
}

const bool saveNetsMode = false;
int globalUserIndex = 0;
vector<float> NetworkProcessor::evaluateImage(const ColorImageR8G8B8A8 &image)
{
    Grid3f inputData(image.getDimX(), image.getDimY(), 3);
    
    for (auto &p : inputData)
    {
        const float scale = 1.0f;
        const float v = image(p.y, p.x)[p.z];
        p.value = v * scale - meanValues(p.y, p.x, p.z);
    }

    if (saveNetsMode) net->ForwardFrom(0);
    
    CaffeUtil::runNetForward(net, "conv1", "data", inputData);

    if (saveNetsMode) CaffeUtil::saveNetToDirectory(net, constants::synthCNNRoot + "netDumps/netAfter" + to_string(globalUserIndex) + "/");
    
    globalUserIndex++;

    if (saveNetsMode && globalUserIndex == 6)
        exit(0);
    
    const auto grid = CaffeUtil::getBlobAsGrid(net, "fc8_synth");
    
    const vector<float> result = CaffeUtil::gridToVector(grid);
    return result;
}

void NetworkProcessor::evaluateRandomImages(const ImageDatabase &database, const DatasetSplit &split, int count, const string &outFilename)
{
    ofstream file(outFilename);

    file << "index,class,prediction,image";

    const int classCount = 14;
    for (int i = 0; i < classCount; i++)
        file << ",c" << i;
    file << endl;

    for (int i = 0; i < count; i++)
    {
        cout << "sample " << i << endl;

        const auto &randomCategory = util::randomElement(database.categories);
        const auto randomImage = randomCategory.makeRandomSample(split);
        const auto croppedImage = cropImage(randomImage.first, 227);

        vector<float> result = evaluateImage(croppedImage);

        size_t prediction = util::maxIndex(result);

        file << i << "," << randomCategory.index << "," << prediction << "," << randomImage.second;

        for (int i = 0; i < result.size(); i++)
            file << "," << result[i];

        file << endl;
    }
}

ColorImageR8G8B8A8 NetworkProcessor::cropImage(const ColorImageR8G8B8A8 &image, int dim)
{
    ColorImageR8G8B8A8 result(dim, dim);

    const vec2i start(14, 14);

    for (auto &p : result)
        p.value = image(p.x + start.x, p.y + start.y);

    return result;
}
