#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

class Shader {
  unsigned int m_ID;
  std::unordered_map<const char *, int> m_UniformLocations;

public:
  Shader(const char *vertex, const char *fragment);
  ~Shader();

  void use();
  void disable();

  unsigned int id() const { return m_ID; }

  void setInt(const char *name, const int &value);
  void setBool(const char *name, const bool &value);
  void setFloat(const char *name, const float &value);
  void setVec3(const char *name, const glm::vec3 &value);
  void setVec4(const char *name, const glm::vec4 &value);
  void setMat3(const char *name, const glm::mat3 &value);
  void setMat4(const char *name, const glm::mat4 &value);

private:
  void check(unsigned int shader, const char *type);
  int getUniformLocation(const char *name);
};