#include "Cube.h"

#include <glm/gtc/matrix_transform.hpp>
#include <utility>

namespace {
// 36 vertices (12 triangles) of a unit cube centered at the origin.
// Layout per vertex: position.xyz, normal.xyz
constexpr float kCubeVertices[] = {
    // back face (-Z)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    // front face (+Z)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    // left face (-X)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    // right face (+X)
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    // bottom face (-Y)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    // top face (+Y)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
};

// 12 edges (24 vertices) of the unit cube, positions only, for the wireframe
// outline. Drawn as GL_LINES.
constexpr float kEdgeVertices[] = {
    // back face
    -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
    // front face
    -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
    // connecting edges
    -0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,
};
} // namespace

Cube::Cube() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices, GL_STATIC_DRAW);

    // position -> location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          reinterpret_cast<void*>(0));
    // normal -> location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);

    // Edge geometry for the wireframe outline (position only, location 0).
    glGenVertexArrays(1, &m_edgeVao);
    glGenBuffers(1, &m_edgeVbo);
    glBindVertexArray(m_edgeVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_edgeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kEdgeVertices), kEdgeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          reinterpret_cast<void*>(0));
    glBindVertexArray(0);
}

Cube::~Cube() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_edgeVbo) glDeleteBuffers(1, &m_edgeVbo);
    if (m_edgeVao) glDeleteVertexArrays(1, &m_edgeVao);
}

Cube::Cube(Cube&& other) noexcept
    : m_model(other.m_model),
      m_color(other.m_color),
      m_pos(other.m_pos),
      m_vao(other.m_vao),
      m_vbo(other.m_vbo),
      m_edgeVao(other.m_edgeVao),
      m_edgeVbo(other.m_edgeVbo) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_edgeVao = 0;
    other.m_edgeVbo = 0;
}

Cube& Cube::operator=(Cube&& other) noexcept {
    if (this != &other) {
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_edgeVbo) glDeleteBuffers(1, &m_edgeVbo);
        if (m_edgeVao) glDeleteVertexArrays(1, &m_edgeVao);
        m_model = other.m_model;
        m_color = other.m_color;
        m_pos = other.m_pos;
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_edgeVao = other.m_edgeVao;
        m_edgeVbo = other.m_edgeVbo;
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_edgeVao = 0;
        other.m_edgeVbo = 0;
    }
    return *this;
}

void Cube::draw(GLuint shader) const {
    const GLint modelLoc = glGetUniformLocation(shader, "uModel");
    const GLint colorLoc = glGetUniformLocation(shader, "uColor");
    if (modelLoc >= 0)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m_model[0][0]);
    if (colorLoc >= 0)
        glUniform3fv(colorLoc, 1, &m_color[0]);
    const GLint selLoc = glGetUniformLocation(shader, "uSelected");
    if (selLoc >= 0)
        glUniform1i(selLoc, m_selected ? 1 : 0);
    const GLint wireLoc = glGetUniformLocation(shader, "uWireframe");

    // Solid pass.
    if (wireLoc >= 0)
        glUniform1i(wireLoc, 0);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Wireframe outline pass: flat-colored edges drawn on top. GL_LEQUAL lets
    // the lines win the depth test against the coincident cube faces.
    if (wireLoc >= 0)
        glUniform1i(wireLoc, 1);
    if (colorLoc >= 0) {
        const glm::vec3 wire =
            m_selected ? glm::vec3(1.0f, 0.8f, 0.1f) : glm::vec3(0.0f, 0.0f, 0.0f);
        glUniform3fv(colorLoc, 1, &wire[0]);
    }
    glDepthFunc(GL_LEQUAL);
    glLineWidth(1.5f);
    glBindVertexArray(m_edgeVao);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    if (wireLoc >= 0)
        glUniform1i(wireLoc, 0);
}
