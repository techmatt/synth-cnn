
struct AppState
{
    D3D11AssetRenderer assets;
    Cameraf camera;

    ModelDatabase modelDatabase;

    Scene activeScene;
    SceneGenerator generator;

    D3D11GraphicsDevice *graphics;
};