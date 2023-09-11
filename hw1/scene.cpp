#include "scene.h"

#include "obj_teapot.h"
#include "tex_flower.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Light* Scene::light = nullptr;
Object* Scene::teapot = nullptr;
Material* Scene::flower = nullptr;
float Scene::time = 0.0f;

void Scene::setup(AAssetManager* aAssetManager) {

    // set asset manager
    Asset::setManager(aAssetManager);

    // create shaders
    vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    // create program
    program = new Program(vertexShader, fragmentShader);

    // create camera
    camera = new Camera(program);
    camera->eye = vec3(60.0f, 60.0f, 60.0f);
    camera->cameraN = glm::normalize(camera->eye - camera->at);
    camera->cameraU = glm::normalize(glm::cross(camera->up, camera->cameraN));
    camera->cameraV = glm::normalize(glm::cross(camera->cameraN, camera->cameraU));

    // create light
    light = new Light(program);
    light->position = vec3(100.0f, 0.0f, 0.0f);

    // create floral texture
    flower = new Material(program, texFlowerData, texFlowerSize);

    // create teapot object
    teapot = new Object(program, flower, objTeapotVertices, objTeapotIndices,
                        objTeapotVerticesSize, objTeapotIndicesSize);
}

void Scene::screen(int width, int height) {

    // set camera aspect ratio
    camera->aspect = (float) width / height;
}

void Scene::update(float deltaTime) {
    static float radius = 15.0f;

    // use program
    program->use();

    // scale teapot
    float scale = 0.5;
    mat4 scaleM;
    scaleM = transpose(mat4(scale, 0.0f, 0.0f, 0.0f,
                            0.0f, scale, 0.0f, 0.0f,
                            0.0f, 0.0f, scale, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f));
    teapot->worldMatrix = scaleM;

    //////////////////////////////
    /* TODO: Problem 2.
    *  Fill in the lines below.
    */
    // TODO : Make the teapot orbit on YZ plane
    // set degree
    float degree = time;
    degree = int(degree) % 360;
    float rad = glm::radians(degree);
    // create matrix for rotate, translate
    glm::mat4 rotateYZ = glm::rotate(glm::mat4(1.0f), -rad, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,radius));
    glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), rad,glm::vec3(1.0f, 0.0f, 0.0f));

    // TODO : Rotate the teapot around its own y-axis
    // create matrix for rotate
    glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), rad*2, glm::vec3(0.f, 1.f, 0.f));
    // apply matrix multiplication to teapot->worldMatrix
    teapot->worldMatrix = rotateX * transMat * rotateYZ * rotateY * teapot->worldMatrix;
    //////////////////////////////

    camera->updateViewMatrix();
    camera->updateProjectionMatrix();

    teapot->viewMatrix = camera->viewMatrix;
    teapot->projMatrix = camera->projMatrix;

    light->setup();

    // increase time
    time = time + deltaTime * 100;

    // draw teapot
    teapot->draw();
}

void Scene::dragScreen(float dx,float dy)
{
    float Sensitivity = 0.1;

    float thetaYaw=glm::radians(Sensitivity*dx);

    moveCamera(thetaYaw);
}
void Scene::moveCamera(float theta)
{
    //////////////////////////////
    /* TODO: Problem 3.
     *  Note that u,v,n should always be orthonormal.
     *  The u vector can be accessed via camera->cameraU.
     *  The v vector can be accessed via camera->cameraV.
     *  The n vector can be accessed via camera->cameraN.
     *  === HINT ===
     *  Initial camera position is (60.0f, 60.0f, 60.0f)
     *  Argument theta is amount of camera rotation in radians
     */
    // initial camera vector
    vec3 initial = vec3(camera->eye);
    vec4 initialPos = vec4(initial, 1.0f);
    vec3 u = camera->cameraU;
    vec3 v = camera->cameraV;
    vec3 n = camera->cameraN;

    mat4 transC = glm::translate(glm::mat4(1.0f),-initial);
    mat4 rotateCamera = glm::mat4(transpose(mat3(u,v,n)));
    mat4 rotateRad = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f,1.0f,0.0f));
    mat4 rotatePos = glm::rotate(glm::mat4(1.0f), -theta, glm::vec3(0.0f,1.0f,0.0f));
    mat4 inverseCamera = glm::mat4(mat3(u,v,n));
    mat4 inverseTrans = glm::translate(glm::mat4(1.0f),initial);

    vec4 cameraPos = rotatePos * inverseTrans * inverseCamera * rotateRad * rotateCamera * transC * initialPos;
    // change camera eye
    camera->eye = vec3(cameraPos);
    // set camera
    camera->cameraN = glm::normalize(camera->eye - camera->at);
    camera->cameraU = glm::normalize(glm::cross(camera->up, camera->cameraN));
    camera->cameraV = glm::normalize(glm::cross(camera->cameraN, camera->cameraU));

    //////////////////////////////
}