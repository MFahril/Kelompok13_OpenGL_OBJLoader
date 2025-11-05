#include <GL/glut.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <algorithm> // For std::min/max
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
GLfloat light3_Position[] = { -0.5f, -0.2f, -0.2f, 1.0f };
// Posisi LIGHT5 (Point Light 2) global
GLfloat light5_Position[] = { 0.1f, 0.5f, -0.8f, 1.0f }; // Posisi default baru

// Variabel Global untuk Spotlight (LIGHT4)
GLfloat spotLightPosition[] = { 0.0f, 8.0f, 0.0f, 1.0f }; // Posisi di atas (y=8)
GLfloat spotLightDirection[] = { 0.0f, -1.0f, 0.0f };   // Arah lurus ke bawah (y=-1)
GLfloat spotLightColor[] = { 1.0f, 1.0f, 0.9f };         // Warna dasar (putih kekuningan)
float spotLightIntensity = 2.8f;                      // Intensitas awal (280%)

// --- Variabel BARU untuk Kontrol Lighting Real-time ---
float globalAmbientLevel = 0.05f; // Diambil dari lightAndMaterial2.cpp
float spotLightAngle = 5.0f;      // Diambil dari spotlight.cpp
float spotLightExponent = 30.0f;  // Diambil dari spotlight.cpp

// Variabel showLightMarker DIHAPUS

// Time tracking for animation
auto lastTime = std::chrono::high_resolution_clock::now();

