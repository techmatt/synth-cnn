
#include "main.h"

void FaceProcessor::convertJPGToPNG()
{
    /*for (auto &jpgImg : Directory::enumerateFiles(baseDir + "fullImagesJPG/", ".jpg"))
    {
    string sourcePath = baseDir + "fullImagesJPG/" + jpgImg;
    string destPath = util::replace(baseDir + "fullImagesPNG/" + jpgImg, ".jpg", ".png");

    if (!util::fileExists(destPath))
    {
    cout << "Converting " << jpgImg << endl;
    ColorImageR8G8B8A8 img;
    FreeImageWrapper::loadImage(sourcePath, img);
    FreeImageWrapper::saveImage(destPath, img);
    }
    }*/
}

void FaceProcessor::evaluateAll(const string &baseDir)
{
    loadFeatures(baseDir + "features/");
    cout << "Evaluating all matches..." << endl;
    for (auto &f : allFaces)
    {
        evaluateMatches(f.second.baseFilename, baseDir);
    }
}

void FaceProcessor::evaluateMatches(const string &baseFilename, const string &baseDir)
{
    cout << "Evaluating " << baseFilename << endl;

    const string croppedDir = baseDir + "croppedImages/";
    const string evalDir = baseDir + "evaluation/";
    util::makeDirectory(evalDir);

    if (allFaces.find(baseFilename) == allFaces.end())
    {
        cout << "Face not found: " << baseFilename << endl;
        return;
    }
    FaceEntry &baseEntry = allFaces[baseFilename];

    vector< pair< FaceEntry*, float > > matchScores;
    for (auto &p : allFaces)
    {
        if (p.second.baseFilename != baseFilename)
        {
            const float matchDist = compareFaces(baseEntry, p.second);
            matchScores.push_back(make_pair(&p.second, matchDist));
        }
    }

    sort(matchScores.begin(), matchScores.end(), [](const pair< FaceEntry*, float > &a, const pair< FaceEntry*, float > &b) { return a.second < b.second; });

    ofstream csvFile(evalDir + baseFilename + ".csv");

    csvFile << "image,dist" << endl;
    for (int matchIndex = 0; matchIndex < min(100, (int)matchScores.size()); matchIndex++)
    {
        auto &m = matchScores[matchIndex];
        csvFile << m.first->baseFilename << "," << m.second << endl;
    }

    ColorImageR8G8B8A8 templateImg = LodePNG::load(baseDir + "template.png");

    const int templateY = 27;

    auto blit = [&](const string &filename, const vec2i &coord) {
        ColorImageR8G8B8A8 localImg;

        if (util::fileExists(filename))
        {
            localImg = LodePNG::load(filename);
        }

        if (localImg.getWidth() <= 32)
        {
            string altFilename = util::replace(filename, "croppedImages", "fullImages");
            altFilename = util::replace(filename, ".png", ".jpg");

            if (util::fileExists(altFilename))
            {
                localImg = LodePNG::load(altFilename);
            }
            else
            {
                cout << "Failed to find file: " << altFilename << endl;
            }
        }
        localImg.reSample(300, 300);

        //templateImg.copyIntoImage(localImg, coord.x, coord.y);
        cout << "localImg: " << localImg.getDimensions() << endl;
        templateImg.copyIntoImage(localImg, 0, 0);
    };

    blit(croppedDir + baseFilename + ".png", vec2i(3, templateY));

    string swappedFilename = baseFilename;
    if (util::contains(baseFilename, "fb")) swappedFilename = util::replace(baseFilename, "fb", "gp");
    else swappedFilename = util::replace(baseFilename, "gp", "fb");

    blit(croppedDir + swappedFilename + ".png", vec2i(332, templateY));

    for (int topK = 0; topK < 5; topK++)
    {
        blit(croppedDir + matchScores[topK].first->baseFilename + ".png", vec2i(661 + 301 * topK, templateY));
    }

    LodePNG::save(templateImg, evalDir + baseFilename + ".png");
}

void FaceProcessor::loadFeatures(const string &featureFolder)
{
    cout << "Loading all features" << endl;
    for (auto &file : Directory::enumerateFiles(featureFolder, ".dat"))
    {
        const auto parts = util::split(file, "_");
        const string baseFilename = parts[0] + "_" + parts[1];
        
        FaceEntry &entry = allFaces[baseFilename];
        entry.baseFilename = baseFilename;

        vector<float> features;
        util::deserializeFromFile(featureFolder + file, features);

        if (features.size() != 4096)
        {
            cout << "Unexpected feature count: " << features.size() << endl;
        }

        entry.featureSet.push_back(features);

        if (allFaces.size() >= 1000)
            return;
    }
}

/*void FaceProcessor::dumpDistanceMatrix(const string &folder, const string &filename)
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
}*/

void FaceProcessor::init()
{
    meanValue = vec3f(93.5940f, 104.7624f, 129.1863f);
    
    net = Netf(new Net<float>(R"(D:\datasets\VGGFace\VGG_FACE_deploy.prototxt)", caffe::TEST));
    net->CopyTrainedLayersFrom(R"(D:\datasets\VGGFace\VGG_FACE.caffemodel)");

    //NetParameter net_param;
    //net_->ToProto(&net_param, param_.snapshot_diff());
    //WriteProtoToBinaryFile(net_param, model_filename);
}

void FaceProcessor::processAll(const string &inputImageFolder, const string &outputCroppedFolder, const string &outputFeatureFolder)
{
    util::makeDirectory(outputCroppedFolder);
    util::makeDirectory(outputFeatureFolder);

    const vector<float> cropSizes = { 0.6f, 0.75f, 0.9f };

    for (auto &inputImageFile : Directory::enumerateFiles(inputImageFolder, ".png"))
    {
        const string baseFilename = util::removeExtensions(inputImageFile);

        for (auto &crop : iterate(cropSizes))
        {
            const string outputFeatureFile = outputFeatureFolder + baseFilename + "_" + to_string(crop.index) + ".dat";
            if (util::fileExists(outputFeatureFile))
                continue;

            ColorImageR8G8B8A8 image = LodePNG::load(inputImageFolder + inputImageFile);

            if (image.getWidth() <= 32 || image.getHeight() <= 32)
                continue;

            //FreeImageWrapper::saveImage(util::replace(file, ".jpg", ".png"), image);

            const vec2f center = vec2f(image.getDimensions()) * 0.5f;
            const vec2f dim = vec2f(image.getDimensions()) * crop.value;
            auto croppedImage = image.getSubregion(bbox2i(math::round(center - dim * 0.5f), math::round(center + dim * 0.5f)));

            croppedImage.reSample(224, 224);

            const vector<float> features = process(croppedImage);

            util::serializeToFile(outputFeatureFile, features);

            //for (auto &p : croppedImage)
            //    p.value = vec3f(p.value.z, p.value.y, p.value.x);
            LodePNG::save(croppedImage, outputCroppedFolder + baseFilename + "_" + to_string(crop.index) + ".png");
        }
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