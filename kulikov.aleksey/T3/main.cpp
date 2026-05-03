#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Point
{
  int x;
  int y;
};

struct Polygon
{
  std::vector< Point > points;
};

std::istream& operator>>(std::istream& in, Point& point)
{
  char left = 0;
  char semi = 0;
  char right = 0;

  in >> left;
  if (!in || left != '(')
  {
    in.setstate(std::ios::failbit);
    return in;
  }

  in >> point.x >> semi >> point.y >> right;
  if (!in || semi != ';' || right != ')')
  {
    in.setstate(std::ios::failbit);
  }

  return in;
}

std::istream& operator>>(std::istream& in, Polygon& polygon)
{
  size_t count = 0;
  in >> count;

  if (!in || count < 3)
  {
    in.setstate(std::ios::failbit);
    return in;
  }

  std::vector< Point > points(count);

  try
  {
    std::generate_n(points.begin(), count,
      [&in]()
      {
        Point point;
        if (!(in >> point))
        {
          throw std::logic_error("wrong point");
        }
        return point;
      });
  }
  catch (...)
  {
    in.setstate(std::ios::failbit);
    return in;
  }

  polygon.points = points;
  return in;
}

bool operator==(const Point& lhs, const Point& rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator==(const Polygon& lhs, const Polygon& rhs)
{
  return lhs.points == rhs.points;
}

long long getCrossSumPart(const Point& first, const Point& second)
{
  return static_cast< long long >(first.x) * second.y -
         static_cast< long long >(second.x) * first.y;
}

double getArea(const Polygon& polygon)
{
  if (polygon.points.size() < 3)
  {
    return 0.0;
  }

  std::vector< Point > shifted = polygon.points;
  std::rotate(shifted.begin(), shifted.begin() + 1, shifted.end());

  long long doubleArea = std::inner_product(
    polygon.points.begin(),
    polygon.points.end(),
    shifted.begin(),
    0LL,
    std::plus< long long >(),
    getCrossSumPart
  );

  return std::abs(doubleArea) / 2.0;
}

long long getScalar(const Point& a, const Point& b, const Point& c)
{
  long long x1 = a.x - b.x;
  long long y1 = a.y - b.y;
  long long x2 = c.x - b.x;
  long long y2 = c.y - b.y;

  return x1 * x2 + y1 * y2;
}

bool hasRightAngle(const Polygon& polygon)
{
  const std::vector< Point >& pts = polygon.points;

  if (pts.size() < 3)
  {
    return false;
  }

  std::vector< size_t > indexes(pts.size());
  std::iota(indexes.begin(), indexes.end(), 0);

  return std::any_of(indexes.begin(), indexes.end(),
    [&pts](size_t i)
    {
      size_t prev = (i + pts.size() - 1) % pts.size();
      size_t next = (i + 1) % pts.size();

      return getScalar(pts[prev], pts[i], pts[next]) == 0;
    });
}

std::vector< Point > addPolygonPoints(std::vector< Point > result, const Polygon& polygon)
{
  std::copy(polygon.points.begin(), polygon.points.end(), std::back_inserter(result));
  return result;
}

bool isInsideFrame(const std::vector< Polygon >& polygons, const Polygon& polygon)
{
  if (polygons.empty() || polygon.points.empty())
  {
    return false;
  }

  std::vector< Point > allPoints = std::accumulate(
    polygons.begin(),
    polygons.end(),
    std::vector< Point >(),
    addPolygonPoints
  );

  if (allPoints.empty())
  {
    return false;
  }

  auto minMaxX = std::minmax_element(allPoints.begin(), allPoints.end(),
    [](const Point& lhs, const Point& rhs)
    {
      return lhs.x < rhs.x;
    });

  auto minMaxY = std::minmax_element(allPoints.begin(), allPoints.end(),
    [](const Point& lhs, const Point& rhs)
    {
      return lhs.y < rhs.y;
    });

  int minX = minMaxX.first->x;
  int maxX = minMaxX.second->x;
  int minY = minMaxY.first->y;
  int maxY = minMaxY.second->y;

  return std::all_of(polygon.points.begin(), polygon.points.end(),
    [minX, maxX, minY, maxY](const Point& point)
    {
      return point.x >= minX && point.x <= maxX &&
             point.y >= minY && point.y <= maxY;
    });
}

bool isNumber(const std::string& str)
{
  if (str.empty())
  {
    return false;
  }

  return std::all_of(str.begin(), str.end(),
    [](char ch)
    {
      return std::isdigit(static_cast< unsigned char >(ch));
    });
}

size_t toNumber(const std::string& str)
{
  if (!isNumber(str))
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  return static_cast< size_t >(std::stoul(str));
}

void printDouble(std::ostream& out, double value)
{
  out << std::fixed << std::setprecision(1) << value << '\n';
}

double getAreaSumByParity(const std::vector< Polygon >& polygons, bool even)
{
  return std::accumulate(polygons.begin(), polygons.end(), 0.0,
    [even](double result, const Polygon& polygon)
    {
      bool isEven = polygon.points.size() % 2 == 0;
      return result + (isEven == even ? getArea(polygon) : 0.0);
    });
}

double getAreaSumByVertexes(const std::vector< Polygon >& polygons, size_t vertexes)
{
  return std::accumulate(polygons.begin(), polygons.end(), 0.0,
    [vertexes](double result, const Polygon& polygon)
    {
      return result + (polygon.points.size() == vertexes ? getArea(polygon) : 0.0);
    });
}

void doArea(std::ostream& out, const std::vector< Polygon >& polygons, std::istream& in)
{
  std::string argument;
  in >> argument;

  if (!in)
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (argument == "EVEN")
  {
    printDouble(out, getAreaSumByParity(polygons, true));
  }
  else if (argument == "ODD")
  {
    printDouble(out, getAreaSumByParity(polygons, false));
  }
  else if (argument == "MEAN")
  {
    if (polygons.empty())
    {
      throw std::logic_error("<INVALID COMMAND>");
    }

    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
      [](double result, const Polygon& polygon)
      {
        return result + getArea(polygon);
      });

    printDouble(out, sum / polygons.size());
  }
  else
  {
    size_t vertexes = toNumber(argument);

    if (vertexes < 3)
    {
      throw std::logic_error("<INVALID COMMAND>");
    }

    printDouble(out, getAreaSumByVertexes(polygons, vertexes));
  }
}

