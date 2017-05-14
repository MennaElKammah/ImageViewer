#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QLabel>
#include <QMouseEvent>
#include <QMenu>
#include<QRubberBand>
#include <QScrollBar>
#include <QScrollArea>
#include <QStack>
#include <QVector>

namespace Ui {
class MainWindow;

}

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    QString on_actionSave_triggered();

    void on_actionRotate_Left_triggered();

    void on_actionRotate_Right_triggered();

    void on_actionSelect_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionCrop_triggered();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_actionReset_triggered();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    QImage *imageObject ;
    QPixmap  map;
    QPixmap  originalMap;
    int angle;
    QLabel *imageLabel ;
    QGraphicsScene * scene;
    bool selectionStarted ;
    bool first_undo;
    bool changed;
    QRect selectionRect;
    QRubberBand * rubberBand;
    QPoint  startPoint ;
    QPoint endPoint ;
    double scale ;
    QScrollArea * scrollArea  ;
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void draw_image();

    QPixmap cropped;
    int original_map_pointer;

    QSize current_size ;

    QVector <QPixmap> mapEvelotion ;

    QStack <int> undoPointer ;
    QStack <int> redoPointer ;
    QStack <double> undoScale ;
    QStack <double> redoScale ;
    QStack <double> undoAngle;
    QStack <double> redoAngle ;
};

#endif // MAINWINDOW_H
