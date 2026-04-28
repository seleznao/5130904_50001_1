#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
#include "commandHandlers.hpp"
#include "polygon.hpp"
#include "iofmtguard.hpp"
#include "isNumber.hpp"
#include "ioHandle.hpp"

void handleArea(std::istringstream &args, const std::vector<Polygon> &figures) {

    std::string arg = "";

    args >> arg;

    std::function<double(const double, const Polygon &)> operation = nullptr;

    if (arg == "EVEN") {
        operation = [] (const double sum, const Polygon &polygon) {
            return (polygon.points.size() % 2 == 0 ? sum + polygon.area() : sum);
        };
    }
    else if (arg == "ODD") {
        operation = [](const double sum, const Polygon &polygon) {
            return (polygon.points.size() % 2 == 1 ? sum + polygon.area() : sum);
        };
    }
    else if (arg == "MEAN") {

        if (figures.empty()) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        operation = [&figures](const double average, const Polygon &polygon) {
            return (average + polygon.area() / figures.size());
        };
    }
    else if (isNumber(arg)) {

        size_t nVertexes = static_cast<size_t>(std::stod(arg));

        if (nVertexes < 3) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        operation = [nVertexes](const double sum, const Polygon &polygon) {
            return (polygon.points.size() == nVertexes
                ? sum + polygon.area() : sum);
        };
    }
    else {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }

    double sumArea = std::accumulate(
        std::cbegin(figures),
        std::cend(figures),
        0.0, operation
    );

    {
        iofmtguard guard(std::cout);
        std::cout << std::fixed << std::setprecision(1) << sumArea << '\n';
    }
}

void handleMax(std::istringstream &args, const std::vector<Polygon> &figures) {

    std::string arg = "";

    args >> arg;

    if (arg == "AREA") {

        if (figures.empty()) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        double maxArea = std::max_element(
            std::cbegin(figures), std::cend(figures),
            [](const Polygon &pg1, const Polygon &pg2) {
                return pg1.area() < pg2.area();
            }
        )->area();

        {
            iofmtguard guard(std::cout);
            std::cout << std::fixed << std::setprecision(1) << maxArea << '\n';
        }
    }
    else if (arg == "VERTEXES") {

        if (figures.empty()) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        size_t maxVertexes = std::max_element(
            std::cbegin(figures), std::cend(figures),
            [](const Polygon &pg1, const Polygon &pg2) {
                return pg1.points.size() < pg2.points.size();
            }
        )->points.size();

        std::cout << maxVertexes << '\n';
    }
    else {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }
}

void handleMin(std::istringstream &args, const std::vector<Polygon> &figures) {

    std::string arg = "";

    args >> arg;

    if (arg == "AREA") {

        if (figures.empty()) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        double minArea = std::min_element(
            std::cbegin(figures), std::cend(figures),
            [](const Polygon &pg1, const Polygon &pg2) {
                return pg1.area() < pg2.area();
            }
        )->area();

        {
            iofmtguard guard(std::cout);
            std::cout << std::fixed << std::setprecision(1) << minArea << '\n';
        }
    }
    else if (arg == "VERTEXES") {

        if (figures.empty()) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        size_t minVertexes = std::min_element(
            std::cbegin(figures), std::cend(figures),
            [](const Polygon &pg1, const Polygon &pg2) {
                return pg1.points.size() < pg2.points.size();
            }
        )->points.size();

        std::cout << minVertexes << '\n';
    }
    else {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }
}

void handleCount(std::istringstream &args, const std::vector<Polygon> &figures) {

    std::string arg = "";

    args >> arg;

    std::function<size_t(const size_t, const Polygon &)> operation = nullptr;

    if (arg == "EVEN") {
        operation = [](const size_t num, const Polygon &polygon) {
            return (polygon.points.size() % 2 == 0 ? num + 1 : num);
        };
    }
    else if (arg == "ODD") {
        operation = [](const size_t num, const Polygon &polygon) {
            return (polygon.points.size() % 2 == 1 ? num + 1 : num);
        };
    }
    else if (isNumber(arg)) {

        size_t nVertexes = static_cast<size_t>(std::stod(arg));

        if (nVertexes < 3) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }

        operation = [nVertexes](const size_t num, const Polygon &polygon) {
            return (polygon.points.size()  == nVertexes ? num + 1 : num);
        };
    }
    else {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }

    size_t count = std::accumulate(
        std::cbegin(figures),
        std::cend(figures),
        static_cast<size_t>(0), operation
    );

    std::cout << count << '\n';
}

void handleRMEcho(std::istringstream &args, std::vector<Polygon> &figures) {

    Polygon arg{ {} };

    if (!(args >> arg)) {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }

    if (figures.empty() || arg.points.size() < 3) {
        std::cout << 0 << '\n';
        return;
    }

    std::vector<Polygon>::iterator i = figures.begin();

    size_t removedCount = 0;

    while (i != figures.end()) {

        i = std::adjacent_find(
            i,
            figures.end(),
            [&arg](const Polygon &polygon1, const Polygon &polygon2) {
                return (polygon1.points == arg.points &&
                    polygon2.points == arg.points);
            }
        );

        if (i == figures.end()) {
            break;
        }

        i = figures.erase(i);
        ++removedCount;
    }

    std::cout << removedCount << '\n';
}

void handleInFrame(std::istringstream &args, std::vector<Polygon> &figures) {

    Polygon arg{ {} };

    if (!(args >> arg)) {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }

    if (figures.empty() || arg.points.size() < 3) {
        std::cout << "<INVALID COMMAND>\n";
        return;
    }

    {
        std::string extra = "";
        if (args >> extra) {
            std::cout << "<INVALID COMMAND>\n";
            return;
        }
    }

    // MaxX MaxY MinX MinY
    std::array<int, 4> frame = std::accumulate(
        figures.cbegin(), figures.cend(),
        std::array<int, 4> {
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            std::numeric_limits<int>::max()
        },
        [](std::array<int, 4> frame, const Polygon &pg) {
            return std::accumulate(
                pg.points.cbegin(), pg.points.cend(),
                frame,
                [](std::array<int, 4> t, const Point &p) {
                    return std::array<int, 4>{
                        std::max(t[0], p.x),
                        std::max(t[1], p.y),
                        std::min(t[2], p.x),
                        std::min(t[3], p.y)
                    };
                }
            );
        }
    );

    // MaxX MaxY MinX MinY
    std::array<int, 4> argFrame = std::accumulate(
        arg.points.cbegin(), arg.points.cend(),
        std::array<int, 4> {
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            std::numeric_limits<int>::max()
        },
        [](std::array<int, 4> frame, const Point &p) {
            return std::array<int, 4>{
                std::max(frame[0], p.x),
                std::max(frame[1], p.y),
                std::min(frame[2], p.x),
                std::min(frame[3], p.y)
            };
        }
    );

    if (argFrame[0] <= frame[0] &&
        argFrame[1] <= frame[1] &&
        frame[2] <= argFrame[2] &&
        frame[3] <= argFrame[3]) {

        std::cout << "<TRUE>\n";
    }
    else {
        std::cout << "<FALSE>\n";
    }
}
