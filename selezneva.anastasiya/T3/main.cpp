#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

using namespace std::placeholders;

const std::string ERROR_FILENAME_REQUIRED = "Error: filename required";
const std::string ERROR_CANNOT_OPEN_FILE = "Cannot open file: ";
const std::string INVALID_COMMAND = "<INVALID COMMAND>";

namespace selezneva {

    struct Point {
        int x = 0;
        int y = 0;
    };

    struct Polygon {
        std::vector<Point> points;
        int vertexCount() const;
        double area() const;
    };

    std::istream& operator>>(std::istream& inputStream, Point& destination);
    std::istream& operator>>(std::istream& inputStream, Polygon& destination);
    bool isValidLine(const std::string& line);
    std::vector<Polygon> readFiguresFromFile(const std::string& filename);
    Polygon normalizePolygon(const Polygon& polygon);
    bool isSameShape(const Polygon& a, const Polygon& b);
    std::string extractPolygonString(std::istringstream& commandStream);
    double getAreaSumForVertexCount(const std::vector<Polygon>& polygons, int targetCount);
    void processMaxCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
    void processMinCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
    void processCountCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
    void processAreaCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
    void processRmechoCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
    void processSameCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream);
    void processUserCommand(std::vector<Polygon>& polygons, const std::string& commandLine);

}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << ERROR_FILENAME_REQUIRED << "\n";
        return EXIT_FAILURE;
    }

    std::vector<selezneva::Polygon> polygons;

    try {
        polygons = selezneva::readFiguresFromFile(argv[1]);
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << std::fixed << std::setprecision(1);

    std::string userInput;
    while (std::getline(std::cin, userInput)) {
        selezneva::processUserCommand(polygons, userInput);
    }

    return EXIT_SUCCESS;
}

namespace selezneva {

    int Polygon::vertexCount() const {
        return static_cast<int>(points.size());
    }

    double Polygon::area() const {
        double sum = 0.0;
        for (size_t i = 0; i < points.size(); ++i) {
            const Point& current = points[i];
            const Point& next = points[(i + 1) % points.size()];
            sum += static_cast<double>(current.x * next.y - next.x * current.y);
        }
        return std::abs(sum) / 2.0;
    }

