#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QPainter>
#include <qstack.h>
#include <QtWidgets>
#include <QRegion>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    selectionStarted=false;
    changed = false;
    scale = 1.0 ;
    angle = 0;
    original_map_pointer = 0 ;
    imageLabel = new QLabel;
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    ui ->scrollAreaWidgetContents->showMaximized();
    ui ->scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui ->scrollArea->setWidget(imageLabel);
    ui ->actionSave->setEnabled(false);
    ui ->actionRotate_Right->setEnabled(false);
    ui ->actionRotate_Left->setEnabled(false);
    ui ->actionSelect->setEnabled(false);
    ui ->actionCrop->setEnabled(false);
    ui ->actionZoom_In->setEnabled(false);
    ui ->actionZoom_Out->setEnabled(false);
    ui ->actionUndo->setEnabled(false);
    ui ->actionRedo->setEnabled(false);
    ui ->actionReset->setEnabled(false);
    first_undo = true ;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    if (changed){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Image Viewer",
                "Do you want to save changes to current image?",
                QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
        else if (reply == QMessageBox::Yes) {
          QString fileName = on_actionSave_triggered();
          if (fileName == NULL){
              changed = true;
              return;
          }
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open Image"),
                       "/",
                       tr("Image Files (*.bmp *.jpg *.png)"));
    if (fileName != NULL)
    {
    imageLabel->setPixmap(QPixmap(fileName));
    ui -> scrollArea -> setWidget(imageLabel);
    imageObject = new QImage();
    imageObject->load(fileName);
    originalMap = QPixmap::fromImage(*imageObject);
    map = originalMap;
    first_undo = true ;
    scale = 1.0;
    angle = 0;

    undoAngle.push(angle);
    undoScale.push(scale);
    undoPointer.push(original_map_pointer);
    mapEvelotion.append(originalMap);
    this -> setWindowTitle(this->windowTitle()+" - "+fileName);

    /* set actions allowed or disabled on opening an image */
       ui ->actionSave->setEnabled(true);
       ui ->actionRotate_Right->setEnabled(true);
       ui ->actionRotate_Left->setEnabled(true);
       ui ->actionReset->setEnabled(true);
       ui ->actionSelect->setEnabled(true);
       ui ->actionCrop->setEnabled(true);
       ui ->actionZoom_In->setEnabled(true);
       ui ->actionZoom_Out->setEnabled(true);
       ui ->actionUndo->setEnabled(false);
       ui ->actionRedo->setEnabled(false);
    }
}

QString MainWindow::on_actionSave_triggered()
{
    QString fileName = NULL;
    changed = false;
      if(!redoScale.isEmpty()){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }
    redoScale.clear();
    redoAngle.clear();
    redoPointer.clear();
    if (!map || !imageObject)
    {
        QMessageBox::critical(this, tr("Error"), tr("No file to save.\n"),
                              QMessageBox::Ok);
    }
    else
    {
        fileName = QFileDialog::getSaveFileName(this,
            tr("Save as"),
            "",
            "24-bit Bitmap (*.bmp);;JPEG (*.jpg);;PNG (*.png)");
        * imageObject = map.toImage();
        imageObject->save(fileName);
    }
    return fileName;
}

void MainWindow::on_actionRotate_Left_triggered()
{
    changed = true;
    first_undo = true ;
        if(!redoScale.isEmpty()){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }
    redoScale.clear();
    redoAngle.clear();
    redoPointer.clear();
    bool ok;
    double  i = QInputDialog::getDouble(this, tr("Angle"),
                                 tr("angle:"), 90.00, 0, 180.0, 1, &ok);
    if (!ok) return;
    angle -= i;
    draw_image() ;

    undoAngle.push(angle);
    undoScale.push(scale);
    undoPointer.push(original_map_pointer);

    ui ->actionUndo->setEnabled(true);
    ui ->actionRedo->setEnabled(false);
}

