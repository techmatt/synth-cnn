
#include "main.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <stdint.h>
#include <sys/stat.h>
#include <direct.h>

#include "caffe/proto/caffe.pb.h"

using namespace caffe;

const int synthImageWidth = 256;
const int synthImageHeight = 256;

pair<ColorImageR8G8B8A8, string> ImageCategory::makeRandomSample(const DatasetSplit &split) const
{
    ColorImageR8G8B8A8 randomImage;
    
    string randomFilename;

    do {
        do {
            randomFilename = util::randomElement(imageFilenames);
        } while (split.check(randomFilename));

        FreeImageWrapper::loadImage(randomFilename, randomImage);
    } while (randomImage.getWidth() != synthImageWidth ||
             randomImage.getWidth() != synthImageWidth);
    
    return make_pair(randomImage, randomFilename);
}

void ImageDatabase::initImageNet()
{
    for (auto &dir : Directory::enumerateDirectories(constants::imageNetDir))
    {
        ImageCategory newCategory;
        newCategory.name = dir;
        newCategory.index = categories.size();

        const string fullDir = constants::imageNetDir + dir + "/";
        for (auto &image : Directory::enumerateFilesWithPath(fullDir))
        {
            newCategory.imageFilenames.push_back(image);
        }

        categories.push_back(newCategory);
    }
}

void ImageDatabase::initSynthNet()
{
    for (auto &dir : Directory::enumerateDirectories(constants::synthSceneDir))
    {
        ImageCategory newCategory;
        newCategory.name = dir;
        newCategory.index = categories.size();

        const string fullDir = constants::synthSceneDir + dir + "/";
        for (auto &image : Directory::enumerateFilesWithPath(fullDir, "_comp.png"))
        {
            newCategory.imageFilenames.push_back(image);
        }

        categories.push_back(newCategory);

        cout << newCategory.name << ": " << newCategory.imageFilenames.size() << " images" << endl;
    }
}

void ImageDatabase::saveLevelDB(const string &outDir, const DatasetSplit &split, int sampleCount)
{
    // leveldb
    leveldb::DB* db;
    leveldb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    options.write_buffer_size = 268435456;

    // Open db
    std::cout << "Opening leveldb " << outDir << endl;
    leveldb::Status status = leveldb::DB::Open(options, outDir, &db);
    if (!status.ok())
    {
        std::cout << "Failed to open " << outDir << " or it already exists" << endl;
        return;
    }

    leveldb::WriteBatch* batch = new leveldb::WriteBatch();

    // Storing to db
    const int imageWidth = synthImageWidth;
    const int imageHeight = synthImageHeight;
    const int channelCount = 3;
    BYTE *rawVector = new BYTE[imageWidth * imageHeight * channelCount];

    int count = 0;
    const int kMaxKeyLength = 10;
    char key_cstr[kMaxKeyLength];

    cout << "A total of " << sampleCount << " samples will be generated." << endl;
    int totalSampleIndex = 0;

    for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
        if (totalSampleIndex % 100 == 0)
            cout << "Sample " << totalSampleIndex << " / " << sampleCount << endl;
        totalSampleIndex++;

        const auto &randomCategory = util::randomElement(categories);
        auto randomImage = randomCategory.makeRandomSample(split);

        Datum datum;

        datum.set_channels(channelCount);
        datum.set_height(imageHeight);
        datum.set_width(imageWidth);

        datum.set_label(randomCategory.index);

        int byteOffset = 0;
        for (int c = 0; c < 3; c++)
        {
            for (int y = 0; y < imageHeight; y++)
            {
                for (int x = 0; x < imageWidth; x++)
                {
                    rawVector[byteOffset++] = randomImage.first(x, y)[c];
                }
            }
        }

        datum.set_data(rawVector, channelCount * imageWidth * imageHeight);

        sprintf_s(key_cstr, kMaxKeyLength, "%08d", totalSampleIndex);

        string value;
        datum.SerializeToString(&value);

        string keystr(key_cstr);

        // Put in db
        batch->Put(keystr, value);

        if (++count % 1000 == 0) {
            // Commit txn
            db->Write(leveldb::WriteOptions(), batch);
            delete batch;
            batch = new leveldb::WriteBatch();
        }
    }

    // write the last batch
    if (count % 1000 != 0) {
        db->Write(leveldb::WriteOptions(), batch);
    }
    delete batch;
    delete db;
    cout << "Processed " << count << " entries." << endl;
}
