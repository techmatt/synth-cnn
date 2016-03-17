
#include "main.h"

#include "mLibFreeImage.h"

const int imageWidth = 256;
const int imageHeight = 256;

void ColorNetDatabase::init()
{
    converter.init();

    cout << "Loading all image filenames..." << endl;
    const string &baseDir = R"(D:\datasets\ImageNet\cat8\)";
    for (auto &dir : Directory::enumerateDirectoriesWithPath(baseDir))
    {
        for (auto &file : Directory::enumerateFilesWithPath(dir))
        {
            allImageFilenames.push_back(file);
        }
    }
    cout << "Image count: " << allImageFilenames.size() << endl;
}

void ColorNetDatabase::testRandomImages(const string &directory, int imageCount) const
{
    util::makeDirectory(directory);
    for (int i = 0; i < imageCount; i++)
    {
        const ColorImageR8G8B8A8 image = randomImage();

        const ColorImageR8G8B8A8 q0 = quantizeImage(image, false);
        const ColorImageR8G8B8A8 q1 = quantizeImage(image, true);

        const ColorImageR8G8B8A8 c0 = extractChannel(image, 0);
        const ColorImageR8G8B8A8 c1 = extractChannel(image, 1);
        const ColorImageR8G8B8A8 c2 = extractChannel(image, 2);

        const string preamble = directory + "i" + to_string(i) + "_";
        FreeImageWrapper::saveImage(preamble + "base.png", image);
        FreeImageWrapper::saveImage(preamble + "q0.png", q0);
        FreeImageWrapper::saveImage(preamble + "q1.png", q1);
        FreeImageWrapper::saveImage(preamble + "cL.png", c0);
        FreeImageWrapper::saveImage(preamble + "cA.png", c1);
        FreeImageWrapper::saveImage(preamble + "cB.png", c2);
    }
}

void ColorNetDatabase::createDatabase(const string &directory, int sampleCount)
{
    const int clusterCount = 32;

    ml::mBase::Writer<ColorNetEntry> writer(directory);

    for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
        if (sampleIndex % 100 == 0)
            cout << "Sample " << sampleIndex << " / " << sampleCount << endl;

        ColorNetEntry sample;

        const ColorImageR8G8B8A8 image = randomImage();
        
        sample.LChannel.allocate(256, 256);
        sample.clusterDistribution.allocate(64, 64, clusterCount);

        for (auto &p : image)
        {
            const vec3f LAB = converter.RGBToLAB( colorUtil::makeColor32(p.value) );
            sample.LChannel(p.x, p.y) = util::boundToByte(LAB.x * (255.0f / 100.0f));
        }

        vector<float> values(clusterCount);
        for (int xCell = 0; xCell < 64; xCell++)
            for (int yCell = 0; yCell < 64; yCell++)
            {
                for (auto &f : values)
                    f = 0.0f;

                const float increment = 1.0f / 16.0f;
                for (int x = 0; x < 4; x++)
                    for (int y = 0; y < 4; y++)
                    {
                        const vec3f rgb = colorUtil::makeColor32(image(xCell * 4 + x, yCell * 4 + y));
                        const int clusterIndex = findClosestCentroidIndex(rgb);
                        values[clusterIndex] += increment;
                    }

                for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
                {
                    sample.clusterDistribution(xCell, yCell, clusterIndex) = util::boundToByte(values[clusterIndex] * 255.0f);
                }
            }

        writer.addRecord(sample);
    }

    writer.finalize();
}

ColorImageR8G8B8A8 ColorNetDatabase::randomImage() const
{
    const string &randomFilename = util::randomElement(allImageFilenames);
    ColorImageR8G8B8A8 result;
    if (util::fileExists(randomFilename))
        FreeImageWrapper::loadImage(randomFilename, result);
    if (result.getWidth() == imageWidth && result.getHeight() == imageHeight)
    {
        return result;
    }
    cout << "Random image load failed for " << randomFilename << endl;
    return randomImage();
}

