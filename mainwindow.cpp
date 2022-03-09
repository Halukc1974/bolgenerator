/*
    BoltGenerator is an automated CAD assistant which produces standard-size 3D
    bolts per ISO and ASME specifications.
    Copyright (C) 2021  Scimulate LLC <solvers@scimulate.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    directory = QDir::home();

    ui->labelLogo->setScaledContents(true);

    ui->cboxThread->insertItems(0, dimensions.Threads());
    ui->cboxHead->insertItems(0, dimensions.Heads());

    connect(ui->actionLicense, &QAction::triggered, this, &MainWindow::License);
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionDirectory, &QAction::triggered, this, &MainWindow::CurrentDirectory);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    connect(ui->pushExport, &QPushButton::clicked, this, &MainWindow::Export);

    ui->advWidget->setSource(QUrl::fromLocalFile(":/adv.qml"));
    ui->advWidget->rootObject()->setProperty("displayMode", 1);
    ui->advWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QMetaObject::invokeMethod(ui->advWidget->rootObject(), "play");

    //ui->advWidget->setClearColor(Qt::transparent);
    //ui->advWidget->setProperty("displayMode", 1);

    //ui->advWidget->setAttribute(Qt::WA_AlwaysStackOnTop, 1);
    //ui->advWidget->setAttribute(Qt::WA_TranslucentBackground, 1);
    //! [8]
    //ui->advWidget.setSizePolicy(QSizePolicy::Expanding, 0);

    //connect(ui->advWidget, SIGNAL(advertisementEnded()), this, SLOT(onAdvEnded()));

    //advWidget->rootObject()->setProperty("displayMode", 1);

    ui->statusbar->showMessage("Ready!");
}

MainWindow::~MainWindow()
{
    delete ui;
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

        QString file = dimensions.Prefix(ui->cboxThread->currentIndex());
        file += ui->cboxHead->currentText()
                + "_" +
                QString::number(1.0e-3*length, 'e', 3).split(".").join("");

        QString unit;
        if(ui->radioInch->isChecked())
        {
            unit = "in";
            length = inchTOmeter(length);
        }
        else
        {
            unit = "mm";
            length = milliTOmeter(length);
        }
        file += unit;

        Bolt bolt = Bolt(ui->cboxHead->currentIndex(),
                         ui->cboxThread->currentIndex(),
                         length);

        file += ".step";

        ExportSTEP(bolt.Solid(), (directory.absolutePath()
                                  + QDir::toNativeSeparators("/")
                                  + file).toStdString().c_str());

        ui->statusbar->showMessage("Success! (" + ui->cboxThread->currentText()
                                   + " " + ui->cboxHead->currentText() + ", L="
                                   + ui->textLength->text() + unit + ")", 5000);
    }
    else
    {
        ui->statusbar->showMessage("Invalid configuration.", 2500);
    }
}

void MainWindow::License()
{
    QMessageBox::about(this,
                       tr("License Information"),
                       tr("<b>BoltGenerator</b> is an automated CAD assistant which produces 3D bolts per ISO and ASME specifications, built using <a href=\"https://www.opencascade.com/open-cascade-technology\">OpenCASCADE</a> and <a href=\"https://www.qt.io/\">Qt</a>."
                          "<br><br>Copyright (C) 2021-2022 <a href=\"https://www.scimulate.com\">Scimulate LLC</a>"
                          "<br><br>BoltGenerator is licensed under GNU GPLv3."
                          "<br><br>Scimulate, \"Solving The Unsolved\", and the Scimulate logos are trademarks of Scimulate LLC."));
}

void MainWindow::CurrentDirectory()
{
    directory = QFileDialog::getExistingDirectory(this, tr("Set Output Directory"));
}
