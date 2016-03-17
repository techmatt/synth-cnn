
class ColorConverter
{
public:
    void init();

    vec3f RGBToLAB(const vec3f &v) const;
    vec3f LABToRGB(const vec3f &v) const;

private:
    Grid3<vec3f> RGBToLABTable;
    Grid3<vec3f> LABToRGBTable;
};

namespace colorUtil
{
    inline vec3f RGBToYUV(const vec3f &v)
    {
        vec3f result;
        result.x = vec3f(0.299f, 0.587f, 0.114f) | v;
        result.y = vec3f(-0.14713f, -0.28886f, 0.436f) | v;
        result.z = vec3f(0.615f, -0.51499f, -0.10001f) | v;
        return result;
    }

    inline vec3f YUVToRGB(const vec3f &v)
    {
        vec3f result;
        result.x = vec3f(1.0f, 0.0f, 1.13983f) | v;
        result.y = vec3f(1.0f, -0.39465f, -0.58060f) | v;
        result.z = vec3f(1.0f, 2.03211f, 0.0f) | v;
        return result;
    }

    inline vec4uc makeColor8(const vec3f &v)
    {
        return vec4uc(util::boundToByte(v.x * 255.0f), 
                      util::boundToByte(v.y * 255.0f), 
                      util::boundToByte(v.z * 255.0f), 255);
    }

    inline vec3f makeColor32(const vec4uc &v)
    {
        return vec3f(v.getVec3()) / 255.0f;
    }
}