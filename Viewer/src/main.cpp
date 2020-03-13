#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>

#include <nfd.h>

#include <stdio.h>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <sstream>
#include <stdlib.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include "Scene.h"
#include "Camera.h"
#include "ImguiMenus.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "Utils.h"


int activeLight = 0;
double zoomFactor = 1;
int windowWidth = 1280;
int windowHeight = 720;
char* windowTitle = "OpenGL Demo";
glm::vec4 clearColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.00f);
bool zoomChanged = false;
std::shared_ptr<Scene> scene;

ImGuiIO* imgui;
GLFWwindow* window;

GLFWwindow* SetupGlfwWindow(int w, int h, const char* window_name);
ImGuiIO& SetupImgui(GLFWwindow* window);
bool Setup(int windowWidth, int windowHeight, const char* windowName);
void Cleanup();

static void GlfwErrorCallback(int error, const char* description);
void RenderFrame(GLFWwindow* window, std::shared_ptr<Scene> scene, Renderer& renderer, ImGuiIO& io);

void glfw_OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
void glfw_OnFramebufferSize(GLFWwindow* window, int width, int height);

float GetAspectRatio();
void DrawImguiMenus();
void HandleImguiInput();

int main(int argc, char **argv)
{

	if (!Setup(windowWidth, windowHeight, windowTitle))
	{
		std::cerr << "Setup failed" << std::endl;
		return -1;
	}

	scene = std::make_shared<Scene>();

	scene->sphere = Utils::LoadMeshModel("C:\\Users\\hamada\\mesh-viewer-ali-hamada\\Data\\sphere.obj");
	scene->sphere->ScaleModel(0.15);
	glm::vec3 eye = glm::vec3(0, 0, 10);
	glm::vec3 at = glm::vec3(0, 0, 0);
	glm::vec3 up = glm::vec3(0, 1, 0);
	Camera camera = Camera(eye, at, up, GetAspectRatio());
	scene->AddCamera(camera);

	scene->AddLight(std::make_shared<PointLight>(glm::vec3(5, 5, 0), glm::vec3(1, 1, 1)));
	scene->AddLight(std::make_shared<PointLight>(glm::vec3(0, 0, 15), glm::vec3(1, 1, 1)));
	scene->AddLight(std::make_shared<PointLight>(glm::vec3(0, 5, 5), glm::vec3(0, 0, 0)));
	//	scene->AddLight(std::make_shared<PointLight>(glm::vec3(-5, 0, 0),  glm::vec3(0, 0, 0)));

	Renderer renderer;
	renderer.LoadShaders();
	//	renderer.LoadTextures();

	while (!glfwWindowShouldClose(window))
	{
		// Poll and process events
		glfwPollEvents();

		// Imgui stuff
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		DrawImguiMenus();
		ImGui::Render();
		HandleImguiInput();

		// Clear the screen and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render scene
		renderer.Render(scene);

		// Imgui stuff
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap front and back buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

static void GlfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool Setup(int windowWidth, int windowHeight, const char* windowName)
{
	GLFWwindow* window = SetupGlfwWindow(windowWidth, windowHeight, windowName);
	if (!window)
	{
		std::cerr << "Window setup failed" << std::endl;
		return false;
	}

	imgui = &SetupImgui(window);

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glEnable(GL_DEPTH_TEST);

	return true;
}

GLFWwindow* SetupGlfwWindow(int windowWidth, int windowHeight, const char* windowName)
{
	// Intialize GLFW
	if (!glfwInit())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create an OpenGL 3.3 core, forward compatible context window
	window = glfwCreateWindow(windowWidth, windowHeight, windowName, NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Make the window's context the current one
	glfwMakeContextCurrent(window);

	// Setup window events callbacks
	glfwSetFramebufferSizeCallback(window, glfw_OnFramebufferSize);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		// An error occured
		std::cerr << "GLAD initialization failed" << std::endl;
		return false;
	}

	return window;
}

ImGuiIO& SetupImgui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsDark();

	glfwSetScrollCallback(window, glfw_OnMouseScroll);

	return io;
}

void HandleImguiInput()
{

	if (!imgui->WantCaptureMouse)
	{
		if (zoomChanged)
		{
			scene->GetActiveCamera().Zoom(zoomFactor);
			zoomChanged = false;
		}
	}
}

void Cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_OnFramebufferSize(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, windowWidth, windowHeight);
	scene->GetActiveCamera().SetAspectRatio(GetAspectRatio());
}

void glfw_OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	zoomFactor = glm::pow(1.1, -yoffset);
	zoomChanged = true;
}

