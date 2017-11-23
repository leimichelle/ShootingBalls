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
#include <object.hpp>
#include <vboindexer.hpp>

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
	window = glfwCreateWindow( 1024, 768, "Tutorial 08 - Basic Shading", NULL, NULL);
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
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Black background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
    /**********************************************************************/
    
    //bullet_object::bullet_object(int _id, const char * _obj_file, const char * _texture_file, rigidbodyshape _shape, btScalar _x, btScalar _y, btScalar _z, btScalar _mass, btVector3 _localInertia, btScalar _Restitution)
    bullet_object* pingpong = new bullet_object(1,"pingpong.obj","pingpong.bmp", sphere, btScalar(0.5), btScalar(0.), btScalar(0.),btScalar(0.0027), btVector3(0,0,0), btScalar(0.9));
    //bullet_object* ground = new bullet_object(2, "pingpong.obj","pingpong.bmp", box, btScalar(10.), btScalar(1.), btScalar(10.), btScalar(0.), btVector3(0,0,0), btScalar(1.));
    //ground->M = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -0.5f, 0.0f));

    /**********************************************************************/

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint MouseHit = glGetUniformLocation(programID, "MouseHit");
    //GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	/*std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("pingpong.obj", vertices, uvs, normals);
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;*/
    
    // Load the texture
    GLuint pingpongTexture = loadBMP_custom("pingpong.bmp");
    GLuint floorTexture = loadBMP_custom("floor.bmp");
    //indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	// Load it into a VBO
    glBindVertexArray(pingpong->VertexArrayID);
	//GLuint vertexbuffer;
	glGenBuffers(1, &(pingpong->vertexbuffer));
	glBindBuffer(GL_ARRAY_BUFFER, pingpong->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, pingpong->vertice_size * sizeof(glm::vec3), &(pingpong->indexed_vertices[0]), GL_STATIC_DRAW);

	//GLuint uvbuffer;
	glGenBuffers(1, &(pingpong->uvbuffer));
	glBindBuffer(GL_ARRAY_BUFFER, pingpong->uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, pingpong->uv_size * sizeof(glm::vec2), &(pingpong->indexed_uvs[0]), GL_STATIC_DRAW);

	//GLuint normalbuffer;
	glGenBuffers(1, &(pingpong->normalbuffer));
	glBindBuffer(GL_ARRAY_BUFFER, pingpong->normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, pingpong->normal_size * sizeof(glm::vec3), &(pingpong->indexed_normals[0]), GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    //GLuint elementbuffer;
    glGenBuffers(1, &(pingpong->elementbuffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pingpong->elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pingpong->index_size * sizeof(unsigned short), &(pingpong->indices[0]), GL_STATIC_DRAW);
    
    /*********************************************************************************************************/

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    
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
    dynamicsWorld->setGravity(btVector3(0,-9.81f,0));
    //Initialization ends
    
    //keep track of the shapes, we release memory at exit.
    //make sure to re-use collision shapes among rigid bodies whenever possible!
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    
    //TODO Make a Table
    {//Ground
    btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(1.), btScalar(1.), btScalar(1.)));
    
    collisionShapes.push_back(groundShape);
    
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -2, 0));
    
    btScalar mass(0.);
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        groundShape->calculateLocalInertia(mass, localInertia);
    
    //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setRestitution(1.);

    //add the body to the dynamics world
    
    dynamicsWorld->addRigidBody(body);
    }
    {//create a dynamic rigidbody
    
    //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    btCollisionShape* colShape = new btSphereShape(btScalar(0.5));
    collisionShapes.push_back(colShape);
    
    
    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    
    btScalar mass(1.f);
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        colShape->calculateLocalInertia(mass, localInertia);
    
    startTransform.setOrigin(btVector3(0, 1, 0));
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setRestitution(0.9);
    dynamicsWorld->addRigidBody(body);
    }

    double startTime = glfwGetTime();
	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
        //Run physical simulations to updates objects' model matrices
        double currentTime = glfwGetTime() - startTime;
        //hack: slow down the bounce a bit
        currentTime/=10.;
        dynamicsWorld->stepSimulation(currentTime, 10);
        glm::mat4 ModelMatrix;
        for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
        {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
            btRigidBody* body = btRigidBody::upcast(obj);
            btTransform trans;
            if (body && body->getMotionState())
            {
                body->getMotionState()->getWorldTransform(trans);
            }
            else
            {
                trans = obj->getWorldTransform();
            }
            //TODO: fix harded coded pingpong object
            if (j==1) {
                ModelMatrix = glm::translate(glm::mat4(1.0),glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
                pingpong->M = glm::translate(glm::mat4(1.0),glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
                printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            }
        }

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        int MHit = 0;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            MHit = 1;
        }
        // Compute the light position from keyboard input
        computeLightPosFromInputs();
        glm::vec3 lightPos = getlightPos();
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        
        glUniform1i(MouseHit, MHit);
        
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingpongTexture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);
        
        pingpong->draw(ViewMatrix,ProjectionMatrix, MatrixID, ModelMatrixID);
        
        /*glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glUniform1i(TextureID, 1);
        ground->draw(ViewMatrix,ProjectionMatrix, MatrixID, ModelMatrixID);*/
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		/*glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);*/
        

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, pingpong->vertexbuffer);
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
		glBindBuffer(GL_ARRAY_BUFFER, pingpong->uvbuffer);
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
		glBindBuffer(GL_ARRAY_BUFFER, pingpong->normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pingpong->elementbuffer);
		// Draw the triangles !
		//glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
        // Draw the triangles !
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       pingpong->index_size,    // count
                       GL_UNSIGNED_SHORT,   // type
                       (void*)0           // element array buffer offset
                       );
        
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteProgram(programID);
	glDeleteTextures(1, &pingpongTexture);
    glDeleteTextures(1, &floorTexture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
    
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
    delete pingpong;
    //delete ground;
    
	return 0;
}

