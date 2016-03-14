
#include "main.h"

/*
void LevelDBExporter::exportDB(const FCSDataset &dataset, const string &outDir, int epochs, int afflictedOversampleCount, int evalSamplesPerPatient, int startPatient, int endPatient)
{
    const int pixelCount = constants::imageSize * constants::imageSize;
    const int featureCount = (int)dataset.selectedFeatures.size();
    const int linearFeatureCount = featureCount * constants::coordsPerFeature;

    vector<const Patient*> patients;
    for (int i = startPatient; i < endPatient; i++)
        patients.push_back(&dataset.patients[i]);

    for (int j = 0; j < afflictedOversampleCount; j++)
    {
        for (int i = startPatient; i < endPatient; i++)
        {
            const Patient &p = dataset.patients[i];
            if (p.getOutcome() < 1.0f)
                patients.push_back(&p);
        }
    }

    const int patientCount = patients.size();

    // leveldb
    leveldb::DB* db;
    leveldb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    options.write_buffer_size = 268435456;

    // Open dbs
    std::cout << "Opening leveldbs " << outDir << endl;
    leveldb::Status status = leveldb::DB::Open(options, outDir, &db);
    if (!status.ok())
    {
        std::cout << "Failed to open " << outDir << " or it already exists" << endl;
        return;
    }

    leveldb::WriteBatch* batch = new leveldb::WriteBatch();

    // Storing to db
    BYTE* rawImageFeatures = new BYTE[pixelCount * featureCount];
    
    int count = 0;
    const int kMaxKeyLength = 10;
    char key_cstr[kMaxKeyLength];
    
    ColorImageR8G8B8A8 dummyImage(constants::imageSize, constants::imageSize);

    vector<FeatureDescription> selectedFeaturesList;
    vector<vec3i> featureCoords;
    for (auto &f : iterate(dataset.selectedFeatures))
    {
        selectedFeaturesList.push_back(f.value->desc);
        for (vec2i c : f.value->bestCoords)
            featureCoords.push_back(vec3i(c.x, c.y, f.index));
    }

    cout << "A total of " << patientCount * epochs << " samples will be generated." << endl;
    int totalSampleIndex = 0;
    for (int epoch = 0; epoch < epochs; epoch++)
    {
        cout << "Start epoch " << epoch << endl;

        auto shuffledPatients = patients;
        random_shuffle(shuffledPatients.begin(), shuffledPatients.end());

        for (auto &p : shuffledPatients)
        {
            if (totalSampleIndex % 20 == 0)
                cout << "Sample " << totalSampleIndex << " / " << patientCount * epochs << endl;
            totalSampleIndex++;

            FCSFile fileUnstim, fileStim;
            fileUnstim.loadBinary(dataset.baseDir + "DAT/" + util::removeExtensions(p->fileUnstim) + ".dat");
            fileStim.loadBinary(dataset.baseDir + "DAT/" + util::removeExtensions(p->fileStim) + ".dat");
            dataset.processor.transform(fileUnstim);
            dataset.processor.transform(fileStim);

            FCSFeatures features;
            features.create(dataset.processor, fileUnstim, fileStim, constants::imageSize, selectedFeaturesList);

            Datum datum;
            datum.set_label(p->index);
            if (constants::linearFeatures)
            {
                vector<BYTE> linearFeatures = features.makeLinearFeatures(featureCoords);

                int predictionChannels = 1;
                if (constants::directPrediction)
                    linearFeatures.push_back(util::boundToByte(math::linearMap(0.0f, 1000.0f, 0.0f, 255.0f, (float)p->survivalTime)));
                else
                {
                    for (float f : p->makeOutcomeVector())
                        linearFeatures.push_back(util::boundToByte(f * 255.0f));
                    predictionChannels = constants::survivalIntervals;
                }

                datum.set_channels(linearFeatureCount + predictionChannels);
                datum.set_height(1);
                datum.set_width(1);
                datum.set_data(linearFeatures.data(), linearFeatureCount + predictionChannels);
            }
            else
            {
                int pIndex = 0;
                for (int feature = 0; feature < featureCount; feature++)
                {
                    for (const auto &p : dummyImage)
                    {
                        rawImageFeatures[pIndex++] = features.features(p.x, p.y, feature);
                    }
                }

                datum.set_channels(featureCount);
                datum.set_height(constants::imageSize);
                datum.set_width(constants::imageSize);
                datum.set_data(rawImageFeatures, pixelCount * featureCount);
            }

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
    }
    // write the last batch
    if (count % 1000 != 0) {
        db->Write(leveldb::WriteOptions(), batch);
    }
    delete batch;
    delete db;
    cout << "Processed " << count << " entries." << endl;

    vector<PatientFeatureSample> allSamples;
    allSamples.reserve(patients.size() * evalSamplesPerPatient);

    for (int i = startPatient; i < endPatient; i++)
    {
        const Patient &p = dataset.patients[i];
        cout << "Sampling patient " << p.index << endl;
        
        FCSFile fileUnstim, fileStim;
        fileUnstim.loadBinary(dataset.baseDir + "DAT/" + util::removeExtensions(p.fileUnstim) + ".dat");
        fileStim.loadBinary(dataset.baseDir + "DAT/" + util::removeExtensions(p.fileStim) + ".dat");
        dataset.processor.transform(fileUnstim);
        dataset.processor.transform(fileStim);

        for (int sample = 0; sample < evalSamplesPerPatient; sample++)
        {
            FCSFeatures features;
            features.create(dataset.processor, fileUnstim, fileStim, constants::imageSize, selectedFeaturesList);

            PatientFeatureSample newPatientSample;
            newPatientSample.patient = p;
            newPatientSample.imageFeatures = features.features;
            newPatientSample.linearFeatures = features.makeLinearFeatures(featureCoords);
            allSamples.push_back(newPatientSample);
        }
    }

    util::serializeToFileCompressed(outDir + "Samples.dat", allSamples);
}
*/