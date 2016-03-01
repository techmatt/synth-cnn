
#include "main.h"

void NetworkProcessor::init()
{
    const string baseDir = R"(D:\datasets\Netflix\caffe\)";
    const string netFilename = baseDir + "netflix-net-eval.prototxt";
    const string modelFilename = baseDir + "netflix.caffemodel";

    net = Netf(new Net<float>(netFilename, caffe::TEST));
    net->CopyTrainedLayersFrom(modelFilename);
}

const bool saveNetsMode = false;
int globalUserIndex = 0;
double NetworkProcessor::evaluateRating(const NetflixDatabase &database, const Rating &rating)
{
    Grid3f inputData;
    const int channelCount = database.movieIndexCount * 3 + 1;
    vector<BYTE> rawVector(channelCount);

    database.makeLinearFeatures(rating, rawVector.data());

    inputData = Grid3f(1, 1, channelCount);
    for (auto &c : inputData)
    {
        const vec3i coord(c.x, c.y, c.z);
        const BYTE b = rawVector[c.z];

        const float scale = 1.0f / 255.0f;
        c.value = ((float)b - 128.0f) * scale;
    }
    
    if (saveNetsMode)
    {
        net->ForwardFrom(0);
        CaffeUtil::saveNetToDirectory(net, "netBefore" + to_string(globalUserIndex) + "/");
    }
    CaffeUtil::runNetForward(net, "slicer", "data", inputData);
    if (saveNetsMode) CaffeUtil::saveNetToDirectory(net, "netAfter" + to_string(globalUserIndex) + "/");
    
    globalUserIndex++;

    if (saveNetsMode && globalUserIndex == 6)
        exit(0);
    
    auto grid = CaffeUtil::getBlobAsGrid(net, "concatFC6");
    
    const double baseScore = grid(0, 0, 0) * 255.0 + 128.0;
    const double score = math::linearMap(0.0, 255.0, 1.0, 5.0, baseScore);
    const double clampedScore = math::clamp(score, 1.0, 5.0);
    return clampedScore;
}

void NetworkProcessor::evaluateAllUsers(const NetflixDatabase &database, const vector<Rating> &ratings, const string &filename)
{
    ofstream file(filename);
    file << "user,movie,rating,prediction,error" << endl;

    double errorSum = 0.0;
    int count = 0;
    int skip = 10000;
    for (int ratingIndex = 0; ratingIndex < ratings.size(); ratingIndex += skip)
    {
        const auto &r = ratings[ratingIndex];

        double score = evaluateRating(database, r);
        double error = r.rating - score;
        
        errorSum += error * error;
        count++;

        if (count % 100 == 0)
            cout << "Rating r=" << (int)r.rating << " p=" << score << " (" << ratingIndex << " / " << ratings.size() << ")" << endl;

        file << r.userID << "," << r.movieIndex << "," << r.rating << "," << score << "," << error << endl;
    }
    
    file << endl << endl;
    file << "RMSE:," << sqrt(errorSum / (double)count) << endl;

}

void NetworkProcessor::evaluateAllUsers(const NetflixDatabase &database)
{
    evaluateAllUsers(database, database.testRatings, constants::netflixDir + "caffe/test.csv");
    evaluateAllUsers(database, database.trainRatings, constants::netflixDir + "caffe/train.csv");
}
/*
void NetworkProcessor::outputPatients(const string &filename) const
{
    ofstream file(filename);

    file << "netoutcome: " << patients[0].netOutcome.size() << endl;
    
    file << "index,unstim,stim,status,label,survival time,truth,pred,diff";
    for (int i = 0; i < constants::survivalIntervals; i++)
        file << ",i" << i;
    file << endl;

    double trainingError = 0.0;
    int trainingCount = 0;
    double testError = 0.0;
    int testCount = 0;

    for (auto &p : patients)
    {
        ostringstream s;
        s << p.patient.index;
        s << "," << p.patient.fileUnstim;
        s << "," << p.patient.fileStim;
        s << "," << p.patient.status;
        s << "," << p.patient.label;
        s << "," << p.patient.survivalTime;

        file << s.str();
        for (auto &v : p.patient.makeOutcomeVector())
            file << "," << v;
        file << ",";
        for (auto &v : p.netOutcome)
            file << "," << setprecision(3) << v;
        file << ",";
        for (int i = 0; i < p.netOutcome.size(); i++)
            file << "," << setprecision(3) << p.patient.makeOutcomeVector()[i] - p.netOutcome[i];

        file << endl;

        double errorSum = 0.0;
        for (int i = 0; i < p.netOutcome.size(); i++)
            errorSum += math::abs(p.netOutcome[i] - p.patient.makeOutcomeVector()[i]);

        if (p.test)
        {
            testError += errorSum;
            testCount++;
        }
        else
        {
            trainingError += errorSum;
            trainingCount++;
        }
    }

    file << "Training error," << trainingError / trainingCount << endl;
    file << "Test error," << testError / testCount << endl;
}
*/