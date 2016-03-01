
typedef caffe::shared_ptr< Blob<float> > Blobf;
typedef caffe::shared_ptr< Net<float> > Netf;

struct BlobInfo
{
    BlobInfo() {}
    BlobInfo(string _name, string _suffix, int _channels) {
        name = _name;
        suffix = _suffix;
        channelsToOutput = _channels;
    }
    string name;
    string suffix;
    int channelsToOutput;

    Blobf data;
};

struct CaffeUtil
{
    static void grid3ToDatum(const Grid3<float> &grid, Datum &datum)
    {
        datum.set_channels(grid.getDimZ());
        datum.set_width(grid.getDimX());
        datum.set_height(grid.getDimY());

        for (int z = 0; z < grid.getDimZ(); z++)
            for (int x = 0; x < grid.getDimX(); x++)
                for (int y = 0; y < grid.getDimY(); y++)
                {
                    datum.add_float_data(grid(x, y, z));
                }

        datum.set_label(0);

        /*if (sampleIndex == 0 && representationFrameIndex == 0)
        {
        LOG(ERROR) << "dataA dim: " << dataA.getDimensions();
        LOG(ERROR) << "dataB dim: " << dataB.getDimensions();

        for (int z = 0; z < dataA.getDimZ(); z++)
        LOG(ERROR) << dataA(0, 0, z);
        for (int z = 0; z < dataB.getDimZ(); z++)
        LOG(ERROR) << dataB(0, 0, z);
        }*/
    }

    static ColorImageR8G8B8A8 blobToImage(const Blobf &blob, int imageIndex, int channelStartIndex, const Grid2<vec3f> &meanValues)
    {
        ColorImageR8G8B8A8 image(blob->width(), blob->height());
        for (auto &p : image)
        {
            for (int channel = 0; channel < 3; channel++)
            {
                const float *dataStart = blob->cpu_data() + blob->offset(imageIndex, channelStartIndex + channel, p.y, p.x);
                const BYTE value = util::boundToByte(*dataStart * 255.0f + meanValues(p.x, p.y)[channel]);
                p.value[channel] = value;
            }
            p.value.a = 255;
        }
        return image;
    }

    static Grid2<float> blobToGridFloat(const Blob<float> &blob, int imageIndex, int channelIndex)
    {
        Grid2<float> grid(blob.width(), blob.height());
        for (auto &p : grid)
        {
            const float *dataStart = blob.cpu_data() + blob.offset(imageIndex, channelIndex, p.y, p.x);
            p.value = *dataStart;
        }
        return grid;
    }

    static void loadGrid2IntoBlob(const Grid2<float> &grid, Blobf &blob, int imageIndex, int channelIndex)
    {
        float *cpuPtr = (float*)blob->data()->mutable_cpu_data();
        for (auto &p : grid)
        {
            float *dataStart = cpuPtr + blob->offset(imageIndex, channelIndex, p.y, p.x);
            *dataStart = p.value;
        }
    }

    static void loadVectorIntoBlob(const vector<float> &values, Blobf &blob, int imageIndex, int channelIndex)
    {
        float *cpuPtr = (float*)blob->data()->mutable_cpu_data();
        for (int v = 0; v < values.size(); v++)
        {
            float *dataStart = cpuPtr + blob->offset(imageIndex, channelIndex, 0, v);
            *dataStart = values[v];
        }
    }

    static Grid3f gridFromBinaryProto(const string &filename)
    {
        BlobProto blobProto;
        caffe::ReadProtoFromBinaryFileOrDie(filename.c_str(), &blobProto);

        Blob<float> blob;
        blob.FromProto(blobProto);

        LOG(ERROR) << "binary proto dimensions (x,y): " << blob.width() << "," << blob.height() << " channels: " << blob.channels();
        return blobToGrid3(blob, 0);
    }

