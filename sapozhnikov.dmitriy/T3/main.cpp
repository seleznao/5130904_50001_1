
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <iterator>
#include <climits>

using namespace std;
using namespace std::placeholders;

struct Point {
    int x, y;

    Point() : x(0), y(0) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

ostream& operator<<(ostream& os, const Point& p) {
    os << "(" << p.x << ";" << p.y << ")";
    return os;
}

istream& operator>>(istream& is, Point& p) {
    char ch1, ch2, ch3;
    int x, y;

    is >> ch1;
    if (ch1 != '(') {
        is.setstate(ios::failbit);
        return is;
    }

    is >> x >> ch2 >> y >> ch3;

    if (ch2 != ';' || ch3 != ')') {
        is.setstate(ios::failbit);
        return is;
    }

    p.x = x;
    p.y = y;
    return is;
}

int orientation(const Point& p, const Point& q, const Point& r) {
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;
    return (val > 0) ? 1 : 2;
}

bool onSegment(const Point& p, const Point& q, const Point& r) {
    return (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y));
}

bool doIntersect(const Point& p1, const Point& q1, const Point& p2, const Point& q2) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;
}

struct Polygon {
    vector<Point> points;

    int getVertexCount() const {
        return points.size();
    }

    double getArea() const {
        if (points.size() < 3) return 0.0;

        double area = 0.0;
        for (size_t i = 0; i < points.size(); ++i) {
            const Point& p1 = points[i];
            const Point& p2 = points[(i + 1) % points.size()];
            area += static_cast<double>(p1.x * p2.y - p2.x * p1.y);
        }
        return fabs(area) / 2.0;
    }

