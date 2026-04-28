#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <cctype>

using namespace std;

const string ERROR_PREFIX = "ОШИБКА:";

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Polygon {
    vector<Point> points;
};

bool operator==(const Polygon& a, const Polygon& b);
double calculateArea(const Polygon& p);
bool parseLine(const string& line, Polygon& out);
bool isNumeric(const string& s);
void executeCommand(vector<Polygon>& polygons, const string& cmd, const string& arg);

struct CompareAreaLess {
    bool operator()(const Polygon& p, double refArea) const {
        return calculateArea(p) < refArea;
    }
};

struct AccumulateMaxSeq {
    Polygon ref;
    struct State { size_t current = 0, max = 0; };
    State operator()(const State& st, const Polygon& p) const {
        State next = st;
        if (p == ref) {
            next.current++;
            if (next.current > next.max) next.max = next.current;
        }
        else {
            next.current = 0;
        }
        return next;
    }
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    try {
        if (argc < 2) {
            cerr << ERROR_PREFIX << " Не указано имя файла. Использование: ./lab filename" << "\n";
            return EXIT_FAILURE;
        }

        vector<Polygon> polygons;
        ifstream file(argv[1]);
        if (!file.is_open()) {
            cerr << ERROR_PREFIX << " Не удалось открыть файл: " << string(argv[1]) << "\n";
            return EXIT_FAILURE;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            Polygon p;
            //игнорируем некорректные строки
            if (parseLine(line, p)) {
                polygons.push_back(p);
            }
        }
        file.close();

        string cmdLine;
        while (getline(cin, cmdLine)) {
            if (cmdLine.empty()) continue;

            istringstream iss(cmdLine);
            string cmd;
            iss >> cmd;

            string arg;
            getline(iss, arg);
            if (!arg.empty() && arg[0] == ' ') {
                arg.erase(0, 1);
            }

            if (cmd.empty()) {
                cout << "<INVALID COMMAND>" << "\n";
                continue;
            }

            try {
                executeCommand(polygons, cmd, arg);
            }
            catch (const exception&) {
                cout << "<INVALID COMMAND>" << "\n";
            }
        }

        return EXIT_SUCCESS;
    }
    catch (const exception& e) {
        cerr << ERROR_PREFIX << " " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size()) return false;
    return equal(a.points.begin(), a.points.end(), b.points.begin());
}

double calculateArea(const Polygon& p) {
    if (p.points.size() < 3) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < p.points.size(); ++i) {
        size_t j = (i + 1) % p.points.size();
        sum += (double)p.points[i].x * p.points[j].y - (double)p.points[i].y * p.points[j].x;
    }
    return std::abs(sum) / 2.0;
}

bool parseLine(const string& line, Polygon& out) {
    out.points.clear();
    if (line.empty()) return false;

    istringstream iss(line);
    int n;
    if (!(iss >> n)) return false;

    //проверка n >= 3
    if (n < 3) return false;

    for (int i = 0; i < n; ++i) {
        string token;
        if (!(iss >> token)) return false;

        //проверка формата (x;y)
        if (token.size() < 5 || token.front() != '(' || token.back() != ')') return false;
        string inner = token.substr(1, token.size() - 2);
        size_t semi = inner.find(';');
        if (semi == string::npos || semi == 0 || semi == inner.size() - 1) return false;

        try {
            Point p;
            p.x = stoi(inner.substr(0, semi));
            p.y = stoi(inner.substr(semi + 1));
            out.points.push_back(p);
        }
        catch (...) {
            return false;
        }
    }

    //проверка на лишние данные после последней вершины
    string dummy;
    if (iss >> dummy) return false;
    return true;
}

