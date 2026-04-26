#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std::placeholders;

const std::string ERROR_FILENAME_REQUIRED = "Error: filename required";
const std::string ERROR_CANNOT_OPEN_FILE = "Cannot open file: ";
const std::string INVALID_COMMAND = "<INVALID COMMAND>";

struct Point {
    int x = 0;
    int y = 0;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Polygon {
    std::vector<Point> points;

    int vertexCount() const {
        return static_cast<int>(points.size());
    }
    bool operator==(const Polygon& other) const {
        if (vertexCount() != other.vertexCount()) return false;
        if (points.empty()) return true;

        auto normalize = [](const std::vector<Point>& pts) {
            int minX = pts[0].x, minY = pts[0].y;
            for (const auto& p : pts) {
                if (p.x < minX) minX = p.x;
                if (p.y < minY) minY = p.y;
            }
            std::vector<Point> res;
            for (const auto& p : pts) {
                res.push_back({ p.x - minX, p.y - minY });
            }
            return res;
            };

        std::vector<Point> n1 = normalize(points);
        std::vector<Point> n2 = normalize(other.points);

        if (n1 == n2) return true;

        std::reverse(n2.begin(), n2.end());
        if (n1 == n2) return true;

        for (size_t start = 1; start < n1.size(); ++start) {
            std::vector<Point> rotated;
            for (size_t i = 0; i < n1.size(); ++i) {
                rotated.push_back(n1[(start + i) % n1.size()]);
            }
            if (rotated == n2) return true;
        }

        return false;
    }

    double area() const {
        double sum = 0.0;

        for (size_t i = 0; i < points.size(); ++i) {
            const Point& current = points[i];
            const Point& next = points[(i + 1) % points.size()];
            sum += static_cast<double>(current.x * next.y - next.x * current.y);
        }

        return std::abs(sum) / 2.0;
    }
};

std::istream& operator>>(std::istream& inputStream, Point& destination);
std::istream& operator>>(std::istream& inputStream, Polygon& destination);
bool isValidLine(const std::string& line);
std::vector<Polygon> readFiguresFromFile(const std::string& filename);
std::string extractPolygonString(std::istringstream& commandStream);
void processMaxCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream);
void processMinCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream);
void processCountCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream);
void processRmechoCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
void processSameCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream);
void processUserCommand(std::vector<Polygon>& polygons, const std::string& commandLine);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << ERROR_FILENAME_REQUIRED << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<Polygon> polygons;

    try {
        polygons = readFiguresFromFile(argv[1]);
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << std::fixed << std::setprecision(1);

    std::string userInput;

    while (std::getline(std::cin, userInput)) {
        processUserCommand(polygons, userInput);
    }

    return EXIT_SUCCESS;
}

std::istream& operator>>(std::istream& inputStream, Point& destination) {
    char openBracket;
    char semicolon;
    char closeBracket;

    inputStream >> openBracket >> destination.x >> semicolon >> destination.y >> closeBracket;

    if (openBracket != '(' || semicolon != ';' || closeBracket != ')') {
        inputStream.setstate(std::ios::failbit);
    }

    return inputStream;
}

std::istream& operator>>(std::istream& inputStream, Polygon& destination) {
    int vertexCount = 0;
    inputStream >> vertexCount;

    if (!inputStream || vertexCount <= 0) {
        inputStream.setstate(std::ios::failbit);
        return inputStream;
    }

    Polygon temporaryPolygon;

    for (int i = 0; i < vertexCount; ++i) {
        Point currentPoint;
        inputStream >> currentPoint;

        if (!inputStream) {
            return inputStream;
        }

        temporaryPolygon.points.push_back(currentPoint);
    }

    if (inputStream) {
        destination = temporaryPolygon;
    }

    return inputStream;
}

bool isValidLine(const std::string& line) {
    std::istringstream lineStream(line);
    int vertexCount  = 0;

    lineStream >> vertexCount;

    if (lineStream.fail() || vertexCount <= 0) {
        return false;
    }

    for (int i = 0; i < vertexCount; ++i) {
        Point testPoint;
        lineStream >> testPoint;

        if (lineStream.fail()) {
            return false;
        }
    }

    char leftoverCharacter;

    if (lineStream >> leftoverCharacter) {
        return false;
    }

    return true;
}

std::vector<Polygon> readFiguresFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        throw std::runtime_error(ERROR_CANNOT_OPEN_FILE + filename);
    }

    std::vector<std::string> allLines;
    std::string currentLine;

    while (std::getline(inputFile, currentLine)) {
        allLines.push_back(currentLine);
    }

    std::vector<std::string> validLines;

    std::copy_if(allLines.begin(), allLines.end(),
        std::back_inserter(validLines),
        isValidLine);

    std::vector<Polygon> polygons(validLines.size());

    std::transform(validLines.begin(), validLines.end(),
        polygons.begin(),
        [](const std::string& line) {
            Polygon polygon;
            std::istringstream(line) >> polygon;
            return polygon;
        });

    return polygons;
}

