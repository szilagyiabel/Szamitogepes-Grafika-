#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <glm/glm.hpp>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

static GLfloat s_vertices[] =
{
	-0.33f, 0.00f, 0.00f,
	 0.33f, 0.00f, 0.00f
};

std::vector<glm::vec3> kor;
std::vector<glm::vec3> green;
std::vector<glm::vec3> red;

#define	numVBOs 3
#define numVAOs 2

GLuint VBO[numVBOs];
GLuint VAO[numVAOs];

GLboolean keyboard[512] = { GL_FALSE };

GLuint renderingProgram;
GLuint renderingProgramKor;
GLuint XoffsetLocation;
GLuint Yoffsetation;

float x = 0.00f;
float y = 0.00f;
float linex = 0;
float liney = 0;
float increment = 0.005f;

bool xDir = true;
bool yDir = false;

bool checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

void printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}

/** Függvény, a shader fájlok sorainak beolvasásához. */
string readShaderSource(const char* filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";

	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint createShaderProgram() {

	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	/** Beolvassuk a shader fájlok tartalmát. */
	string vertShaderStr = readShaderSource("vertexShader.glsl");
	string fragShaderStr = readShaderSource("fragmentShader.glsl");

	/** Létrehozzuk a shader objektumainkat. */
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	/** A shader fájlok tartalmát eltároló string objektum szöveggé konvertálásás is elvégezzük. */
	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	/** Ekkor a betöltött kódot hozzárendelhetjük a shader objektumainkhoz. */
	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	/** Fordítsuk le ezen shader objektumhoz rendelt kódot. */
	glCompileShader(vShader);
	/** Hibakeresési lépések. Például sikeres volt-e a fordítás? Ha nem, mi volt az oka? */
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}

	/** A második shader objektumhoz rendelt kódunkat is lefordítjuk. */
	glCompileShader(fShader);
	/** Ismét hibakeresési lépések. Például sikeres volt-e a fordítás? Ha nem, mi volt az oka? */
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	/** Shader program objektum létrehozása: õ fogja össze a shadereket. Eltároljuk az ID értéket. */
	GLuint vfProgram = glCreateProgram();
	/** Csatoljuk a shadereket az elõzõ lépésben létrehozott objektumhoz. */
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	/** Végül a GLSL fordítónk ellenõrzi, hogy a csatolt shaderek kompatibilisek-e egymással. */
	glLinkProgram(vfProgram);
	/** Ha hiba lépett fel, nézzük meg mi volt ennek az oka. */
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	/** Ha minden rendben ment a linkelés során, az objektumok leválaszthatóak a programról. */
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	/** A kész program a visszatérési értékünk. */
	return vfProgram;
}

GLuint createShaderProgramKor() {

	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	/** Beolvassuk a shader fájlok tartalmát. */
	string vertShaderStr = readShaderSource("vertexShaderKor.glsl");
	string fragShaderStr = readShaderSource("fragmentShaderKor.glsl");

	/** Létrehozzuk a shader objektumainkat. */
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	/** A shader fájlok tartalmát eltároló string objektum szöveggé konvertálásás is elvégezzük. */
	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	/** Ekkor a betöltött kódot hozzárendelhetjük a shader objektumainkhoz. */
	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	/** Fordítsuk le ezen shader objektumhoz rendelt kódot. */
	glCompileShader(vShader);
	/** Hibakeresési lépések. Például sikeres volt-e a fordítás? Ha nem, mi volt az oka? */
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}

	/** A második shader objektumhoz rendelt kódunkat is lefordítjuk. */
	glCompileShader(fShader);
	/** Ismét hibakeresési lépések. Például sikeres volt-e a fordítás? Ha nem, mi volt az oka? */
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	/** Shader program objektum létrehozása: õ fogja össze a shadereket. Eltároljuk az ID értéket. */
	GLuint vfProgram = glCreateProgram();
	/** Csatoljuk a shadereket az elõzõ lépésben létrehozott objektumhoz. */
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	/** Végül a GLSL fordítónk ellenõrzi, hogy a csatolt shaderek kompatibilisek-e egymással. */
	glLinkProgram(vfProgram);
	/** Ha hiba lépett fel, nézzük meg mi volt ennek az oka. */
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	/** Ha minden rendben ment a linkelés során, az objektumok leválaszthatóak a programról. */
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	/** A kész program a visszatérési értékünk. */
	return vfProgram;
}

/** A jelenetünk utáni takarítás. */
void cleanUpScene()
{
	/** Töröljük a vertex array objektumokat. */
	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);

	/** Töröljük a shader programot. */
	glDeleteProgram(renderingProgram);
	glDeleteProgram(renderingProgramKor);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
		cleanUpScene();
	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;

	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		if (liney < 1.0f) liney += 0.02;
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		if (liney > -1.0f) liney -= 0.02;
	}

}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

}

