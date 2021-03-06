#include "SceneLoader.h"
#include <GLFW/glfw3.h>
using namespace std;

SceneLoader::SceneLoader(){}

void SceneLoader::loadScene(string lightsDataPath, string objectsDataPath, vector<DirectionalLight>& dirLights, vector<PointLight>& pointLights, vector<SpotLight>& spotLights, vector<Model>& models, vector<Object>& objects)
{    
    ifstream file;    
    //read point lights info
    try
    {
        file.open(lightsDataPath);
        stringstream lightData;
        lightData << file.rdbuf();
        file.close();

        bool good = true;
        int counter = 1;
        string type;
        while (getline(lightData, type))
        {           
            if (type == "point")            
                pointLights.push_back(loadPointLight(lightData, good));                                
            else if (type == "spot")
                spotLights.push_back(loadSpotLight(lightData, good));            
            else if (type == "directional")
                dirLights.push_back(loadDirectionalLight(lightData, good));
            else
            {
                cout << "ERROR::SCENE_LOADER::UNKNOWN_TYPE type: " << type << endl;
                exitOnError();
            }
            if (!good)
            {
                cout << "ERROR::SCENE_LOADER::FAILED_TO_LOAD_LIGHT type: " << type << ", number: " << counter << endl;
                exitOnError();
            }
            ++counter;   
            //getline(lightData, type);
        }
    }
    catch (std::ifstream::failure e)
    {
        cout << "ERROR::SCENE_LOADER::FAILED_TO_READ_LIGHTS_DATA" << endl;
        exitOnError();
    }

    ifstream modelFile;
    vector<glm::vec3> positions;
    vector<glm::vec3> rotations;
    vector<glm::vec3> scales;
    vector<string> paths;
    vector<int> modelIndexes;

    try
    {
        file.open(objectsDataPath);
        stringstream objectsData;
        objectsData << file.rdbuf();
        file.close();
        int i = 0;

        while (!objectsData.eof())
        {
            positions.push_back(getVec3(objectsData));
            rotations.push_back(getVec3(objectsData));
            scales.push_back(getVec3(objectsData));

            if (!checkRangeVec3(positions[positions.size() - 1], -1000, 1000, "ERROR::SCENE_LOADER::WRONG_MODEL_POSITION model number: " + to_string(positions.size())) ||
                !checkScale(scales[scales.size() - 1]))
                exitOnError();

            string path;                     
            getline(objectsData, path);
            paths.push_back(path);
            
            int index = getModelIndex(path, models);
            if (index < 0)
            {
                Model model(path);
                models.push_back(model);
                modelIndexes.push_back(models.size() - 1);
            }
            else            
                modelIndexes.push_back(index);
        }

        for (int i = 0; i < modelIndexes.size(); ++i)
        {
            Object obj(positions[i], rotations[i], scales[i], models[modelIndexes[i]]);
            objects.push_back(obj);
        }        
    }
    catch (std::ifstream::failure e)
    {
        cout << "ERROR::SCENE_LOADER::FAILED_TO_READ_MODELS_DATA" << endl;  
        exitOnError();
    }
    catch (exception e)
    {
        cout << "ERROR::SCENE_LOADER::FAILED_TO_LOAD_MODELS" << endl;
        exitOnError();
    }
}

DirectionalLight SceneLoader::loadDirectionalLight(stringstream& lightData, bool& good)
{
    glm::vec3 color = getVec3(lightData);
    glm::vec3 direction = getVec3(lightData);
    glm::vec3 ambient = getVec3(lightData);
    glm::vec3 diffuse = getVec3(lightData);
    glm::vec3 specular = getVec3(lightData);

    good = checkRangeVec3(color, 0, 1, "ERROR:SCENE_LOADER::WRONG_COLOR") &&
        checkRangeVec3(ambient, 0, 1, "ERROR:SCENE_LOADER::WRONG_AMBIENT_LIGHT_PROPERTY") &&
        checkRangeVec3(diffuse, 0, 1, "ERROR:SCENE_LOADER::WRONG_DIFFUSE_LIGHT_PROPERTY") &&
        checkRangeVec3(specular, 0, 1, "ERROR:SCENE_LOADER::WRONG_SPECULAR_LIGHT_PROPERTY");

    DirectionalLight light(direction, color, ambient, diffuse, specular);
    return light;
}

