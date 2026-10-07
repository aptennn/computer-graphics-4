#include "geometry.h"
#include "polygonwidget.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QtTest>

class Lab4Tests : public QObject
{
    Q_OBJECT

private slots:
    void segmentIntersection();
    void pointLocation();
    void edgeSide();
    void existingSceneAndTransforms();
    void repeatedChecksInWidget();
};

void Lab4Tests::segmentIntersection()
{
    using namespace Geometry;
    auto result = intersectSegments({0, 0}, {10, 10}, {0, 10}, {10, 0});
    QVERIFY(result.kind == IntersectionKind::Point);
    QCOMPARE(result.first, QPointF(5, 5));

    result = intersectSegments({0, 0}, {10, 0}, {10, 0}, {10, 10});
    QVERIFY(result.kind == IntersectionKind::Point);
    QCOMPARE(result.first, QPointF(10, 0));

    result = intersectSegments({0, 0}, {10, 0}, {5, 0}, {15, 0});
    QVERIFY(result.kind == IntersectionKind::Overlap);
    QCOMPARE(result.first, QPointF(5, 0));
    QCOMPARE(result.last, QPointF(10, 0));

    QVERIFY(intersectSegments({0, 0}, {10, 0}, {0, 1}, {10, 1}).kind
            == IntersectionKind::None);
    QVERIFY(intersectSegments({2, 2}, {2, 2}, {0, 2}, {5, 2}).kind
            == IntersectionKind::Point);
}

void Lab4Tests::pointLocation()
{
    using namespace Geometry;
    QPolygonF square;
    square << QPointF(0, 0) << QPointF(10, 0) << QPointF(10, 10) << QPointF(0, 10);
    QVERIFY(locatePoint(square, {5, 5}) == PointLocation::Inside);
    QVERIFY(locatePoint(square, {10, 5}) == PointLocation::Boundary);
    QVERIFY(locatePoint(square, {0, 0}) == PointLocation::Boundary);
    QVERIFY(locatePoint(square, {15, 5}) == PointLocation::Outside);

    // The notch makes this polygon concave.
    QPolygonF concave;
    concave << QPointF(0, 0) << QPointF(8, 0) << QPointF(8, 8)
            << QPointF(5, 8) << QPointF(5, 3) << QPointF(3, 3)
            << QPointF(3, 8) << QPointF(0, 8);
    QVERIFY(locatePoint(concave, {1, 5}) == PointLocation::Inside);
    QVERIFY(locatePoint(concave, {4, 5}) == PointLocation::Outside);
    QVERIFY(locatePoint(concave, {4, 3}) == PointLocation::Boundary);

    QPolygonF segment;
    segment << QPointF(0, 0) << QPointF(10, 0);
    QVERIFY(locatePoint(segment, {5, 0}) == PointLocation::Boundary);
    QVERIFY(locatePoint(segment, {5, 1}) == PointLocation::Outside);
    QPolygonF single;
    single << QPointF(3, 4);
    QVERIFY(locatePoint(single, {3, 4}) == PointLocation::Boundary);
    QVERIFY(locatePoint(single, {4, 4}) == PointLocation::Outside);
}

void Lab4Tests::edgeSide()
{
    using namespace Geometry;
    // The canvas has a downward Y axis, so a point below a rightward edge is on its right.
    QVERIFY(sideOfEdge({0, 0}, {10, 0}, {5, 5}) == Side::Right);
    QVERIFY(sideOfEdge({0, 0}, {10, 0}, {5, -5}) == Side::Left);
    QVERIFY(sideOfEdge({0, 0}, {10, 0}, {20, 0}) == Side::OnLine);
}

