#include "Flappy.h"

#include <vector>
#include <iostream>
#include <time.h>

#include <Core/Engine.h>
#include "Transform2D.h"

using namespace std;

Flappy::Flappy()
{
}

Flappy::~Flappy()
{
}

void Flappy::Init()
{
	// initierea camerei
	resolution = window->GetResolution();
	auto camera = GetSceneCamera();
	camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 200);
	camera->transform->SetWorldPosition(glm::vec3(0, 0, 50));
	camera->transform->SetWorldRotation(glm::vec3(0, 0, 0));
	camera->Update();
	GetCameraInput()->SetActive(false);

	// seed pentru randomizarile din cadrul codului
	srand(time(NULL));

	spawn_pointX = resolution.x + 50.0f; // tevile vor aparea la 50 de unitati in dreapta ecranului

	flappy_posX = resolution.x / 5; // pozitia pasarii pe OX (constanta)
	flappy_posY = resolution.y / 2; // pozitia pasarii pe OY (variabila)
	speedY = 0; // viteza cu care urca (+) sau coboara (-) pasarea, initial 0 (nu urca si nu coboara)
	g_acceleration = 9.8f * 200; // acceleratia gravitationala (care afecteaza liniar viteza)
	g_inclination = 2 * M_PI; // cu cat se inclina pasarea intr-o secunda in timpul caderii

	ResetGroundVector(); // initializam vectorul ce contine pozitiile dreptunghiurilor ce formeaza pamantul
	current_animation = 0.0f; // animatia aripilor la care ma aflu acum

	// initializarea celor 3 animatii ale aripilor
	animation_vector.push_back("flappy_wing_anim1");
	animation_vector.push_back("flappy_wing_anim2");
	animation_vector.push_back("flappy_wing_anim3");

	{// initializarea texturilor puse peste diverse primitive
		
		Texture2D* texture = new Texture2D();
		texture->Load2D("Resources/Textures/flappy.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_body"] = texture;
	
		Texture2D* texture_eye = new Texture2D();
		texture_eye->Load2D("Resources/Textures/flappy_eye.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_eye"] = texture_eye;
		
		Texture2D* texture_beak = new Texture2D();
		texture_beak->Load2D("Resources/Textures/flappy_beak.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_beak"] = texture_beak;
		

		Texture2D* texture_up = new Texture2D();
		texture_up->Load2D("Resources/Textures/upper_pipe.png", GL_CLAMP_TO_BORDER);
		mapTextures["upper_pipe"] = texture_up;

		Texture2D* texture_low = new Texture2D();
		texture_low->Load2D("Resources/Textures/lower_pipe.png", GL_CLAMP_TO_BORDER);
		mapTextures["lower_pipe"] = texture_low;
		
		Texture2D* texture_ground = new Texture2D();
		texture_ground->Load2D("Resources/Textures/flappy_ground.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_ground"] = texture_ground;
		
		Texture2D* texture_wing_anim1 = new Texture2D();
		texture_wing_anim1->Load2D("Resources/Textures/flappy_wing1.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_wing_anim1"] = texture_wing_anim1;

		Texture2D* texture_wing_anim2 = new Texture2D();
		texture_wing_anim2->Load2D("Resources/Textures/flappy_wing2.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_wing_anim2"] = texture_wing_anim2;

		Texture2D* texture_wing_anim3 = new Texture2D();
		texture_wing_anim3->Load2D("Resources/Textures/flappy_wing3.png", GL_CLAMP_TO_BORDER);
		mapTextures["flappy_wing_anim3"] = texture_wing_anim3;
		
	}

	{// initializarea primitivelor ce formeaza pasarea
		// vertecsi
		vector<glm::vec3> body_vertices
		{
			glm::vec3(25.0f,   25.0f, 0.0f),	// Top Right
			glm::vec3(25.0f,  -25.0f, 0.0f),	// Bottom Right
			glm::vec3(-25.0f, -25.0f, 0.0f),	// Bottom Left
			glm::vec3(-25.0f,  25.0f, 0.0f),	// Top Left
		};

		vector<glm::vec3> eye_vertices
		{
			glm::vec3(25.0f, 25.0f, 1.0f),	// Top Right
			glm::vec3(25.0f, 0.0f,  1.0f),	// Bottom Right
			glm::vec3(0.0f,  0.0f,  1.0f),	// Bottom Left
			glm::vec3(0.0f,  25.0f, 1.0f),	// Top Left
		};

		vector<glm::vec3> beak_vertices
		{
			glm::vec3(25.0f,   25.0f, 1.1f),	// Top Right
			glm::vec3(25.0f,  -25.0f, 1.1f),	// Bottom Right
			glm::vec3(-25.0f, -25.0f, 1.1f),	// Bottom Left
			glm::vec3(-25.0f,  25.0f, 1.1f),	// Top Left
		};

		vector<glm::vec3> normals
		{
			glm::vec3(0, 1, 1),
			glm::vec3(1, 0, 1),
			glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0)
		};

		// coordonatele texturilor
		vector<glm::vec2> textureCoords
		{
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(0.0f, 0.0f)
		};

		// indicii pentru formarea triunghiurilor
		vector<unsigned short> indices =
		{
			0, 1, 3,
			1, 2, 3
		};

		// intializam meshurile
		Mesh* mesh_body = new Mesh("flappy_body");
		mesh_body->InitFromData(body_vertices, normals, textureCoords, indices);
		meshes[mesh_body->GetMeshID()] = mesh_body;

		Mesh* mesh_eye = new Mesh("flappy_eye");
		mesh_eye->InitFromData(eye_vertices, normals, textureCoords, indices);
		meshes[mesh_eye->GetMeshID()] = mesh_eye;

		Mesh* mesh_beak = new Mesh("flappy_beak");
		mesh_beak->InitFromData(beak_vertices, normals, textureCoords, indices);
		meshes[mesh_beak->GetMeshID()] = mesh_beak;
	}

	{// initializarea primitivelor pentru tevi si pamant
		vector<glm::vec3> pipe_vertices
		{
			glm::vec3(50.0f,   resolution.y / 3, -1.0f),	// Top Right
			glm::vec3(50.0f,  -resolution.y / 3, -1.0f),	// Bottom Right
			glm::vec3(-50.0f, -resolution.y / 3, -1.0f),	// Bottom Left
			glm::vec3(-50.0f,  resolution.y / 3, -1.0f),	// Top Left
		};

		vector<glm::vec3> ground_vertices
		{
			glm::vec3(resolution.x / 8,   50.0f, 4.0f),	// Top Right
			glm::vec3(resolution.x / 8,  0.0f, 4.0f),	// Bottom Right
			glm::vec3(-resolution.x / 8, 0.0f, 4.0f),	// Bottom Left
			glm::vec3(-resolution.x / 8,  50.0f, 4.0f),	// Top Left
		};

		vector<glm::vec3> normals
		{
			glm::vec3(0, 1, 1),
			glm::vec3(1, 0, 1),
			glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0)
		};

		vector<glm::vec2> textureCoords
		{
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(0.0f, 0.0f)
		};

		vector<unsigned short> indices =
		{
			0, 1, 3,
			1, 2, 3
		};

		Mesh* mesh_pipe = new Mesh("pipe");
		mesh_pipe->InitFromData(pipe_vertices, normals, textureCoords, indices);
		meshes[mesh_pipe->GetMeshID()] = mesh_pipe;

		Mesh* mesh_ground = new Mesh("ground");
		mesh_ground->InitFromData(ground_vertices, normals, textureCoords, indices);
		meshes[mesh_ground->GetMeshID()] = mesh_ground;
	}

	{// initializarea shaderului folosit (creat pe baza laboratorului 9)
		Shader* shader = new Shader("FlappyShader");
		shader->AddShader("Source/FlappySource/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/FlappySource/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}


}

void Flappy::FrameStart()
{
	//stergerea ecranului si setarea backgroundului la un albastru deschis si setarea portii de vizualizare
	glClearColor(0.1f, 0.6f, 0.75f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	resolution = window->GetResolution();
	spawn_pointX = resolution.x + 50.0f;
	glViewport(0, 0, resolution.x, resolution.y);
}

/* pozitionarea si randarea primitivelor se va face astfel:
 *
 * setam matricea de modelare a corpului la rotatia curenta si la pozitia curenta a pasarii
 * ochiul va avea aceeasi matrice datorita modului in care a fost definit meshul (corpul are centrul in 0,0 si dimensiune de 50 x 50,
 * in timp ce ochiul are coltul de stanga jos in 0,0 si dimensiune de 25 x 25, ceea ce il pozitioneaza perfect relativ cu corpul)
 *
 * pentru ca ni s-a cerut sa facem rotatii si scalari folosind cel putin 3 primitive am ales sa nu mai folosesc acest workaround in
 * continuare pentru cioc si aripa, asa ca ciocul este scalat si pozitionat relativ la corp, dupa care rotatia si translatia sunt
 * aceleasi ca la corp, deci am folosit in continuare aceeasi matrice, precum in laboratorul 4
 *
 * intrucat matricea a fost modificata la cioc, ca sa nu inversez operatiile deja facute, pentru aripa am resetat matricea de modelare
 * si am aplicat aceleasi 4 operatii ca la cioc: Scale, Translate relativ la corp, Rotate-ul pasarii si translatia in punctul in care
 * se afla pasarea
 *
 * pentru randare am folosit o functie ce va fi explicata mai jos
 */
void Flappy::Update(float deltaTimeSeconds)
{
	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(flappy_posX, flappy_posY);
	modelMatrix *= Transform2D::Rotate(flappy_tilt);
	RenderSimpleMesh(meshes["flappy_body"], shaders["FlappyShader"], modelMatrix, mapTextures["flappy_body"]);
	RenderSimpleMesh(meshes["flappy_eye"], shaders["FlappyShader"], modelMatrix, mapTextures["flappy_eye"]);

	modelMatrix *= Transform2D::Translate(beak_offsetX, beak_offsetY);
	modelMatrix *= Transform2D::Scale(beak_scale, beak_scale);
	RenderSimpleMesh(meshes["flappy_beak"], shaders["FlappyShader"], modelMatrix, mapTextures["flappy_beak"]);

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(flappy_posX, flappy_posY);
	modelMatrix *= Transform2D::Rotate(flappy_tilt);
	modelMatrix *= Transform2D::Translate(wing_offsetX, wing_offsetY);
	modelMatrix *= Transform2D::Scale(wing_scale, wing_scale);
	RenderSimpleMesh(meshes["flappy_beak"], shaders["FlappyShader"], modelMatrix, mapTextures[animation_vector[floor(current_animation)]]);

	// daca jocul nu s-a terminat continua animarea pasarii
	if (!game_over) {
		current_animation += 9 * deltaTimeSeconds; // 9 animatii pe secunda (3 bataii de aripa)
		if (current_animation >= 2.9f)
			current_animation = 0.0f;
	}

	// daca pasarea atinge pamantul, jocul se termina
	if (flappy_posY - 25.0f <= 50.0f) {
		game_over = true;
		game_started = false;
	}

	/* daca jocul a inceput, pasarea se misca pe OY folosind o functie clasica din fizica mecanica,
	 * pozitia se modifica cu viteza, viteza se modifica cu acceleratia gravitationala
	 * inclinarea este de maxim -90 grade, si are o viteza de 180 grade pe secunda (g_inclination).
	 * deci pasarea ajunge la inclinatia maxima in 0.75 secunde, cu un delay initial determinat de
	 * viteza curenta a pasarii
	 */
	if (game_started) {
		flappy_posY += speedY * deltaTimeSeconds;
		speedY -= g_acceleration * deltaTimeSeconds;
		if (tilting_up) {
			if (flappy_tilt >= M_PI / 8) {
				tilting_up = false;
			}
			else {
				flappy_tilt += 2 * g_inclination * deltaTimeSeconds;
			}
		}
		else if (speedY < -500.0f && flappy_tilt >= -M_PI / 2) {
			flappy_tilt -= g_inclination * deltaTimeSeconds;
		}
	}

	GroundManager(deltaTimeSeconds);
	PipeManager(deltaTimeSeconds);
}

/* functie ce se ocupa de comportamentul primitivelor ce reprezinta pamantul, la un nivel minimalist
 *
 * practic, pamantul este format din 5 bucati (4 pe ecran, 1 in dreapta ecranului initial), a caror pozitie pe OY este fixata,
 * iar cea pe OX este salvata intr-un vector.
 *
 * intr-o iteratie prin acel vector se intampla urmatoarele:
 * - bucata de la pozitia curenta este randata
 * - daca jocul nu s-a terminat si daca am trecut de primul frame (dintr-un oarecare motiv deltaTimeSeconds se comporta ciudat)
 * atunci muta pozitia curenta cu viteza pipeurilor catre stanga pentru fiecare pozitie salvata in vector
 * (pipe_speed e constanta intr-o iteratie, deltaTimeSeconds la fel, deci se vor misca uniform)
 * - daca o bucata de pamanat iese din ecran (are pozitia mai mica ca 0 - ground_width/2, care este 1/8 * resolution.x)
 * atunci ii modific pozitia la pozitia cea mai din dreapta + gorund_width(1/4 * resolution.x)
 *
 * facand aceasta "rotatie" a bucatilor de pamant, pozitia ce mai din dreapta va fi pozitionata la pozitia curenta - 1,
 * deoarece pozitia curenta este cea mai din stanga (stim sigur ca sunt in ordine pentru ca isi pastreaza distantele intre ele)
 */
void Flappy::GroundManager(float deltaTimeSeconds)
{
	for (auto it = ground_vec.begin(); it != ground_vec.end(); it++) {
		modelMatrix = glm::mat3(1);
		modelMatrix *= Transform2D::Translate(*it, 0.0f);
		RenderSimpleMesh(meshes["ground"], shaders["FlappyShader"], modelMatrix, mapTextures["flappy_ground"]);

		if (deltaTimeSeconds < 1.0f && !game_over) {
			*it -= pipe_speed * deltaTimeSeconds;
			if (*it <= resolution.x * -0.125f) {
				if (it == ground_vec.begin()) {
					*it = *(ground_vec.end() - 1) + resolution.x * 0.25f - pipe_speed * deltaTimeSeconds;
				}
				else {
					*it = *(it - 1) + resolution.x * 0.25f;
				}
			}
		}
	}
}

// functie ce se ocupa de comportamentul tevilor ce apar din dreapta ecranului
void Flappy::PipeManager(float deltaTimeSeconds)
{
	float collision_testX, collision_testY_lower, collision_testY_upper;
	float collision_dist_upper, collision_dist_lower;

	// setez parametrii in functie de dificultate
	switch (current_difficulty)
	{
		int sliding_coin_toss;

	case EASY:
		pipe_speed = 125.0f + score;
		spawn_sliding_pipe = false;
		spawn_frequency = 0.8f;
		break;
	case MEDIUM:
		pipe_speed = 150.0f + score;
		sliding_coin_toss = rand() % 4;
		if (sliding_coin_toss == 0)
			spawn_sliding_pipe = true;
		else
			spawn_sliding_pipe = false;
		spawn_frequency = 0.85f;
		break;
	case HARD:
		pipe_speed = 175.0f + score;
		sliding_coin_toss = rand() % 2;
		if (sliding_coin_toss == 0)
			spawn_sliding_pipe = true;
		else
			spawn_sliding_pipe = false;
		spawn_frequency = 0.9f;
		break;
	default:
		break;
	}

	// introduc o pereche noua de tevi daca este cazul
	if (should_spawn_pipe) {
		pipe_queue.push(generateNewPipePair(spawn_sliding_pipe));
	}

	should_spawn_pipe = true; // presupun ca trebuie sa apara o pereche noua la frameul urmator
	while (!pipe_queue.empty()) {
		pipe_pair p = pipe_queue.front();
		pipe_queue.pop();

		// daca exista o pereche care nu a depasit distanta minima ce trebuie sa se afle intre tevi, presupunerea e falsa
		if (p.center_posX > resolution.x* spawn_frequency) {
			should_spawn_pipe = false;
		}

		modelMatrix = glm::mat3(1);
		modelMatrix *= Transform2D::Translate(p.center_posX, p.pos_upperY);
		RenderSimpleMesh(meshes["pipe"], shaders["FlappyShader"], modelMatrix, mapTextures["upper_pipe"]);

		modelMatrix = glm::mat3(1);
		modelMatrix *= Transform2D::Translate(p.center_posX, p.pos_lowerY);
		RenderSimpleMesh(meshes["pipe"], shaders["FlappyShader"], modelMatrix, mapTextures["lower_pipe"]);

		// daca jocul nu s-a terminat, tevile se misca catre stanga
		if (!game_over && game_started)
			p.center_posX -= pipe_speed * deltaTimeSeconds;

		// daca un pipe se afla in prima treime a ecranului, trebuie sa incep verificarea de coliziuni
		if (p.center_posX < resolution.x / 3) {
			collision_testX = flappy_posX;
			collision_testY_lower = flappy_posY;
			collision_testY_upper = flappy_posY;

			// coordonata X a punctului de pe marginea tevilor cel mai apropiat de pasare
			if (flappy_posX <= p.center_posX - 50.0f) {
				collision_testX = p.center_posX - 50.0f;
			}
			else if (flappy_posX >= p.center_posX + 50.0f) {
				collision_testX = p.center_posX + 50.0f;
			}

			// coordonata Y a punctului de pe marginea tevilor cel mai apropiat de pasare pentru fiecare teava
			if (flappy_posY >= p.center_posY + p.distance / 2 + p.currentSlideDistance) {
				collision_testY_lower = p.center_posY - p.distance / 2 - p.currentSlideDistance;
			}
			else if (flappy_posY <= p.center_posY - p.distance / 2 - p.currentSlideDistance) {
				collision_testY_upper = p.center_posY + p.distance / 2 + p.currentSlideDistance;
			}
			else {
				collision_testY_lower = p.center_posY - p.distance / 2 - p.currentSlideDistance;
				collision_testY_upper = p.center_posY + p.distance / 2 + p.currentSlideDistance;
			}

			// calculam distanta de la centrul pasarii pana la cele doua puncte gasite pe marginea tevilor de sus, respectiv jos
			collision_testX = flappy_posX - collision_testX;
			collision_testY_lower = flappy_posY - collision_testY_lower;
			collision_testY_upper = flappy_posY - collision_testY_upper;
			collision_dist_lower = sqrtf(collision_testX * collision_testX + collision_testY_lower * collision_testY_lower);
			collision_dist_upper = sqrtf(collision_testX * collision_testX + collision_testY_upper * collision_testY_upper);

			// daca distanta este mai mica decat raza cercului, avem coliziune
			if (collision_dist_lower < 25.0f || collision_dist_upper < 25.0f) {
				game_over = true;
				game_started = false;
			}
		}

		// printez scorul cand pasarea trece de centrul unei perechi de tevi
		if (!game_over && !p.passed && flappy_posX > p.center_posX) {
			std::printf("%d\n", ++score);
			p.passed = true;
			if (score >= 25) current_difficulty = MEDIUM;
			if (score >= 50) current_difficulty = HARD;
		}

		// modific distanta dintre tevile glisante, daca jocul nu s-a terminat
		if (!game_over && p.shouldSlide) {
			if (p.isClosing) {
				p.currentSlideDistance += pipe_speed * deltaTimeSeconds;
				if (p.currentSlideDistance * 2 >= p.maxSlideDistance) {
					p.isClosing = false;
				}
				p.pos_upperY = p.center_posY + p.distance / 2 + resolution.y / 3 + p.currentSlideDistance;
				p.pos_lowerY = p.center_posY - p.distance / 2 - resolution.y / 3 - p.currentSlideDistance;
			}
			else {
				p.currentSlideDistance -= pipe_speed * deltaTimeSeconds;
				if (p.currentSlideDistance * 2 <= -p.maxSlideDistance) {
					p.isClosing = true;
				}
				p.pos_upperY = p.center_posY + p.distance / 2 + resolution.y / 3 + p.currentSlideDistance;
				p.pos_lowerY = p.center_posY - p.distance / 2 - resolution.y / 3 - p.currentSlideDistance;
			}
		}

		// salvez tevile intr-o structura auxiliara daca acestea mai exista pe ecran
		if (p.center_posX >= -50.0f)
			pipe_queue_save.push(p);
	}

	// pun tevile inapoi in structura initiala pentru a pastra continuitatea
	while (!pipe_queue_save.empty()) {
		pipe_pair p = pipe_queue_save.front();
		pipe_queue_save.pop();
		pipe_queue.push(p);
	}
}

void Flappy::FrameEnd()
{

}

void Flappy::OnInputUpdate(float deltaTime, int mods)
{

}

void Flappy::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE) {
		// daca s-a terminat jocul, space porneste un joc nou, altfel, ofera un impuls pasarii
		if (game_over) {
			ResetGroundVector();
			current_difficulty = EASY;
			flappy_posX = resolution.x / 5;
			flappy_posY = resolution.y / 2;
			speedY = 0.0f;
			flappy_tilt = 0.0f;
			score = 0;

			while (!pipe_queue.empty())
				pipe_queue.pop();

			should_spawn_pipe = true;
			game_started = false;
			game_over = false;
		}
		else {
			game_started = true;
			speedY = 500.0f;
			//flappy_tilt = M_PI / 8;
			current_animation = 0.0f;
			tilting_up = true;
		}
	}
}