void doMax(std::ostream& out, const std::vector< Polygon >& polygons, std::istream& in)
{
  if (polygons.empty())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  std::string argument;
  in >> argument;

  if (argument == "AREA")
  {
    auto it = std::max_element(polygons.begin(), polygons.end(),
      [](const Polygon& lhs, const Polygon& rhs)
      {
        return getArea(lhs) < getArea(rhs);
      });

    printDouble(out, getArea(*it));
  }
  else if (argument == "VERTEXES")
  {
    auto it = std::max_element(polygons.begin(), polygons.end(),
      [](const Polygon& lhs, const Polygon& rhs)
      {
        return lhs.points.size() < rhs.points.size();
      });

    out << it->points.size() << '\n';
  }
  else
  {
    throw std::logic_error("<INVALID COMMAND>");
  }
}

void doMin(std::ostream& out, const std::vector< Polygon >& polygons, std::istream& in)
{
  if (polygons.empty())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  std::string argument;
  in >> argument;

  if (argument == "AREA")
  {
    auto it = std::min_element(polygons.begin(), polygons.end(),
      [](const Polygon& lhs, const Polygon& rhs)
      {
        return getArea(lhs) < getArea(rhs);
      });

    printDouble(out, getArea(*it));
  }
  else if (argument == "VERTEXES")
  {
    auto it = std::min_element(polygons.begin(), polygons.end(),
      [](const Polygon& lhs, const Polygon& rhs)
      {
        return lhs.points.size() < rhs.points.size();
      });

    out << it->points.size() << '\n';
  }
  else
  {
    throw std::logic_error("<INVALID COMMAND>");
  }
}

void doCount(std::ostream& out, const std::vector< Polygon >& polygons, std::istream& in)
{
  std::string argument;
  in >> argument;

  if (!in)
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (argument == "EVEN")
  {
    out << std::count_if(polygons.begin(), polygons.end(),
      [](const Polygon& polygon)
      {
        return polygon.points.size() % 2 == 0;
      }) << '\n';
  }
  else if (argument == "ODD")
  {
    out << std::count_if(polygons.begin(), polygons.end(),
      [](const Polygon& polygon)
      {
        return polygon.points.size() % 2 != 0;
      }) << '\n';
  }
  else
  {
    size_t vertexes = toNumber(argument);

    if (vertexes < 3)
    {
      throw std::logic_error("<INVALID COMMAND>");
    }

    out << std::count_if(polygons.begin(), polygons.end(),
      [vertexes](const Polygon& polygon)
      {
        return polygon.points.size() == vertexes;
      }) << '\n';
  }
}

void doInFrame(std::ostream& out, const std::vector< Polygon >& polygons, std::istream& in)
{
  Polygon polygon;
  in >> polygon;

  if (!in)
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  out << (isInsideFrame(polygons, polygon) ? "<TRUE>" : "<FALSE>") << '\n';
}

void doRightShapes(std::ostream& out, const std::vector< Polygon >& polygons)
{
  out << std::count_if(polygons.begin(), polygons.end(),
    [](const Polygon& polygon)
    {
      return hasRightAngle(polygon);
    }) << '\n';
}

void processCommand(std::istream& in, std::ostream& out, std::vector< Polygon >& polygons)
{
  std::string line;
  std::getline(in, line);

  if (!in)
  {
    return;
  }

  std::istringstream commandInput(line);

  std::string command;
  commandInput >> command;

  try
  {
    if (!commandInput)
    {
      throw std::logic_error("<INVALID COMMAND>");
    }

    std::ostringstream result;

    if (command == "AREA")
    {
      doArea(result, polygons, commandInput);
    }
    else if (command == "MAX")
    {
      doMax(result, polygons, commandInput);
    }
    else if (command == "MIN")
    {
      doMin(result, polygons, commandInput);
    }
    else if (command == "COUNT")
    {
      doCount(result, polygons, commandInput);
    }
    else if (command == "INFRAME")
    {
      doInFrame(result, polygons, commandInput);
    }
    else if (command == "RIGHTSHAPES")
    {
      doRightShapes(result, polygons);
    }
    else
    {
      throw std::logic_error("<INVALID COMMAND>");
    }

    std::string extra;
    if (commandInput >> extra)
    {
      throw std::logic_error("<INVALID COMMAND>");
    }

    out << result.str();
  }
  catch (...)
  {
    out << "<INVALID COMMAND>" << '\n';
  }
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Error: filename is not specified\n";
    return EXIT_FAILURE;
  }

  std::ifstream input(argv[1]);

  if (!input)
  {
    std::cerr << "Error: file cannot be opened\n";
    return EXIT_FAILURE;
  }

  std::vector< Polygon > polygons;
  std::string line;

  while (std::getline(input, line))
  {
    std::istringstream lineInput(line);

    Polygon polygon;
    lineInput >> polygon;

    std::string extra;
    if (lineInput && !(lineInput >> extra))
    {
      polygons.push_back(polygon);
    }
  }

  while (std::cin)
  {
    processCommand(std::cin, std::cout, polygons);
  }

  return EXIT_SUCCESS;
}
