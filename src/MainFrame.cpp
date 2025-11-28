#include "MainFrame.h"
#include <QTimer>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QStandardPaths>
#include <QCoreApplication>
#include <qcolordialog.h>
#include <QtConcurrent/qtconcurrentrun.h>

MainFrame::MainFrame(QWidget* parent) : QMainWindow(parent) 
{
    SetupWatcher = new QFutureWatcher<bool>(this);
    SegmentWatcher = new QFutureWatcher<bool>(this);

    UI.Init(this);
    ConnectSignals();
    ReadRecentData();
}

void MainFrame::ValidateStart()
{
    QString enc = UI.EncoderPath->text();
    QString dec = UI.DecoderPath->text();

    if (!QFileInfo::exists(enc)) {
        QMessageBox::warning(this, "Error", "Encoder file does not exist.");
        return;
    }
    if (!QFileInfo::exists(dec)) {
        QMessageBox::warning(this, "Error", "Decoder file does not exist.");
        return;
    }

    UI.SetEnabled(false);
    UI.Loading(true);

    QFuture<bool> f = QtConcurrent::run([=]() {
        return SAMAlgo.Init(enc.toStdString(), dec.toStdString());
        });
    SetupWatcher->setFuture(f);
}

void MainFrame::SegmentButtonClicked()
{
    UI.Canvas->setMode(SegmentPoint);
    UI.Canvas->setCursor(Qt::CrossCursor);
}

void MainFrame::Segment(const QPoint& pt)
{
    UI.SetEnabled(false);
    UI.Loading(true);

    std::pair<int, int> p(pt.x(), pt.y());

    QFuture<bool> f = QtConcurrent::run([=]() {

        // Create a temporary empty mask
        QImage newMask(UI.Canvas->mask.size(), QImage::Format_Grayscale8);
        newMask.fill(0);

        // Run SAM into the temporary mask
        bool ok = SAMAlgo.Run(UI.Canvas->image, &newMask, p);
        if (!ok)
            return false;

        // OR-merge new mask into existing mask
        for (int y = 0; y < UI.Canvas->mask.height(); ++y)
        {
            uchar* oldLine = UI.Canvas->mask.scanLine(y);
            const uchar* newLine = newMask.constScanLine(y);

            for (int x = 0; x < UI.Canvas->mask.width(); ++x)
            {
                if (newLine[x] > 0)     // predicted foreground
                    oldLine[x] = 255;   // add to existing mask
            }
        }

        return true;
        });

    SegmentWatcher->setFuture(f);
}

void MainFrame::OnSetupFinished()
{
    UI.Loading(false);
    UI.SetEnabled(true);

    if (!SetupWatcher->result()) {
        QMessageBox::critical(this, "Initialization Failed",
            "Couldn't initialize EfficientViT SAM.");
        return;
    }

    QSettings set(GetRecentFilePath(), QSettings::IniFormat);
    set.setValue("Models/Encoder", UI.EncoderPath->text());
    set.setValue("Models/Decoder", UI.DecoderPath->text());
    set.sync();

    UI.Stacked->setCurrentIndex(1);
}

void MainFrame::OnSegmentFinished()
{
    UI.Loading(false);
    UI.SetEnabled(true);

    if (!SegmentWatcher->result()) {
        QMessageBox::warning(this, "Segmentation Error", "Segmentation failed.");
        return;
    }

    UI.Canvas->update();
}

void MainFrame::ReadRecentData() 
{
    QSettings settings(GetRecentFilePath(), QSettings::IniFormat);
    QString encSaved = settings.value("Models/Encoder", "").toString();
    QString decSaved = settings.value("Models/Decoder", "").toString();

    if (!encSaved.isEmpty() && QFileInfo::exists(encSaved))
        UI.EncoderPath->setText(encSaved);

    if (!decSaved.isEmpty() && QFileInfo::exists(decSaved))
        UI.DecoderPath->setText(decSaved);
}

void MainFrame::ConnectSignals()
{
    connect(UI.BtnBrowseEnc, &QPushButton::clicked, this, [this]() {
        QString f = QFileDialog::getOpenFileName(this, "Select Encoder Model");
        if (!f.isEmpty()) UI.EncoderPath->setText(f);
        });

    connect(UI.BtnBrowseDec, &QPushButton::clicked, this, [this]() {
        QString f = QFileDialog::getOpenFileName(this, "Select Decoder Model");
        if (!f.isEmpty()) UI.DecoderPath->setText(f);
        });

    connect(UI.BtnStart, &QPushButton::clicked, this, &MainFrame::ValidateStart);

    connect(UI.BtnChoose, &QPushButton::clicked, this, [this]() {
        QString f = QFileDialog::getOpenFileName(
            this, "Open Image", QString(), "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!f.isEmpty())
            UI.Canvas->loadImage(f);
        });

    connect(UI.BtnSegment, &QPushButton::clicked, this, &MainFrame::SegmentButtonClicked);
    connect(UI.BtnReset, &QPushButton::clicked, UI.Canvas, &QtImageCanvas::resetMask);

    connect(UI.BtnSave, &QPushButton::clicked, this, [this]() {
        QString f = QFileDialog::getSaveFileName(this, "Save Image", "", "*.png");
        if (!f.isEmpty())
            UI.Canvas->getMaskedImage().save(f);
        });

    connect(UI.Canvas, &QtImageCanvas::segmentationPointSelected,
        this, &MainFrame::Segment);

    connect(UI.BtnColor, &QPushButton::clicked, this, [this]() {
        QColor col = QColorDialog::getColor(UI.Canvas->overlayColor, this, "Choose Overlay Color");
        if (col.isValid())
        {
            col.setAlpha(UI.Canvas->overlayColor.alpha());
            UI.Canvas->overlayColor = col;
            UI.Canvas->update();
        }
        });

    connect(SegmentWatcher, &QFutureWatcher<bool>::finished, this, &MainFrame::OnSegmentFinished);
    connect(SetupWatcher, &QFutureWatcher<bool>::finished, this, &MainFrame::OnSetupFinished);
}