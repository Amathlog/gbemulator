#include <exe/controller.h>
#include <GLFW/glfw3.h>

using GBEmulatorExe::Controller;

Controller::Controller(GLFWwindow* window)
    : m_window(window)
{
    
}

Controller::~Controller()
{

}

void Controller::Update()
{
    ToggleUp(glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS);
    ToggleDown(glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS);
    ToggleLeft(glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS);
    ToggleRight(glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS);
    ToggleA(glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_PRESS);
    ToggleB(glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS);
    ToggleStart(glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS);
    ToggleSelect(glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS);
}