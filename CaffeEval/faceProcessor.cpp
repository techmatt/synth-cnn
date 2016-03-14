
#include "main.h"

void FaceProcessor::dumpDistanceMatrix(const string &folder, const string &filename)
{
    vector<FaceEntry> faces;
    for (auto &file : Directory::enumerateFilesWithPath(folder, ".dat"))
    {
        FaceEntry e;
        e.filename = util::fileNameFromPath(file);
        util::deserializeFromFile(file, e.features);
        faces.push_back(e);
    }

    ofstream csvFile(filename);
    csvFile << "distances";
    for (const FaceEntry &f : faces)
        csvFile << "," << f.filename;
    csvFile << endl;

    for (const FaceEntry &fA : faces)
    {
        csvFile << fA.filename;
        for (const FaceEntry &fB : faces)
        {
            float distSq = 0.0f;
            for (auto &v : iterate(fA.features))
            {
                distSq += math::square(v.value - fB.features[v.index]);
            }
            float dist = sqrtf(distSq);
            csvFile << "," << dist;
        }
        csvFile << endl;
    }
}

void FaceProcessor::init()
{
    const string baseDir = constants::synthCNNRoot + "nets/";
    const string netFilename = baseDir + "flickr-net-eval.prototxt";
    const string modelFilename = baseDir + "flickr.caffemodel";
    const string meanFilename = constants::synthCNNRoot + "databases/synth-mean.binaryproto";

    meanValue = vec3f(93.5940f, 104.7624f, 129.1863f);
    
    net = Netf(new Net<float>(R"(D:\datasets\VGGFace\VGG_FACE_deploy.prototxt)", caffe::TEST));
    net->CopyTrainedLayersFrom(R"(D:\datasets\VGGFace\VGG_FACE.caffemodel)");

    //NetParameter net_param;
    //net_->ToProto(&net_param, param_.snapshot_diff());
    //WriteProtoToBinaryFile(net_param, model_filename);
}

void FaceProcessor::processAll(const string &folder)
{
    const float cropSize = 0.8f;

    for (auto &file : Directory::enumerateFilesWithPath(folder, ".jpg"))
    {
        ColorImageR32G32B32 image;
        FreeImageWrapper::loadImage(file, image);
        
        //FreeImageWrapper::saveImage(util::replace(file, ".jpg", ".png"), image);
        
        const vec2f center = vec2f(image.getDimensions()) * 0.5f;
        const vec2f dim = vec2f(image.getDimensions()) * cropSize;
        auto croppedImage = image.getSubregion(bbox2i(math::round(center - dim * 0.5f), math::round(center + dim * 0.5f)));
        croppedImage.reSample(224, 224);

        ColorImageR8G8B8A8 RGBImage(croppedImage.getDimensions());
        for (auto &p : RGBImage)
        {
            const vec3f v = croppedImage(p.x, p.y) * 255.0f;
            p.value = vec4uc(util::boundToByte(v.x),
                             util::boundToByte(v.y),
                             util::boundToByte(v.z), 255);
        }
        const vector<float> features = process(RGBImage);

        util::serializeToFile(util::replace(file, ".jpg", ".dat"), features);

        for (auto &p : croppedImage)
            p.value = vec3f(p.value.z, p.value.y, p.value.x);
        FreeImageWrapper::saveImage(util::replace(file, ".jpg", "_cropped.png"), croppedImage);
    }
}

vector<float> FaceProcessor::process(const ColorImageR8G8B8A8 &image)
{
    ColorImageR8G8B8A8 imageCopy = image;

    // RGB to BGR
    for (auto &p : imageCopy)
        p.value = vec4uc(p.value.z, p.value.y, p.value.x, 255);

    Grid3f inputData(image.getDimX(), image.getDimY(), 3);

    for (auto &p : inputData)
    {
        const float v = imageCopy(p.x, p.y)[p.z];
        p.value = (v - meanValue[p.z]);
    }

    //net->ForwardFrom(0);

    //CaffeUtil::runNetForward(net, "conv1_1", "data", inputData);
    CaffeUtil::runNetForwardTo(net, "conv1_1", "data", inputData, "fc7");

    //CaffeUtil::saveNetToDirectory(net, R"(D:\datasets\VGGFace\netDumps\)");

    const auto grid = CaffeUtil::getBlobAsGrid(net, "fc7");

    const vector<float> result = CaffeUtil::gridToVector(grid);

    return result;
}