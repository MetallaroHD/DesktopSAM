#pragma once
#include "QtImageCanvas.h"
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <qmainwindow.h>
#include <QMovie>

class MainWindowGUI : public QObject
{
    Q_OBJECT

public:
    QtImageCanvas* Canvas;
    QStackedWidget* Stacked;
    QLineEdit* EncoderPath;
    QLineEdit* DecoderPath;

    QPushButton* BtnBrowseEnc;
    QPushButton* BtnBrowseDec;
    QPushButton* BtnChoose;
    QPushButton* BtnSegment;
    QPushButton* BtnReset;
    QPushButton* BtnSave;
    QPushButton* BtnStart;
    QPushButton* BtnColor;

    QLabel* LoadingLabelM;
    QLabel* LoadingLabelP;
    QMovie* LoadingGIF;

public:
    /* Initialize GUI */
    void Init(QMainWindow* parent);

    /* Enable/disable UI */
    void SetEnabled(bool enabled);

    /* Start/stop the loading animation */
    void Loading(bool enabled);
};