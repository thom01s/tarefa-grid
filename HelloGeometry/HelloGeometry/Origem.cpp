/* Hello Geometry
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Computação Gráfica - Escola Politécnica - Unisinos
 * Versão inicial: 01/06/2022
 * Última atualização em 08/06/2022
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h" //tivemos modificação na classe para suportar a leitura e compilação do geometry shader

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipo da função de callback de mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


// Protótipos das funções
GLuint setupSeedPoints(int rows, int cols, Shader &shader);

void processInput(GLFWwindow* window);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

//Variáveis globais para o controle da câmera
glm::vec3 cameraPos, cameraFront, cameraUp;
bool firstMouse = true;
float lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0; //para calcular o quanto que o mouse deslocou
float yaw = -90.0, pitch = 0.0; //rotação em x e y

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Geometria!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Dasabilitando o cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	//Inicialização dos parâmetros da câmera
	cameraPos = glm::vec3(0.0, 0.0, 3.0); //ponto
	cameraFront = glm::vec3(0.0, 0.0, -1.0); //vetor
	cameraUp = glm::vec3(0.0, 1.0, 0.0);

	// Compilando e buildando o programa de shader
	Shader shader = Shader("../shaders/grid.vs", "../shaders/grid.fs", "../shaders/grid.gs");

	// Ativando o shader para uso 
	glUseProgram(shader.ID);

	// Gerando um buffer que contenha os pontos que gerarão a grid
	int rows = 3, cols = 3;
	int gridSize = rows * cols;
	GLuint VAO = setupSeedPoints(rows, cols, shader);

	// MATRIZ DE VIEW (olhando de frente)
	glm::mat4 view = glm::mat4(1); //matriz identidade;
	view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
	shader.setMat4("view", glm::value_ptr(view));

	//MATRIZ DE PROJEÇÃO - tipo de projeção: perspectiva
	glm::mat4 projection = glm::mat4(1); //matriz identidade;
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));

	//MATRIZ DE TRANSFORMAÇÃO - nesse caso, apenas a matriz identidade
	glm::mat4 model = glm::mat4(1);
	shader.setMat4("model", glm::value_ptr(model));

	// Habilitando teste de profundidade;
	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		processInput(window);

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Setando largura da linha e tamanho do ponto (GL_LINE e GL_POINTS)
		glLineWidth(10);
		glPointSize(20);

		glm::mat4 view = glm::mat4(1); //matriz identidade;
		//Aqui atualizamos o view com a posição e orientação da câmera atualizados
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		shader.setMat4("view", glm::value_ptr(view));
		
		// Chamada de desenho - drawcall
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, gridSize);
		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);

	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//Na primeira vez o deslocamento será zero
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	//Calculando o deslocamento do Mouse
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	//Amortizando, deixando o movimento mais suave
	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	//Atualizando os ângulos de yaw e pitch
	yaw += xoffset;
	pitch += yoffset;

	//Colocando limite no ângulo de pitch (nao é a menina do exorcista)
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	//Finalmente, usando os valiosos conhecimentos de Trigonometria para
	//atualizar o vetor front (Viva Pitágoras!)
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

}

void processInput(GLFWwindow* window)
{
	glfwPollEvents(); //verificar as callbacks de input

	float cameraSpeed = 0.05;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		//Movimenta para frente
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		//Movimenta para trás
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		//Movimenta para esquerda - precisa calcular o vetor Right
		//Calcula-se pelo produto vetorial do Up e do Front
		//O vetor Right precisa ser unitário
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		//Movimenta para direita
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
}

GLuint setupSeedPoints(int rows, int cols, Shader &shader)
{
	// Largura e altura de cada célula da grid, baseado numa janela de mundo normalizada entre -1 e 1 em x e y (1.0 - (-1.0) = 2.0)
	float tileWidth = 2.0 / (float)cols;
	float tileHeight = 2.0 / (float)rows;

	// Mandando para o shader, para poder gerar os quadrados lá dentro
	shader.setVec2("offsets", tileWidth, tileHeight);

	// Tamanho total da grid
	int gridSize = rows * cols;

	// Tamanho total do buffer
	int bufferSize = rows * cols * 6;
	// Alocando o array com os dados 
	float* points = new float[bufferSize];

	//Variáveis de apoio para a criação dos pontos
	float x, y, z = 0.0f;
	float x0 = -1.0f;
	float y0 = -1.0f;

	int k = -1; //indice do array
	bool black = true;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			x = x0 + tileWidth * j;
			y = y0 + tileHeight * i;
			points[++k] = x;
			points[++k] = y;
			points[++k] = z;
			if (black)
			{
				points[++k] = 0.0f;
				points[++k] = 0.0f;
				points[++k] = 0.0f;
				black = !black;
			}
			else
			{
				points[++k] = 1.0f;
				points[++k] = 0.0f;
				points[++k] = 0.0f;
				black = !black;
			}
			
		}
	}

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(GLfloat), points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	//////////////////////////////////
	//Um debugzinho básico
	k = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			cout << points[k] << " ";
			cout << points[k + 1] << " ";
			cout << points[k + 2] << " ";
			cout << points[k + 3] << " ";
			cout << points[k + 4] << " ";
			cout << points[k + 5] << " ";
			cout << "\t";
			k += 6;
		}
		cout << endl;
	}
	return VAO;
}