void init(GLFWwindow* window) {
	/** A rajzoláshoz használt shader programok betöltése. */
	renderingProgram = createShaderProgram();
	renderingProgramKor = createShaderProgramKor();
	/** Vertex Array Objektum létrehozása. */
	glGenBuffers(numVBOs, VBO);
	glGenVertexArrays(numVAOs, VAO);

	                   
	const float r = 1.000 / 6.000;

	for (int i = 0; i <= 360; i++) 
	{
		float angleRad = (i * M_PI / 180);
		float x = r * cosf(angleRad);
		float y = r * sinf(angleRad);

		bool isEven = (i % 2 == 0);
		glm::vec3 newPoint = isEven ? glm::vec3(x, y, 0) : glm::vec3(0, 0, 0); 
		glm::vec3 newRed = isEven ? glm::vec3(0.0, 1.0, 0.0) : glm::vec3(1.0, 0.0, 0.0);
		glm::vec3 newGreen = isEven ? glm::vec3(1.0, 0.0, 0.0) : glm::vec3(0.0, 1.0, 0.0);

		kor.push_back(newPoint);
		red.push_back(newRed);
		green.push_back(newGreen);
	} 

	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

	glBufferData(GL_ARRAY_BUFFER, kor.size() * sizeof(glm::vec3), kor.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

	glBufferData(GL_ARRAY_BUFFER, green.size() * sizeof(glm::vec3), green.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertices), s_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

}

void display(GLFWwindow* window, double currentTime) {

	glClearColor(1.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);
	glBindVertexArray(VAO[1]);
	GLuint offset = glGetUniformLocation(renderingProgram, "offsetY");
	glProgramUniform1f(renderingProgram, offset, liney);
	glLineWidth(3);
	glDrawArrays(GL_LINES, 0, sizeof(s_vertices) / sizeof(s_vertices[0]));
	glBindVertexArray(0);


	glUseProgram(0);
	glUseProgram(renderingProgramKor);
	glBindVertexArray(VAO[0]);
	GLuint offset1 = glGetUniformLocation(renderingProgramKor, "offsetX");
	glProgramUniform1f(renderingProgramKor, offset1, x);
	GLuint offset2 = glGetUniformLocation(renderingProgramKor, "offsetY");
	glProgramUniform1f(renderingProgramKor, offset2, y);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, kor.size());

	const float fal = 1.000 - 1.000 / 6.000;

	if (xDir) {
		x += increment;
		if ((x > fal) || (x < -fal))
			increment = -increment;
		/** Aktiváljuk a shader-program objektumunkat. */
		glProgramUniform1f(renderingProgramKor, XoffsetLocation, x);
	}
	else if (yDir){
		y += increment;
		if ((y > fal) || (y < -fal))
			increment = -increment;
		glProgramUniform1f(renderingProgramKor, Yoffsetation, y);
	}

	const bool isOnLine = (y >= -0.17 + liney && y <= 0.17 + liney) && (x >= -0.5 + linex && x <= 0.5 + linex);
	const auto& colorData = isOnLine ? red : green;

	//adat betöltés
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(glm::vec3), colorData.data(), GL_STATIC_DRAW);
	//vertex atribútumok
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	/** Ha kirajzoltuk a pontunkat, kísérletezzünk a pont méretének növelésével. */
	//glPointSize(30.0);
	/** Ha vonalat rajzolunk ki, ezzel módosíthatjuk a láthatóságot. */
	//glLineWidth(10.0);

	//glDrawArrays(GL_POINTS, 0, 1);
	//glDrawArrays(GL_LINES, 0, 2);
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(void) {

	/** Próbáljuk meg inicializálni a GLFW-t! */
	if (!glfwInit()) {
		exit(EXIT_FAILURE); }

	/** A kívánt OpenGL verzió (4.3) */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	/** Próbáljuk meg létrehozni az ablakunkat. */
	GLFWwindow* window = glfwCreateWindow(600, 600, "Project1", NULL, NULL);

	/** Válasszuk ki az ablakunk OpenGL kontextusát, hogy használhassuk. */
	glfwMakeContextCurrent(window);

	/** Billentyûzethez köthetõ események kezelése. */
	glfwSetKeyCallback(window, keyCallback);
	/** A kurzor helyének vizsgálata. */
	//glfwSetCursorPosCallback(window, cursorPosCallback);
	/** Az egér gombjaihoz köthetõ események kezelése. */
	//glfwSetMouseButtonCallback(window, mouseButtonCallback);

	/** Incializáljuk a GLEW-t, hogy elérhetõvé váljanak az OpenGL függvények. */
	if (glewInit() != GLEW_OK) { 
		exit(EXIT_FAILURE); }

	glfwSwapInterval(1);

	glfwSetWindowSizeLimits(window, 600, 600, 600, 600);
	glfwSetWindowAspectRatio(window, 1, 1);

	/** Az alkalmazáshoz kapcsolódó elõkészítõ lépések, pl. hozd létre a shader objektumokat. */
	init(window);

	while (!glfwWindowShouldClose(window)) {
		/** a kód, amellyel rajzolni tudunk a GLFWwindow ojektumunkba. */
		display(window, glfwGetTime());
		/** double buffered */
		glfwSwapBuffers(window);
		/** események kezelése az ablakunkkal kapcsolatban, pl. gombnyomás */
		glfwPollEvents();
	}

	/** Töröljük a GLFW ablakot. */
	glfwDestroyWindow(window);
	/** Leállítjuk a GLFW-t */
	glfwTerminate();

	/** Felesleges objektumok törlése. */
	cleanUpScene();

	exit(EXIT_SUCCESS);
}