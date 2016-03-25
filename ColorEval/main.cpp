
#include "main.h"

void main(int argc, char** argv)
{
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
    
    mBase::Reader<ColorNetEntry> reader;

    const string baseDir = R"(D:\datasets\ColorNet\data\)";
    const string databaseDir = baseDir + R"(database\)";
    reader.init(databaseDir, 24, 12);

    ColorNetEntry entry;
    reader.readNextRecord(entry);
    reader.readNextRecord(entry);
    reader.readNextRecord(entry);

    ColorProcessor processor;
    processor.init();
    processor.process(entry);
    processor.visualize(entry, baseDir + "test");
}
