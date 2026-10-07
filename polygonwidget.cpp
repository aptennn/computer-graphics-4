#include "polygonwidget.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

#include <cmath>

namespace {

QDoubleSpinBox *makeSpinBox(QWidget *parent, double value = 0.0)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setRange(-100000.0, 100000.0);
    spin->setDecimals(2);
    spin->setValue(value);
    spin->setFixedWidth(88);
    return spin;
}

double distanceToSegment(const QPointF &point, const QPointF &a, const QPointF &b)
{
    const QPointF edge = b - a;
    const double lengthSquared = edge.x() * edge.x() + edge.y() * edge.y();
    if (lengthSquared == 0.0)
        return QLineF(point, a).length();
    const QPointF relative = point - a;
    const double projection = (relative.x() * edge.x() + relative.y() * edge.y()) / lengthSquared;
    const double t = qBound(0.0, projection, 1.0);
    return QLineF(point, a + t * edge).length();
}

} // namespace

// Лилия — создание полигонов и очистка сцены.
PolygonWidget::PolygonWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(900, 500);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    m_controls = new QWidget(this);
    m_controls->setAutoFillBackground(true);
    m_controls->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *panel = new QVBoxLayout(m_controls);
    panel->setContentsMargins(10, 8, 10, 8);
    panel->setSpacing(5);
    root->addWidget(m_controls);
    root->addStretch();

    auto *modeRow = new QHBoxLayout;
    modeRow->addWidget(new QLabel("Режим:", m_controls));
    m_createButton = new QPushButton("Создание", m_controls);
    m_selectButton = new QPushButton("Выбор", m_controls);
    m_createButton->setCheckable(true);
    m_selectButton->setCheckable(true);
    m_createButton->setChecked(true);
    modeRow->addWidget(m_createButton);
    modeRow->addWidget(m_selectButton);
    m_finishButton = new QPushButton("Завершить фигуру", m_controls);
    modeRow->addWidget(m_finishButton);
    auto *clearButton = new QPushButton("Очистить сцену (C)", m_controls);
    modeRow->addWidget(clearButton);
    modeRow->addStretch();
    m_selectedLabel = new QLabel(m_controls);
    modeRow->addWidget(m_selectedLabel);
    panel->addLayout(modeRow);

    auto *translateRow = new QHBoxLayout;
    translateRow->addWidget(new QLabel("Смещение: dx", m_controls));
    m_dx = makeSpinBox(m_controls);
    translateRow->addWidget(m_dx);
    translateRow->addWidget(new QLabel("dy", m_controls));
    m_dy = makeSpinBox(m_controls);
    translateRow->addWidget(m_dy);
    auto *translateButton = new QPushButton("Сместить", m_controls);
    translateRow->addWidget(translateButton);
    translateRow->addStretch();
    panel->addLayout(translateRow);

    auto *rotateRow = new QHBoxLayout;
    rotateRow->addWidget(new QLabel("Поворот: угол °", m_controls));
    m_angle = makeSpinBox(m_controls);
    rotateRow->addWidget(m_angle);
    rotateRow->addWidget(new QLabel("Точка: x", m_controls));
    m_originX = makeSpinBox(m_controls);
    rotateRow->addWidget(m_originX);
    rotateRow->addWidget(new QLabel("y", m_controls));
    m_originY = makeSpinBox(m_controls);
    rotateRow->addWidget(m_originY);
    auto *rotateCenterButton = new QPushButton("Вокруг центра", m_controls);
    auto *rotatePointButton = new QPushButton("Вокруг точки", m_controls);
    rotateRow->addWidget(rotateCenterButton);
    rotateRow->addWidget(rotatePointButton);
    rotateRow->addStretch();
    panel->addLayout(rotateRow);

    auto *scaleRow = new QHBoxLayout;
    scaleRow->addWidget(new QLabel("Масштаб: sx", m_controls));
    m_scaleX = makeSpinBox(m_controls, 1.0);
    scaleRow->addWidget(m_scaleX);
    scaleRow->addWidget(new QLabel("sy", m_controls));
    m_scaleY = makeSpinBox(m_controls, 1.0);
    scaleRow->addWidget(m_scaleY);
    auto *scaleCenterButton = new QPushButton("От центра", m_controls);
    auto *scalePointButton = new QPushButton("От точки", m_controls);
    scaleRow->addWidget(scaleCenterButton);
    scaleRow->addWidget(scalePointButton);
    scaleRow->addWidget(new QLabel("Точка задаётся в строке поворота", m_controls));
    scaleRow->addStretch();
    panel->addLayout(scaleRow);

    m_transformButtons = {translateButton, rotateCenterButton, rotatePointButton,
                          scaleCenterButton, scalePointButton};
    connect(m_createButton, &QPushButton::clicked, this, [this] {
        m_mode = Mode::Create;
        updateActions();
        setFocus();
    });
    connect(m_selectButton, &QPushButton::clicked, this, [this] {
        m_mode = Mode::Select;
        updateActions();
        setFocus();
    });
    connect(m_finishButton, &QPushButton::clicked, this, &PolygonWidget::finishCurrentPolygon);
    connect(clearButton, &QPushButton::clicked, this, &PolygonWidget::clearScene);
    connect(translateButton, &QPushButton::clicked, this, &PolygonWidget::translateSelected);
    connect(rotateCenterButton, &QPushButton::clicked, this, [this] { rotateSelected(true); });
    connect(rotatePointButton, &QPushButton::clicked, this, [this] { rotateSelected(false); });
    connect(scaleCenterButton, &QPushButton::clicked, this, [this] { scaleSelected(true); });
    connect(scalePointButton, &QPushButton::clicked, this, [this] { scaleSelected(false); });

    auto *clearShortcut = new QShortcut(QKeySequence(Qt::Key_C), this);
    clearShortcut->setContext(Qt::ApplicationShortcut);
    connect(clearShortcut, &QShortcut::activated, this, &PolygonWidget::clearScene);
    updateActions();
    setFocus();
}