void Flappy::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Flappy::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Flappy::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Flappy::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Flappy::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Flappy::OnWindowResize(int width, int height)
{
}

/* functia de render este o combinatie intre functiile folosite la laboratorul 3 si cea de la laboratorul 9
 * de la laboratorul 3 am pastrat transformarea modelMatrix din mat3 in mat4, ignorand transformarile aparute pe OZ,
 * in timp ce din laboratorul 9 am pastrat legarea texturilor
*/
void Flappy::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat3& modelMatrix, Texture2D* texture1)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// folosim shaderul specificat pentru randare
	glUseProgram(shader->program);

	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glm::mat3 mm = modelMatrix;
	glm::mat4 model = glm::mat4(
		mm[0][0], mm[0][1], mm[0][2], 0.f,
		mm[1][0], mm[1][1], mm[1][2], 0.f,
		0.f, 0.f, mm[2][2], 0.f,
		mm[2][0], mm[2][1], 0.f, 1.f);

	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	if (texture1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1->GetTextureID());
		glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);
	}

	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

// generaza un obiect pipe_pair, avand valori aleatoare intre anumite valori de minim si maxim
pipe_pair Flappy::generateNewPipePair(bool shouldSlide) {
	pipe_pair new_pipe_pair;
	new_pipe_pair.distance = rand() % (int)(resolution.y * 0.125f) + (resolution.y * 0.25f);
	new_pipe_pair.center_posY = rand() % (int)(resolution.y * 0.5f) + (resolution.y * 0.25f) + 50.0f;
	new_pipe_pair.center_posX = spawn_pointX;
	new_pipe_pair.pos_upperY = new_pipe_pair.center_posY + new_pipe_pair.distance / 2 + resolution.y / 3;
	new_pipe_pair.pos_lowerY = new_pipe_pair.center_posY - new_pipe_pair.distance / 2 - resolution.y / 3;
	new_pipe_pair.passed = false;
	new_pipe_pair.shouldSlide = shouldSlide;
	new_pipe_pair.maxSlideDistance = rand() % (int)(resolution.y * 0.05f) + (resolution.y * 0.05f);
	new_pipe_pair.isClosing = true;
	new_pipe_pair.currentSlideDistance = 0.0f;
	return new_pipe_pair;
}

// resetez vectorul ce reprezinta pozitiile celor 5 primitive ce formeaza pamantul
void Flappy::ResetGroundVector() {
	ground_vec.clear();
	ground_vec.push_back(resolution.x * 0.125f); // 1/8
	ground_vec.push_back(resolution.x * 0.375f); // 3/8
	ground_vec.push_back(resolution.x * 0.625f); // 5/8
	ground_vec.push_back(resolution.x * 0.875f); // 7/8
	ground_vec.push_back(resolution.x * 1.125f); // 9/8
}