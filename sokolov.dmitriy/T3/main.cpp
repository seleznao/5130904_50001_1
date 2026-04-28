// Variant 8
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>
#include "commandHandlers.hpp"
#include "ioHandle.hpp"
#include "polygon.hpp"

void handleCommand(const std::string &commandLine, std::vector<Polygon> &figures);

const std::string ERROR_FILE_NOT_OPEN = "ERROR: File not open\n";

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << ERROR_FILE_NOT_OPEN;
        return EXIT_FAILURE;
    }

    std::ifstream ifs;

    ifs.open(argv[1]);

    if (!ifs) {
        std::cerr << ERROR_FILE_NOT_OPEN;
        return EXIT_FAILURE;
    }

    std::vector<Polygon> figures;

    while (!ifs.eof()) {
        std::copy(
            std::istream_iterator<Polygon>(ifs),
            std::istream_iterator<Polygon>(),
            std::back_inserter(figures)
        );
        if (!ifs.eof() && ifs.fail()) {
            ifs.clear();
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    ifs.close();

    std::string command;

    while (std::getline(std::cin, command)) {
        handleCommand(command, figures);
    }


    return EXIT_SUCCESS;
}

void handleCommand(const std::string &commandLine, std::vector<Polygon> &figures) {

    std::istringstream iss(commandLine);

    std::string command = "";

    if (!(iss >> command)) return;

    if (command == "AREA") {
        handleArea(iss, figures);
    }
    else if (command == "MAX") {
        handleMax(iss, figures);
    }
    else if (command == "MIN") {
        handleMin(iss, figures);
    }
    else if (command == "COUNT") {
        handleCount(iss, figures);
    }
    else if (command == "RMECHO") {
        handleRMEcho(iss, figures);
    }
    else if (command == "INFRAME") {
        handleInFrame(iss, figures);
    }
    else {
        std::cout << "<INVALID COMMAND>\n";
    }
}
