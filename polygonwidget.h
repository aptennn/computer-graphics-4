#ifndef POLYGONWIDGET_H
#define POLYGONWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QPolygonF>
#include <QPushButton>

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
    void finishCurrentPolygon();

    QVector<QPolygonF> m_polygons;

    QPolygonF m_current;

    QPointF m_cursorPos;
    bool    m_hasCursor = false;

    QPushButton *m_clearButton = nullptr;
};

#endif // POLYGONWIDGET_H

