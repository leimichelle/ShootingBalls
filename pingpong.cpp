// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include Bullet
#include "btBulletDynamicsCommon.h"

using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>
#include <vboindexer.hpp>
#include <vboindexer.hpp>
#include <primitives.hpp>

void addPlaneToSim(btVector3 normal, btScalar d, btAlignedObjectArray<btCollisionShape*>& collisionShapes, btDynamicsWorld* dynamicsWorld) {
    btCollisionShape* Shape = new btStaticPlaneShape(normal, d);
    collisionShapes.push_back(Shape);
    
    btScalar mass(0.);
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        Shape->calculateLocalInertia(mass, localInertia);
    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, Shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setRestitution(0.5);
    body->setFriction(0.);
    //add the body to the dynamics world
    
    dynamicsWorld->addRigidBody(body);
    
}

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window and create its OpenGL context
    int screenWidth = 1024;
    int screenHeight = 768;
    window = glfwCreateWindow( screenWidth, screenHeight, "Ball Shotting", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetMouseButtonCallback(window, mouseCallBack);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, screenWidth/2, screenHeight/2);
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    
    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // Create and compile our cursor program from the shaders
    GLuint cursor_programID = LoadShaders( "CursorShading.vertexshader", "CursorShading.fragmentshader" );
    
    // Get a handle for our "MVP" uniform
    GLuint MVPID = glGetUniformLocation(cursor_programID, "MVP");
    GLuint CursID = glGetUniformLocation(cursor_programID, "cur_pos");
    
    // Create and compile our shader for the giant environment ball
    GLuint env_programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
    
    // Get a handle for our "MVP" uniform
    GLuint env_MatrixID = glGetUniformLocation(env_programID, "MVP");

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 env_Model      = glm::scale(glm::mat4(1.0f),vec3(70,70,70));
    
    // Load the texture using any two methods
    //GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    GLuint Texture = loadBMP_custom("./sky.bmp");
    
    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(env_programID, "myTextureSampler");
    
    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
    
    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint ColorID = glGetUniformLocation(programID, "diffuse_colour");
    GLuint TrasparencyID = glGetUniformLocation(programID, "transparency");
    
    // Load the texture
    //GLuint Texture = loadBMP_custom("pingpong.bmp");
    
    // Get a handle for our "myTextureSampler" uniform
    //GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
    
    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ("pingpong.obj", vertices, uvs, normals);
    
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    
    // Load it into a VBO
    
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
    
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
    
    //An array that stores randomly generated colour of each ball
    //randomly generate a colour
    std::vector<vec3> ball_colors;
    vec3 ball_color;
    ball_color[0]= (double)(rand() % 256)/255.;
    ball_color[1] = (double)(rand() % 256)/255.;
    ball_color[2] = (double)(rand() % 256)/255.;
    ball_colors.insert(ball_colors.end(), ball_color);
    
    /*Create a Box*****************************************************************/
    // Read our .obj file
    std::vector<glm::vec3> f_vertices;
    //f_uvs will be arbitrarily initialized such that we can use the indexVBO function but I gave up using texture mapping so it does not matter anymore
    std::vector<glm::vec2> f_uvs;
    std::vector<glm::vec3> f_normals;
    //fail to use maya to properly export a box, might as well write my own
    res = loadOBJ("box.obj", f_vertices, f_uvs, f_normals);
    
    std::vector<unsigned short> f_indices;
    std::vector<glm::vec3> f_indexed_vertices;
    std::vector<glm::vec2> f_indexed_uvs;
    std::vector<glm::vec3> f_indexed_normals;
    indexVBO(f_vertices, f_uvs, f_normals, f_indices, f_indexed_vertices, f_indexed_uvs, f_indexed_normals);
    
    // Load it into a VBO
    
    GLuint f_vertexbuffer;
    glGenBuffers(1, &f_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, f_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, f_indexed_vertices.size() * sizeof(glm::vec3), &f_indexed_vertices[0], GL_STATIC_DRAW);
    
    GLuint f_uvbuffer;
    glGenBuffers(1, &f_uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, f_uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, f_indexed_uvs.size() * sizeof(glm::vec2), &f_indexed_uvs[0], GL_STATIC_DRAW);
    
    GLuint f_normalbuffer;
    glGenBuffers(1, &f_normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, f_normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, f_indexed_normals.size() * sizeof(glm::vec3), &f_indexed_normals[0], GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    GLuint f_elementbuffer;
    glGenBuffers(1, &f_elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f_elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, f_indices.size() * sizeof(unsigned short), &f_indices[0] , GL_STATIC_DRAW);
    
    glm::mat4 ModelMatrix2 = glm::mat4(1.0);
    ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(0,0.5, 2.6f));
    ModelMatrix2 = glm::scale(ModelMatrix2, glm::vec3(2.f, 3.f, 7.2f));
    /******************************************************************************/
    //Initialize Bullet Physics stuff
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    
    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    
    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,0,0));
    //Initialization ends
    
    //keep track of the shapes, we release memory at exit.
    //make sure to re-use collision shapes among rigid bodies whenever possible!
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    //TODO Make a Table
    {//The BOX, made up of six infinite planes (to simplify)
        addPlaneToSim(btVector3(0,1,0), -1., collisionShapes, dynamicsWorld);
        addPlaneToSim(btVector3(0,-1,0), -2., collisionShapes, dynamicsWorld);
        addPlaneToSim(btVector3(1,0,0), -1, collisionShapes, dynamicsWorld);
        addPlaneToSim(btVector3(-1,0,0), -1, collisionShapes, dynamicsWorld);
        addPlaneToSim(btVector3(0,0,1), -1, collisionShapes, dynamicsWorld);
        addPlaneToSim(btVector3(0,0,-1), -6, collisionShapes, dynamicsWorld);
    }
    int num_planes = 6;
    float ball_speed = 1.;
    {//create a dynamic rigidbody
        
        //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
        btCollisionShape* colShape = new btSphereShape(btScalar(0.1));
        collisionShapes.push_back(colShape);
        
        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();
        
        btScalar mass(0.027f);
        
        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);
        
        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            colShape->calculateLocalInertia(mass, localInertia);
        
        startTransform.setOrigin(btVector3(0, 0, 0));
        
        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setRestitution(0.5);
        body->setLinearVelocity(btVector3(0,-ball_speed,0));
        body->setFriction(0.);
        dynamicsWorld->addRigidBody(body);
    }
    //number of rigid bodies that are not balls in the dynamicWorld
    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    double startTime = glfwGetTime();
    do{
        
        // Measure speed
        double currentTime = glfwGetTime();
        while ( currentTime - lastTime <= 0.0166 ){
            currentTime = glfwGetTime();
            nbFrames++;
        }
        // printf and reset timer
        //printf("%f ms/frame\n", 1000.0/double(nbFrames));
        nbFrames = 0;
        lastTime = currentTime;
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        
        //Detect if user shot another ball into the scene
        if (get_hasShot()){
            set_hasShot(false);
            glm::vec3 gun_origin;
            glm::vec3 shoot_direction;
            computeShooting(gun_origin, shoot_direction);
            shoot_direction = ball_speed * shoot_direction;
            btCollisionShape* colShape = new btSphereShape(btScalar(0.1));
            collisionShapes.push_back(colShape);
            
            
            /// Create Dynamic Objects
            btTransform startTransform;
            startTransform.setIdentity();
            
            btScalar mass(0.027f);
            
            //rigidbody is dynamic if and only if mass is non zero, otherwise static
            bool isDynamic = (mass != 0.f);
            
            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
                colShape->calculateLocalInertia(mass, localInertia);
            
            startTransform.setOrigin(btVector3(gun_origin[0], gun_origin[1], gun_origin[2]));
            
            //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
            btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
            btRigidBody* body = new btRigidBody(rbInfo);
            body->setRestitution(0.5);
            body->setLinearVelocity(btVector3(shoot_direction[0],shoot_direction[1],shoot_direction[2]));
            body->setFriction(0.);
            dynamicsWorld->addRigidBody(body);
            //Add a new color for the new ball as well
            vec3 ball_color;
            ball_color[0]= (double)(rand() % 256)/255.;
            ball_color[1] = (double)(rand() % 256)/255.;
            ball_color[2] = (double)(rand() % 256)/255.;
            ball_colors.insert(ball_colors.end(), ball_color);
        }
        
        //Before step into the next simulation timestamp, want to record the old speed so that we can maintain the the same speed after the simulation. We are not trying to follow actual physics here. Instead, we just want to create a visual effect that all the balls keep bouncing forever
        int num_colli_obj = dynamicsWorld->getNumCollisionObjects();
        
        for (int j = num_colli_obj - 1; j >= num_planes; j--) {
            
        }
        
        //Run physical simulations to updates objects' model matrices
        double currentSimTime = currentTime - startTime;
        dynamicsWorld->stepSimulation(currentSimTime, 10);
        ////// Start of the rendering of the environment ball//////
        glUseProgram(env_programID);
        mat4 env_MVP = ProjectionMatrix*ViewMatrix*env_Model;
        glUniformMatrix4fv(env_MatrixID,1, GL_FALSE, &env_MVP[0][0]);
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              0,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                              1,                                // attribute
                              2,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       indices.size(),    // count
                       GL_UNSIGNED_SHORT,   // type
                       (void*)0           // element array buffer offset
                       );
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        ////// End of the rendering of the environment ball//////
        ////// Start of the rendering of the balls//////
        std::vector<glm::mat4> Matrices;
        for (int j = num_colli_obj - 1; j >= 0; j--)
        {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
            btRigidBody* body = btRigidBody::upcast(obj);
            btTransform trans;
            if (body && body->getMotionState())
            {
                body->getMotionState()->getWorldTransform(trans);
                btVector3 velocity = body->getLinearVelocity();
                velocity.normalize();
                velocity = ball_speed * velocity;
                body->setLinearVelocity(velocity);
            }
            else
            {
                trans = obj->getWorldTransform();
            }
            if (j>=num_planes) {
                glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0),glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
                Matrices.insert(Matrices.end(),ModelMatrix);
                //printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            }
        }
        
        // Use our shader
        glUseProgram(programID);
        // Get a handle for our "LightPosition" uniform
        GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
        
        computeLightPosFromInputs();
        glm::vec3 lightPos = getlightPos();
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        
        glUniform1f(TrasparencyID,1.);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              0,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                              1,                                // attribute
                              2,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                              2,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        
        
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        int i = ball_colors.size()-1;
        for (std::vector<mat4>::iterator it=Matrices.begin(); it<Matrices.end(); it++) {
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * *it;
            glUniform3f(ColorID, ball_colors[i][0], ball_colors[i][1],ball_colors[i][2]);
            if (i-1 >=0) { //sanity check, but should always be true
                i--;
            }
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &(*it)[0][0]);
            // Draw the triangles !
            glDrawElements(
                           GL_TRIANGLES,      // mode
                           indices.size(),    // count
                           GL_UNSIGNED_SHORT,   // type
                           (void*)0           // element array buffer offset
                           );
        }
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        
        ////// End of rendering of the balls //////
        ////// Start of the rendering of the box object //////
        
        // Send our transformation to the currently bound shader,
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);
        glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);

        glUniform3f(ColorID, 0.5,0.5,0.5);
        
        glUniform1f(TrasparencyID, 0.25);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, f_vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, f_uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, f_normalbuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f_elementbuffer);
        
        // Draw the triangles !
        glDrawElements(GL_TRIANGLES, f_indices.size(), GL_UNSIGNED_SHORT, (void*)0);
        
        
        ////// End of rendering of the box object //////
        ////// Start of the rendering of the gun object //////
        // Use our cursor shader
        glUseProgram(cursor_programID);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, f_vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Send our transformation to the currently bound shader,
        glm::mat4 ModelMatrix = glm::translate(inverse(ViewMatrix),vec3(0.,0.,-1.));
        ModelMatrix = glm::scale(ModelMatrix,glm::vec3(0.01,0.01,0.01));
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
        double xpos,ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = ((float)xpos/(float)screenWidth  - 0.5f) * 2.0f;
        ypos = -((float)ypos/(float)screenHeight  - 0.5f) * 2.0f;
        vec2 curpos(xpos,ypos);
        glUniform2fv(CursID,1,&curpos[0]);
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f_elementbuffer);
        
        // Draw the triangles !
        glDrawElements(GL_TRIANGLES, f_indices.size(), GL_UNSIGNED_SHORT, (void*)0);
        glDisableVertexAttribArray(0);
        ////// End of the rendering of the gun object //////
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
    //Clean up Bullet Physics stuff
    //remove the rigidbodies from the dynamics world and delete them
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }
    
    //delete collision shapes
    for (int j = 0; j < collisionShapes.size(); j++)
    {
        btCollisionShape* shape = collisionShapes[j];
        collisionShapes[j] = 0;
        delete shape;
    }
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
    return 0;
}