    bool operator==(const Polygon& other) const {
        if (points.size() != other.points.size()) return false;

        for (size_t shift = 0; shift < points.size(); ++shift) {
            bool match = true;
            for (size_t i = 0; i < points.size(); ++i) {
                if (points[i] != other.points[(i + shift) % points.size()]) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        return false;
    }

    bool isPointInside(const Point& p) const {
        int intersections = 0;
        for (size_t i = 0; i < points.size(); ++i) {
            const Point& p1 = points[i];
            const Point& p2 = points[(i + 1) % points.size()];

            if (p1.y == p2.y) continue;

            if (p.y > min(p1.y, p2.y) && p.y <= max(p1.y, p2.y)) {
                double xIntersect = static_cast<double>(p1.x) +
                    static_cast<double>(p.y - p1.y) * static_cast<double>(p2.x - p1.x) /
                    static_cast<double>(p2.y - p1.y);
                if (xIntersect >= static_cast<double>(p.x)) {
                    intersections++;
                }
            }
        }
        return intersections % 2 == 1;
    }

    bool intersects(const Polygon& other) const {
        for (size_t i = 0; i < points.size(); ++i) {
            const Point& p1 = points[i];
            const Point& p2 = points[(i + 1) % points.size()];

            for (size_t j = 0; j < other.points.size(); ++j) {
                const Point& q1 = other.points[j];
                const Point& q2 = other.points[(j + 1) % other.points.size()];

                if (doIntersect(p1, p2, q1, q2)) {
                    return true;
                }
            }
        }

        if (isPointInside(other.points[0]) || other.isPointInside(points[0])) {
            return true;
        }

        return false;
    }

    bool isRectangle() const {
        if (points.size() != 4) return false;

        for (size_t i = 0; i < 4; ++i) {
            const Point& p1 = points[i];
            const Point& p2 = points[(i + 1) % 4];
            const Point& p3 = points[(i + 2) % 4];

            int dx1 = p2.x - p1.x;
            int dy1 = p2.y - p1.y;
            int dx2 = p3.x - p2.x;
            int dy2 = p3.y - p2.y;

            if (dx1 * dx2 + dy1 * dy2 != 0) return false;
        }
        return true;
    }

    bool hasRightAngle() const {
        if (points.size() < 3) return false;

        for (size_t i = 0; i < points.size(); ++i) {
            const Point& p1 = points[i];
            const Point& p2 = points[(i + 1) % points.size()];
            const Point& p3 = points[(i + 2) % points.size()];

            int dx1 = p2.x - p1.x;
            int dy1 = p2.y - p1.y;
            int dx2 = p3.x - p2.x;
            int dy2 = p3.y - p2.y;

            if (dx1 * dx2 + dy1 * dy2 == 0) return true;
        }
        return false;
    }
};

ostream& operator<<(ostream& os, const Polygon& poly) {
    os << poly.points.size();
    for (const auto& p : poly.points) {
        os << " " << p;
    }
    return os;
}

vector<Polygon> polygons;

bool isEvenVertexCount(const Polygon& p) {
    return p.getVertexCount() % 2 == 0;
}

bool isOddVertexCount(const Polygon& p) {
    return p.getVertexCount() % 2 == 1;
}

double getArea(const Polygon& p) { return p.getArea(); }
int getVertexCount(const Polygon& p) { return p.getVertexCount(); }

void processArea(const vector<string>& tokens) {
    if (tokens.size() != 2) {
        cout << "<INVALID COMMAND>" << endl;
        return;
    }

    if (tokens[1] == "EVEN") {
        vector<Polygon> filtered;
        copy_if(polygons.begin(), polygons.end(), back_inserter(filtered),
            bind(isEvenVertexCount, _1));

        vector<double> areas(filtered.size());
        transform(filtered.begin(), filtered.end(), areas.begin(),
            bind(getArea, _1));

        double sum = accumulate(areas.begin(), areas.end(), 0.0);
        cout << fixed << setprecision(1) << sum << endl;
    }
    else if (tokens[1] == "ODD") {
        vector<Polygon> filtered;
        copy_if(polygons.begin(), polygons.end(), back_inserter(filtered),
            bind(isOddVertexCount, _1));

        vector<double> areas(filtered.size());
        transform(filtered.begin(), filtered.end(), areas.begin(),
            bind(getArea, _1));

        double sum = accumulate(areas.begin(), areas.end(), 0.0);
        cout << fixed << setprecision(1) << sum << endl;
    }
    else if (tokens[1] == "MEAN") {
        if (polygons.empty()) {
            cout << "<INVALID COMMAND>" << endl;
            return;
        }

        vector<double> areas(polygons.size());
        transform(polygons.begin(), polygons.end(), areas.begin(),
            bind(getArea, _1));

        double sum = accumulate(areas.begin(), areas.end(), 0.0);
        double mean = sum / static_cast<double>(polygons.size());
        cout << fixed << setprecision(1) << mean << endl;
    }
    else {
        try {
            int vertexCount = stoi(tokens[1]);
            if (vertexCount < 3) {
                cout << "<INVALID COMMAND>" << endl;
                return;
            }
            vector<Polygon> filtered;
            copy_if(polygons.begin(), polygons.end(), back_inserter(filtered),
                [vertexCount](const Polygon& p) { return p.getVertexCount() == vertexCount; });

            vector<double> areas(filtered.size());
            transform(filtered.begin(), filtered.end(), areas.begin(),
                bind(getArea, _1));

            double sum = accumulate(areas.begin(), areas.end(), 0.0);
            cout << fixed << setprecision(1) << sum << endl;
        }
        catch (...) {
            cout << "<INVALID COMMAND>" << endl;
        }
    }
}

void processMax(const vector<string>& tokens) {
    if (tokens.size() != 2 || polygons.empty()) {
        cout << "<INVALID COMMAND>" << endl;
        return;
    }

    if (tokens[1] == "AREA") {
        auto maxIt = max_element(polygons.begin(), polygons.end(),
            bind(less<double>(), bind(getArea, _1), bind(getArea, _2)));

        if (maxIt != polygons.end()) {
            cout << fixed << setprecision(1) << getArea(*maxIt) << endl;
        }
    }
    else if (tokens[1] == "VERTEXES") {
        auto maxIt = max_element(polygons.begin(), polygons.end(),
            bind(less<int>(), bind(getVertexCount, _1), bind(getVertexCount, _2)));

        if (maxIt != polygons.end()) {
            cout << getVertexCount(*maxIt) << endl;
        }
    }
    else {
        cout << "<INVALID COMMAND>" << endl;
    }
}

void processMin(const vector<string>& tokens) {
    if (tokens.size() != 2 || polygons.empty()) {
        cout << "<INVALID COMMAND>" << endl;
        return;
    }

    if (tokens[1] == "AREA") {
        auto minIt = min_element(polygons.begin(), polygons.end(),
            bind(less<double>(), bind(getArea, _1), bind(getArea, _2)));

        if (minIt != polygons.end()) {
            cout << fixed << setprecision(1) << getArea(*minIt) << endl;
        }
    }
    else if (tokens[1] == "VERTEXES") {
        auto minIt = min_element(polygons.begin(), polygons.end(),
            bind(less<int>(), bind(getVertexCount, _1), bind(getVertexCount, _2)));

        if (minIt != polygons.end()) {
            cout << getVertexCount(*minIt) << endl;
        }
    }
    else {
        cout << "<INVALID COMMAND>" << endl;
    }
}

void processCount(const vector<string>& tokens) {
    if (tokens.size() != 2) {
        cout << "<INVALID COMMAND>" << endl;
        return;
    }

    if (tokens[1] == "EVEN") {
        int count = count_if(polygons.begin(), polygons.end(), bind(isEvenVertexCount, _1));
        cout << count << endl;
    }
    else if (tokens[1] == "ODD") {
        int count = count_if(polygons.begin(), polygons.end(), bind(isOddVertexCount, _1));
        cout << count << endl;
    }
    else {
        try {
            int vertexCount = stoi(tokens[1]);
            if (vertexCount < 3) {
                cout << "<INVALID COMMAND>" << endl;
                return;
            }
            int count = count_if(polygons.begin(), polygons.end(),
                [vertexCount](const Polygon& p) { return p.getVertexCount() == vertexCount; });
            cout << count << endl;
        }
        catch (...) {
            cout << "<INVALID COMMAND>" << endl;
        }
    }
}

void processRects() {
    int count = count_if(polygons.begin(), polygons.end(),
        bind(&Polygon::isRectangle, _1));
    cout << count << endl;
}

void processRightShapes() {
    int count = count_if(polygons.begin(), polygons.end(),
        bind(&Polygon::hasRightAngle, _1));
    cout << count << endl;
}

void processIntersections(const vector<string>& tokens) {
    if (tokens.size() < 2) {
        cout << "<INVALID COMMAND>" << endl;
        return;
    }

    try {
        int vertexCount = stoi(tokens[1]);
        if (vertexCount < 3) {
            cout << "<INVALID COMMAND>" << endl;
            return;
        }
        if (static_cast<int>(tokens.size()) < 2 + vertexCount * 2) {
            cout << "<INVALID COMMAND>" << endl;
            return;
        }

        Polygon query;
        for (int i = 0; i < vertexCount; ++i) {
            stringstream ss(tokens[2 + i * 2] + " " + tokens[3 + i * 2]);
            Point p;
            ss >> p;
            if (ss.fail()) {
                cout << "<INVALID COMMAND>" << endl;
                return;
            }
            query.points.push_back(p);
        }

        int count = count_if(polygons.begin(), polygons.end(),
            [&query](const Polygon& p) { return query.intersects(p); });
        cout << count << endl;
    }
    catch (...) {
        cout << "<INVALID COMMAND>" << endl;
    }
}

void readPolygons(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        int vertexCount;
        ss >> vertexCount;

        if (ss.fail() || vertexCount < 3) continue;

        Polygon poly;
        bool valid = true;

        for (int i = 0; i < vertexCount; ++i) {
            Point p;
            ss >> p;
            if (ss.fail()) {
                valid = false;
                break;
            }
            poly.points.push_back(p);
        }

        string remaining;
        ss >> remaining;
        if (valid && remaining.empty() && poly.points.size() == static_cast<size_t>(vertexCount)) {
            polygons.push_back(poly);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Error: Filename not specified" << endl;
        return 1;
    }

    readPolygons(argv[1]);

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        vector<string> tokens;
        stringstream ss(line);
        string token;

        while (ss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) continue;

        string command = tokens[0];
        transform(command.begin(), command.end(), command.begin(), ::toupper);

        if (command == "AREA") {
            processArea(tokens);
        }
        else if (command == "MAX") {
            processMax(tokens);
        }
        else if (command == "MIN") {
            processMin(tokens);
        }
        else if (command == "COUNT") {
            processCount(tokens);
        }
        else if (command == "RECTS") {
            processRects();
        }
        else if (command == "RIGHTSHAPES") {
            processRightShapes();
        }
        else if (command == "INTERSECTIONS") {
            processIntersections(tokens);
        }
        else {
            cout << "<INVALID COMMAND>" << endl;
        }
    }

    return 0;
}