    std::istream& operator>>(std::istream& inputStream, Point& destination) {
        char openBracket = '\0';
        char semicolon = '\0';
        char closeBracket = '\0';
        int xCoordinate = 0;
        int yCoordinate = 0;

        inputStream >> openBracket >> xCoordinate >> semicolon >> yCoordinate >> closeBracket;

        if (openBracket == '(' && semicolon == ';' && closeBracket == ')') {
            destination.x = xCoordinate;
            destination.y = yCoordinate;
        }
        else {
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
            Point currentPoint = { 0, 0 };
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
        size_t firstChar = line.find_first_not_of(" \t");
        if (firstChar == std::string::npos) {
            return false;
        }

        std::string trimmedLine = line.substr(firstChar);
        std::istringstream lineStream(trimmedLine);

        int vertexCount = 0;
        lineStream >> vertexCount;

        if (lineStream.fail() || vertexCount < 3) {
            return false;
        }

        for (int i = 0; i < vertexCount; ++i) {
            Point testPoint = { 0, 0 };
            lineStream >> testPoint;
            if (lineStream.fail()) {
                return false;
            }
        }

        char leftover;
        if (lineStream >> leftover) {
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
            if (currentLine.find_first_not_of(" \t") == std::string::npos) {
                continue;
            }
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

    Polygon normalizePolygon(const Polygon& polygon) {
        if (polygon.points.empty()) {
            return polygon;
        }

        int minX = std::min_element(polygon.points.begin(), polygon.points.end(),
            [](const Point& a, const Point& b) { return a.x < b.x; })->x;

        int minY = std::min_element(polygon.points.begin(), polygon.points.end(),
            [](const Point& a, const Point& b) { return a.y < b.y; })->y;

        Polygon result;
        result.points.resize(polygon.points.size());

        std::transform(polygon.points.begin(), polygon.points.end(),
            result.points.begin(),
            [minX, minY](const Point& p) {
                return Point{ p.x - minX, p.y - minY };
            });

        return result;
    }

    bool isSameShape(const Polygon& a, const Polygon& b) {
        if (a.vertexCount() != b.vertexCount()) {
            return false;
        }
        if (a.points.empty()) {
            return true;
        }

        Polygon normA = normalizePolygon(a);
        Polygon normB = normalizePolygon(b);

        auto pointEqual = [](const Point& p1, const Point& p2) {
            return p1.x == p2.x && p1.y == p2.y;
            };

        if (std::equal(normA.points.begin(), normA.points.end(),
            normB.points.begin(), pointEqual)) {
            return true;
        }

        std::reverse(normB.points.begin(), normB.points.end());
        return std::equal(normA.points.begin(), normA.points.end(),
            normB.points.begin(), pointEqual);
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

    double getAreaSumForVertexCount(const std::vector<Polygon>& polygons, int targetCount) {
        std::vector<double> areas(polygons.size());
        std::transform(polygons.begin(), polygons.end(), areas.begin(),
            [](const Polygon& p) { return p.area(); });

        std::vector<int> counts(polygons.size());
        std::transform(polygons.begin(), polygons.end(), counts.begin(),
            [](const Polygon& p) { return p.vertexCount(); });

        std::vector<bool> mask(polygons.size());
        std::transform(counts.begin(), counts.end(), mask.begin(),
            [targetCount](int c) { return c == targetCount; });

        return std::inner_product(areas.begin(), areas.end(), mask.begin(), 0.0,
            std::plus<double>(),
            [](double area, bool match) { return match ? area : 0.0; });
    }

    void processMaxCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
        std::string parameter;
        commandStream >> parameter;

        if (polygons.empty()) {
            std::cout << INVALID_COMMAND << std::endl;
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
            std::cout << INVALID_COMMAND << std::endl;
        }
    }

    void processMinCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
        std::string parameter;
        commandStream >> parameter;

        if (polygons.empty()) {
            std::cout << INVALID_COMMAND << std::endl;
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
            std::cout << INVALID_COMMAND << std::endl;
        }
    }

    void processCountCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
        std::string parameter;
        commandStream >> parameter;

        if (parameter.empty()) {
            std::cout << INVALID_COMMAND << std::endl;
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
            if (targetCount <= 2) {
                std::cout << INVALID_COMMAND << std::endl;
                return;
            }
            int result = std::count_if(polygons.begin(), polygons.end(),
                [targetCount](const Polygon& polygon) {
                    return polygon.vertexCount() == targetCount;
                });
            std::cout << result << std::endl;
        }
        else {
            std::cout << INVALID_COMMAND << std::endl;
        }
    }

    void processAreaCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
        std::string parameter;
        commandStream >> parameter;

        if (parameter.empty()) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        if (parameter == "EVEN") {
            std::vector<double> areas(polygons.size());
            std::transform(polygons.begin(), polygons.end(), areas.begin(),
                [](const Polygon& p) { return p.area(); });

            std::vector<int> counts(polygons.size());
            std::transform(polygons.begin(), polygons.end(), counts.begin(),
                [](const Polygon& p) { return p.vertexCount(); });

            std::vector<bool> mask(polygons.size());
            std::transform(counts.begin(), counts.end(), mask.begin(),
                [](int c) { return c % 2 == 0; });

            double sum = std::inner_product(areas.begin(), areas.end(), mask.begin(), 0.0,
                std::plus<double>(),
                [](double area, bool match) { return match ? area : 0.0; });

            std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
        }
        else if (parameter == "ODD") {
            std::vector<double> areas(polygons.size());
            std::transform(polygons.begin(), polygons.end(), areas.begin(),
                [](const Polygon& p) { return p.area(); });

            std::vector<int> counts(polygons.size());
            std::transform(polygons.begin(), polygons.end(), counts.begin(),
                [](const Polygon& p) { return p.vertexCount(); });

            std::vector<bool> mask(polygons.size());
            std::transform(counts.begin(), counts.end(), mask.begin(),
                [](int c) { return c % 2 != 0; });

            double sum = std::inner_product(areas.begin(), areas.end(), mask.begin(), 0.0,
                std::plus<double>(),
                [](double area, bool match) { return match ? area : 0.0; });

            std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
        }
        else if (parameter == "MEAN") {
            if (polygons.empty()) {
                std::cout << INVALID_COMMAND << std::endl;
                return;
            }
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                [](double acc, const Polygon& p) { return acc + p.area(); });
            double mean = sum / polygons.size();
            std::cout << std::fixed << std::setprecision(1) << mean << std::endl;
        }else if (std::all_of(parameter.begin(), parameter.end(), ::isdigit)) {
            int targetCount = std::stoi(parameter);
            if (targetCount < 3) {
                std::cout << INVALID_COMMAND << std::endl;
                return;
            }
            double sum = getAreaSumForVertexCount(polygons, targetCount);
            std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
        }
        else {
            std::cout << INVALID_COMMAND << std::endl;
        }
    }

    void processRmechoCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
        std::string polygonString = extractPolygonString(commandStream);

        if (polygonString.empty()) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        std::istringstream polyStream(polygonString);
        Polygon targetPolygon;
        polyStream >> targetPolygon;

        if (polyStream.fail() || targetPolygon.vertexCount() < 3) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        char leftover;
        if (polyStream >> leftover) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        size_t originalSize = polygons.size();

        auto newEnd = std::unique(polygons.begin(), polygons.end(),
            [&targetPolygon](const Polygon& first, const Polygon& second) {
                return isSameShape(first, targetPolygon) && isSameShape(second, targetPolygon);
            });

        polygons.erase(newEnd, polygons.end());

        int removedCount = static_cast<int>(originalSize - polygons.size());
        std::cout << removedCount << std::endl;
    }

    void processSameCommand(std::vector<Polygon>& polygons, std::istringstream& commandStream) {
        std::string polygonString = extractPolygonString(commandStream);

        if (polygonString.empty()) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        std::istringstream polyStream(polygonString);
        Polygon targetPolygon;
        polyStream >> targetPolygon;

        if (polyStream.fail() || targetPolygon.vertexCount() < 3) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        char leftover;
        if (polyStream >> leftover) {
            std::cout << INVALID_COMMAND << std::endl;
            return;
        }

        int result = std::count_if(polygons.begin(), polygons.end(),
            [&targetPolygon](const Polygon& polygon) {
                return isSameShape(polygon, targetPolygon);
            });

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
        else if (commandName == "AREA") {
            processAreaCommand(polygons, commandStream);
        }
        else if (commandName == "RMECHO") {
            processRmechoCommand(polygons, commandStream);
        }
        else if (commandName == "SAME") {
            processSameCommand(polygons, commandStream);
        }
        else {
            std::cout << INVALID_COMMAND << std::endl;
        }
    }

}
