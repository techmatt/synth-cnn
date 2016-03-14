
#include "main.h"

void goB()
{
    /*NetflixDatabase database;
    //database.loadText();
    //database.saveBinary();
    database.loadBinary();

    cout << "movies: " << database.movieIndexCount << endl;

    database.saveLevelDB(constants::netflixDir + "caffe/LevelDBTrain", 1000);
    database.saveLevelDB(constants::netflixDir + "caffe/LevelDBTest", 1000);*/

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
    database.saveLevelDB(constants::synthLearningDir + "trainSynthDatabase", TestSplit(trainChars), 50000);
    database.saveLevelDB(constants::synthLearningDir + "testSynthDatabase", TestSplit(testChars), 10000);
}

void main()
{
    goB();
    //goA();
    
    cout << "done!" << endl;
    cin.get();
}
