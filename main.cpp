#include <QApplication>
#include "polygonwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PolygonWidget w;
    w.setWindowTitle("Polygon Editor — ЛКМ: добавить точку, ПКМ/Enter: завершить, C: очистить");
    w.resize(900, 650);
    w.show();

    return app.exec();
}