void MainWindow::on_actionRotate_Right_triggered()
{
    changed = true;
    first_undo = true ;
      if(!redoScale.isEmpty()){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }
    redoScale.clear();
    redoAngle.clear();
    redoPointer.clear();
    bool ok;
    double  i = QInputDialog::getDouble(this, tr("Angle"),
                                 tr("angle:"), 90, 0.00, 180.0, 1, &ok);
    if (!ok) return;
    angle += i;
    draw_image() ;

    undoAngle.push(angle);
    undoScale.push(scale);
    undoPointer.push(original_map_pointer);

    ui ->actionUndo->setEnabled(true);
    ui ->actionRedo->setEnabled(false);
}
void MainWindow::on_actionSelect_triggered()
{
    ui->scrollArea->setCursor(Qt::CrossCursor);
}

void MainWindow::on_actionCrop_triggered()
{
    changed = true;
    first_undo = true ;
        if(!redoScale.isEmpty()){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }
    redoScale.clear();
    redoAngle.clear();
    redoPointer.clear();
    if  (selectionStarted){
        map = cropped ;
        originalMap = map ;
        imageLabel->setPixmap(cropped);
        selectionStarted= false;
        rubberBand->hide();

        undoAngle.push(angle);
        undoScale.push(scale);
        mapEvelotion.append(originalMap);
        undoPointer.push(++original_map_pointer);

        ui ->actionUndo->setEnabled(true);
        ui ->actionRedo->setEnabled(false);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if (!selectionStarted) {
        ui->scrollArea->setCursor(Qt::CrossCursor);
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        startPoint = e->pos() ;
        rubberBand->setGeometry(QRect(startPoint,QSize()));
        rubberBand->show();
        selectionStarted = true;
        selectionRect.setTopLeft(QPoint(startPoint.rx()-25,
                                        startPoint.ry()-90));
    }
    else{
         selectionStarted= false;
         rubberBand->hide();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(selectionStarted){
    rubberBand->setGeometry(QRect(startPoint,e->pos()).normalized());
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
  if(selectionStarted){
    endPoint = e->pos();
    selectionRect.setBottomRight(QPoint(endPoint.rx()-25,endPoint.ry()-90));
    cropped = map.copy(selectionRect);
  }
}


void MainWindow::on_actionZoom_Out_triggered()
{
    changed = true;
    first_undo = true ;
    if(!redoScale.isEmpty()){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }
    redoScale.clear();
    redoAngle.clear();
    redoPointer.clear();
    if (scale *0.8 <0.33 ||scale *0.8 >3.0){
        return ;
    }
    scale *= 0.8 ;
    current_size = map.size() ;
    if (selectionStarted){
        selectionStarted  = false  ;
        rubberBand -> hide();

    }

    draw_image() ;

    undoAngle.push(angle);
    undoScale.push(scale);
    undoPointer.push(original_map_pointer);

    ui -> scrollArea -> setWidget(imageLabel);
    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), 0.8);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), 0.8);
    ui ->actionUndo->setEnabled(true);
    ui ->actionRedo->setEnabled(false);
    ui ->actionZoom_In->setEnabled(true);
    QString str = QString("scale: %1").arg(scale, 0, 'g', 6);
    ui->scaleLabel->setText(str);
    if (scale *0.8 <0.33 ||scale *0.8 >3.0){
        ui ->actionZoom_Out->setEnabled(false);
    }

}