int PolygonWidget::canvasTop() const
{
    return m_controls->geometry().bottom() + 1;
}

void PolygonWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(245, 245, 250));
    p.setClipRect(QRect(0, canvasTop(), width(), height() - canvasTop()));
    p.translate(0, canvasTop());

    for (int i = 0; i < m_polygons.size(); ++i) {
        const QPolygonF &poly = m_polygons[i];
        const int n = poly.size();
        if (n == 0) continue;
        const bool selected = i == m_selected;
        const QColor color = selected ? QColor(225, 105, 20) : QColor(30, 90, 200);
        const QPen outline(color, selected ? 3 : 2);
        if (n >= 3) {
            p.setBrush(selected ? QColor(255, 170, 70, 90) : QColor(100, 150, 255, 90));
            p.setPen(outline);
            p.drawPolygon(poly);
        } else if (n == 2) {
            p.setBrush(Qt::NoBrush);
            p.setPen(outline);
            p.drawLine(poly[0], poly[1]);
        }
        p.setBrush(Qt::white);
        p.setPen(outline);
        for (const QPointF &pt : poly)
            p.drawEllipse(pt, selected ? 6.0 : 5.0, selected ? 6.0 : 5.0);
        p.setPen(color);
        p.drawText(poly.boundingRect().center() + QPointF(8, -8),
                   QString("P%1 (%2)").arg(i + 1).arg(n));
    }

    if (!m_current.isEmpty()) {
        if (m_hasCursor && m_mode == Mode::Create) {
            p.setPen(QPen(QColor(200, 60, 60), 1, Qt::DashLine));
            p.drawLine(m_current.last(), m_cursorPos);
            if (m_current.size() >= 2) {
                p.setPen(QPen(QColor(200, 60, 60, 120), 1, Qt::DotLine));
                p.drawLine(m_cursorPos, m_current.first());
            }
        }
        p.setPen(QPen(QColor(200, 60, 60), 2));
        p.setBrush(Qt::NoBrush);
        for (int i = 1; i < m_current.size(); ++i)
            p.drawLine(m_current[i - 1], m_current[i]);
        p.setBrush(QColor(255, 220, 220));
        for (const QPointF &pt : m_current)
            p.drawEllipse(pt, 5.0, 5.0);
    }

    p.setPen(Qt::darkGray);
    p.drawText(QRect(10, 10, width() - 20, 40),
               Qt::AlignTop | Qt::AlignLeft,
               "Создание: ЛКМ — вершина, ПКМ / Enter / двойной клик — завершить.\n"
               "Выбор: ЛКМ по фигуре. Оранжевая фигура — выбранная. "
               "Координаты — от угла поля.");
}

void PolygonWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->pos().y() < canvasTop())
        return;
    if (event->button() == Qt::LeftButton) {
        const QPointF canvasPoint(event->pos().x(), event->pos().y() - canvasTop());
        if (m_mode == Mode::Create) {
            m_current << canvasPoint;
            m_cursorPos = canvasPoint;
            m_hasCursor = true;
            updateActions();
        } else {
            selectPolygon(polygonAt(canvasPoint));
        }
        setFocus();
        update();
    } else if (event->button() == Qt::RightButton && m_mode == Mode::Create) {
        finishCurrentPolygon();
    }
}

void PolygonWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_cursorPos = QPointF(event->pos().x(), event->pos().y() - canvasTop());
    m_hasCursor = true;
    if (!m_current.isEmpty() && m_mode == Mode::Create)
        update();
}

void PolygonWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_mode == Mode::Create
        && event->pos().y() >= canvasTop())
        finishCurrentPolygon();
}

void PolygonWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape:
        if (m_mode == Mode::Create)
            finishCurrentPolygon();
        break;
    case Qt::Key_C:
        clearScene();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void PolygonWidget::finishCurrentPolygon()
{
    if (m_current.isEmpty())
        return;
    if (m_current.size() >= 3
        && QLineF(m_current.first(), m_current.last()).length() < 3.0)
        m_current.removeLast();
    if (!m_current.isEmpty()) {
        m_polygons << m_current;
        m_selected = m_polygons.size() - 1;
    }
    m_current.clear();
    updateActions();
    update();
    setFocus();
}

void PolygonWidget::clearScene()
{
    m_polygons.clear();
    m_current.clear();
    m_selected = -1;
    updateActions();
    update();
    setFocus();
}

int PolygonWidget::polygonAt(const QPointF &point) const
{
    for (int i = m_polygons.size() - 1; i >= 0; --i) {
        const QPolygonF &poly = m_polygons[i];
        if (poly.isEmpty()) continue;
        if (poly.size() >= 3 && poly.containsPoint(point, Qt::OddEvenFill)) return i;
        if (poly.size() == 1 && QLineF(point, poly.first()).length() <= 10.0) return i;
        for (int j = 0; j < poly.size() - 1; ++j)
            if (distanceToSegment(point, poly[j], poly[j + 1]) <= 8.0) return i;
        if (poly.size() >= 3
            && distanceToSegment(point, poly.last(), poly.first()) <= 8.0) return i;
    }
    return -1;
}

QPointF PolygonWidget::polygonCenter(const QPolygonF &polygon) const
{
    if (polygon.size() == 1) return polygon.first();
    if (polygon.size() == 2) return (polygon[0] + polygon[1]) / 2.0;
    double twiceArea = 0.0;
    double centerX = 0.0;
    double centerY = 0.0;
    for (int i = 0; i < polygon.size(); ++i) {
        const QPointF &a = polygon[i];
        const QPointF &b = polygon[(i + 1) % polygon.size()];
        const double cross = a.x() * b.y() - b.x() * a.y();
        twiceArea += cross;
        centerX += (a.x() + b.x()) * cross;
        centerY += (a.y() + b.y()) * cross;
    }
    if (std::abs(twiceArea) > 1e-9)
        return QPointF(centerX / (3.0 * twiceArea), centerY / (3.0 * twiceArea));
    QPointF average;
    for (const QPointF &point : polygon) average += point;
    return average / polygon.size();
}

void PolygonWidget::selectPolygon(int index)
{
    m_selected = index;
    updateActions();
    update();
}

void PolygonWidget::updateActions()
{
    m_createButton->setChecked(m_mode == Mode::Create);
    m_selectButton->setChecked(m_mode == Mode::Select);
    m_finishButton->setEnabled(!m_current.isEmpty());
    const bool hasSelection = m_selected >= 0 && m_selected < m_polygons.size();
    for (QPushButton *button : m_transformButtons) button->setEnabled(hasSelection);
    m_selectedLabel->setText(hasSelection
        ? QString("Выбрана P%1 (%2 вершин)").arg(m_selected + 1).arg(m_polygons[m_selected].size())
        : QString("Фигура не выбрана"));
}

void PolygonWidget::applyTransform(const QTransform &transform)
{
    if (m_selected < 0 || m_selected >= m_polygons.size()) return;
    m_polygons[m_selected] = transform.map(m_polygons[m_selected]);
    update();
}

void PolygonWidget::translateSelected()
{
    applyTransform(QTransform(1, 0, 0, 1, m_dx->value(), m_dy->value()));
}

void PolygonWidget::rotateSelected(bool aroundCenter)
{
    if (m_selected < 0 || m_selected >= m_polygons.size()) return;
    const QPointF center = aroundCenter
        ? polygonCenter(m_polygons[m_selected])
        : QPointF(m_originX->value(), m_originY->value());
    const double radians = m_angle->value() * std::acos(-1.0) / 180.0;
    const double c = std::cos(radians);
    const double s = std::sin(radians);
    // T(center) * R(angle) * T(-center).
    applyTransform(QTransform(c, s, -s, c,
                              center.x() * (1.0 - c) + center.y() * s,
                              center.y() * (1.0 - c) - center.x() * s));
}

void PolygonWidget::scaleSelected(bool aroundCenter)
{
    if (m_selected < 0 || m_selected >= m_polygons.size()) return;
    const QPointF center = aroundCenter
        ? polygonCenter(m_polygons[m_selected])
        : QPointF(m_originX->value(), m_originY->value());
    const double sx = m_scaleX->value();
    const double sy = m_scaleY->value();
    // T(center) * S(sx, sy) * T(-center).
    applyTransform(QTransform(sx, 0, 0, sy,
                              center.x() * (1.0 - sx), center.y() * (1.0 - sy)));
}
