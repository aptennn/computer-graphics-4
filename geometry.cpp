#include "geometry.h"

#include <algorithm>
#include <cmath>

namespace Geometry {
namespace {

constexpr double epsilon = 1e-9;

double cross(const QPointF &a, const QPointF &b)
{
    return a.x() * b.y() - a.y() * b.x();
}

double dot(const QPointF &a, const QPointF &b)
{
    return a.x() * b.x() + a.y() * b.y();
}

double squaredLength(const QPointF &v)
{
    return dot(v, v);
}

bool nearZero(double value, double scale = 1.0)
{
    return std::abs(value) <= epsilon * std::max(1.0, scale);
}

bool onSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF edge = b - a;
    const QPointF relative = p - a;
    if (nearZero(squaredLength(edge)))
        return nearZero(squaredLength(relative));
    if (!nearZero(cross(edge, relative), std::sqrt(squaredLength(edge) * squaredLength(relative))))
        return false;
    return dot(relative, edge) >= -epsilon
        && dot(p - b, edge) <= epsilon;
}

} // namespace

Intersection intersectSegments(const QPointF &a, const QPointF &b,
                               const QPointF &c, const QPointF &d)
{
    const QPointF r = b - a;
    const QPointF s = d - c;
    const double rr = squaredLength(r);
    const double ss = squaredLength(s);
    if (nearZero(rr) && nearZero(ss))
        return onSegment(a, c, d) ? Intersection{IntersectionKind::Point, a, a} : Intersection{};
    if (nearZero(rr))
        return onSegment(a, c, d) ? Intersection{IntersectionKind::Point, a, a} : Intersection{};
    if (nearZero(ss))
        return onSegment(c, a, b) ? Intersection{IntersectionKind::Point, c, c} : Intersection{};

    const QPointF difference = c - a;
    const double denominator = cross(r, s);
    if (nearZero(denominator, std::sqrt(rr * ss))) {
        if (!nearZero(cross(difference, r), std::sqrt(squaredLength(difference) * rr)))
            return {};
        const double t0 = dot(difference, r) / rr;
        const double t1 = dot(d - a, r) / rr;
        const double begin = std::max(0.0, std::min(t0, t1));
        const double end = std::min(1.0, std::max(t0, t1));
        if (begin > end + epsilon)
            return {};
        const QPointF first = a + std::clamp(begin, 0.0, 1.0) * r;
        const QPointF last = a + std::clamp(end, 0.0, 1.0) * r;
        if (nearZero(squaredLength(last - first)))
            return {IntersectionKind::Point, first, first};
        return {IntersectionKind::Overlap, first, last};
    }

    const double t = cross(difference, s) / denominator;
    const double u = cross(difference, r) / denominator;
    if (t < -epsilon || t > 1.0 + epsilon || u < -epsilon || u > 1.0 + epsilon)
        return {};
    const QPointF point = a + std::clamp(t, 0.0, 1.0) * r;
    return {IntersectionKind::Point, point, point};
}

PointLocation locatePoint(const QPolygonF &polygon, const QPointF &point)
{
    const int count = polygon.size();
    if (count == 0)
        return PointLocation::Outside;
    if (count == 1)
        return onSegment(point, polygon[0], polygon[0])
            ? PointLocation::Boundary : PointLocation::Outside;
    if (count == 2)
        return onSegment(point, polygon[0], polygon[1])
            ? PointLocation::Boundary : PointLocation::Outside;

    bool inside = false;
    for (int i = 0; i < count; ++i) {
        const QPointF &a = polygon[i];
        const QPointF &b = polygon[(i + 1) % count];
        if (onSegment(point, a, b))
            return PointLocation::Boundary;
        if ((a.y() > point.y()) != (b.y() > point.y())) {
            const double crossingX = a.x() + (point.y() - a.y())
                * (b.x() - a.x()) / (b.y() - a.y());
            if (crossingX > point.x())
                inside = !inside;
        }
    }
    return inside ? PointLocation::Inside : PointLocation::Outside;
}

Side sideOfEdge(const QPointF &a, const QPointF &b, const QPointF &point)
{
    const QPointF edge = b - a;
    const QPointF relative = point - a;
    const double value = cross(edge, relative);
    if (nearZero(value, std::sqrt(squaredLength(edge) * squaredLength(relative))))
        return Side::OnLine;
    return value > 0.0 ? Side::Right : Side::Left;
}

} // namespace Geometry
