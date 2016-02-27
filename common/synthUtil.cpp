
#include "main.h"

void RandomImageLoader::init()
{
    for(auto &d : Directory::enumerateDirectoriesWithPath(constants::randomImagePath))
    {
        for (auto &s : Directory::enumerateFilesWithPath(d, ".jpg"))
        {
            allImages.push_back(s);
        }
    }
    cout << "Total images: " << allImages.size() << endl;
}

ColorImageR8G8B8A8 RandomImageLoader::getRandomImage()
{
    ColorImageR8G8B8A8 image;
    const string &filename = util::randomElement(allImages);
    FreeImageWrapper::loadImage(filename, image);
    return image;
}

ColorImageR8G8B8A8 RandomImageLoader::getRandomImage(int width, int height)
{
    ColorImageR8G8B8A8 source;
    while ((int)source.getWidth() < width || (int)source.getHeight() < height)
    {
        source = getRandomImage();
    }

    ColorImageR8G8B8A8 result(width, height);

    const vec2i seed(util::randomInteger(0, source.getWidth()  - width ),
                     util::randomInteger(0, source.getHeight() - height));

    for (auto &p : result)
    {
        p.value = source(p.x + seed.x, p.y + seed.y);
    }

    return result;
}

ColorImageR8G8B8A8 synthUtil::compositeRandomImage(AppState &state, const ColorImageR8G8B8A8 &renderedImage, const ColorImageR8G8B8A8 &mask)
{
    const ColorImageR8G8B8A8 background = state.randomImageLoader.getRandomImage(renderedImage.getWidth(), renderedImage.getHeight());

    ColorImageR8G8B8A8 result = renderedImage;
    for (auto &p : result)
    {
        const float s = mask(p.x, p.y).r / 255.0f;
        const vec4f bColor = background(p.x, p.y);
        const vec4f fColor = p.value;
        const vec4f composite = math::lerp(bColor, fColor, s);
        p.value = vec4uc(util::boundToByte(composite.x), util::boundToByte(composite.y), util::boundToByte(composite.z), 255);
    }
    return result;
}
