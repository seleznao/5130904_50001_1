#include "Graph.h"
#include "IoFmtGuard.h"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <stdexcept>


const std::string ERROR_INVALID_COUNT = "Неверное число вершин";
const std::string ERROR_NOT_FOUND = "Ребро не найдено";
const std::string ERROR_NOT_EXIST = "Ребро не существует";
const std::string ERROR_WEIGHT = "Вес должен быть > 0";
const std::string ERROR_SORTING = "Топологическая сортировка для ориентированных графов!";
const std::string ERROR_CYCLE = "Обнаружен цикл!";
const std::string ERROR_OUT_OF_RANGE = "Неверный индекс вершины";

std::ostream & operator<<(std::ostream & os, const Graph & graph) {
    std::ostream::sentry sentry(os);
    if (sentry) {
        IoFmtGuard guard(os);
        os << "Граф (" << (graph.oriented_ ? "ориент." : "неориент.")
            << "): вершин " << graph.vertexCount_ << ", ребер " << graph.edgeCount_ << "\n";

        int i = 0;
        std::for_each(graph.adjacencyList_.begin(), graph.adjacencyList_.end(),
            [&](const auto& list) {
            os << "  " << i++ << ":";
            std::copy(list.begin(), list.end(), std::ostream_iterator<Graph::Edge>(os, ""));
            os << "\n";
            });
    }
    return os;
}

Graph::Graph(bool oriented) : oriented_(oriented) {}

Graph::Graph(int vertexCount, bool oriented) : oriented_(oriented), vertexCount_(vertexCount) {
    if (vertexCount < 0) {
        throw std::invalid_argument(ERROR_INVALID_COUNT);
    }
    adjacencyList_.resize(vertexCount);
}

bool Graph::isEmpty() const {
    return vertexCount_ == 0;
}

bool Graph::hasVertex(int vertex) const {
    return vertex >= 0 && vertex < vertexCount_;
}

bool Graph::hasEdge(int u, int v) const {
    validateVertex(u); validateVertex(v);
    const auto& list = adjacencyList_[u];
    return std::find_if(list.begin(), list.end(), [&](const Edge& e) {
        return e.vertex == v;
        }) != list.end();
}

int Graph::getEdgeWeight(int u, int v) const {
    validateVertex(u); validateVertex(v);
    const auto& list = adjacencyList_[u];
    auto it = std::find_if(list.begin(), list.end(), [&](const Edge& e) {
        return e.vertex == v;
        });
    if (it != list.end()) {
        return it->weight;
    }
    throw std::runtime_error(ERROR_NOT_FOUND);
}

int Graph::getVertexCount() const {
    return vertexCount_;
}
int Graph::getEdgeCount() const {
    return edgeCount_;
}
bool Graph::isOriented() const {
    return oriented_;
}

int Graph::addVertex() {
    adjacencyList_.emplace_back();
    return vertexCount_++;
}

void Graph::removeVertex(int vertex) {
    validateVertex(vertex);

    std::for_each(adjacencyList_.begin(), adjacencyList_.end(), [&](auto& list) {
        list.remove_if([&](const Edge& e) { return e.vertex == vertex; });
        });

    adjacencyList_.erase(adjacencyList_.begin() + vertex);
    --vertexCount_;

    auto decrementIfGreater = [vertex](Edge edge) {
        if (edge.vertex > vertex) {
            --edge.vertex;
        }
        return edge;
        };

    std::for_each(adjacencyList_.begin(), adjacencyList_.end(),
        [&decrementIfGreater](auto& list) {
            std::transform(list.begin(), list.end(), list.begin(), decrementIfGreater);
        });

    auto countEdges = [](const auto& list) {
        return static_cast<int>(std::distance(list.begin(), list.end()));
        };

    auto sumEdges = [&countEdges](int sum, const auto& list) {
        return sum + countEdges(list);
        };

    int totalPhysicalEdges = std::accumulate(adjacencyList_.begin(), adjacencyList_.end(),
        0, sumEdges);

    edgeCount_ = oriented_ ? totalPhysicalEdges : (totalPhysicalEdges / 2);
}

void Graph::addEdge(int u, int v, int weight) {
    validateVertex(u); validateVertex(v);
    if (weight <= 0) {
        throw std::invalid_argument(ERROR_WEIGHT);
    }

    bool exists = hasEdge(u, v);
    adjacencyList_[u].remove_if([&](const Edge& e) { return e.vertex == v; });
    adjacencyList_[u].push_front(Edge(v, weight));
    if (!exists) {
        ++edgeCount_;
    }

    if (!oriented_ && u != v) {
        adjacencyList_[v].remove_if([&](const Edge& e) { return e.vertex == u; });
        adjacencyList_[v].push_front(Edge(u, weight));
    }
}

int Graph::removeEdge(int u, int v) {
    validateVertex(u); validateVertex(v);
    if (!hasEdge(u, v)) {
        throw std::runtime_error(ERROR_NOT_EXIST);
    }

    int weight = getEdgeWeight(u, v);
    adjacencyList_[u].remove_if([&](const Edge& e) { return e.vertex == v; });
    --edgeCount_;

    if (!oriented_ && u != v) {
        adjacencyList_[v].remove_if([&](const Edge& e) { return e.vertex == u; });
    }
    return weight;
}

std::vector<int> Graph::dfs(int startVertex) {
    validateVertex(startVertex);
    std::vector<int> colors(vertexCount_, WHITE);
    std::vector<int> result;

    dfsVisit(startVertex, colors, result);

    int i = 0;
    std::for_each(colors.begin(), colors.end(), [&](int color) {
        if (color == WHITE) {
            dfsVisit(i, colors, result);
        }
        ++i;
        });

    return result;
}

void Graph::dfsVisit(int vertex, std::vector<int>& colors, std::vector<int>& result) {
    colors[vertex] = GRAY;
    result.push_back(vertex);

    std::for_each(adjacencyList_[vertex].begin(), adjacencyList_[vertex].end(),
        [&](const Edge& e) {
        if (colors[e.vertex] == WHITE) {
            dfsVisit(e.vertex, colors, result);
        }
        });

    colors[vertex] = BLACK;
}

std::vector<int> Graph::topologicalSort() {
    if (!oriented_) {
        throw std::runtime_error(ERROR_SORTING);
    }
    std::vector<int> colors(vertexCount_, WHITE);
    std::vector<int> stack;

    int i = 0;
    std::for_each(colors.begin(), colors.end(), [&](int color) {
        if (color == WHITE) {
            topoVisit(i, colors, stack);
        }
        ++i;
        });

    return std::vector<int>(stack.rbegin(), stack.rend());
}

void Graph::topoVisit(int vertex, std::vector<int>& colors, std::vector<int>& stack) {
    colors[vertex] = GRAY;

    auto processNeighbor = [&](const Edge& edge) {
        if (colors[edge.vertex] == WHITE) {
            topoVisit(edge.vertex, colors, stack);
        }
        else if (colors[edge.vertex] == GRAY) {
            throw std::runtime_error(ERROR_CYCLE);
        }
        };

    std::for_each(adjacencyList_[vertex].begin(), adjacencyList_[vertex].end(),
        processNeighbor);

    colors[vertex] = BLACK;
    stack.push_back(vertex);
}

void Graph::validateVertex(int vertex) const {
    if (!hasVertex(vertex)) {
        throw std::out_of_range(ERROR_OUT_OF_RANGE);
    }
}
