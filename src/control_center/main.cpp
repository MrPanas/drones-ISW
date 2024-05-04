//
// Created by Patryk Mulica    on 02/05/24.
//

#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    string id = argv[1];
    cout << "Hello I'm CC" << id << "!!" << endl;


    // Call a drone
    string droneId = "1";
    string droneCommand = "./drone " + droneId;
    int res = system(droneCommand.c_str());

    if (res != 0) {
        cout << "Error calling drone" << endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
