
#include "main.h"

struct ImageData
{
    Grid3f g0;
    Grid3f g1;
};

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator<<(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, const ImageData &data) {
    s.writePrimitive(data.g0);
    s.writePrimitive(data.g1);
    return s;
}

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator>>(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, ImageData &data) {
    s.readPrimitive(data.g0);
    s.readPrimitive(data.g1);
    return s;
}

void goB()
{
    ml::mBase::Writer<ImageData> writer(R"(D:\datasets\ColorNet\mBaseTest\)");

    ImageData data;
    data.g0.allocate(1, 1, 1);
    writer.addRecord(data);
    data.g0.allocate(2, 2, 2);
    writer.addRecord(data);

    writer.finalize();

    ml::mBase::Reader<ImageData> reader(R"(D:\datasets\ColorNet\mBaseTest\)", 32);

    ImageData data2;
    reader.readNextRecord(data2);
    cout << data2.g0.getDimensions() << endl;
    reader.readNextRecord(data2);
    cout << data2.g0.getDimensions() << endl;
    reader.readNextRecord(data2);
    cout << data2.g0.getDimensions() << endl;
    reader.readNextRecord(data2);
    cout << data2.g0.getDimensions() << endl;

    cin.get();

    return;

    set<char> trainChars, testChars;
    for (char c = '0'; c <= '9'; c++)
    {
        trainChars.insert(c);
    }

    trainChars.insert('a');
    trainChars.insert('b');

    testChars.insert('c');
    testChars.insert('d');
    testChars.insert('e');
    testChars.insert('f');

    ImageDatabase database;
    database.initSynthNet();
    database.saveLevelDB(constants::synthDatabaseDir + "trainSynthDatabase", TestSplit(trainChars), 200000);
    database.saveLevelDB(constants::synthDatabaseDir + "testSynthDatabase", TestSplit(testChars), 10000);
}

void main()
{
    goB();
    //goA();
    
    cout << "done!" << endl;
    cin.get();
}
