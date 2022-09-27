#include "app.h"
#include <iostream>

using namespace std;

int main() {
    char hostname[] = "assignment.jigentec.com";
    char port[] = "49152";
    App* app = new App(hostname, port);
    app->run();

    delete app;
    return 0;
}