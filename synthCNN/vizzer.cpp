
#include "main.h"

void Vizzer::init(ApplicationData &app)
{
    state.graphics = &app.graphics.castD3D11();
    state.renderer.init(app.graphics);

    state.environmentDatabase.init();
    state.materialDatabase.init();

    vec3f eye(0.5f, 0.2f, 0.5f);
    vec3f worldUp(0.0f, 0.0f, 1.0f);
    state.camera = Cameraf(-eye, eye, worldUp, 60.0f, (float)app.window.getWidth() / app.window.getHeight(), 0.01f, 10000.0f);
    state.camera = Cameraf("14.8408 15.786 14.2102 -0.754726 0.656041 6.95734e-010 -0.523082 -0.601766 -0.603542 -0.395948 -0.455509 0.797331 0 0 1 60 1.25 0.01 10000");
    //-0.774448 1.24485 -1.35404 0.999848 1.80444e-009 -0.0174517 0.0152652 -0.484706 0.874544 -0.00845866 -0.874677 -0.484632 0 1 0 60 1.25 0.01 10000
    font.init(app.graphics, "Calibri");

    state.synthRenderer.init(*state.graphics, 256, 1024);

    state.modelDatabase.init();

    state.activeScene = state.generator.makeRandomScene(state);
    
    state.randomImageLoader.init();

    state.autoGenerateMode = false;

    state.timingLog.open(constants::synthCNNRoot + "timings.txt", ios::app);
}

void Vizzer::render(ApplicationData &app)
{
    timer.frame();

    state.activeScene.render(state, state.camera);

    //state.assets.renderSphere(state.camera.getCameraPerspective(), vec3f::origin, 0.5f, vec3f(1.0f, 1.0f, 1.0f));

    vector<string> text;
    text.push_back(string("FPS: ") + convert::toString(timer.framesPerSecond()));

    auto &centralModel = *state.activeScene.objects[state.activeScene.mainObjectIndex].model;
    text.push_back(string("Category: ") + centralModel.categoryName);
    text.push_back(string("Model: ") + centralModel.modelName);
    text.push_back(string("Up: ") + centralModel.up.toString());
    
    for (const string &s : state.currentRendering.annotations)
        text.push_back(s);

    //if (rand() % 100)
    //    cout << state.camera.toString() << endl;

    if (state.autoGenerateMode)
        runNewMistubaScene(app);

    const bool useText = true;
    if (useText)
        drawText(app, text);
}

void Vizzer::resize(ApplicationData &app)
{
    state.camera.updateAspectRatio((float)app.window.getWidth() / app.window.getHeight());
}

void Vizzer::drawText(ApplicationData &app, vector<string> &text)
{
    int y = 0;
    for (auto &entry : text)
    {
        font.drawString(app.graphics, entry, vec2i(10, 5 + y++ * 25), 24.0f, RGBColor::Red);
    }
}

void Vizzer::keyDown(ApplicationData &app, UINT key)
{
    if (key == KEY_F) app.graphics.castD3D11().toggleWireframe();

    if (key == KEY_R)
    {
        state.activeScene = state.generator.makeRandomScene(state);
    }

    if (key == KEY_T)
    {
        const Cameraf randomCamera = state.synthRenderer.randomCamera(state.activeScene);
        state.currentRendering = state.synthRenderer.render(state, state.activeScene, randomCamera, false);
        state.camera = randomCamera;
    }

    if (key == KEY_G)
    {
        LodePNG::save(state.currentRendering.occludedObjectColor, constants::synthCNNRoot + "occluded.png");

        auto mask = synthUtil::makeDownsampledMask(state.currentRendering.occludedObjectColor, 4);
        LodePNG::save(mask, constants::synthCNNRoot + "mask.png");

        state.activeScene.saveMitsuba(state, constants::synthCNNRoot + "debugScene.xml", state.camera);

        synthUtil::runMitsuba(constants::synthCNNRoot + "debugScene.xml");

        const ColorImageR8G8B8A8 renderedImage = LodePNG::load(constants::synthCNNRoot + "debugScene.png");

        const ColorImageR8G8B8A8 compositedImage = synthUtil::compositeRandomImage(state, renderedImage, mask);
        LodePNG::save(compositedImage, constants::synthCNNRoot + "composite.png");
    }

    if (key == KEY_P)
    {
        state.autoGenerateMode = !state.autoGenerateMode;
    }
}

