
struct AppState
{
    D3D11AssetRenderer renderer;
    Cameraf camera;

    ModelDatabase modelDatabase;
    MaterialDatabase materialDatabase;

    Scene activeScene;
    SceneGenerator generator;

    D3D11GraphicsDevice *graphics;

    SynthRenderer synthRenderer;
    SynthRenderResult currentRendering;

    RandomImageLoader randomImageLoader;

    bool autoGenerateMode;
};