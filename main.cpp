#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

void createStaticCubeMap();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void setShaderParams(int index);
void drawSkybox();
void drawOrbitingObject();
void drawTeapot();

void setupFramebuffers();
void dynamicMappingPasses();
void finalRenderPass();
void createDynamicCubemapTexture();

GLuint gProgram[3];
GLint modelingMatrixLoc[3];
GLint viewingMatrixLoc[3];
GLint projectionMatrixLoc[3];
GLint eyePosLoc[3];
int gWidth = 640;
int gHeight = 480;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
//glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 1;

unsigned int cupemapTextureID;
vector<string> skyboxTextures = { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };

glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 7.0f);
glm::vec3 cameraGaze = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float defaultFovyRad = (float)(45.0 / 180.0) * M_PI;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float lastX, lastY;
bool firstTimeReceivingMouseInput = true;

float mouseYaw = -90.0f;
float mousePitch = 0.0f;

float centralObjectRotDeg = 0.0f;
float orbitObjectRotDef = 0.0;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

vector<Vertex> orbitVertices;
vector<Texture> orbitTextures;
vector<Normal> orbitNormals;
vector<Face> orbitFaces;


// for drawing the Teapot
GLuint gVertexAttribBuffer, gIndexBuffer;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
GLuint teapotVAO;

// for drawing the orbitObject(s)
GLuint orbitVertexAttribBuffer, orbitIndexBuffer;
int orbitVertexDataSizeInBytes, orbitNormalDataSizeInBytes;
GLuint orbitVAO;   
//glm::vec3 teapotPosition(0.0f, -8.f, -20.0f);

glm::vec3 teapotPosition(0.0f, -0.2f, -0.15f);

// Dynamic cube mapping resources.
GLuint framebuffers[6];
GLuint colorTextures[6];
//GLuint depthTextures[6];
GLuint depthRenderbuffers[6];
GLuint dynamicCubemapTexId;
GLuint fb;

GLsizei captureWidth = 1024;
GLsizei captureHeight = captureWidth;
float captureFovy = (float)(90 / 180.0) * M_PI;

vector<glm::vec3> camGazeVectors = {
      glm::vec3(1.0f,0.f,0.f),    // right
      glm::vec3(-1.0f,0.f, 0.0f), // left
      glm::vec3(0.0f, 1.f, 0.0f), // top
      glm::vec3(0.0f, -1.f, 0.0f), // bottom
      glm::vec3(0.0f, 0.f, 1.0f), // back
      glm::vec3(0.0f, 0.f, -1.0f), // front
};

//vector<glm::vec3> camUpVectors = {
//    glm::vec3(0.0f, 1.0f, 0.0f), // right
//    glm::vec3(0.0f, 1.0f, 0.0f), // left
//    glm::vec3(0.0f, 0.0f, 1.0f), // top
//    glm::vec3(0.0f, 0.0f, -1.0f), // bottom
//    glm::vec3(0.0f, 1.0f, 0.0f), // back
//    glm::vec3(0.0f, 1.0f, 0.0f), // front
//};

vector<glm::vec3> camUpVectors = {
    glm::vec3(0.0f, -1.0f, 0.0f), // right
    glm::vec3(0.0f, -1.0f, 0.0f), // left
    glm::vec3(0.0f, 0.0f, 1.0f), // top
    glm::vec3(0.0f, 0.0f, -1.0f), // bottom
    glm::vec3(0.0f, -1.0f, 0.0f), // back
    glm::vec3(0.0f, -1.0f, 0.0f), // front
};

// for drawing the static cubemapped skybox
GLuint skyboxVAO;
float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

bool ParseObj(const string& fileName, vector<Texture>& textures, vector<Normal>& normals, vector<Vertex>& vertices, vector<Face>& faces)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        textures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        normals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        vertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    faces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	assert(gVertices.size() == gNormals.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

GLuint createVS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log for %s: %s\n",shaderName, output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl");
    GLuint fs1 = createFS("frag.glsl");
	GLuint vs2 = createVS("vert2.glsl");
	GLuint fs2 = createFS("frag2.glsl");

    GLuint teapotvs = createVS("teapotVertex.glsl");
    GLuint teapotfs = createFS("teapotFragment.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

    glAttachShader(gProgram[2], teapotvs);
    glAttachShader(gProgram[2], teapotfs);

	// Link the programs

    glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}


    glLinkProgram(gProgram[2]);
    glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        cout << "Teapot Program link failed" << endl;
        exit(-1);
    }
	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 3; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO(GLuint& vao, GLuint& vertexAttribBuffer, GLuint& indexBuffer, 
            vector<Face>& faces, vector<Normal>& normals, vector<Vertex>& vertices, 
            int& verticesByteSize, int& normalsByteSize)
{
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &vertexAttribBuffer);
	glGenBuffers(1, &indexBuffer);

	assert(vertexAttribBuffer > 0 && indexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    verticesByteSize = vertices.size() * 3 * sizeof(GLfloat);
    normalsByteSize = normals.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = faces.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat [vertices.size() * 3];
	GLfloat* normalData = new GLfloat [normals.size() * 3];
	GLuint* indexData = new GLuint [faces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < vertices.size(); ++i)
	{
		vertexData[3*i] = vertices[i].x;
		vertexData[3*i+1] = vertices[i].y;
		vertexData[3*i+2] = vertices[i].z;

        minX = std::min(minX, vertices[i].x);
        maxX = std::max(maxX, vertices[i].x);
        minY = std::min(minY, vertices[i].y);
        maxY = std::max(maxY, vertices[i].y);
        minZ = std::min(minZ, vertices[i].z);
        maxZ = std::max(maxZ, vertices[i].z);
	}

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < normals.size(); ++i)
	{
		normalData[3*i] = normals[i].x;
		normalData[3*i+1] = normals[i].y;
		normalData[3*i+2] = normals[i].z;
	}

	for (int i = 0; i < faces.size(); ++i)
	{
		indexData[3*i] = faces[i].vIndex[0];
		indexData[3*i+1] = faces[i].vIndex[1];
		indexData[3*i+2] = faces[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, verticesByteSize + normalsByteSize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, verticesByteSize, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, verticesByteSize, normalsByteSize, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(verticesByteSize));
}

