#include <GL/glut.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include "ObjLoader.h"
#include "AnimationLoader.h"

// Global variables
ObjLoader* objModel = nullptr;
AnimationLoader* animation = nullptr;
bool useAnimation = false;

float angleX = 0.0f;
float angleY = 0.0f;
float zoom = -5.0f;
int lastMouseX = 0;
int lastMouseY = 0;
bool isRotating = false;
bool showWireframe = false;
bool enableLighting = true;
bool showAxis = false;

// Posisi LIGHT3 (Point Light) global
GLfloat light3_Position[] = { 1.5f, 1.0f, 2.0f, 1.0f };

// Time tracking for animation
auto lastTime = std::chrono::high_resolution_clock::now();

// Function prototypes
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void initLighting();

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OBJ Model Viewer - Blinn-Phong (Legacy)"); // Judul diubah

    // Set callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Initialize lighting
    initLighting();

    // Parse command line arguments
    if (argc < 2) {
        std::cerr << "Error: No OBJ file specified!" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <objfile> [-a startFrame endFrame fps]" << std::endl;
        std::cerr << "Example: " << argv[0] << " Models/cube.obj" << std::endl;
        std::cerr << "Example: " << argv[0] << " Models/Anim/Allanim -a 0 20 24" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int startFrame = 0, endFrame = 60;
    float fps = 30.0f;

    // Check if animation flag is present
    if (argc > 2 && std::string(argv[2]) == "-a") {
        useAnimation = true;
        
        // Parse animation parameters if provided
        if (argc > 3) startFrame = std::atoi(argv[3]);
        if (argc > 4) endFrame = std::atoi(argv[4]);
        if (argc > 5) fps = std::atof(argv[5]);
        
        std::cout << "Loading animation: " << filename << std::endl;
        std::cout << "  Frames: " << startFrame << " to " << endFrame << std::endl;
        std::cout << "  FPS: " << fps << std::endl;
    }
    else {
        std::cout << "Loading static model: " << filename << std::endl;
    }

    // Load animation or static model
    if (useAnimation) {
        animation = new AnimationLoader();

        if (animation->loadAnimationSequence(filename, startFrame, endFrame)) {
            animation->setFPS(fps);
            animation->setLoop(true);
            animation->play();
            std::cout << "Animation ready!" << std::endl;
        }
        else {
            std::cerr << "Failed to load animation sequence." << std::endl;
            delete animation;
            animation = nullptr;
            useAnimation = false;
            return 1;
        }
    }
    else {
        objModel = new ObjLoader();
        if (!objModel->loadObj(filename)) {
            std::cerr << "Failed to load OBJ file." << std::endl;
            delete objModel;
            objModel = nullptr;
            return 1;
        }
    }

    // ... (Sisa kode Controls tidak berubah, masih sama) ...
    std::cout << "\n=== Controls ===" << std::endl;
    std::cout << "Mouse drag: Rotate model" << std::endl;
    std::cout << "W/S: Zoom in/out" << std::endl;
    std::cout << "L: Toggle lighting" << std::endl;
    std::cout << "F: Toggle wireframe" << std::endl;
    std::cout << "R: Reset view" << std::endl;
    if (useAnimation) {
        std::cout << "SPACE: Play/Pause animation" << std::endl;
        std::cout << "P: Play animation" << std::endl;
        std::cout << "O: Stop animation" << std::endl;
        std::cout << "+/-: Increase/Decrease FPS" << std::endl;
    }
    std::cout << "--- Point Light Controls ---" << std::endl;
    std::cout << "Numpad 4/6: Move Light X (Left/Right)" << std::endl;
    std::cout << "Numpad 8/2: Move Light Y (Up/Down)" << std::endl;
    std::cout << "Numpad 7/9: Move Light Z (Fwd/Back)" << std::endl;
    std::cout << "ESC: Exit" << std::endl;

    // Set up idle function for continuous animation updates
    if (useAnimation) {
        glutIdleFunc([]() { glutPostRedisplay(); });
    }

    // Start main loop
    glutMainLoop();

    if (objModel) delete objModel;
    if (animation) delete animation;
    return 0;
}