float GetAspectRatio()
{
	return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

void DrawImguiMenus()
{
	ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Once);
	{
		ImGui::Begin("Scene Menu");
		if (ImGui::ColorEdit3("Clear Color", (float*)&clearColor))
		{
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		}

		if (ImGui::CollapsingHeader("Cameras"))
		{
			if (ImGui::Button("Add Camera"))
			{

				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_real_distribution<double> dist(0, 2 * M_PI);
				std::uniform_real_distribution<double> dist2(2, 10);
				double angle = dist(mt);
				double radius = dist2(mt);

				glm::vec4 eye = glm::vec4(radius * glm::cos(angle), 0, radius * glm::sin(angle), 1);
				glm::vec4 at = glm::vec4(0, 0, 0, 1);
				glm::vec4 up = glm::vec4(0, 1, 0, 1);
				scene->AddCamera(Camera(/*-scene->GetActiveCamera().GetEye()*/ eye, at, up, GetAspectRatio()));
			}

			const char** items;
			std::vector<std::string> cameraStrings;
			items = new const char*[scene->GetCameraCount()];
			for (int i = 0; i < scene->GetCameraCount(); i++)
			{
				std::ostringstream s;
				s << "Camera " << i;
				std::string mystring = s.str();
				cameraStrings.push_back(mystring);
			}

			for (int i = 0; i < scene->GetCameraCount(); i++)
			{
				items[i] = cameraStrings[i].c_str();
			}

			int currentCamera = scene->GetActiveCameraIndex();
			ImGui::Combo("Active Camera", &currentCamera, items, scene->GetCameraCount());

			if (currentCamera != scene->GetActiveCameraIndex())
			{
				scene->SetActiveCameraIndex(currentCamera);
				scene->GetActiveCamera().SetAspectRatio(GetAspectRatio());
			}

			delete items;

			float sph_rot1 = 0;
			if (ImGui::SliderFloat("spherical rotate by UP-Axis", &sph_rot1, -0.1f, 10.1f))
			{
				scene->GetActiveCamera().SphericalRotate(glm::vec2(sph_rot1, 0.0f));
			}

			float sph_rot2 = 0;
			if (ImGui::SliderFloat("spherical rotate by Left-Axis", &sph_rot2, -0.1f, 10.1f))
			{
				scene->GetActiveCamera().SphericalRotate(glm::vec2(0.0f, sph_rot2));
			}


			int newProjectionType = scene->GetActiveCamera().IsPrespective() ? 0 : 1;
			ImGui::RadioButton("Prespective", &newProjectionType, 0);
			ImGui::RadioButton("Orthographic", &newProjectionType, 1);

			if (newProjectionType == 0)
			{
				float fovy = scene->GetActiveCamera().GetFovy();
				float zNear = scene->GetActiveCamera().GetNear();
				float zFar = scene->GetActiveCamera().GetFar();

				scene->GetActiveCamera().SwitchToPrespective();

				if (ImGui::SliderFloat("Fovy", &fovy, 0.0f, M_PI))
				{
					scene->GetActiveCamera().SetFovy(fovy);
				}

				if (ImGui::SliderFloat("Near", &zNear, 1.0f, 10.0f))
				{
					scene->GetActiveCamera().SetNear(zNear);
				}

				if (ImGui::SliderFloat("Far", &zFar, 1.0f, 10.0f))
				{
					scene->GetActiveCamera().SetFar(zFar);
				}
			}
			else if (newProjectionType == 1)
			{
				float height;
				float zNear;
				float zFar;

				scene->GetActiveCamera().SwitchToOrthographic();

				if (ImGui::SliderFloat("Height", &height, 0.0f, M_PI))
				{
					scene->GetActiveCamera().SetHeight(height);
				}

				if (ImGui::SliderFloat("Near", &zNear, 1.0f, 10.0f))
				{
					scene->GetActiveCamera().SetNear(zNear);
				}

				if (ImGui::SliderFloat("Far", &zFar, 1.0f, 10.0f))
				{
					scene->GetActiveCamera().SetFar(zFar);
				}
			}
		}

		if (ImGui::CollapsingHeader("Models"))
		{
			const char** items;
			std::vector<std::string> modelStrings;
			items = new const char*[scene->GetModelCount()];
			for (int i = 0; i < scene->GetModelCount(); i++)
			{
				std::ostringstream s;
				s << scene->GetModel(i)->GetModelName();
				std::string mystring = s.str();
				modelStrings.push_back(mystring);
			}

			for (int i = 0; i < scene->GetModelCount(); i++)
			{
				items[i] = modelStrings[i].c_str();
			}

			int currentModelIndex = scene->GetActiveModelIndex();
			ImGui::Combo("Active Model", &currentModelIndex, items, scene->GetModelCount());

			if (currentModelIndex != scene->GetActiveModelIndex())
			{
				scene->SetActiveModelIndex(currentModelIndex);
			}

			delete items;

			glm::vec3 modelColor = scene->GetActiveModel()->GetColor();
			if (ImGui::ColorEdit3("Model Color", (float*)&modelColor))
			{
				scene->GetActiveModel()->SetColor(modelColor);
			}
			if (scene->GetModelCount() > 0)
			{
				ImGui::SliderFloat("KA", (float*)(&scene->GetActiveModel()->KA), 0.0f, 1.0f);
				ImGui::SliderFloat("KD", (float*)(&scene->GetActiveModel()->KD), 0.0f, 1.0f);
				ImGui::SliderFloat("KS", (float*)(&scene->GetActiveModel()->KS), 0.0f, 1.0f);
				//				ImGui::InputInt("Specular Shinness", (int*)(&scene->GetActiveModel()->shine));
			}

			float scale = 1.0f;
			if (ImGui::SliderFloat("Scale", &scale, 0.999f, 1.001f))
			{

				scene->GetActiveModel()->ScaleModel(scale);
			}

			float rotx = 0.0f;
			if (ImGui::SliderFloat("Rotate X", &rotx, -0.01f, 0.01f))
			{
				scene->GetActiveModel()->RotateXModel(rotx);
			}

			float roty = 0.0f;
			if (ImGui::SliderFloat("Rotate Y", &roty, -0.01f, 0.01f))
			{
				scene->GetActiveModel()->RotateYModel(roty);
			}

			float rotz = 0.0f;
			if (ImGui::SliderFloat("Rotate Z", &rotz, -0.01f, 0.01f))
			{
				scene->GetActiveModel()->RotateZModel(rotz);
			}


			float translatex = 0.0f;
			if (ImGui::SliderFloat("translate x", &translatex, -0.01f, 0.01f))
			{
				scene->GetActiveModel()->TranslateModel(glm::vec3(translatex, 0.0f, 0.0f));
			}

			float translatey = 0.0f;
			if (ImGui::SliderFloat("translate y", &translatey, -0.01f, 0.01f))
			{
				scene->GetActiveModel()->TranslateModel(glm::vec3(0.0f, translatey, 0.0f));
			}

			float translatez = 0.0f;
			if (ImGui::SliderFloat("translate z", &translatez, -0.01f, 0.01f))
			{
				scene->GetActiveModel()->TranslateModel(glm::vec3(0.0f, 0.0f, translatez));
			}


			std::shared_ptr<MeshModel> meshModel = std::dynamic_pointer_cast<MeshModel>(scene->GetActiveModel());
			if (meshModel)
			{
				//glm::vec4 normalColor = meshModel->GetNormalModel().GetColor();
				//if (ImGui::ColorEdit3("Normal Color", (float*)&normalColor))
				//{
				//	meshModel->GetNormalModel().SetColor(normalColor);
				//}
			}
		}

		if (ImGui::CollapsingHeader("Lights"))
		{
			const char** items;
			std::vector<std::string> lightStrings;
			items = new const char*[scene->GetLightCount()];
			for (int i = 0; i < scene->GetLightCount(); i++)
			{
				std::ostringstream s;
				s << "Light " << i;
				std::string mystring = s.str();
				lightStrings.push_back(mystring);
			}

			for (int i = 0; i < scene->GetLightCount(); i++)
			{
				items[i] = lightStrings[i].c_str();
			}


			int light = activeLight;
			ImGui::Combo("Active Light", &light, items, scene->GetLightCount());


			if (light != activeLight)
				activeLight = light;

			std::shared_ptr<PointLight> l = scene->GetLight(activeLight);



			glm::vec3 lightColor = l->GetColor();
			if (ImGui::ColorEdit3("Light Color", (float*)&lightColor))
			{
				l->SetColor(lightColor);
			}
			glm::vec3 lightpos = l->position;
			if (ImGui::InputFloat3("Light position", (float*)&lightpos))
			{
				l->position = lightpos;
			}

			if (l->OnOff) {
				if (ImGui::Button("ON"))
					l->OnOff = false;
			}
			else
			{
				if (ImGui::Button("OFF"))
					l->OnOff = true;

			}

			delete items;

		}


		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "CTRL+O"))
				{
					nfdchar_t *outPath = NULL;
					nfdresult_t result = NFD_OpenDialog("obj;png,jpg", NULL, &outPath);
					if (result == NFD_OKAY) {
						scene->AddModel(Utils::LoadMeshModel(outPath));
						free(outPath);
					}
					else if (result == NFD_CANCEL) {
					}
					else {
					}

				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}
