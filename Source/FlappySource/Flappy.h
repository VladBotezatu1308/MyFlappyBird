#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include <queue>

enum difficulty {EASY, MEDIUM, HARD};
/* struct ce reprezinta o pereche de tevi, cu toate informatiile
 * de care acestea au nevoie
 */
struct pipe_pair {
	float center_posX, center_posY; //pozitia XY a punctului din mijlocul celor doua tevi
	float distance; //distanta dintre cele doua tevi
	float pos_upperY, pos_lowerY; //pozitia Y (X-ul e acelasi cu centrul) al fiecarei tevi
	bool passed; //bool care verifica daca pasarea a trecut sau nu prin teava, pentru a stii cand se marcheaza un punct
	bool shouldSlide, isClosing; //ar trebui sa aiba animatie de deschidere/inchidere? Daca da, se inchide sau se deschide?
	float currentSlideDistance; //cat s-au deschis deja cele doua pipeuri
	float maxSlideDistance; //cat ar trebui sa fie deschiderea maxima
};

class Flappy : public SimpleScene
{
public:
	Flappy();
	~Flappy();

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;

	void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat3& modelMatrix, Texture2D* texture1 = NULL);

	pipe_pair generateNewPipePair(bool shouldSlide);
	void PipeManager(float deltaTimeSeconds);
	void GroundManager(float deltaTimeSeconds);
	void ResetGroundVector();


protected:
	glm::mat3 modelMatrix;
	glm::ivec2 resolution;
	const float beak_offsetX = 12.5f, beak_offsetY = -10.0f;
	const float wing_offsetX = -12.5f, wing_offsetY = -5.0f;
	const float wing_scale = 0.5f;
	const float beak_scale = 0.5f;
	float flappy_posX, flappy_posY;
	float flappy_tilt = 0;
	bool tilting_up;
	float speedY, g_acceleration, g_inclination;
	int score = 0;
	difficulty current_difficulty = EASY;
	bool game_started = false, game_over = false;
	float spawn_pointX;
	bool spawn_sliding_pipe = false, should_spawn_pipe = true;
	float pipe_speed = 125.0f, spawn_frequency = 0.8f;
	std::queue<pipe_pair> pipe_queue;
	std::queue<pipe_pair> pipe_queue_save;
	std::unordered_map<std::string, Texture2D*> mapTextures;
	std::vector<float> ground_vec;
	std::vector<std::string> animation_vector;
	float current_animation;
};