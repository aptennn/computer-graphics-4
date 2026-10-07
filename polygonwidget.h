#ifndef POLYGONWIDGET_H
#define POLYGONWIDGET_H

#include <QPolygonF>
#include <QTransform>
#include <QPair>
#include <QVector>
#include <QWidget>

class QDoubleSpinBox;
class QLabel;
class QPushButton;

class PolygonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PolygonWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void clearScene();

private:
    enum class Mode { Create, Select, Intersection, Containment, Side };

    int canvasTop() const;
    int polygonAt(const QPointF &point) const;
    QPair<int, int> edgeAt(const QPointF &point) const;
    QPair<QPointF, QPointF> activeEdge() const;
    QPointF polygonCenter(const QPolygonF &polygon) const;
    void setMode(Mode mode);
    void resetCheckEdge();
    void clearCheckResult();
    void handleCheckClick(const QPointF &point);
    void finishCurrentPolygon();
    void selectPolygon(int index);
    void updateActions();
    void applyTransform(const QTransform &transform);
    void translateSelected();
    void rotateSelected(bool aroundCenter);
    void scaleSelected(bool aroundCenter);

    QVector<QPolygonF> m_polygons;
    QPolygonF m_current;
    QPointF m_cursorPos;
    bool m_hasCursor = false;
    int m_selected = -1;
    Mode m_mode = Mode::Create;
    int m_edgePolygon = -1;
    int m_edgeIndex = -1;
    bool m_hasSecondStart = false;
    QPointF m_secondStart;
    bool m_hasSecondSegment = false;
    QPointF m_secondEnd;
    bool m_hasTestPoint = false;
    QPointF m_testPoint;
    bool m_hasIntersectionPoint = false;
    QPointF m_intersectionPoint;
    bool m_hasOverlap = false;
    QPointF m_overlapStart;
    QPointF m_overlapEnd;

    QWidget *m_controls = nullptr;
    QPushButton *m_createButton = nullptr;
    QPushButton *m_selectButton = nullptr;
    QPushButton *m_intersectionButton = nullptr;
    QPushButton *m_containmentButton = nullptr;
    QPushButton *m_sideButton = nullptr;
    QPushButton *m_changeEdgeButton = nullptr;
    QPushButton *m_finishButton = nullptr;
    QLabel *m_selectedLabel = nullptr;
    QLabel *m_checkResultLabel = nullptr;
    QVector<QPushButton *> m_transformButtons;
    QDoubleSpinBox *m_dx = nullptr;
    QDoubleSpinBox *m_dy = nullptr;
    QDoubleSpinBox *m_angle = nullptr;
    QDoubleSpinBox *m_scaleX = nullptr;
    QDoubleSpinBox *m_scaleY = nullptr;
    QDoubleSpinBox *m_originX = nullptr;
    QDoubleSpinBox *m_originY = nullptr;
};

#endif // POLYGONWIDGET_H
