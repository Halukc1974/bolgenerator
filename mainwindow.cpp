#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setup Logos
    MainWindow::setWindowIcon(QIcon(QString("/home/samjacobs/Pictures/Scimulate.svg")));
    ui->labelLogo->setPixmap(QString("/home/samjacobs/Pictures/ScimulateLogoFull.svg"));
    ui->labelLogo->setScaledContents(true);

    ui->cboxThread->insertItems(0, dimensions.Threads());
    ui->cboxHead->insertItems(0, dimensions.Heads());

    directory = QDir::home();

    //connect(ui->cboxThread, &QComboBox::, this, &MainWindow::ClearStatus);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::About);
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionDirectory, &QAction::triggered, this, &MainWindow::SetCurrentDirectory);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    connect(ui->pushExport, &QPushButton::clicked, this, &MainWindow::Export);

    ui->statusbar->showMessage("Ready!");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::About()
{
    QMessageBox::about(this, tr("About Application"),
                       tr("<b>BoltGenerator</b> is an automated CAD assistant "
                          "which produces standard-size 3D bolts per ISO and "
                          "ASME specifications."));
}

void MainWindow::Export()
{
    bool ok;
    double length = ui->textLength->text().toDouble(&ok);

    // This checks whether the thread and head selections were set to valid
    // input. If either is at the base (0) index, they have not been properly
    // set.
    ok *= ui->cboxThread->currentIndex(); // ok == 0 if cbox was not selected.
    ok *= ui->cboxHead->currentIndex();   // ditto.

    if(ok) // All input parameters are valid
    {
        ui->statusbar->clearMessage();

        Bolt bolt = Bolt(ui->cboxHead->currentIndex(),
                         ui->cboxThread->currentIndex(),
                         length);

        QString file = dimensions.Prefix(ui->cboxThread->currentIndex());
        file += ".step";

        ExportSTEP(bolt.Solid(), (directory.absolutePath()
                                  + QDir::toNativeSeparators("/")
                                  + file).toStdString().c_str());
        ui->statusbar->showMessage("Export Complete! (" + file, 5000);

        //QStringList filename;
        //filename.append(QFileDialog::getExistingDirectory());
        //filename.append(dimensions.Prefix(ui->cboxThread->currentIndex()));
        //ui->statusbar->showMessage(filename.join(""));

        /*
        QFileDialog dialog;
        dialog.setDefaultSuffix(QString("step"));
        QString file = dialog.getSaveFileName(this,
                                              tr("Save File"),
                                              NULL,
                                              tr("STEP Files (*.step);;"
                                                 "BRep Files (*.brep)"));
        QFileInfo f = QFileInfo(file);
        if(f.suffix().isEmpty())
        {
            file += "." + dialog.defaultSuffix();
            f = QFileInfo(file);
        }

        if(!QString::compare(f.suffix(), "brep", Qt::CaseInsensitive))
        {
            ExportBRep(bolt.Solid(), file.toStdString().c_str());
            ui->statusbar->showMessage("Export complete!");
        }
        else if(!QString::compare(f.suffix(), "step", Qt::CaseInsensitive))
        {
            ExportSTEP(bolt.Solid(), file.toStdString().c_str());
            ui->statusbar->showMessage("Export complete!");
        }
        else
        {
            ui->statusbar->showMessage("Invalid file format.");
        }
        */
    }
    else
    {
        ui->statusbar->showMessage("Invalid configuration.");
    }
}

void MainWindow::SetCurrentDirectory()
{
    directory = QFileDialog::getExistingDirectory(this, tr("Set Output Directory..."));
}

void MainWindow::ClearStatus()
{
    ui->statusbar->clearMessage();
}
