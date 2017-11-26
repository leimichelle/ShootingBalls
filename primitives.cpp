#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>
using namespace glm;

void createUnitCube(std::vector<glm::vec3>& vertices,std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals) {
    glm::vec3 template_vertices[] =
    {
        vec3(-1.0f, +1.0f, +1.0f),  // 0
        vec3(+1.0f, +1.0f, +1.0f),  // 1
        vec3(+1.0f, +1.0f, -1.0f),  // 2
        vec3(-1.0f, +1.0f, -1.0f),  // 3
        vec3(-1.0f, +1.0f, -1.0f),  // 4
        vec3(+1.0f, +1.0f, -1.0f),  // 5
        vec3(+1.0f, -1.0f, -1.0f),  // 6
        vec3(-1.0f, -1.0f, -1.0f),  // 7
        vec3(+1.0f, +1.0f, -1.0f),  // 8
        vec3(+1.0f, +1.0f, +1.0f),  // 9
        vec3(+1.0f, -1.0f, +1.0f),  // 10
        vec3(+1.0f, -1.0f, -1.0f),  // 11
        vec3(-1.0f, +1.0f, +1.0f),  // 12
        vec3(-1.0f, +1.0f, -1.0f),  // 13
        vec3(-1.0f, -1.0f, -1.0f),  // 14
        vec3(-1.0f, -1.0f, +1.0f),  // 15
        vec3(+1.0f, +1.0f, +1.0f),  // 16
        vec3(-1.0f, +1.0f, +1.0f),  // 17
        vec3(-1.0f, -1.0f, +1.0f),  // 18
        vec3(+1.0f, -1.0f, +1.0f),  // 19
        vec3(+1.0f, -1.0f, -1.0f),  // 20
        vec3(-1.0f, -1.0f, -1.0f),  // 21
        vec3(-1.0f, -1.0f, +1.0f),  // 22
        vec3(+1.0f, -1.0f, +1.0f),  // 23
    };
    std::vector<glm::vec3> tmp(template_vertices, template_vertices + sizeof(template_vertices) / sizeof(glm::vec3));
    vertices.insert(vertices.begin(),tmp.begin(),tmp.end());
    
    glm::vec3 template_normals[] = {
        vec3(+0.0f, +1.0f, +0.0f),  // Normal
        vec3(+0.0f, +1.0f, +0.0f),  // Normal
        vec3(+0.0f, +1.0f, +0.0f),  // Normal
        vec3(+0.0f, +1.0f, +0.0f),  // Normal
        vec3(+0.0f, +0.0f, -1.0f),  // Normal
        vec3(+0.0f, +0.0f, -1.0f),  // Normal
        vec3(+0.0f, +0.0f, -1.0f),  // Normal
        vec3(+0.0f, +0.0f, -1.0f),  // Normal
        vec3(+1.0f, +0.0f, +0.0f),  // Normal
        vec3(+1.0f, +0.0f, +0.0f),  // Normal
        vec3(+1.0f, +0.0f, +0.0f),  // Normal
        vec3(+1.0f, +0.0f, +0.0f),  // Normal
        vec3(-1.0f, +0.0f, +0.0f),  // Normal
        vec3(-1.0f, +0.0f, +0.0f),  // Normal
        vec3(-1.0f, +0.0f, +0.0f),  // Normal
        vec3(-1.0f, +0.0f, +0.0f),  // Normal
        vec3(+0.0f, +0.0f, +1.0f),  // Normal
        vec3(+0.0f, +0.0f, +1.0f),  // Normal
        vec3(+0.0f, +0.0f, +1.0f),  // Normal
        vec3(+0.0f, +0.0f, +1.0f),  // Normal
        vec3(+0.0f, -1.0f, +0.0f),  // Normal
        vec3(+0.0f, -1.0f, +0.0f),  // Normal
        vec3(+0.0f, -1.0f, +0.0f),  // Normal
        vec3(+0.0f, -1.0f, +0.0f),  // Normal

    };
    std::vector<glm::vec3> tmp1(template_normals, template_normals + sizeof(template_normals) / sizeof(glm::vec3));
    normals.insert(normals.begin(),tmp1.begin(),tmp1.end());
    
    std::vector<glm::vec2> tmp2 (24,glm::vec2(0.,0.));
    uvs.insert(uvs.begin(),tmp2.begin(),tmp2.end());
}