#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QMainWindow>

#include "bolt.h"
#include "export.h"
#include "dimensions.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void About();
    void Export();

private:
    Ui::MainWindow *ui;
    Dimensions dimensions;
};
#endif // MAINWINDOW_H
