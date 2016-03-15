
namespace ml
{

namespace mBase
{

struct RecordOffset : public BinaryDataSerialize< RecordOffset >
{
    UINT64 recordIndex;
    UINT64 fileIndex;
    UINT64 recordSize;
    UINT64 byteOffset;
};

template<class RecordType>
class Writer
{
public:
    Writer(const string &directory)
    {
        init(directory);
    }

    ~Writer()
    {
        finalize();
    }

    void init(const string &_directory)
    {
        util::makeDirectory(_directory);
        directory = _directory;
        activeFile = nullptr;
        activeFileIndex = 0;
        activeFileSize = 0;
    }

    void updateActiveFile()
    {
        const int maxFileSize = 1024 * 1024 * 256;
        if (activeFileSize >= maxFileSize && activeFile)
        {
            fclose(activeFile);
            activeFile = nullptr;
            activeFileSize = 0;
            activeFileIndex++;
        }

        if (activeFile == nullptr)
        {
            activeFile = util::checkedFOpen(directory + util::zeroPad(activeFileIndex, 3), "wb");
        }
    }

    void addRecord(const RecordType &record)
    {
        updateActiveFile();

        BinaryDataStreamVector out;
        out << record;
        const vector<BYTE> compressedData = ZLibWrapper::CompressStreamToMemory(out.getData(), false);

        RecordOffset newRecord;
        newRecord.recordIndex = records.size();
        newRecord.byteOffset = activeFileSize;
        newRecord.fileIndex = activeFileIndex;
        newRecord.recordSize = compressedData.size();
        records.push_back(newRecord);

        util::checkedFWrite(compressedData.data(), sizeof(BYTE), compressedData.size(), activeFile);
        activeFileSize += compressedData.size();
    }

    void finalize()
    {
        if(activeFile) fclose(activeFile);
        util::serializeToFile(directory + "records.dat", records);
        cout << "Saved " << records.size() << " records" << endl;
    }

private:
    string directory;
    UINT64 activeFileSize;
    int activeFileIndex;
    FILE *activeFile;
    vector<RecordOffset> records;
};

template<class RecordType>
class Reader
{
public:
    Reader(const string &_directory, size_t _cacheSize)
    {
        init(directory, _cacheSize);
    }

    void init(const string &_directory, size_t _cacheSize)
    {
        directory = _directory;
        cacheSize = _cacheSize;
        terminateThread = false;
        activeRecordIndex = 0;
        util::deserializeFromFile(directory + "records.dat", records);
        cout << "Loaded " << records.size() << " records" << endl;
    }

    void readRecord(UINT64 recordIndex, RecordType &result)
    {
        const RecordOffset &offset = records[recordIndex];
        const string filename = directory + util::zeroPad(offset.fileIndex, 3);
        
        FILE* file = fopen(filename.c_str(), "rb");
        if (file == nullptr || ferror(file))
        {
            cout << "Failed to open file: " << filename << endl;
            return;
        }

        fseek(file, offset.byteOffset, SEEK_SET);

        if (cacheStorage.size() < offset.recordSize)
            cacheStorage.resize(offset.recordSize);
        fread(cacheStorage.data(), offset.recordSize, 1, file);

        fclose(file);
        
        vector<BYTE> uncompressedData = ZLibWrapper::DecompressStreamFromMemory(cacheStorage);

        BinaryDataStreamVector out;
        out.setData(std::move(uncompressedData));
        out >> result;
    }

    void readNextRecord(RecordType &result)
    {
        readRecord(activeRecordIndex, result);
        activeRecordIndex++;
        if (activeRecordIndex == records.size())
        {
            cout << "All records read; restarting from beginning" << endl;
            activeRecordIndex = 0;
        }
    }

private:
    string directory;
    vector<RecordOffset> records;
    UINT64 activeRecordIndex;

    size_t cacheSize;
    std::list<RecordType> cache;
    std::thread decompThread;
    std::mutex listMutex;
    bool terminateThread;
    vector<BYTE> cacheStorage;
};

}

}