#include "AnimationLoader.h"
#include <iostream>
#include <sstream>
#include <iomanip>

AnimationLoader::AnimationLoader() 
    : currentFrame(0), totalFrames(0), fps(24.0f), 
      frameTime(1.0f/24.0f), elapsedTime(0.0f), 
      isPlaying(false), loop(true) {
}

AnimationLoader::~AnimationLoader() {
    for (auto frame : frames) {
        delete frame;
    }
    frames.clear();
}

bool AnimationLoader::loadAnimationSequence(const std::string& baseFilename, int startFrame, int endFrame) {
    // Clear existing frames
    for (auto frame : frames) {
        delete frame;
    }
    frames.clear();
    
    std::cout << "Loading animation sequence..." << std::endl;
    
    // Extract directory and base name from the filename
    size_t lastSlash = baseFilename.find_last_of("/\\");
    std::string directory = "";
    std::string baseName = baseFilename;
    
    if (lastSlash != std::string::npos) {
        directory = baseFilename.substr(0, lastSlash + 1);
        baseName = baseFilename.substr(lastSlash + 1);
    }
    
    // Remove the frame number and extension from base name if present
    // e.g., "Allanim0000.obj" -> "Allanim"
    size_t lastDigit = baseName.find_last_not_of("0123456789");
    if (lastDigit != std::string::npos) {
        size_t dotPos = baseName.find('.', lastDigit);
        if (dotPos != std::string::npos) {
            baseName = baseName.substr(0, lastDigit + 1);
        }
    }
    
    int loadedFrames = 0;
    
    // Load each frame
    for (int i = startFrame; i <= endFrame; i++) {
        std::ostringstream oss;
        oss << directory << baseName << std::setw(4) << std::setfill('0') << i << ".obj";
        std::string filename = oss.str();
        
        ObjLoader* frame = new ObjLoader();
        if (frame->loadObj(filename)) {
            frames.push_back(frame);
            loadedFrames++;
            std::cout << "  Loaded frame " << i << ": " << filename << std::endl;
        } else {
            delete frame;
            std::cerr << "  Failed to load frame " << i << ": " << filename << std::endl;
        }
    }
    
    totalFrames = frames.size();
    currentFrame = 0;
    
    if (totalFrames > 0) {
        std::cout << "Animation loaded: " << totalFrames << " frames" << std::endl;
        std::cout << "FPS: " << fps << std::endl;
        return true;
    } else {
        std::cerr << "Failed to load any animation frames!" << std::endl;
        return false;
    }
}

void AnimationLoader::play() {
    if (totalFrames > 0) {
        isPlaying = true;
        std::cout << "Animation playing" << std::endl;
    }
}

void AnimationLoader::pause() {
    isPlaying = false;
    std::cout << "Animation paused" << std::endl;
}

void AnimationLoader::stop() {
    isPlaying = false;
    currentFrame = 0;
    elapsedTime = 0.0f;
    std::cout << "Animation stopped" << std::endl;
}

void AnimationLoader::setFPS(float newFps) {
    if (newFps > 0) {
        fps = newFps;
        frameTime = 1.0f / fps;
        std::cout << "FPS set to: " << fps << std::endl;
    }
}

void AnimationLoader::setLoop(bool shouldLoop) {
    loop = shouldLoop;
    std::cout << "Loop: " << (loop ? "ON" : "OFF") << std::endl;
}

void AnimationLoader::update(float deltaTime) {
    if (!isPlaying || totalFrames == 0) {
        return;
    }
    
    elapsedTime += deltaTime;
    
    // Check if it's time to advance to the next frame
    if (elapsedTime >= frameTime) {
        elapsedTime -= frameTime;
        currentFrame++;
        
        // Handle looping
        if (currentFrame >= totalFrames) {
            if (loop) {
                currentFrame = 0;
            } else {
                currentFrame = totalFrames - 1;
                isPlaying = false;
                std::cout << "Animation finished" << std::endl;
            }
        }
    }
}

void AnimationLoader::draw() {
    if (totalFrames > 0 && currentFrame >= 0 && currentFrame < totalFrames) {
        frames[currentFrame]->draw();
    }
}

void AnimationLoader::drawWithMaterials() {
    if (totalFrames > 0 && currentFrame >= 0 && currentFrame < totalFrames) {
        if (frames[currentFrame]->hasMaterials()) {
            frames[currentFrame]->drawWithMaterials();
        } else {
            frames[currentFrame]->draw();
        }
    }
}
