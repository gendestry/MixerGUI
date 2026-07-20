#include "Shader.h"
#include <fstream>
#include <glad/gl.h>
#include <iostream>
#include <sstream>
#include <string>

Shader::Shader(const char *vertex, const char *fragment) {
  std::string vertString, fragString;

  // Reading from file
  std::ifstream vertFile(vertex), fragFile(fragment);
  std::stringstream vertStream, fragStream;

  if (!vertFile.is_open())
    std::cerr << vertex << " not found!" << std::endl;

  if (!fragFile.is_open())
    std::cerr << fragment << " not found!" << std::endl;

  vertStream << vertFile.rdbuf();
  fragStream << fragFile.rdbuf();

  vertFile.close();
  fragFile.close();

  vertString = vertStream.str();
  fragString = fragStream.str();

  const char *vertData = vertString.c_str();
  const char *fragData = fragString.c_str();

  // Shader compilation
  unsigned int vertShader, fragShader;
  vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, &vertData, NULL);
  glCompileShader(vertShader);
  check(vertShader, "VERTEX");

  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, &fragData, NULL);
  glCompileShader(fragShader);
  check(fragShader, "FRAGMENT");

  // Shader program creation
  m_ID = glCreateProgram();
  glAttachShader(m_ID, vertShader);
  glAttachShader(m_ID, fragShader);
  glLinkProgram(m_ID);
  check(m_ID, "PROGRAM");

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
}

Shader::~Shader() { glDeleteProgram(m_ID); }

void Shader::use() { glUseProgram(m_ID); }

void Shader::disable() { glUseProgram(0); }

void Shader::setInt(const char *name, const int &value) {
  glUniform1i(getUniformLocation(name), value);
}

void Shader::setBool(const char *name, const bool &value) {
  glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const char *name, const float &value) {
  glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec3(const char *name, const glm::vec3 &value) {
  glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec4(const char *name, const glm::vec4 &value) {
  glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setMat3(const char *name, const glm::mat3 &value) {
  glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::setMat4(const char *name, const glm::mat4 &value) {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::check(unsigned int shader, const char *type) {
  int success;
  char infoLog[1024];

  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR - " << type << ":\n" << infoLog << std::endl;
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR - PROGRAM:\n" << infoLog << std::endl;
    }
  }
}

int Shader::getUniformLocation(const char *name) {
  // Check if we already have the location
  if (m_UniformLocations.find(name) != m_UniformLocations.end())
    return m_UniformLocations[name];

  // If we don't we find it and add it to the list
  int location = glGetUniformLocation(m_ID, name);
  m_UniformLocations[name] = location;
  return location;
}