void Vizzer::runNewMistubaScene(ApplicationData &app)
{
    Timer timer;
    state.activeScene = state.generator.makeRandomScene(state);

    if (!state.activeScene.valid(state))
    {
        cout << "Scene not valid" << endl;
        return;
    }

    if (!state.synthRenderer.goodRandomCamera(state, state.activeScene, state.camera))
    {
        cout << "No good camera found" << endl;
        return;
    }

    auto &object = state.activeScene.objects[state.activeScene.mainObjectIndex];
    const string categoryDir = constants::synthCNNRoot + "mitsubaScenes/" + object.model->categoryName + "/";
    util::makeDirectory(categoryDir);

    const UINT randomInt32 = util::randomInteger(0, std::numeric_limits<long>::max());
    const string randomHash = util::encodeBytes(randomInt32);

    const string scenePrefix = categoryDir + object.model->modelName + "_" + randomHash;

    state.currentRendering = state.synthRenderer.render(state, state.activeScene, state.camera, false);

    //LodePNG::save(state.currentRendering.occludedObjectColor, constants::synthCNNRoot + "occluded.png");

    auto mask = synthUtil::makeDownsampledMask(state.currentRendering.occludedObjectColor, 4);
    LodePNG::save(mask, scenePrefix + "_mask.png");

    state.activeScene.saveMitsuba(state, scenePrefix + ".xml", state.camera);

    synthUtil::runMitsuba(scenePrefix + ".xml");

    const ColorImageR8G8B8A8 renderedImage = LodePNG::load(scenePrefix + ".png");
    const ColorImageR8G8B8A8 compositedImage = synthUtil::compositeRandomImage(state, renderedImage, mask);
    const ColorImageR8G8B8A8 bwImage = synthUtil::decolorize(compositedImage);
    
    LodePNG::save(bwImage, scenePrefix + "_comp.png");

    util::deleteFile(scenePrefix + ".exr");
    util::deleteFile(scenePrefix + ".png");

    state.timingLog << object.model->categoryName << "," << object.model->modelName << "," << timer.getElapsedTime() << endl;
}

void Vizzer::keyPressed(ApplicationData &app, UINT key)
{
    const float distance = 0.1f;
    const float theta = 3.0f;

    //if (key == KEY_Z) physicsWorld.step();

    if(key == KEY_S) state.camera.move(-distance);
    if(key == KEY_W) state.camera.move(distance);
    if(key == KEY_A) state.camera.strafe(-distance);
    if(key == KEY_D) state.camera.strafe(distance);
	if(key == KEY_E) state.camera.jump(distance);
	if(key == KEY_Q) state.camera.jump(-distance);

    if(key == KEY_UP) state.camera.lookUp(-theta);
    if(key == KEY_DOWN) state.camera.lookUp(theta);
    if(key == KEY_LEFT) state.camera.lookRight(-theta);
    if(key == KEY_RIGHT) state.camera.lookRight(theta);
}

void Vizzer::mouseDown(ApplicationData &app, MouseButtonType button)
{

}

void Vizzer::mouseWheel(ApplicationData &app, int wheelDelta)
{
    const float distance = 0.002f;
    state.camera.move(distance * wheelDelta);
}

void Vizzer::mouseMove(ApplicationData &app)
{
    const float distance = 0.01f;
    const float theta = 0.4f;

    vec2i posDelta = app.input.mouse.pos - app.input.prevMouse.pos;

    if(app.input.mouse.buttons[MouseButtonRight])
    {
        state.camera.strafe(distance * posDelta.x);
        state.camera.jump(-distance * posDelta.y);
    }

    if(app.input.mouse.buttons[MouseButtonLeft])
    {
        state.camera.lookRight(-theta * posDelta.x);
        state.camera.lookUp(theta * posDelta.y);
    }

}