bool isNumeric(const string& s) {
    if (s.empty()) return false;
    size_t start = 0;
    if (s[0] == '-' || s[0] == '+') start = 1;
    if (start >= s.size()) return false;

    for (size_t i = start; i < s.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

void executeCommand(vector<Polygon>& polygons, const string& cmd, const string& arg) {
    auto checkEmpty = [&]() {
        if (polygons.empty()) throw logic_error("Empty collection");
        };

    if (cmd == "AREA") {
        if (arg == "ODD") {
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                [](double acc, const Polygon& p) { return acc + (p.points.size() % 2 != 0 ? calculateArea(p) : 0.0); });
            cout << fixed << setprecision(1) << sum << "\n";
        }
        else if (arg == "EVEN") {
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                [](double acc, const Polygon& p) { return acc + (p.points.size() % 2 == 0 ? calculateArea(p) : 0.0); });
            cout << fixed << setprecision(1) << sum << "\n";
        }
        else if (arg == "MEAN") {
            checkEmpty();
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                [](double acc, const Polygon& p) { return acc + calculateArea(p); });
            cout << fixed << setprecision(1) << sum / polygons.size() << "\n";
        }
        else if (!arg.empty() && isNumeric(arg)) {
            int n = stoi(arg);
            //проверка n >= 3 для числовых аргументов
            if (n < 3) throw invalid_argument("");
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                [n](double acc, const Polygon& p) { return acc + (p.points.size() == n ? calculateArea(p) : 0.0); });
            cout << fixed << setprecision(1) << sum << "\n";
        }
        else {
            throw invalid_argument("");
        }
    }
    else if (cmd == "MAX") {
        checkEmpty();
        if (arg == "AREA") {
            auto max_it = std::max_element(polygons.begin(), polygons.end(),
                [](const Polygon& a, const Polygon& b) { return calculateArea(a) < calculateArea(b); });
            cout << fixed << setprecision(1) << calculateArea(*max_it) << "\n";
        }
        else if (arg == "VERTEXES") {
            auto max_it = std::max_element(polygons.begin(), polygons.end(),
                [](const Polygon& a, const Polygon& b) { return a.points.size() < b.points.size(); });
            cout << max_it->points.size() << "\n";
        }
        else {
            throw invalid_argument("");
        }
    }
    else if (cmd == "MIN") {
        checkEmpty();
        if (arg == "AREA") {
            auto min_it = std::min_element(polygons.begin(), polygons.end(),
                [](const Polygon& a, const Polygon& b) { return calculateArea(a) < calculateArea(b); });
            cout << fixed << setprecision(1) << calculateArea(*min_it) << "\n";
        }
        else if (arg == "VERTEXES") {
            auto min_it = std::min_element(polygons.begin(), polygons.end(),
                [](const Polygon& a, const Polygon& b) { return a.points.size() < b.points.size(); });
            cout << min_it->points.size() << "\n";
        }
        else {
            throw invalid_argument("");
        }
    }
    else if (cmd == "COUNT") {
        if (arg == "ODD") {
            size_t cnt = std::count_if(polygons.begin(), polygons.end(),
                [](const Polygon& p) { return p.points.size() % 2 != 0; });
            cout << cnt << "\n";
        }
        else if (arg == "EVEN") {
            size_t cnt = std::count_if(polygons.begin(), polygons.end(),
                [](const Polygon& p) { return p.points.size() % 2 == 0; });
            cout << cnt << "\n";
        }
        else if (!arg.empty() && isNumeric(arg)) {
            int n = stoi(arg);
            //проверка n >= 3
            if (n < 3) throw invalid_argument("");
            size_t cnt = std::count_if(polygons.begin(), polygons.end(),
                [n](const Polygon& p) { return p.points.size() == n; });
            cout << cnt << "\n";
        }
        else {
            throw invalid_argument("");
        }
    }
    else if (cmd == "LESSAREA") {
        if (arg.empty()) throw invalid_argument("");
        Polygon ref;
        if (!parseLine(arg, ref)) throw invalid_argument("");
        double refArea = calculateArea(ref);

        auto pred = std::bind(CompareAreaLess(), std::placeholders::_1, refArea);
        size_t cnt = std::count_if(polygons.begin(), polygons.end(), pred);
        cout << cnt << "\n";
    }
    else if (cmd == "MAXSEQ") {
        if (arg.empty()) throw invalid_argument("");
        Polygon ref;
        if (!parseLine(arg, ref)) throw invalid_argument("");

        auto res = std::accumulate(polygons.begin(), polygons.end(), AccumulateMaxSeq::State{}, AccumulateMaxSeq{ ref });
        cout << res.max << "\n";
    }
    else {
        throw invalid_argument("");
    }
}