PointLight SceneLoader::loadPointLight(stringstream& lightData, bool& good)
{
    glm::vec3 position = getVec3(lightData);
    glm::vec3 color = getVec3(lightData);
    glm::vec3 ambient = getVec3(lightData);
    glm::vec3 diffuse = getVec3(lightData);
    glm::vec3 specular = getVec3(lightData);

    float constant;
    float linear;
    float quadratic;      
    lightData >> constant;
    lightData >> linear;
    lightData >> quadratic;    
    string str;//dummy
    getline(lightData, str);

    good = checkRangeVec3(position, -1000, 1000, "ERROR::SCENE_LOADER::WRONG_POSITION") &&
        checkRangeVec3(color, 0, 1, "ERROR:SCENE_LOADER::WRONG_COLOR") &&
        checkRangeVec3(ambient, 0, 1, "ERROR:SCENE_LOADER::WRONG_AMBIENT_LIGHT_PROPERTY") &&
        checkRangeVec3(diffuse, 0, 1, "ERROR:SCENE_LOADER::WRONG_DIFFUSE_LIGHT_PROPERTY") &&
        checkRangeVec3(specular, 0, 1, "ERROR:SCENE_LOADER::WRONG_SPECULAR_LIGHT_PROPERTY") &&
        checkAttenuation(constant, linear, quadratic);

    PointLight pointLight(position, color, 
                     ambient, diffuse, specular,
                     constant, linear, quadratic);
    return pointLight;
}

SpotLight SceneLoader::loadSpotLight(std::stringstream & lightData, bool& good)
{
    glm::vec3 position = getVec3(lightData);
    glm::vec3 color = getVec3(lightData);
    glm::vec3 direction = getVec3(lightData);
    glm::vec3 ambient = getVec3(lightData);
    glm::vec3 diffuse = getVec3(lightData);
    glm::vec3 specular = getVec3(lightData);
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;  
    string str;//dummy
    lightData >> constant;
    lightData >> linear;
    lightData >> quadratic;    
    getline(lightData, str);
    lightData >> cutOff;
    lightData >> outerCutOff;
    getline(lightData, str);

    good = checkRangeVec3(position, -1000, 1000, "ERROR::SCENE_LOADER::WRONG_POSITION") &&
        checkRangeVec3(color, 0, 1, "ERROR:SCENE_LOADER::WRONG_COLOR") &&
        checkRangeVec3(ambient, 0, 1, "ERROR:SCENE_LOADER::WRONG_AMBIENT_LIGHT_PROPERTY") &&
        checkRangeVec3(diffuse, 0, 1, "ERROR:SCENE_LOADER::WRONG_DIFFUSE_LIGHT_PROPERTY") &&
        checkRangeVec3(specular, 0, 1, "ERROR:SCENE_LOADER::WRONG_SPECULAR_LIGHT_PROPERTY") &&
        checkAttenuation(constant, linear, quadratic) &&
        checkAngles(cutOff, outerCutOff);

    SpotLight spotLight(position, color, direction, 
                        ambient, diffuse, specular,
                        constant, linear, quadratic,
                        cutOff, outerCutOff);
    return spotLight;
}

int SceneLoader::getModelIndex(string path, vector<Model>& models)
{
    int i = 0;
    bool modelExists = false;
    while (!modelExists && i < models.size())
    {
        modelExists = (path.substr(0, path.find_last_of('/')) == models[i].directory);
        ++i;
    }
    if (modelExists)
        return i - 1;
    else
        return -1;
}

glm::vec3 SceneLoader::getVec3(stringstream& data)
{
    glm::vec3 vec3;
    data >> vec3.x;
    data >> vec3.y;
    data >> vec3.z; 
    string str;//dummy
    getline(data, str);
    return vec3;
}

bool SceneLoader::checkRangeVec3(glm::vec3 vector, double left, double right, string message)
{
    if (left <= vector.x && vector.x <= right &&
        left <= vector.y && vector.y <= right &&
        left <= vector.z && vector.z <= right)
        return true;

    cout << message << endl;
    return false;
}

void SceneLoader::exitOnError()
{
    glfwTerminate();
    //cin.ignore();
    cin.get();    
    exit(-1);
}

bool SceneLoader::checkAttenuation(double constant, double linear, double quadratic)
{
    if (constant >= 0 && linear >= 0 && quadratic >= 0 &&
        constant * constant + linear * linear + quadratic * quadratic > 0)
        return true;

    cout << "ERROR::SCENE_LOADER::WRONG_ATTENUATION_COEFFICIENTS" << endl;
    return false;
}

bool SceneLoader::checkAngles(double cutOff, double outerCutOff)
{
    if (cutOff > 0 && cutOff < 90 &&
        outerCutOff > 0 && outerCutOff < 90 &&
        outerCutOff > cutOff)
        return true;

    cout << "ERROR::SCENE_LOADER::WRONG_ANGLES" << endl;
    return false;
}

bool SceneLoader::checkScale(glm::vec3 scale)
{
    if (scale.x > 0 && scale.y > 0 && scale.z > 0)
        return true;
    
    cout << "ERROR::SCENE_LOADER::WRONG_SCALE" << endl;
    return false;
}

