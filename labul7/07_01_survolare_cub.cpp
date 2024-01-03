//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Curs VII - 07_01_survolare_cub.cpp        |
// =============================================
// 
//	Program ce deseneaza un cub ce poate fi survolat, folosindu-se tehnicile MODERN OpenGL;
//	ELEMENTE DE NOUTATE:
//	 - survolarea unui obiect folosind coordonate sferice;
// 
// 
// 
//	Biblioteci
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define INSTANCE_COUNT 40

//  Identificatorii obiectelor de tip OpenGL;
GLuint
	VaoId,
	VboId,
	EboId,
	ProgramId,
	viewLocation,
	projLocation,
	codColLocation;
//	Dimensiunile ferestrei de afisare;
GLfloat
	winWidth = 1200, winHeight = 900;
//	Variabile catre matricile de transformare;
glm::mat4
	view, projection;

//	Variabila ce determina schimbarea culorii pixelilor in shader;
int codCol;
//	Valaorea lui pi;
float const PI = 3.141592;
//	Elemente pentru matricea de vizualizare;
float refX = 0.0f, refY = 0.0f, refZ = 0.0f,
	  obsX, obsY, obsZ,
	  vX = 0.0f, vY = 0.0f, vZ = 1.0f;
//	Elemente pentru deplasarea pe sfera;
float alpha = 0.0f, beta = 0.0f, dist = 200.0f,
	  incrAlpha1 = 0.01, incrAlpha2 = 0.01;
//	Elemente pentru matricea de proiectie;
float
	width = 800, height = 600, 
	zNear = 1.f, fov = 90.f*PI/180;

void ProcessNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '-':
		dist -= 5.0;	//	Apasarea tastelor `+` si `-` schimba pozitia observatorului (se departeaza / aproprie);
		break;
	case '+':
		dist += 5.0;
		break;
	}
	if (key == 27)
		exit(0);
}

void ProcessSpecialKeys(int key, int xx, int yy)
{
	switch (key)				//	Procesarea tastelor 'LEFT', 'RIGHT', 'UP', 'DOWN';
	{							//	duce la deplasarea observatorului pe suprafata sferica in jurul cubului;
	case GLUT_KEY_LEFT:
		beta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01;
		break;
	case GLUT_KEY_UP:
		alpha += incrAlpha1;
		if (abs(alpha - PI / 2) < 0.05)
		{
			incrAlpha1 = 0.f;
		}
		else
		{
			incrAlpha1 = 0.01f;
		}
		break;
	case GLUT_KEY_DOWN:
		alpha -= incrAlpha2;
		if (abs(alpha + PI / 2) < 0.05)
		{
			incrAlpha2 = 0.f;
		}
		else
		{
			incrAlpha2 = 0.01f;
		}
		break;
	}
}

//  Crearea si compilarea obiectelor de tip shader;
//	Trebuie sa fie in acelasi director cu proiectul actual;
//  Shaderul de varfuri / vertex shader - afecteaza geometria scenei;
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
void CreateShaders(void)
{
	ProgramId = LoadShaders("07_01_Shader.vert", "07_01_Shader.frag");
	glUseProgram(ProgramId);
}

//  Se initializeaza un vertex Buffer Object (VBO) pentru tranferul datelor spre memoria placii grafice (spre shadere);
//  In acesta se stocheaza date despre varfuri (coordonate, culori, indici, texturare etc.);
void CreateVBO(void)
{
	//	Atributele varfurilor -  COORDONATE si CULORI;
	GLfloat Vertices[] =
	{
		//	Varfurile VERZI din planul z = -50;  
		//	Coordonate;					Culori;			
		-50.0f, -50.0f, -50.0f,  1.0f,
		 50.0f, -50.0f, -50.0f,  1.0f,

		 //	Varfurile ROSII din planul z = +50;
		 //	Coordonate;					Culori;			
		 -50.0f, -50.0f,  50.0f,  1.0f,
		  50.0f, -50.0f,  50.0f,  1.0f,

		  // Varfurile ALBASTRE din planul z = 0;
		 0.0f, 50.0f, 0.0f,     1.0f,
	};

	glm::vec4 Colors[INSTANCE_COUNT];
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		float a = float(i) / 4.0f;
		float b = float(i) / 5.0f;
		float c = float(i) / 6.0f;
		Colors[i][0] = 0.35f + 0.30f * (sinf(a + 2.0f) + 1.0f);
		Colors[i][1] = 0.25f + 0.25f * (sinf(b + 3.0f) + 1.0f);
		Colors[i][2] = 0.25f + 0.35f * (sinf(c + 4.0f) + 1.0f);
		Colors[i][3] = 1.0f;
	}

	//  MATRICELE instantelor - se defineste tiparul de desenare al cuburilor prin ROTIREA si TRANSLATIA cubului initial (INSTANCED RENDERING);
	glm::mat4 MatModel[INSTANCE_COUNT];
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		MatModel[i] =
			glm::translate(glm::mat4(1.0f), glm::vec3(20 * i % 2 * cos(10.f * i * 180 / PI),	//	Deplasarea pe OX;
				100 * i * sin(10.f * i * 180 / PI),	//	Deplasarea pe OY; 
				50 * i * cos(10.f * i * 180 / PI)))									//	Deplasarea pe OZ;
			* glm::rotate(glm::mat4(1.0f), i * PI / 8, glm::vec3(i, 2 * i * i, i / 3));
	}

	//	Indicii pentru varfuri;
	GLubyte Indices[] =
	{
		0, 1, 3,   3, 2, 0, //  Fata "de jos";
		1, 0 ,4, 3, 1, 4, 2, 3, 4, 2, 0, 4,
		0, 1, 3, 2,
		0, 4,
		1, 4,
		2, 4,
		3, 4
	};

	//  Transmiterea datelor prin buffere;

	//  Se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO;
	glGenVertexArrays(1, &VaoId);                                                   //  Generarea VAO si indexarea acestuia catre variabila VaoId;
	glBindVertexArray(VaoId);

	//  Se creeaza un buffer pentru VARFURI - COORDONATE si CULORI;
	glGenBuffers(1, &VboId);														//  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId);											//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	//	Se creeaza un buffer pentru INDICI;
	glGenBuffers(1, &EboId);														//  Generarea bufferului si indexarea acestuia catre variabila EboId;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);									//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	//	Se activeaza lucrul cu atribute;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
}

// Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru VARFURI (Coordonate, Culori), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);		//  Culoarea de fond a ecranului;
	CreateVBO();								//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateShaders();							//  Initilizarea shaderelor;
	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	viewLocation = glGetUniformLocation(ProgramId, "view");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
}

//	Functia de desenare a graficii pe ecran;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//  Se curata ecranul OpenGL pentru a fi desenat noul continut (bufferul de culoare & adancime);
	glEnable(GL_DEPTH_TEST);

	//	CreateVBO(); //	 Decomentati acest rand daca este cazul;
	glBindVertexArray(VaoId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);

	//	Matricea de vizualizare;
	//	Pozitia observatorului - se deplaseaza pe sfera;
	obsX = refX + dist * cos(alpha) * cos(beta);
	obsY = refY + dist * cos(alpha) * sin(beta);
	obsZ = refZ + dist * sin(alpha);

	//	Matricea de vizualizare;
	glm::vec3 obs = glm::vec3(obsX, obsY, obsZ);		//	Pozitia observatorului;	
	glm::vec3 pctRef = glm::vec3(refX, refY, refZ); 	//	Pozitia punctului de referinta;
	glm::vec3 vert = glm::vec3(vX, vY, vZ);				//	Verticala din planul de vizualizare; 
	view = glm::lookAt(obs, pctRef, vert);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	//	Realizarea proiectiei;
	projection = glm::infinitePerspective(GLfloat(fov), GLfloat(width) / GLfloat(height), zNear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
	
	//	Desenarea fetelor;
	codCol = 0;															//  Culoarea;
	glUniform1i(codColLocation, codCol);								//	Transmiterea variabilei uniforme pentru COLORARE spre shadere;
	//	Functia glDrawElements primeste 4 argumente:
	//	 - arg1 = modul de desenare;
	//	 - arg2 = numarul de varfuri;
	//	 - arg3 = tipul de date al indicilor;
	//	 - arg4 = pointer spre indici (EBO): pozitia de start a indicilor;
	glDrawElementsInstanced(GL_TRIANGLES, 18, GL_UNSIGNED_BYTE, 0, INSTANCE_COUNT);

	//  Desenarea muchiilor;
	codCol = 1;																					//	Se schimba culoarea;
	glUniform1i(codColLocation, codCol);
	glLineWidth(2.5);
	glDrawElementsInstanced(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(18), INSTANCE_COUNT);
	glDrawElementsInstanced(GL_LINES, 8, GL_UNSIGNED_BYTE, (void*)(22), INSTANCE_COUNT);

	glutSwapBuffers();	//	Inlocuieste imaginea deseneata in fereastra cu cea randata; 
	glFlush();			//  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);		//	Se folosesc 2 buffere pentru desen (unul pentru afisare si unul pentru randare => animatii cursive) si culori RGB + 1 buffer pentru adancime;
	glutInitWindowSize(winWidth, winHeight);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Survolarea unui cub - OpenGL <<nou>>");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();							//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);		//  Desenarea scenei in fereastra;
	glutIdleFunc(RenderFunction);			//	Asigura rularea continua a randarii;
	glutKeyboardFunc(ProcessNormalKeys);	//	Functii ce proceseaza inputul de la tastatura utilizatorului;
	glutSpecialFunc(ProcessSpecialKeys);
	glutCloseFunc(Cleanup);					//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}
