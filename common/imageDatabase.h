
struct TestSplit
{
    TestSplit() {}
    TestSplit(const set<char> &_acceptedCharacters)
    {
        acceptedCharacters = _acceptedCharacters;
    }
    bool check(const string &filename) const
    {
        const char c = util::getFilenameFromPath(filename)[0];
        return acceptedCharacters.count(c) > 0;
    }

    set<char> acceptedCharacters;
};

struct ImageCategory
{
    ColorImageR8G8B8A8 makeRandomSample(const TestSplit &split) const;

    string name;
    int index;

    vector<string> imageFilenames;
};

struct ImageDatabase
{
    void initImageNet();
    void initSynthNet();
    void saveLevelDB(const string &outDir, const TestSplit &split, int sampleCount);

    vector<ImageCategory> categories;
};
