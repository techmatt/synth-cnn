
#include "main.h"

void main(int argc, char** argv)
{
    ParameterFile params("parameters.txt");

    google::InitGoogleLogging(argv[0]);
    
    const bool useGPU = true;
    if (useGPU)
    {
        LOG(ERROR) << "Using GPU";
        uint device_id = 0;
        LOG(ERROR) << "Using Device_id=" << device_id;
        Caffe::SetDevice(device_id);
        Caffe::set_mode(Caffe::GPU);
    }
    else
    {
        LOG(ERROR) << "Using CPU";
        Caffe::set_mode(Caffe::CPU);
    }

    FaceProcessor face;
    face.init();

    //ColorImageR8G8B8A8 image;
    //FreeImageWrapper::loadImage(R"(D:\datasets\VGGFace\ak.png)", image);
    //vector<float> values = face.process(image);

    face.processAll(R"(D:\datasets\VGGFace\faceTest\)");
    face.dumpDistanceMatrix(R"(D:\datasets\VGGFace\faceTest\)", R"(D:\datasets\VGGFace\distances.csv)");

    return;

    ImageDatabase database;
    database.initSynthNet();

    NetworkProcessor processor;
    processor.init();
    processor.evaluateRandomImages(database, DatasetSplit::splitTrain(), 100, constants::synthCNNRoot + "train.csv");
    processor.evaluateRandomImages(database, DatasetSplit::splitTest(), 100, constants::synthCNNRoot + "test.csv");
    //processor.outputUsers(R"(D:\datasets\Netflix\caffe\results.csv)");
}