void Lab4Tests::existingSceneAndTransforms()
{
    PolygonWidget widget;
    widget.resize(1200, 850);
    widget.show();
    QApplication::processEvents();
    auto button = [&widget](const char *name) {
        auto *found = widget.findChild<QPushButton *>(name);
        Q_ASSERT(found);
        return found;
    };
    auto spin = [&widget](const char *name) {
        auto *found = widget.findChild<QDoubleSpinBox *>(name);
        Q_ASSERT(found);
        return found;
    };
    auto *result = widget.findChild<QLabel *>("checkResultLabel");
    auto *selected = widget.findChild<QLabel *>("selectedLabel");
    QVERIFY(result);
    QVERIFY(selected);
    const int canvasTop = result->parentWidget()->geometry().bottom() + 1;
    auto clickCanvas = [&widget, canvasTop](int x, int y) {
        QTest::mouseClick(&widget, Qt::LeftButton, Qt::NoModifier, QPoint(x, canvasTop + y));
    };

    clickCanvas(100, 100);
    clickCanvas(200, 100);
    clickCanvas(200, 140);
    clickCanvas(100, 140);
    QTest::mouseClick(button("finishButton"), Qt::LeftButton);
    QVERIFY(selected->text().contains("P1 (4 вершин)"));
    QTest::mouseClick(button("selectButton"), Qt::LeftButton);
    clickCanvas(150, 120);
    QVERIFY(selected->text().contains("P1 (4 вершин)"));

    spin("angleInput")->setValue(90);
    QTest::mouseClick(button("rotateCenterButton"), Qt::LeftButton);
    QTest::mouseClick(button("containmentButton"), Qt::LeftButton);
    clickCanvas(150, 80);
    QVERIFY(result->text().contains("внутри"));
    clickCanvas(180, 120);
    QVERIFY(result->text().contains("снаружи"));

    spin("scaleXInput")->setValue(2);
    QTest::mouseClick(button("scaleCenterButton"), Qt::LeftButton);
    clickCanvas(180, 120);
    QVERIFY(result->text().contains("внутри"));

    spin("originXInput")->setValue(200);
    spin("originYInput")->setValue(120);
    spin("angleInput")->setValue(180);
    QTest::mouseClick(button("rotatePointButton"), Qt::LeftButton);
    clickCanvas(250, 120);
    QVERIFY(result->text().contains("внутри"));

    spin("scaleXInput")->setValue(0.5);
    QTest::mouseClick(button("scalePointButton"), Qt::LeftButton);
    clickCanvas(240, 120);
    QVERIFY(result->text().contains("внутри"));
    clickCanvas(250, 120);
    QVERIFY(result->text().contains("снаружи"));

    QTest::mouseClick(button("clearButton"), Qt::LeftButton);
    clickCanvas(350, 200);
    QTest::mouseClick(button("finishButton"), Qt::LeftButton);
    QVERIFY(selected->text().contains("P1 (1 вершин)"));
    clickCanvas(400, 200);
    clickCanvas(450, 200);
    QTest::mouseClick(button("finishButton"), Qt::LeftButton);
    QVERIFY(selected->text().contains("P2 (2 вершин)"));
    QTest::mouseClick(button("selectButton"), Qt::LeftButton);
    clickCanvas(350, 200);
    QVERIFY(selected->text().contains("P1 (1 вершин)"));
    clickCanvas(425, 200);
    QVERIFY(selected->text().contains("P2 (2 вершин)"));
}

void Lab4Tests::repeatedChecksInWidget()
{
    PolygonWidget widget;
    widget.resize(1200, 850);
    widget.show();
    QApplication::processEvents();
    auto button = [&widget](const char *name) {
        auto *found = widget.findChild<QPushButton *>(name);
        Q_ASSERT(found);
        return found;
    };
    auto *result = widget.findChild<QLabel *>("checkResultLabel");
    QVERIFY(result);
    auto *selected = widget.findChild<QLabel *>("selectedLabel");
    QVERIFY(selected);
    const int canvasTop = result->parentWidget()->geometry().bottom() + 1;
    auto clickCanvas = [&widget, canvasTop](int x, int y) {
        QTest::mouseClick(&widget, Qt::LeftButton, Qt::NoModifier, QPoint(x, canvasTop + y));
    };

    clickCanvas(100, 100);
    clickCanvas(200, 100);
    clickCanvas(200, 200);
    clickCanvas(100, 200);
    QTest::mouseClick(button("finishButton"), Qt::LeftButton);
    QVERIFY(selected->text().contains("P1 (4 вершин)"));

    auto *dx = widget.findChild<QDoubleSpinBox *>("dxInput");
    QVERIFY(dx);
    dx->setValue(20);
    QTest::mouseClick(button("translateButton"), Qt::LeftButton);

    QTest::mouseClick(button("containmentButton"), Qt::LeftButton);
    clickCanvas(150, 150);
    QVERIFY(result->text().contains("внутри"));
    clickCanvas(110, 150);
    QVERIFY(result->text().contains("снаружи"));
    QVERIFY(button("containmentButton")->isChecked());

    QTest::mouseClick(button("intersectionButton"), Qt::LeftButton);
    clickCanvas(160, 100); // Existing top edge, now shifted by dx=20.
    clickCanvas(160, 50);
    QTest::mouseMove(&widget, QPoint(160, canvasTop + 250));
    clickCanvas(160, 250);
    QVERIFY(result->text().contains("Пересечение в точке"));
    clickCanvas(50, 50);
    clickCanvas(50, 250);
    QVERIFY(result->text().contains("не пересекаются"));
    QVERIFY(button("intersectionButton")->isChecked());

    QTest::mouseClick(button("sideButton"), Qt::LeftButton);
    clickCanvas(160, 100);
    clickCanvas(160, 150);
    QVERIFY(result->text().contains("справа"));
    clickCanvas(160, 50);
    QVERIFY(result->text().contains("слева"));
    QVERIFY(button("sideButton")->isChecked());

    QTest::mouseClick(button("clearButton"), Qt::LeftButton);
    QVERIFY(result->text().contains("Сцена очищена"));
    clickCanvas(350, 250);
    QTest::mouseClick(button("finishButton"), Qt::LeftButton);
    QVERIFY(selected->text().contains("P1 (1 вершин)"));
}

QTEST_MAIN(Lab4Tests)
#include "lab4_tests.moc"
