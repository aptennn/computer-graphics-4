#ifndef POLYGONWIDGET_H
#define POLYGONWIDGET_H

#include <QPolygonF>
#include <QTransform>
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
    enum class Mode { Create, Select };

    int canvasTop() const;
    int polygonAt(const QPointF &point) const;
    QPointF polygonCenter(const QPolygonF &polygon) const;
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

    QWidget *m_controls = nullptr;
    QPushButton *m_createButton = nullptr;
    QPushButton *m_selectButton = nullptr;
    QPushButton *m_finishButton = nullptr;
    QLabel *m_selectedLabel = nullptr;
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
