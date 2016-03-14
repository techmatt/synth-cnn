
struct DatasetSplit
{
    DatasetSplit() {}
    DatasetSplit(const set<char> &_acceptedCharacters)
    {
        acceptedCharacters = _acceptedCharacters;
    }

    bool check(const string &filename) const
    {
        const char c = util::getFilenameFromPath(filename)[0];
        return acceptedCharacters.count(c) > 0;
    }

    static DatasetSplit splitTest()
    {
        set<char> testChars;
        testChars.insert('c');
        testChars.insert('d');
        testChars.insert('e');
        testChars.insert('f');
        return DatasetSplit(testChars);
    }

    static DatasetSplit splitTrain()
    {
        set<char> trainChars;
        for (char c = '0'; c <= '9'; c++)
            trainChars.insert(c);
        trainChars.insert('a');
        trainChars.insert('b');
        return DatasetSplit(trainChars);
    }

    set<char> acceptedCharacters;
};

struct ImageCategory
{
    pair<ColorImageR8G8B8A8, string> makeRandomSample(const DatasetSplit &split) const;

    string name;
    int index;

    vector<string> imageFilenames;
};

struct ImageDatabase
{
    void initImageNet();
    void initSynthNet();
    void saveLevelDB(const string &outDir, const DatasetSplit &split, int sampleCount);

    vector<ImageCategory> categories;
};