void init() 
{
	ParseObj("teapot.obj",gTextures, gNormals, gVertices, gFaces);
    ParseObj("cube.obj", orbitTextures, orbitNormals, orbitVertices, orbitFaces);

    glEnable(GL_DEPTH_TEST);
    initShaders();

    // init teapot
    initVBO(teapotVAO, gVertexAttribBuffer, gIndexBuffer, gFaces, gNormals, gVertices, gVertexDataSizeInBytes, gNormalDataSizeInBytes);

    // init orbiting object
    initVBO(orbitVAO, orbitVertexAttribBuffer, orbitIndexBuffer, orbitFaces, orbitNormals, orbitVertices, orbitVertexDataSizeInBytes, orbitNormalDataSizeInBytes);

    // create skybox
    createStaticCubeMap();
    createDynamicCubemapTexture();
    setupFramebuffers();
}

void setupFramebuffers() {

    glEnable(GL_TEXTURE_2D);
    //glCreateFramebuffers(6, framebuffers);
    //glGenTextures(6, colorTextures);
    //glGenTextures(6, depthTextures);
    glGenRenderbuffers(6, depthRenderbuffers);


    glGenFramebuffers(1, &fb);

    //for (int i = 0; i < 6; i++) {

    //    //glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
    //    //glBindTexture(GL_TEXTURE_2D, colorTextures[i]);
    //    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, captureWidth, captureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    //    //glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
    //    //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, captureWidth, captureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    //    //glNamedFramebufferTexture(framebuffers[i], GL_COLOR_ATTACHMENT0, colorTextures[i], 0);
    //    //glNamedFramebufferTexture(framebuffers[i], GL_DEPTH_ATTACHMENT, depthTextures[i], 0);

    //    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffers[i]);
    //    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureWidth, captureHeight);
    //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffers[i]);

    //    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
    //        cout << "succesfully setup framebuffer: "<< i << endl;
    //    }
    //}

    // Bind the default framebuffer for usual rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void createStaticCubeMap() 
{
    GLuint skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Create & setup the static cubemap texture.
    glGenTextures(1, &cupemapTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cupemapTextureID);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < skyboxTextures.size(); i++)
    {
        data = stbi_load(skyboxTextures[i].c_str(), &width, &height, &nrChannels, 0);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

void createDynamicCubemapTexture() 
{
    // Create & setup the static cubemap texture.
    glGenTextures(1, &dynamicCubemapTexId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubemapTexId);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    for (unsigned int i = 0; i < skyboxTextures.size(); i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, captureWidth, captureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
void drawModel(GLuint vao, size_t faceCount)
{
    glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
}

void display()
{
  /*  glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);*/

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    //cout << "Fps:" << 1000/(deltaTime * 1000) << endl;

    // Render offscreen 6 times for th dynamic cube map
    // all objects except teapot.
    // specific camera setup for each render.
    dynamicMappingPasses();

    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    finalRenderPass();

    // End of frame, update interpolating variables.
    centralObjectRotDeg += 25.0f * deltaTime;
    orbitObjectRotDef += 50.0f * deltaTime;
    lastFrame = currentFrame;
}

// across passes only camGaze and camUp will change. if store them in an array of 6, we can easily do this in a loop.
void dynamicMappingPasses() 
{
    // Want to obtain square image for cubemap. 
    // then, aspect ratio = 1, fovy = fovx = 90 deg.
    glViewport(0, 0, captureWidth, captureHeight);
    projectionMatrix = glm::perspective(captureFovy, 1.0f, 0.1f, 100.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffers[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureWidth, captureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffers[0]);

    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubemapTexId);
    for (int i = 0; i < 6; i++) {
        // render each face
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[i]);

        //glBindTexture(GL_TEXTURE_2D, colorTextures[i]);a
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, captureWidth, captureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        //glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, captureWidth, captureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        //glNamedFramebufferTexture(framebuffers[i], GL_COLOR_ATTACHMENT0, colorTextures[i], 0);
        //glNamedFramebufferTexture(framebuffers[i], GL_DEPTH_ATTACHMENT, depthTextures[i], 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dynamicCubemapTexId, 0);
        // clear
        glClearColor(0, 0, 0, 1);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glm::vec3 renderPos(0, 0, 0);
        viewingMatrix = glm::lookAt(teapotPosition, teapotPosition + camGazeVectors[i], camUpVectors[i]);

        //viewingMatrix = glm::lookAt(renderPos, renderPos + camGazeVectors[i], camUpVectors[i]);

        // draw scene, except mirror object(teapot)
        drawSkybox();
        drawOrbitingObject();

        glTextureBarrier();
    }

    // end of dynamic mapping.
    // bind default framebuffer, set newly created textures as readable textures to be used in the final render pass.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void finalRenderPass() 
{
    // use actual camera setup determined via user interaction.
    viewingMatrix = glm::lookAt(cameraPos, cameraPos + cameraGaze, cameraUp);
    glViewport(0, 0, gWidth, gHeight); // reset viewport and projection to actual values.

    float aspect = (float)gWidth / (float)gHeight;
    projectionMatrix = glm::perspective(defaultFovyRad, aspect, 0.1f, 100.0f);

    // Draw all objects
    drawSkybox(); // uses shader/program 1
    drawTeapot(); // uses program 2
    drawOrbitingObject(); // uses program 0, default shading.
}
void drawSkybox() 
{
    activeProgramIndex = 1;
    setShaderParams(activeProgramIndex);

    glDepthMask(GL_FALSE);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cupemapTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void drawOrbitingObject() 
{
    float orbitRotRad = (float)(orbitObjectRotDef / 180.f) * M_PI;

    glm::vec3 orbitObjectPosition = teapotPosition;
    orbitObjectPosition.y -= 4.5f;
    glm::mat4 orbitObjectTranslation = glm::translate(glm::mat4(1.0), orbitObjectPosition);



    float scalef = 1.5f;
    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(scalef, scalef, scalef));

    auto t1 = glm::translate(scale, teapotPosition);
    //auto t1 = glm::translate(glm::mat4(1.0), teapotPosition);
    auto rot1 = glm::rotate(t1, orbitRotRad, glm::vec3(1, 0, 0));
    auto t2 = glm::translate(rot1, -teapotPosition);


    modelingMatrix = t2 * orbitObjectTranslation;

    activeProgramIndex = 0;
    setShaderParams(activeProgramIndex);
    drawModel(orbitVAO, orbitFaces.size());
}


void drawTeapot() 
{
    glm::mat4 teapotTranslation = glm::translate(glm::mat4(1.0), teapotPosition);
    float rotRad = (float)(centralObjectRotDeg / 180.f) * M_PI;
    glm::quat rotQuat(cos(rotRad / 2), 0, 1 * sin(rotRad / 2), 0);

    float scalef = 0.5f;
    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(scalef, scalef, scalef));
    modelingMatrix = teapotTranslation * glm::toMat4(rotQuat) * scale * glm::mat4(1.0);

    activeProgramIndex = 2;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubemapTexId);

    setShaderParams(activeProgramIndex);
    drawModel(teapotVAO, gFaces.size());
}
void setShaderParams(int index) 
{
    glUseProgram(gProgram[index]);
    glUniformMatrix4fv(projectionMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc[index], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glUniform3fv(eyePosLoc[index], 1, glm::value_ptr(cameraPos));
}
void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

	// Use perspective projection

    float aspect = (float)w / (float)h;
    projectionMatrix = glm::perspective(defaultFovyRad, aspect, 0.1f , 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)

    lastX = w / 2;
    lastY = h / 2;

	viewingMatrix = glm::mat4(1);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const float cameraSpeed = 30.0f * deltaTime; 
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraGaze;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraGaze;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraGaze, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraGaze, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos -= cameraUp * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos += cameraUp * cameraSpeed;
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int width = 1366, height = 766;
    window = glfwCreateWindow(width, height, "HW2 -- Cubemapping!", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy_s(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat_s(rendererInfo, " - ");
    strcat_s(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstTimeReceivingMouseInput)
    {
        lastX = xpos;
        lastY = ypos;
        firstTimeReceivingMouseInput = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    mouseYaw += xoffset;
    mousePitch += yoffset;

    if (mousePitch > 89.0f)
        mousePitch = 89.0f;
    if (mousePitch < -89.0f)
        mousePitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(mouseYaw)) * cos(glm::radians(mousePitch));
    direction.y = sin(glm::radians(mousePitch));
    direction.z = sin(glm::radians(mouseYaw)) * cos(glm::radians(mousePitch));
    cameraGaze = glm::normalize(direction);
}
