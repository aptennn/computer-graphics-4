#include "polygonwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QShortcut>
#include <QLineF>

//Лилия - реализовала создание полигонов и очистку сцены
PolygonWidget::PolygonWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);

    m_clearButton = new QPushButton("Очистить сцену (C)", this);
    m_clearButton->setGeometry(10, 10, 180, 30);
    m_clearButton->setFocusPolicy(Qt::NoFocus);
    connect(m_clearButton, &QPushButton::clicked,
            this, &PolygonWidget::clearScene);

    auto *clearShortcut = new QShortcut(QKeySequence(Qt::Key_C), this);
    clearShortcut->setContext(Qt::ApplicationShortcut);
    connect(clearShortcut, &QShortcut::activated,
            this, &PolygonWidget::clearScene);

    setFocus();
}

void PolygonWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(245, 245, 250));

    for (int i = 0; i < m_polygons.size(); ++i) {
        const QPolygonF &poly = m_polygons[i];
        const int n = poly.size();
        if (n == 0) continue;

        if (n >= 3) {
            p.setBrush(QColor(100, 150, 255, 90));
            p.setPen(QPen(QColor(30, 90, 200), 2));
            p.drawPolygon(poly);
        } else if (n == 2) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(30, 90, 200), 2));
            p.drawLine(poly[0], poly[1]);
        }

        p.setBrush(Qt::white);
        p.setPen(QPen(QColor(30, 90, 200), 2));
        for (const QPointF &pt : poly)
            p.drawEllipse(pt, 5.0, 5.0);

        if (n >= 1) {
            p.setPen(Qt::darkBlue);
            QPointF c = poly.boundingRect().center();
            p.drawText(c + QPointF(8, -8),
                       QString("P%1 (%2)").arg(i + 1).arg(n));
        }
    }

    if (!m_current.isEmpty()) {
        if (m_hasCursor) {
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
        p.setPen(QPen(QColor(200, 60, 60), 2));
        for (const QPointF &pt : m_current)
            p.drawEllipse(pt, 5.0, 5.0);
    }

    p.setPen(Qt::darkGray);
    p.drawText(rect().adjusted(10, 50, -10, -10),
               Qt::AlignTop | Qt::AlignLeft,
               "ЛКМ — добавить точку   |   ПКМ / Enter / Двойной клик — завершить полигон\n"
               "C или кнопка — очистить сцену");
}

void PolygonWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_current << event->pos();
        m_cursorPos = event->pos();
        m_hasCursor = true;
        setFocus();
        update();
    } else if (event->button() == Qt::RightButton) {
        finishCurrentPolygon();
    }
}

void PolygonWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_cursorPos = event->pos();
    m_hasCursor = true;
    if (!m_current.isEmpty())
        update();
}

void PolygonWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishCurrentPolygon();
}

void PolygonWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape:
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

    if (m_current.size() >= 2 &&
        QLineF(m_current.first(), m_current.last()).length() < 3.0) {
        m_current.removeLast();
    }

    if (!m_current.isEmpty())
        m_polygons << m_current;

    m_current.clear();
    update();
}

void PolygonWidget::clearScene()
{
    m_polygons.clear();
    m_current.clear();
    update();
    setFocus();
}
