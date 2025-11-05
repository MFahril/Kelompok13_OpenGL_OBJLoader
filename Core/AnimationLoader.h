#ifndef ANIMATION_LOADER_H
#define ANIMATION_LOADER_H

#include <vector>
#include <string>
#include "ObjLoader.h"

class AnimationLoader {
private:
    std::vector<ObjLoader*> frames;
    int currentFrame;
    int totalFrames;
    float fps;
    float frameTime;
    float elapsedTime;
    bool isPlaying;
    bool loop;

public:
    AnimationLoader();
    ~AnimationLoader();
    
    // Load animation sequence
    bool loadAnimationSequence(const std::string& baseFilename, int startFrame, int endFrame);
    
    // Animation control
    void play();
    void pause();
    void stop();
    void setFPS(float fps);
    void setLoop(bool loop);
    void update(float deltaTime);
    
    // Drawing
    void draw();
    void drawWithMaterials();
    
    // Getters
    int getCurrentFrame() const { return currentFrame; }
    int getTotalFrames() const { return totalFrames; }
    bool isAnimationPlaying() const { return isPlaying; }
    bool hasFrames() const { return !frames.empty(); }
    float getFPS() const { return fps; }
};

#endif
