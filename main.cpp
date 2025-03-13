#include <iostream>
#include <cstring>

// Declare the eh_sender function from eh_sender.c
extern "C" {
    void eh_sender(); // Assuming it returns void - adjust return type if needed
    void eh_receiver(); // Assuming it returns void - adjust return type if needed
}

int main(int argc, char** argv) {
    std::cout << "Event Hub Tester!" << std::endl;
    
    // Call the eh_sender function     

    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " [send | receive | both]" << std::endl;
        return 1;
    }

    if (argc > 2) {
        std::cout << "Usage: " << argv[0] << " [send | receive | both]" << std::endl;
        return 1;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "send") == 0) {
            eh_sender();
            return 0;
        }

        if (strcmp(argv[1], "receive") == 0) {
            eh_receiver();
            return 0;
        }

        if (strcmp(argv[1], "both") == 0) {
            eh_sender();
            eh_receiver();
            return 0;
        }

        std::cout << "Usage: " << argv[0] << " [send | receive | both]" << std::endl;
        return 1;
    }
    return 0;
 
}