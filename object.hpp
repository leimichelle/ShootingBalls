#ifndef OBJECTS_HPP
#define OBJECTS_HPP
#include <string.h>
// Include GLEW
#include <GL/glew.h>
// Include Bullet
#include "btBulletDynamicsCommon.h"
enum rigidbodyshape {sphere,box};

class object{
public:
    //Make them all public to reduce overhead of function calls since I am the only developer anyways.
    int id;
    glm::mat4 M;
    object *next;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    std::vector<unsigned short> indices;
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;
    GLsizeiptr vertice_size;
    GLsizeiptr uv_size;
    GLsizeiptr normal_size;
    GLsizeiptr index_size;
    GLsizeiptr buffersize;
    GLintptr objoffset;
    GLuint VertexArrayID;
    GLuint Texture;
    object();
    object(int _id, const char * _obj_file, const char * _texture_file);
    void draw(glm::mat4 V, glm::mat4 P,GLuint& MatrixID, GLuint& ModelMatrixID);
    ~object();
};

class bullet_object : public object {
public:
    rigidbodyshape shape;
    btCollisionShape* colShape;
    btScalar mass;
    btTransform startTransform;
    btVector3 localInertia;
    btDefaultMotionState* myMotionState;
    btRigidBody* body;
    bullet_object *next;
    bullet_object(int _id, const char * _obj_file, const char * _texture_file, rigidbodyshape _shape, btScalar _x, btScalar _y, btScalar _z, btScalar _mass, btVector3 _localInertia, btScalar _Restitution);
    ~bullet_object();
private:
};

#endif