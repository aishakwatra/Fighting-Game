#ifndef COUNTDOWN_TIMER_H
#define COUNTDOWN_TIMER_H

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class CountdownTimer {
public:
    CountdownTimer()
        : running(false), startTime(0.0f), remainingTime(60.0f), defaultTime(60.0f) {}

    void start() {
        if (!running) {
            running = true;
            startTime = static_cast<float>(glfwGetTime());
        }
    }

    void stop() {
        if (running) {
            running = false;
            float elapsed = static_cast<float>(glfwGetTime()) - startTime;
            remainingTime -= elapsed; // Update remaining time based on elapsed time
            if (remainingTime < 0.0f) {
                remainingTime = 0.0f;
            }
        }
    }

    void restart() {
        running = false;
        remainingTime = defaultTime;
    }

    void resetToDefault(float newTime = 60.0f) {
        running = false;
        defaultTime = newTime;
        remainingTime = defaultTime;
    }

    void update() {
        if (running) {
            float elapsed = static_cast<float>(glfwGetTime()) - startTime;
            remainingTime = defaultTime - elapsed;
            if (remainingTime <= 0.0f) {
                remainingTime = 0.0f;
                running = false;
            }
        }
    }

    std::string getFormattedTime() const {
        int totalSeconds = static_cast<int>(remainingTime);
        int seconds = totalSeconds % 60;

        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%02d", seconds);
        return std::string(buffer);
    }

    bool isRunning() const {
        return running;
    }

    bool isTimeUp() const {
        return remainingTime <= 0.0f;
    }

private:
    bool running;
    float startTime;      // When the timer started
    float remainingTime;  // Remaining time in seconds
    float defaultTime;    // Default countdown duration
};

#endif