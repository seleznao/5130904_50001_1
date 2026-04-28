#include "ioHandle.hpp"
#include <iostream>
#include "point.hpp"

std::istream &operator>>(std::istream &in, DelimiterIO &&dest) {

    std::istream::sentry sentry(in);

    if (!sentry) {
        return in;
    }

    char c = '0';

    in >> c;

    if (in && (c != dest.expected)) {
        in.setstate(std::ios::failbit);
    }

    return in;
}

std::istream &operator>>(std::istream &in, Point &p) {

    std::istream::sentry sentry(in);

    if (!sentry) {
        return in;
    }

    using sep = DelimiterIO;

    int x = 0, y = 0;

    if (in >> sep{ '(' } >> x >> sep{ ';' } >> y >> sep{ ')' }) {
        p.x = x;
        p.y = y;
    }

    return in;
}

std::istream &operator>>(std::istream &in, Polygon &p) {

    std::istream::sentry sentry(in);

    if (!sentry) {
        return in;
    }

    Point temp{ 0, 0 };

    size_t nPoints = 0;

    if (!(in >> nPoints) || nPoints < 3) {
        return in;
    }

    p.points.clear();
    p.points.reserve(nPoints);

    for (size_t i = 0; i < nPoints; ++i) {
        if (in >> temp) {
            p.points.push_back(temp);
        }
        else {
            return in;
        }
    }

    return in;
}