// Function prototypes
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void initLighting();

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OBJ Model Viewer - Cinematic Lighting (Interactive)");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(specialKeyboard);

    glEnable(GL_DEPTH_TEST);

    initLighting();

    // Parse command line arguments
    if (argc < 2) {
        std::cerr << "Error: No OBJ file specified!" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <objfile> [-a startFrame endFrame fps]" << std::endl;
        std::cerr << "Example: " << argv[0] << " Models/cube.obj" << std::endl;
        std::cerr << "Example: " << argv[0] << " Models/Anim/Allanim -a 0 60 30" << std::endl;
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

    // --- Tampilan Kontrol Diperbarui ---
    std::cout << "\n=== View Controls ===" << std::endl;
    std::cout << "Mouse drag: Rotate model" << std::endl;
    std::cout << "W/S: Zoom in/out" << std::endl;
    std::cout << "L: Toggle lighting" << std::endl;
    std::cout << "F: Toggle wireframe" << std::endl;
    std::cout << "R: Reset view" << std::endl;
    std::cout << "A: Toggle axis" << std::endl;
    // Baris untuk tombol 'B' DIHAPUS
    if (useAnimation) {
        std::cout << "SPACE: Play/Pause animation" << std::endl;
        std::cout << "P: Play animation" << std::endl;
        std::cout << "O: Stop animation" << std::endl;
        std::cout << "]/[: Increase/Decrease FPS" << std::endl;
    }
    std::cout << "\n=== Lighting Controls ===" << std::endl;
    std::cout << "M/m: Increase/Decrease Global Ambient" << std::endl;
    std::cout << "--- Point Light 1 (GL_LIGHT3) ---" << std::endl;
    std::cout << "Numpad 4/6: Move Light X (Left/Right)" << std::endl;
    std::cout << "Numpad 8/2: Move Light Y (Up/Down)" << std::endl;
    std::cout << "Numpad 7/9: Move Light Z (Fwd/Back)" << std::endl;
    std::cout << "--- Point Light 2 (GL_LIGHT5) ---" << std::endl;
    std::cout << "Arrow Left/Right: Move Light X" << std::endl;
    std::cout << "Arrow Up/Down: Move Light Y" << std::endl;
    std::cout << "Home/End: Move Light Z" << std::endl;
    std::cout << "--- Spotlight (GL_LIGHT4) ---" << std::endl;
    std::cout << "+/-: Increase/Decrease Spotlight Intensity" << std::endl;
    std::cout << "T/t: Increase/Decrease Spotlight Focus (Exponent)" << std::endl;
    std::cout << "PgUp/PgDn: Increase/Decrease Spotlight Angle (Cutoff)" << std::endl;
    std::cout << "\nESC: Exit" << std::endl;
    // --- Selesai Tampilan Kontrol ---


    if (useAnimation) {
        glutIdleFunc([]() { glutPostRedisplay(); });
    }

    glutMainLoop();

    if (objModel) delete objModel;
    if (animation) delete animation;
    return 0;
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Key light
    glEnable(GL_LIGHT1); // Fill light
    glEnable(GL_LIGHT2); // Rim light
    glEnable(GL_LIGHT3); // Point light
    glEnable(GL_LIGHT4); // Spotlight
    glEnable(GL_LIGHT5); // Point light 2

    glDisable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // Blinn-Phong

    // --- 1. KEY LIGHT (GL_LIGHT0) ---
    GLfloat light0_Diffuse[] = { 0.6f, 0.6f, 0.7f, 1.0f };
    GLfloat light0_Specular[] = { 0.4f, 0.4f, 0.5f, 1.0f };
    GLfloat light0_Position[] = { 3.0f, 4.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_Position);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01f);


    // --- 2. FILL LIGHT (GL_LIGHT1) ---
    GLfloat light1_Diffuse[] = { 0.15f, 0.15f, 0.18f, 1.0f };
    GLfloat light1_Specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light1_Position[] = { -3.0f, 2.0f, 4.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_Specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_Position);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05f);


    // --- 3. RIM LIGHT (GL_LIGHT2) ---
    GLfloat light2_Diffuse[] = { 0.25f, 0.20f, 0.15f, 1.0f };
    GLfloat light2_Specular[] = { 0.2f, 0.15f, 0.1f, 1.0f };
    GLfloat light2_Position[] = { 0.0f, 2.0f, -8.0f, 1.0f };
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_Diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_Specular);
    glLightfv(GL_LIGHT2, GL_POSITION, light2_Position);
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.07f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.03f);


    // --- 4. POINT LIGHT (GL_LIGHT3) ---
    GLfloat light3_Diffuse[] = { 0.5f, 0.4f, 0.3f, 1.0f };
    GLfloat light3_Specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_Diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, light3_Specular);
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 0.8f);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.8f);


    // --- 5. SPOT LIGHT (GL_LIGHT4) ---
    glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, 0.08f);

    // --- 6. POINT LIGHT 2 (GL_LIGHT5) ---
    GLfloat light5_Diffuse[] = { 0.3f, 0.5f, 1.0f, 1.0f }; // Warna biru redup
    GLfloat light5_Specular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Tanpa specular
    glLightfv(GL_LIGHT5, GL_DIFFUSE, light5_Diffuse);
    glLightfv(GL_LIGHT5, GL_SPECULAR, light5_Specular);
    glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 0.8f);
    glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.8f);

    // --- Set material properties default ---
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
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // Update Global Ambient Light
    GLfloat currentGlobalAmbient[] = { globalAmbientLevel, globalAmbientLevel, globalAmbientLevel, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, currentGlobalAmbient);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, zoom);
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    // Update posisi LIGHT3 (Point Light)
    glLightfv(GL_LIGHT3, GL_POSITION, light3_Position);
    // Update posisi LIGHT5 (Point Light 2)
    glLightfv(GL_LIGHT5, GL_POSITION, light5_Position);

    // Update Spotlight (Posisi, Arah, Intensitas, Angle, Exponent)
    glLightfv(GL_LIGHT4, GL_POSITION, spotLightPosition);
    glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spotLightDirection);
    glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, spotLightAngle);
    glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, spotLightExponent);

    GLfloat currentSpotDiffuse[] = {
        spotLightColor[0] * spotLightIntensity,
        spotLightColor[1] * spotLightIntensity,
        spotLightColor[2] * spotLightIntensity,
        1.0f
    };
    GLfloat currentSpotSpecular[] = {
        spotLightColor[0] * spotLightIntensity * 0.5f,
        spotLightColor[1] * spotLightIntensity * 0.5f,
        spotLightColor[2] * spotLightIntensity * 0.5f,
        1.0f
    };

    glLightfv(GL_LIGHT4, GL_DIFFUSE, currentSpotDiffuse);
    glLightfv(GL_LIGHT4, GL_SPECULAR, currentSpotSpecular);

    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (enableLighting) {
        glEnable(GL_LIGHTING);
    }
    else {
        glDisable(GL_LIGHTING);
    }

    // --- Gambar Model ---
    if (useAnimation && animation && animation->hasFrames()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        animation->update(deltaTime);
        animation->drawWithMaterials();
    }
    else if (objModel) {
        if (objModel->hasMaterials()) {
            objModel->drawWithMaterials();
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

    // --- BLOK ALAT BANTU (HELPER) ---

    // Matikan pencahayaan SATU KALI untuk semua alat bantu
    glDisable(GL_LIGHTING);

    // Blok "if (showLightMarker)" DIHAPUS

    // Gambar Sumbu (jika aktif)
    if (showAxis) {
        glBegin(GL_LINES);
        // Sumbu X (Merah)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        // Sumbu Y (Hijau)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        // Sumbu Z (Biru)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glEnd();
    }

    // Kembalikan status lighting ke status awal (yang diatur di atas)
    if (enableLighting) {
        glEnable(GL_LIGHTING);
    }
    // --- SELESAI BLOK ALAT BANTU ---


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

        // Case untuk 'b' / 'B' DIHAPUS

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

    case '[':
        if (useAnimation && animation) {
            float newFPS = animation->getFPS() - 5.0f;
            animation->setFPS(std::max(1.0f, newFPS));
        }
        break;
    case ']':
        if (useAnimation && animation) animation->setFPS(animation->getFPS() + 5.0f);
        break;

        // --- Kontrol Spotlight Intensity ---
    case '-':
    case '_':
        spotLightIntensity -= 0.1f;
        if (spotLightIntensity < 0.0f) spotLightIntensity = 0.0f;
        std::cout << "Spotlight Intensity: " << spotLightIntensity << std::endl;
        break;
    case '+':
    case '=':
        spotLightIntensity += 0.1f;
        std::cout << "Spotlight Intensity: " << spotLightIntensity << std::endl;
        break;

        // --- Kontrol Global Ambient ---
    case 'm':
        if (globalAmbientLevel > 0.0f) globalAmbientLevel -= 0.01f;
        std::cout << "Global Ambient: " << globalAmbientLevel << std::endl;
        break;
    case 'M':
        if (globalAmbientLevel < 1.0f) globalAmbientLevel += 0.01f;
        std::cout << "Global Ambient: " << globalAmbientLevel << std::endl;
        break;

        // --- Kontrol Spotlight Exponent ---
    case 't':
        if (spotLightExponent > 0.0f) spotLightExponent -= 1.0f;
        std::cout << "Spotlight Exponent: " << spotLightExponent << std::endl;
        break;
    case 'T':
        if (spotLightExponent < 128.0f) spotLightExponent += 1.0f;
        std::cout << "Spotlight Exponent: " << spotLightExponent << std::endl;
        break;

        // --- Kontrol Numpad Point Light (LIGHT3) ---
    case '4': // Numpad 4
        light3_Position[0] -= 0.2f;
        std::cout << "Light3 Pos X: " << light3_Position[0] << std::endl;
        break;
    case '6': // Numpad 6
        light3_Position[0] += 0.2f;
        std::cout << "Light3 Pos X: " << light3_Position[0] << std::endl;
        break;
    case '2': // Numpad 2
        light3_Position[1] -= 0.2f;
        std::cout << "Light3 Pos Y: " << light3_Position[1] << std::endl;
        break;
    case '8': // Numpad 8
        light3_Position[1] += 0.2f;
        std::cout << "Light3 Pos Y: " << light3_Position[1] << std::endl;
        break;
    case '7': // Numpad 7
        light3_Position[2] += 0.2f;
        std::cout << "Light3 Pos Z: " << light3_Position[2] << std::endl;
        break;
    case '9': // Numpad 9
        light3_Position[2] -= 0.2f;
        std::cout << "Light3 Pos Z: " << light3_Position[2] << std::endl;
        break;
    }
    glutPostRedisplay();
}

