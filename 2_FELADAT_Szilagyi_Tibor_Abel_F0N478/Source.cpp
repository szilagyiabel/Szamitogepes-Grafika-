#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <array>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <time.h>

using namespace std;

static vector<glm::vec3> myPoints =
{
	glm::vec3(-0.8f, -0.8f, 0.0f),
	glm::vec3(-0.8f, 0.8f, 0.0f),
	glm::vec3(0.8f, -0.8f, 0.0f),
	glm::vec3(0.8f, 0.8f, 0.0f),
};

vector<glm::vec3> bezierPoints;

#define		numVBOs			2
#define		numVAOs			2
GLuint		VBO[numVBOs];
GLuint		VAO[numVAOs];



GLint			window_width = 600;
GLint			window_height = 600;

GLboolean	keyboard[512] = { GL_FALSE };
GLFWwindow* window = nullptr;
GLuint		renderingProgram;

/* -1 jelentése, hogy nem vonszolunk semmit. */
GLint		dragged = -1;

//rekurzív kiszámítása a bézier görbe t-nél lévő pontjait
glm::vec3 deCasteljau(int r, int i, float t) {
	if (r == 0) return myPoints[i];
	return (1 - t) * deCasteljau(r - 1, i, t) + t * deCasteljau(r - 1, i + 1, t);
}

//brézier görbe
void drawBezier() {

	bezierPoints.clear();
	GLfloat t = 0.0f, increment = 1.0f / 50.0f;

	while (t <= 1.0f) {
		glm::vec3 nextPoint = deCasteljau(myPoints.size() - 1, 0, t);
		bezierPoints.push_back(nextPoint);
		t += increment;
	}
}

GLfloat dist2(glm::vec3 P1, glm::vec3 P2) {
	GLfloat dx = P1.x - P2.x;
	GLfloat dy = P1.y - P2.y;

	/** delta X és delta Y segítségével ki tudjuk számolni P1 és P2 pontok távolságának négyzetét. */
	return dx * dx + dy * dy;
}

GLint getActivePoint(vector<glm::vec3> p, GLfloat sensitivity, GLfloat x, GLfloat y) {
	GLfloat s = sensitivity * sensitivity;
	GLint		size = p.size();

	GLfloat		xNorm = x / ((GLfloat)window_width / 2.0) - 1.0f;
	GLfloat		yNorm = y / ((GLfloat)window_height / 2.0) - 1.0f;
	glm::vec3	mousePos = glm::vec3(xNorm, yNorm, 0.0f);

	for (GLint i = 0; i < size; i++) 
		if (dist2(p[i], mousePos) < s) 
			return i;
	


	return -1;

}

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

/** A jelenetünk utáni takarítás. */
void cleanUpScene()
{
	/** Töröljük a vertex array objektumokat. */
	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);

	/** Töröljük a shader programot. */
	glDeleteProgram(renderingProgram);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void init(GLFWwindow* window) {

	renderingProgram = createShaderProgram();

	/** Vertex Array Objektum létrehozása. */
	glGenBuffers(numVBOs, VBO);
	glGenVertexArrays(numVAOs, VAO);

	drawBezier();
	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, bezierPoints.size() * sizeof(glm::vec3), bezierPoints.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, myPoints.size() * sizeof(glm::vec3), myPoints.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glClearColor(0.68, 0.95, 0.90, 1.0);
}

void display(GLFWwindow* window, double currentTime) {

	glClear(GL_COLOR_BUFFER_BIT);

	GLuint color = glGetUniformLocation(renderingProgram, "szinez");
	//GLuint color1 = glGetUniformLocation(renderingProgram1, "color1");
	//GLuint color2 = glGetUniformLocation(renderingProgram2, "color2");

	//bezier
	glUseProgram(renderingProgram);
	glBindVertexArray(VAO[0]);
	glLineWidth(3.0f);
	glProgramUniform1f(renderingProgram, color, 0);
	glDrawArrays(GL_LINE_STRIP, 0, bezierPoints.size());
	glBindVertexArray(0);


	glBindVertexArray(VAO[1]);

	//vobal
	glLineWidth(5.0f);
	glProgramUniform1f(renderingProgram, color, 1);
	glDrawArrays(GL_LINE_LOOP, 0, myPoints.size());

	//pont
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glPointSize(30.0f);
	glProgramUniform1f(renderingProgram, color, 2);
	glDrawArrays(GL_POINTS, 0, myPoints.size());
	glBindVertexArray(0);

}


void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;
	glViewport(0, 0, width, height);
}

void updateBezier() {

	drawBezier();

	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, bezierPoints.size() * sizeof(glm::vec3), bezierPoints.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, myPoints.size() * sizeof(glm::vec3), myPoints.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/** ESC billentyűre kilépés. */
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
		cleanUpScene();

	/** A billentyűk lenyomásának és felengedésének regisztrálása. Lehetővé teszi gombkombinációk használatát. */
	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{
	/** Az egér mutató helyét kezelő függvény. */
	if (dragged >= 0) {

		GLfloat	xNorm = xPos / ((GLfloat)window_width / 2.0f) - 1.0f;
		GLfloat	yNorm = ((GLfloat)window_height - yPos) / ((GLfloat)window_height / 2.0f) - 1.0f;

		myPoints.at(dragged).x = xNorm;
		myPoints[dragged].x = xNorm;
		myPoints.at(dragged).y = yNorm;

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, myPoints.size() * sizeof(glm::vec3), myPoints.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	updateBezier();
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	GLint n;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	/** Az egér bal gombjának megnyomása indíthat el vonszolást. */
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if ((n = getActivePoint(myPoints, 0.1f, x, window_height - y)) != -1) {
			dragged = n;
		}
		else {
			myPoints.push_back(glm::vec3(x / (window_width / 2.0f) - 1.0f, (window_height - y) / (window_height / 2.0f) - 1.0f, 0));
			updateBezier();
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) { //jobb egérgombal törlünk
		if ((n = getActivePoint(myPoints, 0.1f, x, window_height - y)) != -1) {
			myPoints.erase(myPoints.begin() + n);
			updateBezier();
		}
	}

	/** Az egér bal gombjának felengedése mindenképp megszünteti a vonszolási üzemmódot. */
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		dragged = -1;
	}
}

int main(void) {

	/** Próbáljuk meg inicializálni a GLFW-t! */
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	/** A kívánt OpenGL verzió (4.3) */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	/** Próbáljuk meg létrehozni az ablakunkat. */
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Project2", NULL, NULL);

	/** Válasszuk ki az ablakunk OpenGL kontextusát, hogy használhassuk. */
	glfwMakeContextCurrent(window);


	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	/** Billentyûzethez köthetõ események kezelése. */
	glfwSetKeyCallback(window, keyCallback);
	/** A kurzor helyének vizsgálata. */
	glfwSetCursorPosCallback(window, cursorPosCallback);
	/** Az egér gombjaihoz köthetõ események kezelése. */
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	/** Incializáljuk a GLEW-t, hogy elérhetõvé váljanak az OpenGL függvények. */
	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);


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