void MainWindow::on_actionZoom_In_triggered()
{
    changed = true;
    first_undo = true ;
      if(!redoScale.isEmpty()){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }
    redoScale.clear();
    redoAngle.clear();
    redoPointer.clear();
    if (scale *1.25 <0.33 ||scale *1.25 >3.0){
            return ;
    }
    scale *= 1.25 ;
    current_size = map.size() ;
    if (selectionStarted){
        selectionStarted  = false  ;
        rubberBand -> hide();
    }
     draw_image( ) ;

    undoAngle.push(angle);
    undoScale.push(scale);
    undoPointer.push(original_map_pointer);

    ui -> scrollArea -> setWidget(imageLabel);
    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), 1.25);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), 1.25);
    ui ->actionUndo->setEnabled(true);
    ui ->actionRedo->setEnabled(false);
    QString str = QString("scale: %1").arg(scale, 0, 'g', 6);
    ui->scaleLabel->setText(str);
    ui ->actionZoom_Out->setEnabled(true);
    if (scale *1.25 <0.33 ||scale *1.25 >3.0){
        ui ->actionZoom_In->setEnabled(false);
    }
}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor) {
    scrollBar->setValue(int(factor * scrollBar->value()
                        + ((factor - 1) * scrollBar->pageStep()/2)));
}


void MainWindow::on_actionUndo_triggered()
{
    changed = true;
    if((first_undo && (undoScale.size()<2)) || (!first_undo && undoScale.empty())){
        return ;
    }
    if(first_undo)
    {
        redoScale.push(undoScale.pop());
        redoAngle.push(undoAngle.pop());
        redoPointer.push(undoPointer.pop());
     }
     double s = undoScale.pop();
     redoScale.push(s);
     double a = undoAngle.pop() ;
     redoAngle.push(a);
    int p = undoPointer.pop();
    redoPointer.push(p);
    scale = s ;
    angle = a ;
    original_map_pointer = p ;
    draw_image();
     first_undo = false ;
     if((first_undo && undoScale.size()<2) || (!first_undo && undoScale.empty())){
         ui ->actionUndo->setEnabled(false);
     }
     ui ->actionRedo->setEnabled(true);
}

void MainWindow::on_actionRedo_triggered()
{
    changed = true;
    if(redoScale.isEmpty()){
        //ui ->actionRedo->setEnabled(false);
        return ;
    }
    if(!first_undo){
        undoScale.push(redoScale.pop());
        undoAngle.push(redoAngle.pop());
        undoPointer.push(redoPointer.pop());
    }

    double s = redoScale.pop();
    undoScale.push(s);
    double a = redoAngle.pop() ;
    undoAngle.push(a);
    int p = redoPointer.pop();
    undoPointer.push(p);

    scale = s ;
    angle = a ;
    original_map_pointer = p ;
    draw_image();

    first_undo = true ;
    if(redoScale.isEmpty()){
        ui ->actionRedo->setEnabled(false);
        //return ;
    }
    ui ->actionUndo->setEnabled(true);
}

void MainWindow::on_actionReset_triggered()
{
    changed = true;
    first_undo = true ;
    if(undoScale.isEmpty())return ;
  scale = 1.0;
  angle = 0 ;
  original_map_pointer = 0 ;
  undoScale.push(1);
  undoAngle.push(0);
  undoPointer.push(0) ;
  draw_image();
  ui ->actionUndo->setEnabled(false);
    ui ->actionRedo->setEnabled(false);
}

void MainWindow::on_actionExit_triggered()
{
    if (changed){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Exit Image Viewer",
                "Do you want to save changes to current image?",
                QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel)
            return;
        else if (reply == QMessageBox::Yes) {
          QString fileName = on_actionSave_triggered();
          if (fileName == NULL){
              changed = true;
              return;
          }
        }
    }
    QApplication::quit();
}
void MainWindow::draw_image(){
    int w = (mapEvelotion.at(original_map_pointer).width())*scale;
    int h = (mapEvelotion.at(original_map_pointer).height())*scale;
    map = mapEvelotion.at(original_map_pointer).scaled(w, h, Qt::KeepAspectRatio);

    QPixmap pixmap(*imageLabel->pixmap());
    QMatrix rm;
    rm.rotate(angle);
    pixmap = map.transformed(rm);
    map = pixmap ;
    imageLabel->resize(map.size());
    imageLabel->setPixmap(map);
}
