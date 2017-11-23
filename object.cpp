#include <stdio.h>
#include <stdlib.h>
#include <regex>
#include <iterator>
#include <assert.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <object.hpp>
#include <objloader.hpp>
#include <texture.hpp>
#include <vboindexer.hpp>

object::object(){}

object::object(int _id, const char * _obj_file, const char * _texture_file) {
    M = glm::mat4(1.0f);
    next = NULL;
    loadOBJ(_obj_file, vertices, uvs, normals);
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    vertice_size = indexed_vertices.size()*sizeof(glm::vec3);
    uv_size = indexed_uvs.size() * sizeof(glm::vec2);
    normal_size = indexed_normals.size() * sizeof(glm::vec3);
    index_size = indices.size() * sizeof(unsigned short);
    buffersize = vertice_size + uv_size + normal_size + index_size;
    std::regex bmp_regex(".+\\.bmp");
    std::regex dds_regex(".+\\.dds");
    if (std::regex_search(_texture_file, bmp_regex)) {
        Texture = loadBMP_custom(_texture_file);
    }
    else if (std::regex_search(_texture_file, dds_regex)) {
        Texture = loadDDS(_texture_file);
    }
    else {
        printf("Invalid texture file format!\n");
        assert(false);
    }
    glGenVertexArrays(1, &VertexArrayID);
    /*glBindVertexArray(VertexArrayID);
    
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec3), &indexed_uvs[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
    glBindVertexArray(0); // Disable our Vertex Buffer Object*/
}
void object::draw(glm::mat4 V, glm::mat4 P, GLuint& MatrixID, GLuint& ModelMatrixID) {
    glBindVertexArray(VertexArrayID);
    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glm::mat4 MVP = P * V * M;
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &M[0][0]);
    
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
    
    // Draw the triangles !
    //glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
    // Draw the triangles !
    glDrawElements(
                   GL_TRIANGLES,      // mode
                   indices.size(),    // count
                   GL_UNSIGNED_SHORT,   // type
                   (void*)0           // element array buffer offset
                   );
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindVertexArray(0); // Disable our Vertex Buffer Object
}

object::~object() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
}

//y and z are not used if it is a sphere object
bullet_object::bullet_object(int _id, const char * _obj_file, const char * _texture_file, rigidbodyshape _shape, btScalar _x, btScalar _y, btScalar _z, btScalar _mass, btVector3 _localInertia, btScalar _Restitution) {
    object::object(_id, _obj_file, _texture_file);
    shape = _shape;
    
    if (shape == sphere) {
        colShape = new btSphereShape(_x);
    }
    else if (shape == box) {
        colShape =  new btBoxShape(btVector3(_x,_y,_z));
    }
    
    
    /// Create Dynamic Objects
    startTransform.setIdentity();
    mass = _mass;
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    localInertia = _localInertia;
    if (isDynamic)
        colShape->calculateLocalInertia(mass, localInertia);
    
    startTransform.setOrigin(btVector3(0, 1, 0));
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
    body = new btRigidBody(rbInfo);
    body->setRestitution(_Restitution);
    next = NULL;
}

bullet_object::~bullet_object() {
    object::~object();
    delete colShape;
    delete body;
    delete myMotionState;
}