    static Grid3<float> blobToGrid3(const Blob<float> &blob, int imageIndex)
    {
        Grid3<float> result;
        result.allocate(blob.width(), blob.height(), blob.channels());

        for (int z = 0; z < result.getDimZ(); z++)
            for (int y = 0; y < result.getDimY(); y++)
                for (int x = 0; x < result.getDimX(); x++)
                {
                    const float *dataStart = blob.cpu_data() + blob.offset(imageIndex, z, y, x);
                    result(x, y, z) = *dataStart;
                }
        return result;
    }

    static void loadGrid3IntoBlob(const Grid3<float> &grid, Blobf &blob, int imageIndex)
    {
        //LOG(ERROR) << "dim: " << grid.getDimensions() << ", blob: " << blob->width() << " " << blob->height() << " " << blob->channels();
        float *cpuPtr = (float*)blob->data()->mutable_cpu_data();
        for (int z = 0; z < grid.getDimZ(); z++)
            for (int y = 0; y < grid.getDimY(); y++)
                for (int x = 0; x < grid.getDimX(); x++)
                {
                    float *dataStart = cpuPtr + blob->offset(imageIndex, z, y, x);
                    *dataStart = grid(x, y, z);
                }
    }

    static int getLayerIndex(const Netf &net, const string &layerName)
    {
        int result = -1;
        for (int layerIndex = 0; layerIndex < net->layers().size(); layerIndex++)
        {
            if (net->layer_names()[layerIndex] == layerName)
                result = layerIndex;
        }
        if (result == -1)
            LOG(ERROR) << "Input layer not found: " << layerName;
        return result;
    }

    static void runNetForward(const Netf &net, const string &inputLayerName)
    {
        const int inputLayerIndex = getLayerIndex(net, inputLayerName);
        net->ForwardFrom(inputLayerIndex + 1);
    }

    static void runNetForward(const Netf &net, const string &inputLayerName, const string &blobName, const Grid3<float> &inputLayerData)
    {
        if (!net->has_blob(blobName))
            LOG(ERROR) << "Blob not found: " << blobName;

        Blobf inputBlob = net->blob_by_name(blobName);

        loadGrid3IntoBlob(inputLayerData, inputBlob, 0);

        const int inputLayerIndex = getLayerIndex(net, inputLayerName);
        net->ForwardFrom(inputLayerIndex);
    }

    static void runNetForwardTo(const Netf &net, const string &inputLayerName, const string &blobName, const Grid3<float> &inputLayerData, const string &finalLayerName)
    {
        if (!net->has_blob(blobName))
            LOG(ERROR) << "Blob not found: " << blobName;

        Blobf inputBlob = net->blob_by_name(blobName);

        loadGrid3IntoBlob(inputLayerData, inputBlob, 0);

        const int inputLayerIndex = getLayerIndex(net, inputLayerName);
        const int finalLayerIndex = getLayerIndex(net, finalLayerName);
        net->ForwardFromTo(inputLayerIndex, finalLayerIndex);
    }

    static void saveGrid3ToFile(const Grid3f &grid, const string &filename)
    {
        ofstream file(filename);
        file << "dimensions: " << grid.getDimensions() << endl;
        for (auto &p : grid)
        {
            if (p.x == 0)
                file << endl;
            file << p.value << " ";
        }
    }

    static void saveNetToDirectory(const Netf &net, const string &dir)
    {
        util::makeDirectory(dir);
        for (auto &blobName : net->blob_names())
        {
            auto &blob = net->blob_by_name(blobName);
            Grid3f g = blobToGrid3(*blob, 0);
            saveGrid3ToFile(g, dir + blobName + ".txt");
        }
    }

    static Grid3<float> getBlobAsGrid(const Netf &net, const string &blobName)
    {
        if (!net->has_blob(blobName))
            LOG(ERROR) << "blob not found: " << blobName;

        auto blob = net->blob_by_name(blobName);

        return blobToGrid3(*blob.get(), 0);
    }

    static vector<float> gridToVector(const Grid3f &grid)
    {
        vector<float> result;
        for (auto &p : grid)
            result.push_back(p.value);
        return result;
    }
};