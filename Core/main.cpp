#include <GL/glut.h>
#include <iostream>
#include <chrono>
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
    glutCreateWindow("OBJ Model Viewer - OpenGL");

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

    // Check if loading animation or single model
    std::string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        std::cout << "Enter OBJ file path (or animation base path like 'Models/Anim/Allanim'): ";
        std::getline(std::cin, filename);
    }
    
    // Check if this is an animation sequence
    std::cout << "\nLoad as animation sequence? (y/n): ";
    char response;
    std::cin >> response;
    
    if (response == 'y' || response == 'Y') {
        useAnimation = true;
        animation = new AnimationLoader();
        
        int startFrame = 0, endFrame = 20;
        std::cout << "Start frame (default 0): ";
        std::cin >> startFrame;
        std::cout << "End frame (default 20): ";
        std::cin >> endFrame;
        
        float fps = 24.0f;
        std::cout << "FPS (default 24): ";
        std::cin >> fps;
        
        if (animation->loadAnimationSequence(filename, startFrame, endFrame)) {
            animation->setFPS(fps);
            animation->setLoop(true);
            animation->play();
        } else {
            std::cerr << "Failed to load animation sequence." << std::endl;
            delete animation;
            animation = nullptr;
            useAnimation = false;
        }
    } else {
        // Load single OBJ file
        objModel = new ObjLoader();
        if (!objModel->loadObj(filename)) {
            std::cerr << "Failed to load OBJ file. Using default cube." << std::endl;
            delete objModel;
            objModel = nullptr;
        }
    }

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
    glEnable(GL_LIGHT0);
    // Disable GL_COLOR_MATERIAL so materials from MTL files work properly
    glDisable(GL_COLOR_MATERIAL);
    
    // Enable two-sided lighting to fix normal orientation issues
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Set light properties - balanced lighting
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat lightSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightPosition[] = {2.0f, 5.0f, 3.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Set material properties
    GLfloat matSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat matShininess[] = {50.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    
    // Disable back-face culling to see all faces
    glDisable(GL_CULL_FACE);
    
    // Enable normalization of normals after scaling
    glEnable(GL_NORMALIZE);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera position
    glTranslatef(0.0f, 0.0f, zoom);
    
    // Rotate the model
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    // Toggle wireframe mode
    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Toggle lighting
    if (enableLighting) {
        glEnable(GL_LIGHTING);
    } else {
        glDisable(GL_LIGHTING);
    }

    // Draw the model or animation
    if (useAnimation && animation && animation->hasFrames()) {
        // Update animation
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        animation->update(deltaTime);
        animation->drawWithMaterials();
    } else if (objModel) {
        if (objModel->hasMaterials()) {
            // Draw with materials if available
            objModel->drawWithMaterials();
        } else {
            // Fallback to simple colored rendering
            glColor3f(0.7f, 0.7f, 0.9f); // Light blue color
            objModel->draw();
        }
    } else {
        // Draw a default cube if no model loaded
        glColor3f(1.0f, 0.5f, 0.0f);
        glutSolidCube(1.0);
    }

    // Draw axes for reference (if enabled)
    if (showAxis) {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
            // X axis - Red
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(1.0f, 0.0f, 0.0f);
            
            // Y axis - Green
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 1.0f, 0.0f);
            
            // Z axis - Blue
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
        case 'w':
        case 'W':
            zoom += 0.5f;
            break;
        case 's':
        case 'S':
            zoom -= 0.5f;
            break;
        case 'l':
        case 'L':
            enableLighting = !enableLighting;
            std::cout << "Lighting: " << (enableLighting ? "ON" : "OFF") << std::endl;
            break;
        case 'f':
        case 'F':
            showWireframe = !showWireframe;
            std::cout << "Wireframe: " << (showWireframe ? "ON" : "OFF") << std::endl;
            break;
        case 'r':
        case 'R':
            angleX = 0.0f;
            angleY = 0.0f;
            zoom = -5.0f;
            std::cout << "View reset" << std::endl;
            break;
        case 'a':
        case 'A':
            showAxis = !showAxis;
            std::cout << "Axis: " << (showAxis ? "ON" : "OFF") << std::endl;
            break;
        case ' ': // SPACE - Play/Pause animation
            if (useAnimation && animation) {
                if (animation->isAnimationPlaying()) {
                    animation->pause();
                } else {
                    animation->play();
                }
            }
            break;
        case 'p':
        case 'P': // Play animation
            if (useAnimation && animation) {
                animation->play();
            }
            break;
        case 'o':
        case 'O': // Stop animation
            if (useAnimation && animation) {
                animation->stop();
            }
            break;
        case '+':
        case '=': // Increase FPS
            if (useAnimation && animation) {
                animation->setFPS(animation->getFPS() + 5.0f);
            }
            break;
        case '-':
        case '_': // Decrease FPS
            if (useAnimation && animation) {
                float newFPS = animation->getFPS() - 5.0f;
                if (newFPS > 0) animation->setFPS(newFPS);
            }
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
        } else {
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
