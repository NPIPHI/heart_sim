#include <iostream>
#include "App.h"
#include <vector>
#include <stdexcept>
int main() {
    glfwInit();
    try {
        App app{1400, 1000};
        app.run();
    } catch(std::exception & e){
        std::cout << e.what() << std::endl;
    }
    glfwTerminate();
}
