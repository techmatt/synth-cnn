
#include "main.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <stdint.h>
#include <sys/stat.h>
#include <direct.h>

#include "caffe/proto/caffe.pb.h"

#include "mLibFreeImage.h"

using namespace caffe;

const int synthImageWidth = 256;
const int synthImageHeight = 256;

ColorImageR8G8B8A8 ImageCategory::makeRandomSample(const TestSplit &split) const
{
    ColorImageR8G8B8A8 randomImage;
    
    do {

        string randomFilename;
        do {
            randomFilename = util::randomElement(imageFilenames);
        } while (split.check(randomFilename));

        FreeImageWrapper::loadImage(randomFilename, randomImage);
    } while (randomImage.getWidth() != synthImageWidth ||
             randomImage.getWidth() != synthImageWidth);
    
    return randomImage;
}

void ImageDatabase::initImageNet()
{
    cout << "debug imagenet directories" << endl;
    /*const string baseDir = constants::synthLearningDir + "images/";
    for (auto &dir : Directory::enumerateDirectories(baseDir))
    {
        ImageCategory newCategory;
        newCategory.name = dir;
        newCategory.index = categories.size();

        const string fullDir = baseDir + dir + "/";
        for (auto &image : Directory::enumerateFilesWithPath(fullDir))
        {
            newCategory.imageFilenames.push_back(image);
        }

        categories.push_back(newCategory);
    }*/
}

void ImageDatabase::initSynthNet()
{
    const string baseDir = constants::synthSceneDir;
    for (auto &dir : Directory::enumerateDirectories(baseDir))
    {
        ImageCategory newCategory;
        newCategory.name = dir;
        newCategory.index = categories.size();

        const string fullDir = baseDir + dir + "/";
        for (auto &image : Directory::enumerateFilesWithPath(fullDir, "_comp.png"))
        {
            newCategory.imageFilenames.push_back(image);
        }

        categories.push_back(newCategory);

        cout << newCategory.name << ": " << newCategory.imageFilenames.size() << " images" << endl;
    }
}

void ImageDatabase::saveLevelDB(const string &outDir, const TestSplit &split, int sampleCount)
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
            for (int x = 0; x < imageWidth; x++)
            {
                for (int y = 0; y < imageHeight; y++)
                {
                    rawVector[byteOffset++] = randomImage(x, y)[c];
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

/*void NetflixDatabase::makeLinearFeatures(const Rating &r, BYTE *output) const
{
    auto ratingToByte = [](int rating)
    {
        if (rating == 1) return (BYTE)0;
        if (rating == 2) return (BYTE)64;
        if (rating == 3) return (BYTE)128;
        if (rating == 4) return (BYTE)192;
        if (rating == 5) return (BYTE)255;
        return (BYTE)128;
    };

    BYTE *userRating = output + movieIndexCount * 0;
    BYTE *userIndicator = output + movieIndexCount * 1;
    BYTE *movieIndicator = output + movieIndexCount * 2;
    BYTE *targetRating = output + movieIndexCount * 3;

    const User &u = *allUsers.find(r.userID)->second;

    for (int i = 0; i < movieIndexCount; i++)
    {
        userRating[i] = 128;
        userIndicator[i] = 128;
        movieIndicator[i] = 128;
    }

    movieIndicator[r.movieIndex] = 255;

    for (Rating otherR : u.ratings)
    {
        if (otherR.movieIndex != r.movieIndex)
        {
            userRating[otherR.movieIndex] = ratingToByte(otherR.rating);
            userIndicator[otherR.movieIndex] = 255;
        }
    }

    targetRating[0] = ratingToByte(r.rating);
}

void NetflixDatabase::loadText()
{
    for (auto &filename : Directory::enumerateFiles(constants::trainingDir))
    {
        cout << "loading " << filename << endl;
        const string baseFilename = util::removeExtensions(filename);
        const int movieIndex = convert::toInt(util::remove(baseFilename, "mv_"));
        processMovieFile(constants::trainingDir + filename, movieIndex);
    }
}

void NetflixDatabase::saveBinary()
{
    BinaryDataStreamFile out(constants::netflixDir + "database.dat", true);
    out.writePrimitive(allRatingsStorage);
    out.closeStream();
}

void NetflixDatabase::loadBinary()
{
    cout << "Loading from disk..." << endl;
    BinaryDataStreamFile in(constants::netflixDir + "database.dat", false);
    in.readPrimitive(allRatingsStorage);
    in.closeStream();

    cout << "Adding all users..." << endl;
    movieIndexCount = 0;
    for (const Rating &r : allRatingsStorage)
    {
        if (allUsers.count(r.userID) == 0)
        {
            User *newUser = new User(r.userID);
            allUsers[r.userID] = newUser;

            if (newUser->test)
                testUsers.push_back(newUser);
            else
                trainUsers.push_back(newUser);
        }

        movieIndexCount = max(movieIndexCount, r.movieIndex + 1);
        
        User &u = *allUsers[r.userID];
        u.ratings.push_back(r);

        if (u.test)
            testRatings.push_back(r);
        else
            trainRatings.push_back(r);
    }
    cout << "done" << endl;
}

void NetflixDatabase::processMovieFile(const string &filename, int movieIndex)
{
    for (const string &line : util::getFileLines(filename, 3))
    {
        const auto words = util::split(line, ',');
        if (words.size() != 3)
            continue;

        const int userID = convert::toInt(words[0]);
        const int rating = convert::toInt(words[1]);

        allRatingsStorage.push_back(Rating(movieIndex, userID, rating));
    }
}
*/