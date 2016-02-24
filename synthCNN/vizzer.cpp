
#include "main.h"

void Vizzer::init(ApplicationData &app)
{
    assets.init(app.graphics);

    vec3f eye(0.5f, 0.2f, 0.5f);
    vec3f worldUp(0.0f, 1.0f, 0.0f);
    camera = Cameraf(-eye, eye, worldUp, 60.0f, (float)app.window.getWidth() / app.window.getHeight(), 0.01f, 10000.0f);
    camera = Cameraf("-0.774448 1.24485 -1.35404 0.999848 1.80444e-009 -0.0174517 0.0152652 -0.484706 0.874544 -0.00845866 -0.874677 -0.484632 0 1 0 60 1.25 0.01 10000");
    //-0.774448 1.24485 -1.35404 0.999848 1.80444e-009 -0.0174517 0.0152652 -0.484706 0.874544 -0.00845866 -0.874677 -0.484632 0 1 0 60 1.25 0.01 10000
    font.init(app.graphics, "Calibri");

}

void Vizzer::render(ApplicationData &app)
{
    timer.frame();

    vector<string> text;
    text.push_back(string("FPS: ") + convert::toString(timer.framesPerSecond()));
    
    const bool useText = true;
    if (useText)
        drawText(app, text);
}

void Vizzer::resize(ApplicationData &app)
{
    camera.updateAspectRatio((float)app.window.getWidth() / app.window.getHeight());
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
}

void Vizzer::keyPressed(ApplicationData &app, UINT key)
{
    const float distance = 0.1f;
    const float theta = 3.0f;

    //if (key == KEY_Z) physicsWorld.step();

    if(key == KEY_S) camera.move(-distance);
    if(key == KEY_W) camera.move(distance);
    if(key == KEY_A) camera.strafe(-distance);
    if(key == KEY_D) camera.strafe(distance);
	if(key == KEY_E) camera.jump(distance);
	if(key == KEY_Q) camera.jump(-distance);

    if(key == KEY_UP) camera.lookUp(-theta);
    if(key == KEY_DOWN) camera.lookUp(theta);
    if(key == KEY_LEFT) camera.lookRight(-theta);
    if(key == KEY_RIGHT) camera.lookRight(theta);
}

void Vizzer::mouseDown(ApplicationData &app, MouseButtonType button)
{

}

void Vizzer::mouseWheel(ApplicationData &app, int wheelDelta)
{
    const float distance = 0.001f;
    camera.move(distance * wheelDelta);
}

void Vizzer::mouseMove(ApplicationData &app)
{
    const float distance = 0.01f;
    const float theta = 0.4f;

    vec2i posDelta = app.input.mouse.pos - app.input.prevMouse.pos;

    if(app.input.mouse.buttons[MouseButtonRight])
    {
        camera.strafe(distance * posDelta.x);
        camera.jump(-distance * posDelta.y);
    }

    if(app.input.mouse.buttons[MouseButtonLeft])
    {
        camera.lookRight(theta * posDelta.x);
        camera.lookUp(theta * posDelta.y);
    }

}
