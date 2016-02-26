
struct SynthRenderResult
{
    SynthRenderResult()
	{
		quality = 0.0;
	}

    Cameraf camera;

    ColorImageR8G8B8A8 occludedObjectColor;
    
    vector<string> annotations;

	double quality;
};

class SynthRenderer
{
public:
	void init(D3D11GraphicsDevice &graphics, UINT smallDimension, UINT bigDimension);

    static Cameraf randomCamera(const Scene &s);

    bool goodRandomCamera(AppState &state, const Scene &s, Cameraf &bestCamera);

    SynthRenderResult renderRandomCamera(AppState &state, const Scene &s);

    SynthRenderResult render(AppState &state, const Scene &s, const Cameraf &c, bool qualityEstimateOnly);

	D3D11RenderTarget renderTargetSmall, renderTargetBig;
};