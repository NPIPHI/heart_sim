#include <iostream>
#include "App.h"
#include <vector>
int main() {
    glfwInit();
    {
        App app{1400, 1000};
        app.run();
    }
    glfwTerminate();
}
