
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

    NetflixDatabase database;
    database.loadBinary();

    NetworkProcessor processor;
    processor.init();
    processor.evaluateAllUsers(database);
    //processor.outputUsers(R"(D:\datasets\Netflix\caffe\results.csv)");
}
