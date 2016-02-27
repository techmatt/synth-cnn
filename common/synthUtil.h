
struct RandomImageLoader
{
    void init();

    vector<string> allImages;

    ColorImageR8G8B8A8 getRandomImage();
    ColorImageR8G8B8A8 getRandomImage(int width, int height);
};

struct synthUtil
{
    static void runMitsuba(const string &filename)
    {
        util::runCommand(string("mitsuba \"") + filename + "\"");
        util::runCommand(string("mtsutil tonemap \"") + util::replace(filename, ".xml", ".exr") + "\"");
    }

    static ColorImageR8G8B8A8 makeDownsampledMask(const ColorImageR8G8B8A8 &input, int downsampleFactor)
    {
        ColorImageR8G8B8A8 result(input.getWidth() / downsampleFactor, input.getHeight() / downsampleFactor);

        for (auto & p : result)
        {
            int inPixels = 0;
            for (int y = 0; y < downsampleFactor; y++)
                for (int x = 0; x < downsampleFactor; x++)
                {
                    if (input(p.x * downsampleFactor + x, p.y * downsampleFactor + y) != vec4uc(255, 0, 255, 255))
                    {
                        inPixels++;
                    }
                }

            const float v = (float)inPixels / (float)(downsampleFactor * downsampleFactor) * 255.0f;
            const BYTE b = util::boundToByte(v);
            p.value = vec4uc(b, b, b, 255);
        }
        
        return result;
    }

    static ColorImageR8G8B8A8 compositeRandomImage(AppState &state, const ColorImageR8G8B8A8 &renderedImage, const ColorImageR8G8B8A8 &mask);
};