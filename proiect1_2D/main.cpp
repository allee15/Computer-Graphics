// Codul sursa este adaptat dupa OpenGLBook.com

#include <windows.h>  // biblioteci care urmeaza sa fie incluse
#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <GL/glew.h> // glew apare inainte de freeglut
#include <GL/freeglut.h> // nu trebuie uitat freeglut.h
#include "loadShaders.h"
#include "SOIL.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <chrono>
using namespace std;

//////////////////////////////////////

GLuint
VaoId,
VboId,
EboId,
ProgramId,
myMatrixLocation,
viewLocation,
projLocation,
renderTextureLocation;
GLuint
texture;

GLint tx = 450, ty = 30;
GLint firstCarX = 0, firstCarY = 20;
GLfloat angle, timer = 0;

bool leftKeyActive = false;
bool rightKeyActive = false;
bool upKeyActive = false;
bool downKeyActive = false;

//	Elemente pentru matricea de vizualizare;
float obsX = 0.0, obsY = 0.0, obsZ = 800.f;
float refX = 0.0f, refY = 0.0f;
float vX = 0.0;

float width = 800, height = 600, xMin = -800.f, xMax = 800, yMin = -600, yMax = 600, zNear = 0, zFar = 1000, fov = 45;
glm::mat4 projection, myMatrix, view;
glm::mat4 resizeMatrix, matrTransl1, matrTransl2, matrRot;
float acceleration = 1.0f;
bool isLost = false;


void LoadTexture(const char* photoPath) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// desfasurarea imaginii pe orizontala/verticala in functie de parametrii de texturare
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height;
	unsigned char* image = SOIL_load_image(photoPath, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
}

const float startX = -150.0f, endX = 150.0f, startY = 30.0f, endY = 180.0f;

std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now(), currentTime;

void CreateVBO(void)
{
	//	Atributele varfurilor -  COORDONATE, CULORI, COORDONATE DE TEXTURARE;
	GLfloat Vertices[] = {
		//	Coordonate;					Culori;				Coordonate de texturare;
		  -2700.0f, -600.0f, 0.0f, 1.0f,1.0f, 1.0f, 1.0f,	     0.0f, 0.0f,	// Stanga jos;
		   600.0f, -600.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,	     1.0f, 0.0f, // Dreapta jos;
		   600.0f,  600.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,	     1.0f, 1.0f,	// Dreapta sus;
		  -2700.0f,  600.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,	 0.0f, 1.0f,  // Stanga sus;

		  // Prima masina
		startX, 30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,    0.0f, 0.0f, //4
		endX, 30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,    1.0f, 0.0f,
		endX, 180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,    1.0f, 1.0f,
		startX, 180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,	 0.0f, 1.0f,

		//a 2-a masina
		300.0f, 30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f, //4
		600.0f, 30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,	  1.0f, 0.0f,
		600.0f, 180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		300.0f, 180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 1.0f,


		//message placeholder
		300.0f, 200.0f, 0.0f, 1.0f,         1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		600.0f, 200.0f, 0.0f, 1.0f,         1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
		600.0f, 450.0f, 0.0f, 1.0f,         1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		300.0f, 450.0f, 0.0f, 1.0f,        1.0f, 1.0f, 1.0f,     0.0f, 1.0f,

		//semnalizare stanga
		300.0f, 30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		200.0f, 30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		200.0f, 0.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,

		//semnalizare dreapta
		300.0f, 180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		200.0f, 180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		200.0f, 210.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,

		//a 3-a masina
		-1500.0f, -180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		-1200.0f, -180.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,	  1.0f, 0.0f,
		-1200.0f, -30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		-1500.0f, -30.0f, 0.0f, 1.0f,			1.0f, 1.0f, 1.0f,     0.0f, 1.0f,


	};

	//	Indicii care determina ordinea de parcurgere a varfurilor;
	GLuint Indices[] = {
	  0, 1, 2,  //	Primul triunghi;
	  0, 2, 3,	//  Al doilea triunghi;

	  // Prima masina
	  4, 5, 6,
	  4, 6, 7,

	  8, 9, 10,
	  8, 10, 11,

	  12, 13, 14,
	  12, 14, 15,

	  //semnalizare
	  16, 17, 18,

	  19, 20, 21,

	  22,23,24,25
	};

	// Transmiterea datelor prin buffere;

	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);

	// Se creeaza un buffer pentru VARFURI - COORDONATE, CULORI si TEXTURARE
	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// Se creeaza un buffer pentru indici
	glGenBuffers(1, &EboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Se activeaza lucrul cu atribute;
	// Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
	// Se asociaza atributul (1 = culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	// Se asociaza atributul (2 = texturare) pentru shader;
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GL_FLOAT), (GLvoid*)(7 * sizeof(GLfloat)));
}
void DestroyVBO(void)
{
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("example.vert", "example.frag");
	glUseProgram(ProgramId);
}
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void Initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // culoarea de fond a ecranului
	CreateShaders();
	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	viewLocation = glGetUniformLocation(ProgramId, "view");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	//	Pentru shaderul de fragmente;
	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);
	//resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);

}

bool goesLeft = false, goesRight = false;
void ProcessNormalKeys(unsigned char key, int xx, int yy)
{
	if (!isLost) {
		// Update key states based on the key events
		switch (key)
		{
		case 'a':
			tx -= 20 * acceleration;
			goesLeft = false;
			goesRight = false;
			break;
		case 'd':
			tx += 20 * acceleration;
			break;
		case 'w':
			ty += 20;
			tx -= 10;
			acceleration -= acceleration / 12;
			goesRight = !goesRight;
			goesLeft = false;
			break;
		case 's':
			ty -= 20;
			tx -= 10;
			acceleration += acceleration / 12;
			goesRight = false;
			goesLeft = !goesLeft;
			break;
		}
	}
}

