

/* 
Practica 2                                   Morales Soto Fernando
Fecha de entrega:21 de agosto del 2025        Numero de cuenta 315143977

*/

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"

void resize(GLFWwindow* window, int width, int height);

const GLint WIDTH = 800, HEIGHT = 600;
const int N = 32; // 32x32 celdas

//  para celdas "insertadas" 
std::vector<float> g_cellVertices; // (x,y,z, r,g,b, u,v) por vértice
GLuint VAO_cells = 0, VBO_cells = 0;

GLuint VAO_grid = 0, VBO_grid = 0;
std::vector<float> g_gridVertices;

// Mapea 0..N -> [-1,1] en NDC
inline float posNDC(int i) { return -1.0f + 2.0f * (float)i / (float)N; }

// Inserta una celda (i,j) con color: val=0 negro, val=1 gris
void addCell(int i, int j, int val) {
    if (i < 0 || i >= N || j < 0 || j >= N) return;

    const float z = 0.0f;
    float x0 = posNDC(i), y0 = posNDC(j);
    float x1 = posNDC(i + 1), y1 = posNDC(j + 1);

    float r, g, b;
    if (val == 0) { r = 0.0f; g = 0.0f; b = 0.0f; }      // negro
    else { r = 0.6f; g = 0.6f; b = 0.6f; }      // gris

    float u = 0.0f, v = 0.0f;

    // Dos triángulos por celda
    float quad[] = {
        // x   y   z    r    g    b    u  v
         x0, y0, z,   r,   g,   b,   u,  v,
         x1, y0, z,   r,   g,   b,   u,  v,
         x1, y1, z,   r,   g,   b,   u,  v,

         x0, y0, z,   r,   g,   b,   u,  v,
         x1, y1, z,   r,   g,   b,   u,  v,
         x0, y1, z,   r,   g,   b,   u,  v,
    };

    // Añadimos al vector
    g_cellVertices.insert(g_cellVertices.end(), std::begin(quad), std::end(quad));

    // Subimos/actualizamos al VBO (dinámico)
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cells);
    glBufferData(GL_ARRAY_BUFFER, g_cellVertices.size() * sizeof(float), g_cellVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Genera las líneas de la cuadricula
void buildGrid() {
    const float z = 0.0f;
    auto addLine = [&](float x0, float y0, float x1, float y1) {
        // negro
        g_gridVertices.insert(g_gridVertices.end(), { x0,y0,z, 0.0f,0.0f,0.0f, 0.0f,0.0f });
        g_gridVertices.insert(g_gridVertices.end(), { x1,y1,z, 0.0f,0.0f,0.0f, 0.0f,0.0f });
    };

    for (int i = 0; i <= N; ++i) { float x = posNDC(i); addLine(x, -1.0f, x, 1.0f); }
    for (int j = 0; j <= N; ++j) { float y = posNDC(j); addLine(-1.0f, y, 1.0f, y); }

    glBindVertexArray(VAO_grid);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_grid);
    glBufferData(GL_ARRAY_BUFFER, g_gridVertices.size() * sizeof(float), g_gridVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Fernando Morales", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, resize);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cout << "Failed to initialise GLEW\n"; return EXIT_FAILURE; }

    Shader shader("Shader/core.vs", "Shader/core.frag");

    // VAO/VBO para CELDAS
    glGenVertexArrays(1, &VAO_cells);
    glGenBuffers(1, &VBO_cells);
    glBindVertexArray(VAO_cells);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cells);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); // vacío al inicio
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // VAO/VBO para GRID
    glGenVertexArrays(1, &VAO_grid);
    glGenBuffers(1, &VBO_grid);
    buildGrid();
    //Para acelerar el proceso de coloreado se uso el ciclo for para colorear filas enteras 
    for (int j = 0; j < 32; j++) {
        addCell(0, j, 0);
    }
    for (int j = 0; j < 32; j++) {
        addCell(31, j, 0);

    }
    for (int j = 0; j < 32; j++) {
        addCell(30, j, 0);
    }
    for (int j = 0; j < 32; j++) {
        addCell(29, j, 0);
    }
    for (int j = 0; j < 32; j++) {
        addCell(1, j, 0);

    }
    for (int j = 0; j < 32; j++) {
        addCell(2, j, 0);
    }
    
    for (int i = 0; i < 32; i++) {
        addCell(i, 0, 0);
    }
    for (int i = 0; i < 32; i++) {
        addCell(i, 1, 0);
    }
    for (int i = 0; i < 32; i++) {
        addCell(i, 2, 0);
    }
    for (int i = 0; i < 32; i++) {
        addCell(i, 30, 0);
    }
    for (int i = 0; i < 32; i++) {
        addCell(i, 29, 0);
    }
    
    for (int i = 0; i < 32; i++) {
        addCell(i, 31, 0);
    }

    

    //Negro
    // j=3
    addCell(3, 3, 0); addCell(4, 3, 0); addCell(5, 3, 0); addCell(6, 3, 0); addCell(7, 3, 0); addCell(8, 3, 0); addCell(9, 3, 0); addCell(10, 3, 0); addCell(11, 3, 0); addCell(12, 3, 0);
    addCell(19, 3, 0); addCell(20, 3, 0); addCell(21, 3, 0); addCell(22, 3, 0); addCell(23, 3, 0); addCell(24, 3, 0); addCell(25, 3, 0); addCell(26, 3, 0); addCell(27, 3, 0); addCell(28, 3, 0);

    // j=4
    addCell(3, 4, 0); addCell(4, 4, 0); addCell(5, 4, 0); addCell(6, 4, 0); addCell(7, 4, 0); addCell(8, 4, 0); addCell(9, 4, 0);
    addCell(15, 4, 0); addCell(16, 4, 0);
    addCell(22, 4, 0); addCell(23, 4, 0); addCell(24, 4, 0); addCell(25, 4, 0); addCell(26, 4, 0); addCell(27, 4, 0); addCell(28, 4, 0);

    // j=5
    addCell(3, 5, 0); addCell(4, 5, 0); addCell(5, 5, 0); addCell(6, 5, 0); addCell(7, 5, 0); addCell(8, 5, 0);
    addCell(11, 5, 0); addCell(12, 5, 0); addCell(13, 5, 0); addCell(14, 5, 0); addCell(15, 5, 0); addCell(16, 5, 0); addCell(17, 5, 0); addCell(18, 5, 0); addCell(19, 5, 0); addCell(20, 5, 0);
    addCell(23, 5, 0); addCell(24, 5, 0); addCell(25, 5, 0); addCell(26, 5, 0); addCell(27, 5, 0); addCell(28, 5, 0);

    // j=6
    addCell(3, 6, 0); addCell(4, 6, 0); addCell(5, 6, 0); addCell(6, 6, 0); addCell(7, 6, 0);
    addCell(10, 6, 0); addCell(11, 6, 0);
    addCell(15, 6, 0); addCell(16, 6, 0);
    addCell(20, 6, 0); addCell(21, 6, 0);
    addCell(24, 6, 0); addCell(25, 6, 0); addCell(26, 6, 0); addCell(27, 6, 0); addCell(28, 6, 0);

    // j=7
    addCell(3, 7, 0); addCell(4, 7, 0); addCell(5, 7, 0); addCell(6, 7, 0);
    addCell(9, 7, 0);
    addCell(11, 7, 0); addCell(12, 7, 0);
    addCell(14, 7, 0); addCell(15, 7, 0); addCell(16, 7, 0); addCell(17, 7, 0);
    addCell(19, 7, 0); addCell(20, 7, 0);
    addCell(22, 7, 0);
    addCell(25, 7, 0); addCell(26, 7, 0); addCell(27, 7, 0); addCell(28, 7, 0);

    // j=8
    addCell(3, 8, 0); addCell(4, 8, 0); addCell(5, 8, 0);
    addCell(8, 8, 0); addCell(9, 8, 0); addCell(10, 8, 0); addCell(11, 8, 0); addCell(12, 8, 0); addCell(13, 8, 0); addCell(14, 8, 0); addCell(15, 8, 0); addCell(16, 8, 0); addCell(17, 8, 0); addCell(18, 8, 0); addCell(19, 8, 0); addCell(20, 8, 0); addCell(21, 8, 0); addCell(22, 8, 0); addCell(23, 8, 0);
    addCell(26, 8, 0); addCell(27, 8, 0); addCell(28, 8, 0);

    // j=9
    addCell(3, 9, 0); addCell(4, 9, 0);
    addCell(7, 9, 0); addCell(8, 9, 0);
    addCell(13, 9, 0); addCell(14, 9, 0); addCell(15, 9, 0); addCell(16, 9, 0); addCell(17, 9, 0); addCell(18, 9, 0);
    addCell(23, 9, 0); addCell(24, 9, 0);
    addCell(26, 9, 0); addCell(27, 9, 0); addCell(28, 9, 0);

    // j=10
    addCell(3, 10, 0); addCell(4, 10, 0);
    addCell(28, 10, 0);

    // j=11
    addCell(3, 11, 0); addCell(6, 11, 0); addCell(25, 11, 0); addCell(28, 11, 0);

    // j=12
    addCell(3, 12, 0); addCell(28, 12, 0);

    // j=14
    addCell(8, 14, 0); addCell(9, 14, 0); addCell(10, 14, 0);
    addCell(21, 14, 0); addCell(22, 14, 0); addCell(23, 14, 0);

    // j=15
    addCell(5, 15, 0); addCell(6, 15, 0);
    addCell(25, 15, 0); addCell(26, 15, 0);

    // j=16
    addCell(6, 16, 0); addCell(12, 16, 0); addCell(19, 16, 0); addCell(25, 16, 0);

    // j=17
    addCell(12, 17, 0); addCell(19, 17, 0);

    // j=18
    addCell(12, 18, 0); addCell(19, 18, 0);

    // j=19
    addCell(3, 19, 0); addCell(6, 19, 0); addCell(12, 19, 0); addCell(19, 19, 0); addCell(25, 19, 0); addCell(28, 19, 0);

    // j=20
    addCell(3, 20, 0); addCell(7, 20, 0); addCell(11, 20, 0); addCell(20, 20, 0); addCell(24, 20, 0); addCell(28, 20, 0);

    // j=21
    addCell(3, 21, 0); addCell(4, 21, 0); addCell(28, 21, 0);

    // j=22
    addCell(3, 22, 0); addCell(4, 22, 0); addCell(27, 22, 0); addCell(28, 22, 0);

    // j=23
    addCell(3, 23, 0); addCell(4, 23, 0); addCell(5, 23, 0);
    addCell(26, 23, 0); addCell(27, 23, 0); addCell(28, 23, 0);

    // j=24
    addCell(3, 24, 0); addCell(4, 24, 0); addCell(5, 24, 0);
    addCell(26, 24, 0); addCell(27, 24, 0); addCell(28, 24, 0);

    // j=25
    addCell(3, 25, 0); addCell(4, 25, 0); addCell(5, 25, 0); addCell(6, 25, 0);
    addCell(25, 25, 0); addCell(26, 25, 0); addCell(27, 25, 0); addCell(28, 25, 0);

    // j=26
    addCell(3, 26, 0); addCell(4, 26, 0); addCell(5, 26, 0); addCell(6, 26, 0); addCell(7, 26, 0); addCell(8, 26, 0);
    addCell(23, 26, 0); addCell(24, 26, 0); addCell(25, 26, 0); addCell(26, 26, 0); addCell(27, 26, 0); addCell(28, 26, 0);

    // j=27
    addCell(3, 27, 0); addCell(4, 27, 0); addCell(5, 27, 0); addCell(6, 27, 0); addCell(7, 27, 0); addCell(8, 27, 0); addCell(9, 27, 0);
    addCell(22, 27, 0); addCell(23, 27, 0); addCell(24, 27, 0); addCell(25, 27, 0); addCell(26, 27, 0); addCell(27, 27, 0); addCell(28, 27, 0);

    // j=28
    addCell(3, 28, 0); addCell(4, 28, 0); addCell(5, 28, 0); addCell(6, 28, 0); addCell(7, 28, 0); addCell(8, 28, 0); addCell(9, 28, 0); addCell(10, 28, 0); addCell(11, 28, 0);
    addCell(20, 28, 0); addCell(21, 28, 0); addCell(22, 28, 0); addCell(23, 28, 0); addCell(24, 28, 0); addCell(25, 28, 0); addCell(26, 28, 0); addCell(27, 28, 0); addCell(28, 28, 0);

    //gris
    // j=4
    addCell(12, 4, 1); addCell(19, 4, 1);

    // j=15
    addCell(8, 15, 1); addCell(9, 15, 1);
    addCell(22, 15, 1); addCell(23, 15, 1);

    // j=16
    addCell(7, 16, 1); addCell(8, 16, 1); addCell(9, 16, 1); addCell(10, 16, 1); addCell(11, 16, 1);
    addCell(20, 16, 1); addCell(21, 16, 1); addCell(22, 16, 1); addCell(23, 16, 1); addCell(24, 16, 1);

    // j=17
    addCell(7, 17, 1); addCell(8, 17, 1); addCell(9, 17, 1); addCell(10, 17, 1); addCell(11, 17, 1);
    addCell(20, 17, 1); addCell(21, 17, 1); addCell(22, 17, 1); addCell(23, 17, 1); addCell(24, 17, 1);

    // j=18
    addCell(7, 18, 1); addCell(8, 18, 1); addCell(9, 18, 1); addCell(10, 18, 1); addCell(11, 18, 1);
    addCell(20, 18, 1); addCell(22, 18, 1); addCell(23, 18, 1); addCell(24, 18, 1);

    // j=19
    addCell(8, 19, 1); addCell(9, 19, 1); addCell(10, 19, 1);
    addCell(21, 19, 1); addCell(22, 19, 1); addCell(23, 19, 1); addCell(24, 19, 1);



    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.Use();

        // Dibujar celdas insertadas
        glBindVertexArray(VAO_cells);
        GLsizei nCellVerts = (GLsizei)(g_cellVertices.size() / 8);
        if (nCellVerts > 0) glDrawArrays(GL_TRIANGLES, 0, nCellVerts);
        glBindVertexArray(0);

        // Dibujar la cuadricula por encima
        glBindVertexArray(VAO_grid);
        glLineWidth(2.0f);
        GLsizei nGridVerts = (GLsizei)(g_gridVertices.size() / 8);
        glDrawArrays(GL_LINES, 0, nGridVerts);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

void resize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