void specialKeyboard(int key, int x, int y) {
    switch (key) {
        // Kontrol Spotlight Angle
    case GLUT_KEY_PAGE_UP:
        if (spotLightAngle < 90.0f) spotLightAngle += 1.0f;
        std::cout << "Spotlight Angle: " << spotLightAngle << std::endl;
        break;
    case GLUT_KEY_PAGE_DOWN:
        if (spotLightAngle > 0.0f) spotLightAngle -= 1.0f;
        std::cout << "Spotlight Angle: " << spotLightAngle << std::endl;
        break;

        // Kontrol Point Light 2 (GL_LIGHT5)
    case GLUT_KEY_LEFT:
        light5_Position[0] -= 0.2f;
        std::cout << "Light5 Pos X: " << light5_Position[0] << std::endl;
        break;
    case GLUT_KEY_RIGHT:
        light5_Position[0] += 0.2f;
        std::cout << "Light5 Pos X: " << light5_Position[0] << std::endl;
        break;
    case GLUT_KEY_DOWN:
        light5_Position[1] -= 0.2f;
        std::cout << "Light5 Pos Y: " << light5_Position[1] << std::endl;
        break;
    case GLUT_KEY_UP:
        light5_Position[1] += 0.2f;
        std::cout << "Light5 Pos Y: " << light5_Position[1] << std::endl;
        break;
    case GLUT_KEY_HOME: // Untuk Z+
        light5_Position[2] += 0.2f;
        std::cout << "Light5 Pos Z: " << light5_Position[2] << std::endl;
        break;
    case GLUT_KEY_END: // Untuk Z-
        light5_Position[2] -= 0.2f;
        std::cout << "Light5 Pos Z: " << light5_Position[2] << std::endl;
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