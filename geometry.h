#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <QPointF>
#include <QPolygonF>

namespace Geometry {

enum class IntersectionKind { None, Point, Overlap };
struct Intersection {
    IntersectionKind kind = IntersectionKind::None;
    QPointF first;
    QPointF last;
};

enum class PointLocation { Outside, Inside, Boundary };
enum class Side { Left, Right, OnLine };

Intersection intersectSegments(const QPointF &a, const QPointF &b,
                               const QPointF &c, const QPointF &d);
PointLocation locatePoint(const QPolygonF &polygon, const QPointF &point);
// Left and right are visual directions on the canvas (the Y axis points down).
Side sideOfEdge(const QPointF &a, const QPointF &b, const QPointF &point);

} // namespace Geometry

#endif // GEOMETRY_H
