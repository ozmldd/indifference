#pragma once
#include "timer.h"
class Animation {
    timer timer;
    int frameCount;

public:
    Animation() : timer(0), frameCount(0) {}
    Animation(int frameCount, float length) : frameCount(frameCount), timer(length) {}

    float getlength() const { return timer.getLength(); }

    int currentFrame() const {
        if (frameCount <= 0) {
            return 0;
        }
        float progress = timer.getTime() / timer.getLength();
        int frame = static_cast<int>(progress * frameCount);
        if (frame < 0) frame = 0;
        if (frame >= frameCount) frame = frameCount - 1;
        return frame;
    }

    void step(float deltaTime) {
        timer.step(deltaTime);
    }

    bool isDone() const { return timer.isTimeout(); }
};