void ColorNetDatabase::clusterColors(const string &filenameBase, int imageCount, int samplesPerImage, int clusterCount)
{
    if (!util::fileExists(filenameBase + ".dat"))
    {
        const int maxIters = 300;

        cout << "Clustering LAB..." << endl;

        vector<vec3f> allLAB;

        bbox3f labBBox;
        vec3d runningAverage = vec3d::origin;
        int runningAverageCount = 0;
        for (int i = 0; i < imageCount; i++)
        {
            ColorImageR8G8B8A8 image = randomImage();
            for (int j = 0; j < samplesPerImage; j++)
            {
                const vec4uc value = image(util::randomInteger(0, image.getWidth() - 1), util::randomInteger(0, image.getHeight() - 1));
                const vec3f rgb = vec3f(value.getVec3()) / 255.0f;
                const vec3f lab = converter.RGBToLAB(rgb);
                labBBox.include(lab);
                runningAverage += vec3d(lab);
                runningAverageCount++;
                allLAB.push_back(vec3f(lab.x * LScale, lab.y * 1.0f, lab.z * 1.0f));
            }
        }
        runningAverage /= (double)runningAverageCount;

        KMeansClustering<vec3f, vec3fKMeansMetric> clustering;
        clustering.cluster(allLAB, clusterCount, maxIters);

        cout << "Average color: " << runningAverage << endl;
        cout << "LAB min: " << labBBox.getMin() << endl;
        cout << "LAB max: " << labBBox.getMax() << endl;

        cout << "Cluster centroids:" << endl;
        for (int i = 0; i < clusterCount; i++)
        {
            vec3f c = clustering.clusterCenter(i);
            //c.x /= LScale;
            c.x = 60.0f;
            clusterLABCentroids.push_back(c);
        }

        sort(clusterLABCentroids.begin(), clusterLABCentroids.end(), [](const vec3f &a, const vec3f &b) { return (a.y) < (b.y); });

        util::serializeToFilePrimitive(filenameBase + ".dat", clusterLABCentroids);

        for (auto &c : clusterLABCentroids)
            cout << c << endl;

        const int colorWidth = 16;
        const int totalHeight = 300;
        ColorImageR8G8B8A8 colorVisualization(colorWidth * clusterLABCentroids.size(), totalHeight);

        for (auto &c : iterate(clusterLABCentroids))
        {
            for (int x = 0; x < colorWidth; x++)
            {
                for (int y = 0; y < totalHeight; y++)
                {
                    //const vec3f yuv(y / (totalHeight - 1.0f), c.value.x, c.value.y);
                    //const vec3f rgb = colorUtil::YUVToRGB(yuv);
                    const vec3f rgb = converter.LABToRGB(vec3f(c.value.x, c.value.y, c.value.z));
                    colorVisualization((int)c.index * colorWidth + x, y) = colorUtil::makeColor8(rgb);
                }
            }
        }

        FreeImageWrapper::saveImage(filenameBase + ".png", colorVisualization);
    }

    cout << "Loading " << filenameBase << ".dat" << endl;
    util::deserializeFromFilePrimitive(filenameBase + ".dat", clusterLABCentroids);
}

ColorImageR8G8B8A8 ColorNetDatabase::quantizeImage(const ColorImageR8G8B8A8 &image, bool useL) const
{
    ColorImageR8G8B8A8 result = image;
    for (auto &p : result)
    {
        const vec3f c32 = colorUtil::makeColor32(p.value);
        const vec3f LAB = converter.RGBToLAB(c32);

        vec3f quantizedLAB = findClosestCentroid(c32);

        if (useL)
            quantizedLAB.x = LAB.x;

        p.value = colorUtil::makeColor8(converter.LABToRGB(quantizedLAB));
    }
    return result;
}

ColorImageR8G8B8A8 ColorNetDatabase::extractChannel(const ColorImageR8G8B8A8 &image, int channelIndex) const
{
    ColorImageR8G8B8A8 result = image;
    for (auto &p : result)
    {
        const vec3f c32 = colorUtil::makeColor32(p.value);
        
        vec3f LAB = converter.RGBToLAB(c32);

        //Min: 0, -86, -107
        //Max: 100, 98, 94

        LAB = LAB + vec3f(0.0f, 86.0f, 107.0f);
        LAB.x /= 100.0f;
        LAB.y /= 98.0f + 86.0f;
        LAB.z /= 94.0f + 107.0f;

        const int v = util::boundToByte(LAB[channelIndex] * 255.0f);
        p.value = vec4uc(v, v, v, 255);
    }
    return result;
}
