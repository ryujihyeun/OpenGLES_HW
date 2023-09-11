#include "scene.h"
#include "binary/animation.h"
#include "binary/skeleton.h"
#include "binary/player.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Object* Scene::player = nullptr;
Texture* Scene::diffuse = nullptr;
Material* Scene::material = nullptr;
Object* Scene::lineDraw = nullptr;
Texture* Scene::lineColor = nullptr;
Material* Scene::lineMaterial = nullptr;

bool Scene::upperFlag = true;
bool Scene::lowerFlag = true;
float lowerTime = 0.0, upperTime = 0.0;
vector<Vertex> originalVertices = playerVertices;
vector<mat4> prevLower, prevUpper;

void Scene::setup(AAssetManager* aAssetManager) {
    Asset::setManager(aAssetManager);

    Scene::vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    Scene::fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    Scene::program = new Program(Scene::vertexShader, Scene::fragmentShader);

    Scene::camera = new Camera(Scene::program);
    Scene::camera->eye = vec3(0.0f, 0.0f, 80.0f);

    Scene::diffuse = new Texture(Scene::program, 0, "textureDiff", playerTexels, playerSize);
    Scene::material = new Material(Scene::program, diffuse);
    Scene::player = new Object(program, material, playerVertices, playerIndices);
    player->worldMat = scale(vec3(1.0f / 3.0f));

    Scene::lineColor = new Texture(Scene::program, 0, "textureDiff", {{0xFF, 0x00, 0x00}}, 1);
    Scene::lineMaterial = new Material(Scene::program, lineColor);
    Scene::lineDraw = new Object(program, lineMaterial, {{}}, {{}}, GL_LINES);
}

void Scene::screen(int width, int height) {
    Scene::camera->aspect = (float) width/height;
}

void Scene::update(float deltaTime) {
    Scene::program->use();
    Scene::camera->update();

    // Calculate the elapsed time from the start by accumulating deltaTime.
    if(upperFlag) upperTime += deltaTime;
    if(lowerFlag) lowerTime += deltaTime;

    if(lowerTime >= 4.0f){
        lowerTime -= 4.0f;
    }

    if(upperTime >= 4.0f){
        upperTime -= 4.0f;
    }

    // create toParent toBone
    vector<mat4> toParent, toBone;
    toParent.push_back(translate(jOffsets[0]));
    toBone.push_back(inverse(toParent[0]));

    for(int i = 1; i < jOffsets.size(); i++){
        toParent.push_back(translate(jOffsets[i]));
        toBone.push_back(inverse(toParent[i]) * toBone[jParents[i]]);
    }
    LOG_PRINT_DEBUG("toParent, toBone %d %d ", toParent.size(), toBone.size());

    // create animated Matrix
    vector<mat4> animateMatrix, localMatrix;
    vector<float> curMotion, nextMotion;
    int curframe, nextframe;
    curframe = floor(lowerTime);
    nextframe = (curframe + 1) % 4;

    for(int i = 0; i < 12; i++){
        mat4 curLocal = rotate(radians(motions[curframe][i*3 + 5]),vec3(0.0,0.0,1.0)) *
                        rotate(radians(motions[curframe][i*3 + 3]),vec3(1.0,0.0,0.0)) *
                        rotate(radians(motions[curframe][i*3 + 4]),vec3(0.0,1.0,0.0));
        mat4 nextLocal = rotate(radians(motions[nextframe][i*3 + 5]),vec3(0.0,0.0,1.0)) *
                         rotate(radians(motions[nextframe][i*3 + 3]),vec3(1.0,0.0,0.0)) *
                         rotate(radians(motions[nextframe][i*3 + 4]),vec3(0.0,1.0,0.0));
        localMatrix.push_back(mat4_cast(mix(quat_cast(curLocal),quat_cast(nextLocal),lowerTime - curframe)));
        if(i == 0){
            mat4 curTrans = translate(vec3(motions[curframe][0],motions[curframe][1],motions[curframe][2]));
            mat4 nextTrans = translate(vec3(motions[nextframe][0],motions[nextframe][1],motions[nextframe][2]));

            mat4 interpolateTrans = mix(curTrans,nextTrans,lowerTime - curframe);
            animateMatrix.push_back(toParent[i] * interpolateTrans * localMatrix[i]);
        }
        else animateMatrix.push_back(animateMatrix[jParents[i]] * toParent[i] * localMatrix[i]);
    }

    curframe = floor(upperTime);
    nextframe = (curframe + 1) % 4;
    for(int i = 12; i < jOffsets.size(); i++){
        mat4 curLocal = rotate(radians(motions[curframe][i*3 + 5]),vec3(0.0,0.0,1.0)) *
                        rotate(radians(motions[curframe][i*3 + 3]),vec3(1.0,0.0,0.0)) *
                        rotate(radians(motions[curframe][i*3 + 4]),vec3(0.0,1.0,0.0));
        mat4 nextLocal = rotate(radians(motions[nextframe][i*3 + 5]),vec3(0.0,0.0,1.0)) *
                         rotate(radians(motions[nextframe][i*3 + 3]),vec3(1.0,0.0,0.0)) *
                         rotate(radians(motions[nextframe][i*3 + 4]),vec3(0.0,1.0,0.0));
        localMatrix.push_back(mat4_cast(mix(quat_cast(curLocal),quat_cast(nextLocal),upperTime - curframe)));
        animateMatrix.push_back(animateMatrix[jParents[i]] * toParent[i] * localMatrix[i]);
    }
    LOG_PRINT_DEBUG("localMatrix, animateMatrix %d %d ", localMatrix.size(), animateMatrix.size());

    // skinning
    for (int i = 0; i < playerVertices.size(); i++) {
        vec3 pos = originalVertices[i].pos, normal = originalVertices[i].nor;
        vec4 weight = originalVertices[i].weight;
        ivec4 boneIndex = originalVertices[i].bone;
        mat4 M = mat4(0.0f);

        for (int j = 0; j < 4; j++) {
            if (boneIndex[j] == -1)
                continue;
            M += weight[j] * animateMatrix[boneIndex[j]] * toBone[boneIndex[j]];
        }

        playerVertices[i].pos = vec3(M * vec4(pos, 1.0f));
        playerVertices[i].nor = transpose(inverse(mat3(M))) * normal;
    }

    // Line Drawer
    //glLineWidth(20);
    //Scene::lineDraw->load(playerVertices, playerIndices);
    //Scene::lineDraw->draw();

    Scene::player->load(playerVertices, playerIndices);
    Scene::player->draw();
}

void Scene::setUpperFlag(bool flag)
{
    Scene::upperFlag = flag;
}

void Scene::setLowerFlag(bool flag)
{
    Scene::lowerFlag = flag;
}