#include "scene.h"
#include "binary/teapot.h"
#include "binary/rgb.h"
#include "binary/cloud.h"
#include "binary/tex_flower.h"
#include "checker.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Object* Scene::teapot = nullptr;
Texture* Scene::diffuse = nullptr;
Texture* Scene::dissolve = nullptr;
Material* Scene::material = nullptr;
Light* Scene::light = nullptr;

int Scene::width = 0;
int Scene::height = 0;

// Arcball variables
float lastMouseX = 0, lastMouseY = 0;
float currentMouseX = 0, currentMouseY = 0;

void Scene::setup(AAssetManager* aAssetManager) {
    Asset::setManager(aAssetManager);

    Scene::vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    Scene::fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    Scene::program = new Program(Scene::vertexShader, Scene::fragmentShader);

    Scene::camera = new Camera(Scene::program);
    Scene::camera->eye = vec3(20.0f, 30.0f, 20.0f);

    Scene::light = new Light(program);
    Scene::light->position = vec3(15.0f, 15.0f, 0.0f);

    //////////////////////////////
    /* TODO: Problem 2 : Change the texture of the teapot
     *  Modify and fill in the lines below.
     */
    vector<Texel> rgbChange = rgbTexels;
    for(int i =0; i < rgbChange.size(); i++){
        if(rgbTexels[i].red == 0xFF){
            rgbChange[i].red = 0x0;
            rgbChange[i].green = 0x0;
            rgbChange[i].blue = 0xFF;
        }
        if(rgbTexels[i].green == 0xFF){
            rgbChange[i].red = 0xFF;
            rgbChange[i].green = 0x0;
            rgbChange[i].blue = 0x0;
        }
        if(rgbTexels[i].blue == 0xFF){
            rgbChange[i].red = 0x0;
            rgbChange[i].green = 0xFF;
            rgbChange[i].blue = 0x0;
        }
    }

    Scene::diffuse  = new Texture(Scene::program, 0, "textureDiff", rgbChange, rgbSize);
    //////////////////////////////

    Scene::material = new Material(Scene::program, diffuse, dissolve);
    Scene::teapot = new Object(program, material, teapotVertices, teapotIndices);
}

void Scene::screen(int width, int height) {
    Scene::camera->aspect = (float) width/height;
    Scene::width = width;
    Scene::height = height;
}

void Scene::update(float deltaTime) {
    static float time = 0.0f;

    Scene::program->use();

    Scene::camera->update();
    Scene::light->update();

    Scene::teapot->draw();

    time += deltaTime;
}

void Scene::mouseDownEvents(float x, float y) {
    lastMouseX = currentMouseX = x;
    lastMouseY = currentMouseY = y;
}


void Scene::mouseMoveEvents(float x, float y) {
    //////////////////////////////
    /* TODO: Problem 3 : Implement Phong lighting
     *  Fill in the lines below.
     */
    currentMouseX = x;
    currentMouseY = y;
    // prev mouse
    glm::vec3 v = glm::vec3(2.0 * lastMouseX / width - 1.0, 2.0 * lastMouseY / height - 1.0 ,0.0f);
    v.y = -v.y;
    float prev_xy = pow(v.x,2.0) + pow(v.y, 2.0);

    if (prev_xy <= 1) {
        v.z = sqrt(1.0- prev_xy);
    }
    else glm::normalize(v);

    // current mouse
    glm::vec3 v_i = glm::vec3(2.0 * currentMouseX / width - 1.0, 2.0 * currentMouseY / height - 1.0 ,0.0f);
    v_i.y = -v_i.y;
    float v_xy = pow(v_i.x,2.0) + pow(v_i.y,2.0);
    if (v_xy <= 1) {
        v_i.z = sqrt(1.0- v_xy);
    }
    else glm::normalize(v_i);

    // view transform
    vec3 cameraN = glm::normalize(camera->eye - camera->at);
    vec3 cameraU = glm::normalize(glm::cross(camera->up, cameraN));
    vec3 cameraV = glm::normalize(glm::cross(cameraN, cameraU));
    mat4 iViewTrans = glm::mat4(mat3(cameraU,cameraV,cameraN));

    // rotation
    vec3 cameraAxis = cross(v,v_i);
    vec3 axis = vec3(iViewTrans * vec4(cameraAxis,1));
    float angle = acos(glm::dot(v,v_i));
    glm::mat4 rotate = glm::rotate(angle,axis);

    // implement
    vec3 prevPos = Scene::light->position;
    Scene::light->position = vec3( rotate * vec4(Scene::light->position,1.0f));

    if (isnan(Scene::light->position.x)){
        Scene::light->position = prevPos;
    }

    lastMouseX = x;
    lastMouseY = y;
    //////////////////////////////
}