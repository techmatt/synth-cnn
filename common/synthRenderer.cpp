#include "Main.h"

void SynthRenderer::init(D3D11GraphicsDevice &graphics, UINT smallDimension, UINT bigDimension)
{
    renderTargetSmall.load(graphics, smallDimension, smallDimension);
    renderTargetBig.load(graphics, bigDimension, bigDimension);
}

Cameraf SynthRenderer::randomCamera(const Scene &s)
{
    const float randomLookVariance = 0.2f;
    const float targetBBoxExpansion = 1.5f;

    const OBB3f objectOBB = s.objects[s.mainObjectIndex].worldBBox;
    
    bbox3f targetBBox;
    targetBBox.include(bbox3f(objectOBB));

    const auto r = [&](float a, float b) { return (float)ml::util::randomUniform(a, b); };
    const auto rA = [&]() { return (float)r(0.0f, 1.0f); };
    const auto rB = [&]() { return (float)r(-1.0f, 1.0f); };

    const vec3f lookAt = objectOBB.getCenter() +
                         vec3f(rB(), rB(), rB()) * randomLookVariance;

    vec3f eye;

    eye.x = r(targetBBox.getMin().x - targetBBoxExpansion, targetBBox.getMax().x + targetBBoxExpansion);
    eye.y = r(targetBBox.getMin().y - targetBBoxExpansion, targetBBox.getMax().y + targetBBoxExpansion);
    eye.z = r(0.5f, 2.0f);

    return Cameraf(eye, lookAt - eye, vec3f::eZ, constants::fieldOfView, 1.0f, 0.01f, 100.0f);
}

bool SynthRenderer::goodRandomCamera(AppState &state, const Scene &s, Cameraf &bestCamera)
{
    double bestCameraQuality = 0.0;
    for (UINT i = 0; i < 100; i++)
    {
        const Cameraf c = randomCamera(s);
        auto result = render(state, s, c, true);
        if (result.quality > bestCameraQuality)
        {
            bestCameraQuality = result.quality;
            bestCamera = c;
            break;
        }
    }

    if (bestCameraQuality > 0.0)
        return true;
    return false;
}

SynthRenderResult SynthRenderer::renderRandomCamera(AppState &state, const Scene &s)
{
    Cameraf bestCamera;
    if (!goodRandomCamera(state, s, bestCamera))
    {
        return SynthRenderResult();
    }

    return render(state, s, bestCamera, false);
}

SynthRenderResult SynthRenderer::render(AppState &state, const Scene &s, const Cameraf &c, bool qualityEstimateOnly)
{
    D3D11RenderTarget &renderTarget = qualityEstimateOnly ? renderTargetSmall : renderTargetBig;

    Bitmap occludedObjectColor, unoccludedObjectColor;
    
    //
    // unoccluded rendering
    //
    renderTarget.bind();
    renderTarget.clear(vec4f(1.0f, 0.0f, 1.0f, 1.0f));
    s.objects[s.mainObjectIndex].render(state, c);
    renderTarget.captureColorBuffer(unoccludedObjectColor);

    //
    // occluded rendering
    //
    renderTarget.bind();
    renderTarget.clear(vec4f(1.0f, 0.0f, 1.0f, 1.0f));

    for (UINT objectIndex = 0; objectIndex < s.objects.size(); objectIndex++)
    {
        if (objectIndex != s.mainObjectIndex)
            s.objects[objectIndex].render(state, c);
    }

    renderTarget.clearColor(vec4f(1.0f, 0.0f, 1.0f, 1.0f));

    s.objects[s.mainObjectIndex].render(state, c);
    
    renderTarget.captureColorBuffer(occludedObjectColor);

    state.graphics->bindRenderTarget();

    SynthRenderResult result;

    if (!qualityEstimateOnly)
    {
        result.occludedObjectColor = occludedObjectColor;
    }

    ml::vec4uc magenta(255, 0, 255, 255);

    auto countPixels = [&](const Bitmap &image)
    {
        size_t count = 0;
        for (const auto &p : image)
            if (p.value != magenta)
                count++;
        return count;
    };

    auto borderClear = [&](const Bitmap &image)
    {
        for (int y = 0; y < (int)image.getHeight(); y++)
            if (image(y, 0) != magenta || image(y, (int)image.getWidth() - 1) != magenta)
                return false;
        for (int x = 0; x < (int)image.getWidth(); x++)
            if (image(0, x) != magenta || image((int)image.getHeight() - 1, x) != magenta)
                return false;
        return true;
    };
    
    double occlusionPercentage = 0.0;
    double totalScreenCoverage = 0.0;
    const size_t occludedObjectPixels = countPixels(occludedObjectColor);
    const size_t totalObjectPixels = countPixels(unoccludedObjectColor);
    double borderQuality = 0.0;

    if (borderClear(unoccludedObjectColor))
    {
        borderQuality = 1.0;
    }

    result.quality = 0.0;

    if (totalObjectPixels > 0)
    {
        occlusionPercentage = 1.0 - (double)occludedObjectPixels / (double)totalObjectPixels;
        totalScreenCoverage = (double)occludedObjectPixels / double(occludedObjectColor.getNumPixels());

        const int minTotalPixelCount = 1000;
        const float maxOcclusionPercentage = 0.4;
        const float minScreenCoverage = 0.15;
        const float maxScreenCoverage = 0.7;

        if (occludedObjectPixels >= minTotalPixelCount &&
            occlusionPercentage <= maxOcclusionPercentage &&
            totalScreenCoverage >= minScreenCoverage &&
            totalScreenCoverage <= maxScreenCoverage &&
            borderQuality > 0.0)
        {
            // TODO: this is not a good metric because it favors unoccluded too much...
            //result.quality = ml::math::max(totalScreenCoverage, 0.1);
            result.quality = 1.0;
        }
    }

    const ModelInstance &object = s.objects[s.mainObjectIndex];
    //const vec3f objectCenter = object.modelToWorld * object.object.model->bbox.getCenter();
    //const vec3f objectFrameX = (object.modelToWorld * (object.object.model->bbox.getCenter() + vec3f::eX) - objectCenter).getNormalized();
    //const vec3f objectFrameY = (object.modelToWorld * (object.object.model->bbox.getCenter() + vec3f::eY) - objectCenter).getNormalized();
    //const vec3f objectFrameZ = (object.modelToWorld * (object.object.model->bbox.getCenter() + vec3f::eZ) - objectCenter).getNormalized();

    result.camera = c;
    result.annotations.push_back("modelName=" + object.model->modelName);
    result.annotations.push_back("modelCategory=" + object.model->categoryName);
    result.annotations.push_back("cameraEye=" + c.getEye().toString());
    result.annotations.push_back("cameraRight=" + c.getRight().toString());
    result.annotations.push_back("cameraLook=" + c.getLook().toString());
    result.annotations.push_back("occludedObjectPixels=" + std::to_string(occludedObjectPixels));
    result.annotations.push_back("totalObjectPixels=" + std::to_string(totalObjectPixels));
    result.annotations.push_back("occlusionPercentage=" + std::to_string(occlusionPercentage));
    result.annotations.push_back("totalScreenCoverage=" + std::to_string(totalScreenCoverage));
    result.annotations.push_back("borderQuality=" + std::to_string(borderQuality));
    result.annotations.push_back("quality=" + std::to_string(result.quality));
    
    return result;
}