void initLighting() {
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // 1. Key light
    glEnable(GL_LIGHT1); // 2. Fill light
    glEnable(GL_LIGHT2); // 3. Rim light
    glEnable(GL_LIGHT3); // 4. Point light

    // Disable GL_COLOR_MATERIAL agar material dari MTL berfungsi
    glDisable(GL_COLOR_MATERIAL);

    // Enable two-sided lighting (jika bagian dalam vending machine terlihat)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // --- PENTING: AKTIVASI BLINN-PHONG ---
    // Ini memberitahu OpenGL untuk menggunakan kalkulasi "Half-Vector"
    // yang lebih akurat untuk specular (kilapan) berdasarkan posisi viewer.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    // --- Set global ambient light ---
    GLfloat globalAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // --- 1. KEY LIGHT (GL_LIGHT0) ---
    GLfloat light0_Diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light0_Specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat light0_Position[] = { 3.0f, 4.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_Position);

    // --- 2. FILL LIGHT (GL_LIGHT1) ---
    GLfloat light1_Diffuse[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light1_Specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light1_Position[] = { -3.0f, 2.0f, 4.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_Specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_Position);

    // --- 3. RIM LIGHT (GL_LIGHT2) ---
    GLfloat light2_Diffuse[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light2_Specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat light2_Position[] = { 0.0f, 2.0f, -8.0f, 1.0f };
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_Diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_Specular);
    glLightfv(GL_LIGHT2, GL_POSITION, light2_Position);

    // --- 4. POINT LIGHT (GL_LIGHT3) ---
    GLfloat light3_Diffuse[] = { 1.0f, 0.8f, 0.6f, 1.0f };
    GLfloat light3_Specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_Diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, light3_Specular);
    // (Posisi diatur di display() karena interaktif)

    // --- Attenuation (Falloff) untuk LIGHT3 ---
    GLfloat constant_attenuation = 1.0f;
    GLfloat linear_attenuation = 0.35f;
    GLfloat quadratic_attenuation = 0.44f;
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, constant_attenuation);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, linear_attenuation);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);


    // --- Set material properties default ---
    // Ini akan ditimpa oleh ObjLoader.cpp jika ada file .mtl
    GLfloat matSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat matShininess[] = { 80.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    glShadeModel(GL_SMOOTH);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera position
    glTranslatef(0.0f, 0.0f, zoom);

    // Rotate the model
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    // Update posisi LIGHT3 (Point Light) setiap frame
    glLightfv(GL_LIGHT3, GL_POSITION, light3_Position);

    // Toggle wireframe mode
    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Toggle lighting
    if (enableLighting) {
        glEnable(GL_LIGHTING);
    }
    else {
        glDisable(GL_LIGHTING);
    }

    // Draw the model or animation
    if (useAnimation && animation && animation->hasFrames()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        animation->update(deltaTime);
        animation->drawWithMaterials();
    }
    else if (objModel) {
        if (objModel->hasMaterials()) {
            objModel->drawWithMaterials(); // Fungsi ini sudah ada di ObjLoader.cpp
        }
        else {
            glColor3f(0.7f, 0.7f, 0.9f);
            objModel->draw();
        }
    }
    else {
        glColor3f(1.0f, 0.5f, 0.0f);
        glutSolidCube(1.0);
    }

    // Draw axes for reference (if enabled)
    if (showAxis) {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glEnd();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC
        exit(0);
        break;
    case 'w': case 'W':
        zoom += 0.5f;
        break;
    case 's': case 'S':
        zoom -= 0.5f;
        break;
    case 'l': case 'L':
        enableLighting = !enableLighting;
        std::cout << "Lighting: " << (enableLighting ? "ON" : "OFF") << std::endl;
        break;
    case 'f': case 'F':
        showWireframe = !showWireframe;
        std::cout << "Wireframe: " << (showWireframe ? "ON" : "OFF") << std::endl;
        break;
    case 'r': case 'R':
        angleX = 0.0f;
        angleY = 0.0f;
        zoom = -5.0f;
        std::cout << "View reset" << std::endl;
        break;
    case 'a': case 'A':
        showAxis = !showAxis;
        std::cout << "Axis: " << (showAxis ? "ON" : "OFF") << std::endl;
        break;

        // Kontrol animasi
    case ' ':
        if (useAnimation && animation) {
            if (animation->isAnimationPlaying()) animation->pause();
            else animation->play();
        }
        break;
    case 'p': case 'P':
        if (useAnimation && animation) animation->play();
        break;
    case 'o': case 'O':
        if (useAnimation && animation) animation->stop();
        break;
    case '+': case '=':
        if (useAnimation && animation) animation->setFPS(animation->getFPS() + 5.0f);
        break;
    case '-': case '_':
        if (useAnimation && animation) {
            float newFPS = animation->getFPS() - 5.0f;
            if (newFPS > 0) animation->setFPS(newFPS);
        }
        break;

        // Kontrol Numpad untuk LIGHT3
    case '4': // Numpad 4 - Geser Kiri (X-)
        light3_Position[0] -= 0.2f;
        std::cout << "Light Pos X: " << light3_Position[0] << std::endl;
        break;
    case '6': // Numpad 6 - Geser Kanan (X+)
        light3_Position[0] += 0.2f;
        std::cout << "Light Pos X: " << light3_Position[0] << std::endl;
        break;
    case '2': // Numpad 2 - Geser Bawah (Y-)
        light3_Position[1] -= 0.2f;
        std::cout << "Light Pos Y: " << light3_Position[1] << std::endl;
        break;
    case '8': // Numpad 8 - Geser Atas (Y+)
        light3_Position[1] += 0.2f;
        std::cout << "Light Pos Y: " << light3_Position[1] << std::endl;
        break;
    case '7': // Numpad 7 - Geser Maju (Z+)
        light3_Position[2] += 0.2f;
        std::cout << "Light Pos Z: " << light3_Position[2] << std::endl;
        break;
    case '9': // Numpad 9 - Geser Mundur (Z-)
        light3_Position[2] -= 0.2f;
        std::cout << "Light Pos Z: " << light3_Position[2] << std::endl;
        break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isRotating = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else {
            isRotating = false;
        }
    }
}

void motion(int x, int y) {
    if (isRotating) {
        angleY += (x - lastMouseX) * 0.5f;
        angleX += (y - lastMouseY) * 0.5f;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}