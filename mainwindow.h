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
    //void Exit();
    void Export();
    void SetCurrentDirectory();
    void ClearStatus();

    //void on_actionExit_triggered();

    //void on_actionAbout_Qt_triggered();

    //void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    Dimensions dimensions;
    QDir directory;
};
#endif // MAINWINDOW_H
