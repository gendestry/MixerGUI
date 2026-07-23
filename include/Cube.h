#pragma once

#include "glm/ext/matrix_transform.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>

// A unit cube (centered at the origin) with its own model matrix and color.
// Owns a VAO/VBO holding the cube geometry.
class Cube {
public:
  Cube();
  ~Cube();

  // Non-copyable (owns GL resources), movable.
  Cube(const Cube &) = delete;
  Cube &operator=(const Cube &) = delete;
  Cube(Cube &&other) noexcept;
  Cube &operator=(Cube &&other) noexcept;

  // Draws the cube. `shader` must expose:
  //   uniform mat4 uModel;
  //   uniform vec3 uColor;
  void draw(GLuint shader) const;

  // Model matrix.
  void setModel(const glm::mat4 &model) { m_model = model; }
  const glm::mat4 &model() const { return m_model; }
  glm::mat4 &model() { return m_model; }

  // World position (applied as a translation on top of the model matrix).
  void setPosition(const glm::vec3 &pos) {
    m_pos = pos;
    updateModel();
  }
  const glm::vec3 &position() const { return m_pos; }
  // glm::vec3 &position() { return m_pos; }

  void setRotation(const glm::vec3 &rot) {
    m_rot = rot;
    updateModel();
  }
  const glm::vec3 &rotation() const { return m_rot; }
  // glm::vec3 &rotation() { return m_rot; }

  void updateModel() {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_pos);
    model =
        glm::rotate(model, glm::radians(m_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model =
        glm::rotate(model, glm::radians(m_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model =
        glm::rotate(model, glm::radians(m_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m_model = model;
  }

  // Color (RGB, 0..1).
  void setColor(const glm::vec3 &color) { m_color = color; }
  const glm::vec3 &color() const { return m_color; }
  glm::vec3 &color() { return m_color; }

  // Selection highlight.
  void setSelected(bool selected) { m_selected = selected; }
  bool selected() const { return m_selected; }

private:
  glm::mat4 m_model{1.0f};
  glm::vec3 m_color{1.0f};
  glm::vec3 m_pos{0.0f};
  glm::vec3 m_rot{0.0f};
  bool m_selected = false;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  // Separate line geometry (12 edges) for the wireframe outline pass.
  GLuint m_edgeVao = 0;
  GLuint m_edgeVbo = 0;
};
