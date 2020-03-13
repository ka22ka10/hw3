#include "Renderer.h"
#include "MeshModel.h"
#include "ShaderProgram.h"
#include <imgui/imgui.h>
#include <vector>
#include <memory>
#include <algorithm>

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::Render(const std::shared_ptr<Scene>& scene)
{
	int cameraCount = scene->GetCameraCount();
	if (cameraCount > 0)
	{
		glm::vec3 lightPos[5];
		glm::vec3 lightCol[5];
		int modelCount = scene->GetModelCount();
		const Camera& camera = scene->GetActiveCamera();
		int i, j;
		for (i = 0; i < scene->GetLightCount(); i++)
		{
			if (scene->GetLight(i)->OnOff == false)
			{
				lightPos[i] = glm::vec3(0.0f);
				lightCol[i] = glm::vec3(0.0f);
			}
			else
			{
				lightPos[i] = scene->GetLight(i)->GetPosition();
				lightCol[i] = scene->GetLight(i)->GetColor();
			}
		}
		for (j = i; j < 5; j++)
		{
			lightPos[j] = glm::vec3(0.0f);
			lightCol[j] = glm::vec3(0.0f);
		}
		glm::vec3 eye = scene->GetActiveCamera().GetEye();

		for (int currentModelIndex = 0; currentModelIndex < modelCount; currentModelIndex++)
		{
			std::shared_ptr<MeshModel> currentModel = scene->GetModel(currentModelIndex);
			// Activate the 'colorShader' program (vertex and fragment shaders)
			colorShader.use();


			colorShader.setUniform("model", currentModel->GetWorldTransformation() * currentModel->GetModelTransformation());
			colorShader.setUniform("view", camera.GetViewTransformation());
			colorShader.setUniform("projection", camera.GetProjectionTransformation());
			//			colorShader.setUniform("material.textureMap", 0);
			colorShader.setUniform("lightPos", lightPos);
			colorShader.setUniform("lightCol", lightCol);
			colorShader.setUniform("eye", eye);
			//			colorShader.setUniform("material.KA", 0.5f);
			colorShader.setUniform("material.KA", currentModel->KA);
			//			colorShader.setUniform("material.KD", 0.5f);
			colorShader.setUniform("material.KD", currentModel->KD);
			//			colorShader.setUniform("material.KS", 0.5f);
			colorShader.setUniform("material.KS", currentModel->KS);
			//			colorShader.setUniform("material.shine", 3);
			colorShader.setUniform("material.shine", currentModel->shine);
			//			colorShader.setUniform("material.color", glm::vec3(0.3f,0.0f,0.0f));
			colorShader.setUniform("material.color", currentModel->GetColor());

			// Drag our model's faces (triangles) in fill mode
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glBindVertexArray(currentModel->GetVAO());
			glDrawArrays(GL_TRIANGLES, 0, currentModel->GetModelVertices().size());
			glBindVertexArray(0);
		}
		for (i = 0; i < scene->GetLightCount(); i++)
		{
			if (scene->GetLight(i)->OnOff == false)	continue;
			if (glm::vec3(scene->GetLight(i)->GetColor()) == glm::vec3(0.0f)) continue;
			colorShader.use();
			glm::vec3 currLightPos = scene->GetLight(i)->GetPosition();
			// Set the uniform variables
			scene->sphere->TranslateModel(currLightPos);
			colorShader.setUniform("model", scene->sphere->GetModelTransformation());
			colorShader.setUniform("view", camera.GetViewTransformation());
			colorShader.setUniform("projection", camera.GetProjectionTransformation());
			colorShader.setUniform("lightPos", glm::vec3(0.0f));
			colorShader.setUniform("lightCol", glm::vec3(0.0f));
			colorShader.setUniform("eye", eye);
			colorShader.setUniform("material.KA", 1.0f);
			colorShader.setUniform("material.KD", 0.0f);
			colorShader.setUniform("material.KS", 0.0f);
			colorShader.setUniform("material.shine", 3);
			colorShader.setUniform("material.color", glm::vec3(scene->GetLight(i)->GetColor()));

			// Drag our model's faces (triangles) in fill mode
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glBindVertexArray(scene->sphere->GetVAO());
			glDrawArrays(GL_TRIANGLES, 0, scene->sphere->GetModelVertices().size());
			glBindVertexArray(0);

			glm::vec3 minusPosition = glm::vec3(-currLightPos.x, -currLightPos.y, -currLightPos.z);
			scene->sphere->TranslateModel(minusPosition);

		}

	}
}

void Renderer::LoadShaders()
{
	colorShader.loadShaders("vshader_color.glsl", "fshader_color.glsl");
}

void Renderer::LoadTextures()
{
	if (!texture1.loadTexture("bin\\Debug\\crate.jpg", true))
	{
		texture1.loadTexture("bin\\Release\\crate.jpg", true);
	}
}