std::string extractPolygonString(std::istringstream& commandStream) {
    std::string polygonString;
    std::getline(commandStream, polygonString);

    size_t firstNonSpace = polygonString.find_first_not_of(" \t");

    if (firstNonSpace != std::string::npos) {
        return polygonString.substr(firstNonSpace);
    }

    return "";
}

void processMaxCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream) {
    std::string parameter;
    commandStream >> parameter;

    if (polygons.empty()) {
        std::cerr << INVALID_COMMAND << std::endl;
        return;
    }

    if (parameter == "AREA") {
        auto maxElement = std::max_element(polygons.begin(), polygons.end(),
            [](const Polygon& a, const Polygon& b) {
                return a.area() < b.area();
            });

        std::cout << std::fixed << std::setprecision(1) << maxElement->area() << std::endl;
    }
    else if (parameter == "VERTEXES") {
        auto maxElement = std::max_element(polygons.begin(), polygons.end(),
            [](const Polygon& a, const Polygon& b) {
                return a.vertexCount() < b.vertexCount();
            });

        std::cout << maxElement->vertexCount() << std::endl;
    }
    else {
        std::cerr << INVALID_COMMAND << std::endl;
    }
}

void processMinCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream) {
    std::string parameter;
    commandStream >> parameter;

    if (polygons.empty()) {
        std::cerr << INVALID_COMMAND << std::endl;
        return;
    }

    if (parameter == "AREA") {
        auto minElement = std::min_element(polygons.begin(), polygons.end(),
            [](const Polygon& a, const Polygon& b) {
                return a.area() < b.area();
            });

        std::cout << std::fixed << std::setprecision(1) << minElement->area() << std::endl;
    }
    else if (parameter == "VERTEXES") {
        auto minElement = std::min_element(polygons.begin(), polygons.end(),
            [](const Polygon& a, const Polygon& b) {
                return a.vertexCount() < b.vertexCount();
            });

        std::cout << minElement->vertexCount() << std::endl;
    }
    else {
        std::cerr << INVALID_COMMAND << std::endl;
    }
}

void processCountCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream) {
    std::string parameter;
    commandStream >> parameter;

    if (parameter.empty()) {
        std::cerr << INVALID_COMMAND << std::endl;
        return;
    }

    if (parameter == "EVEN") {
        int result = std::count_if(polygons.begin(), polygons.end(),
            [](const Polygon& polygon) {
                return polygon.vertexCount() % 2 == 0;
            });

        std::cout << result << std::endl;
    }
    else if (parameter == "ODD") {
        int result = std::count_if(polygons.begin(), polygons.end(),
            [](const Polygon& polygon) {
                return polygon.vertexCount() % 2 != 0;
            });

        std::cout << result << std::endl;
    }
    else if (std::all_of(parameter.begin(), parameter.end(), ::isdigit)) {
        int targetCount = std::stoi(parameter);

        int result = std::count_if(polygons.begin(), polygons.end(),
            [targetCount](const Polygon& polygon) {
                return polygon.vertexCount() == targetCount;
            });

        std::cout << result << std::endl;
    }
    else {
        std::cerr << INVALID_COMMAND << std::endl;
    }
}

void processRmechoCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
    std::string polygonString = extractPolygonString(commandStream);

    if (polygonString.empty()) {
        std::cerr << INVALID_COMMAND << std::endl;
        return;
    }

    Polygon targetPolygon;
    std::istringstream(polygonString) >> targetPolygon;

    size_t originalSize = polygons.size();

    auto newEnd = std::unique(polygons.begin(), polygons.end(),
        [&targetPolygon](const Polygon& first, const Polygon& second) {
            return first == targetPolygon && second == targetPolygon;
        });

    polygons.erase(newEnd, polygons.end());

    int removedCount = static_cast<int>(originalSize - polygons.size());
    std::cout << removedCount << std::endl;
}

void processSameCommand(const std::vector<Polygon>& polygons, std::istringstream& commandStream) {
    std::string polygonString = extractPolygonString(commandStream);

    if (polygonString.empty()) {
        std::cerr << INVALID_COMMAND << std::endl;
        return;
    }

    Polygon targetPolygon;
    std::istringstream(polygonString) >> targetPolygon;

    int result = std::count_if(polygons.begin(), polygons.end(),
        std::bind(std::equal_to<Polygon>(), _1, targetPolygon));

    std::cout << result << std::endl;
}

void processUserCommand(std::vector<Polygon>& polygons, const std::string& commandLine) {
    if (commandLine.empty()) {
        return;
    }

    std::istringstream commandStream(commandLine);
    std::string commandName;
    commandStream >> commandName;

    if (commandName.empty()) {
        return;
    }

    for (char& character : commandName) {
        character = std::toupper(character);
    }

    if (commandName == "MAX") {
        processMaxCommand(polygons, commandStream);
    }
    else if (commandName == "MIN") {
        processMinCommand(polygons, commandStream);
    }
    else if (commandName == "COUNT") {
        processCountCommand(polygons, commandStream);
    }
    else if (commandName == "RMECHO") {
        processRmechoCommand(polygons, commandStream);
    }
    else if (commandName == "SAME") {
        processSameCommand(polygons, commandStream);
    }
    else {
        std::cerr << INVALID_COMMAND << std::endl;
    }
}