bool CheckIfFirstOverTakeIsSuccessful() {
	return tx + 600.0f <= firstCarX - 150;
}

bool CheckIfSecondOverTakeIsSuccessful() {
	return tx + 600.0f <= -1500.0f;
}

void CheckIfItsLost() {
	if (isLost) {
		return;
	}
	const int startCarX = tx + 300, startCarY = ty - 10;
	const int endCarX = tx + 600, endCarY = ty + 130;

	bool hasLostToFirstCar = (startCarX <= firstCarX + 150 && (!CheckIfFirstOverTakeIsSuccessful() && (endCarY >= startCarY)))
		&& ((startCarX <= firstCarX + 150) && (endCarX > firstCarX - 150) && endCarY >= firstCarY);

	bool hasLostToSecondCar = (startCarX <= -1200 && startCarX >= -1500 && startCarY + 30 >= -180 && startCarY + 30 <= -30)
		|| (startCarX <= -1200 && startCarX >= -1500 && endCarY + 30 >= -180 && endCarY + 30 <= -30)
		|| (endCarX <= -1200 && endCarX >= -1500 && startCarY + 30 >= -180 && startCarY + 30 <= -30)
		|| (endCarX <= -1200 && endCarX >= -1500 && endCarY + 30 >= -180 && endCarY + 30 <= -30);

	isLost = hasLostToFirstCar || hasLostToSecondCar;
}


void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	CheckIfItsLost();
	//std::cout << (std::chrono::steady_clock::now() - startTime).count() / 100000000<<endl;
	obsX -= CheckIfFirstOverTakeIsSuccessful() ? 3 : 1.5;
	//	Realizarea perspectivei;
	//	Pozitia observatorului;
	glm::vec3 obs = glm::vec3(obsX, obsY, obsZ);
	//	Pozitia punctului de referinta;
	refX = obsX; refY = obsY;
	glm::vec3 refPoint = glm::vec3(refX, refY, -1.0f);
	//	Verticala din planul de vizualizare; 
	glm::vec3 vert = glm::vec3(vX, 1.0f, 0.0f);
	view = glm::lookAt(obs, refPoint, vert);

	//move the first car
	firstCarX -= 1;
	//	Realizarea proiectiei;
	projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), zNear, zFar);
	myMatrix = glm::mat4(1.0f);
	matrTransl1 = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, 0.0));
	matrTransl2 = glm::translate(glm::mat4(1.0f), glm::vec3(firstCarX, firstCarY, 0.0));
	matrRot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0, 0.0, 1.0));


	//	myMatrix = matrRot;
	//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateVBO();
	//	Incarcarea texturii si legarea acesteia cu shaderul;
	LoadTexture("road2.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE, PERSPECTIVA si PROIECTIE spre shadere;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
	//	Transmiterea variabilei uniforme pentru TEXTURARE spre shaderul de fragmente;
	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);
	glUniform1i(glGetUniformLocation(ProgramId, "renderTexture"), 1);

	//	Desenarea primitivelor;
	//	Functia glDrawElements primeste 4 argumente:
	//	 - arg1 = modul de desenare;
	//	 - arg2 = numarul de varfuri;
	//	 - arg3 = tipul de date al indicilor;
	//	 - arg4 = pointer spre indici (EBO): pozitia de start a indicilor;
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	LoadTexture("masinuta.png");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);
	glUniform1i(glGetUniformLocation(ProgramId, "renderTexture"), 1);

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE, PERSPECTIVA si PROIECTIE spre shadere;
	glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_INT, (void*)(30 * sizeof(GLfloat)));

	myMatrix = matrTransl2;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	// Pass the data to the shaders
	LoadTexture("car1.png");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE, PERSPECTIVA si PROIECTIE spre shadere;
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLfloat)));


	myMatrix = matrTransl1;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	LoadTexture("car2.png");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE, PERSPECTIVA si PROIECTIE spre shadere;
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(GLfloat)));


	if ((CheckIfFirstOverTakeIsSuccessful() == 1 && CheckIfSecondOverTakeIsSuccessful() == 1) || isLost) {
		//timer += 0.5f;
		LoadTexture(isLost ? "youlose.png" : "youwin.png");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

		//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE, PERSPECTIVA si PROIECTIE spre shadere;
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(GLfloat)));

		/*if (timer >= 5.0f) {
			exit(0);
		}*/
	}


	////placeholder
	//if (CheckIfFirstOverTakeIsSuccessful() || isLost) {
	//	LoadTexture(isLost ? "youlose.png" : "youwin.png");
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, texture);
	//	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

	//	//	Transmiterea variabilelor uniforme pentru MATRICEA DE TRANSFORMARE, PERSPECTIVA si PROIECTIE spre shadere;
	//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(GLfloat)));
	//}

	glUniform1i(glGetUniformLocation(ProgramId, "renderTexture"), 3);

	if (goesLeft) {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(24 * sizeof(GLfloat)));
	}

	if (goesRight) {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(27 * sizeof(GLfloat)));
	}


	glutSwapBuffers();
	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100, 100); // pozitia initiala a ferestrei
	glutInitWindowSize(1200, 600); //dimensiunile ferestrei
	glutCreateWindow("Road Simulator"); // titlul ferestrei
	glewInit(); // nu uitati de initializare glew; trebuie initializat inainte de a a initializa desenarea
	Initialize();
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(RenderFunction);
	glutKeyboardFunc(ProcessNormalKeys);
	glutCloseFunc(Cleanup);
	glutMainLoop();

	return 0;
}

