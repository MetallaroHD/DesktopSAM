#include "MainWindowGUI.h"

void MainWindowGUI::Init(QMainWindow* parent)
{
    Stacked = new QStackedWidget(parent);

    // Init page 0
    QWidget* page0 = new QWidget();
    QVBoxLayout* layout0 = new QVBoxLayout(page0);
    layout0->addStretch();
    QLabel* title = new QLabel("DesktopSAM");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: bold;");
    title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout0->addWidget(title);
    layout0->addStretch();
    EncoderPath = new QLineEdit();
    BtnBrowseEnc = new QPushButton("Browse");
    QHBoxLayout* encLayout = new QHBoxLayout();
    encLayout->addWidget(new QLabel("Encoder model:"));
    encLayout->addWidget(EncoderPath);
    encLayout->addWidget(BtnBrowseEnc);
    layout0->addLayout(encLayout);
    DecoderPath = new QLineEdit();
    BtnBrowseDec = new QPushButton("Browse");
    QHBoxLayout* decLayout = new QHBoxLayout();
    decLayout->addWidget(new QLabel("Decoder model:"));
    decLayout->addWidget(DecoderPath);
    decLayout->addWidget(BtnBrowseDec);
    layout0->addLayout(decLayout);
    BtnStart = new QPushButton("Start");
    BtnStart->setFixedWidth(120);
    BtnStart->setStyleSheet("font-size: 16px;");
    LoadingLabelM = new QLabel();
    LoadingLabelM->setFixedSize(24, 24);
    LoadingLabelM->setVisible(false);
    LoadingGIF = new QMovie(":/resources/resources/loading.gif");
    LoadingGIF->setScaledSize(QSize(24, 24));
    LoadingLabelM->setMovie(LoadingGIF);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(BtnStart);
    btnLayout->addSpacing(10);
    btnLayout->addWidget(LoadingLabelM);
    btnLayout->addStretch();
    layout0->addLayout(btnLayout);
    layout0->addStretch();
    Stacked->addWidget(page0);

    // Init page 1
    QWidget* page1 = new QWidget();
    QVBoxLayout* layout1 = new QVBoxLayout(page1);
    layout1->setAlignment(Qt::AlignTop);
    Canvas = new QtImageCanvas();
    layout1->addWidget(Canvas, 1);
    BtnChoose = new QPushButton("Choose Image");
    BtnSegment = new QPushButton("Segment");
    BtnReset = new QPushButton("Reset");
    BtnSave = new QPushButton("Save");
    BtnColor = new QPushButton("Color");
    LoadingLabelP = new QLabel();
    LoadingLabelP->setFixedSize(24, 24);
    LoadingLabelP->setVisible(false);
    LoadingLabelP->setMovie(LoadingGIF);
    QHBoxLayout* buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    buttonRow->addWidget(BtnChoose);
    buttonRow->addWidget(BtnSegment);
    buttonRow->addWidget(BtnReset);
    buttonRow->addWidget(BtnSave);
    buttonRow->addWidget(BtnColor);
    buttonRow->addSpacing(10);
    buttonRow->addWidget(LoadingLabelP);
    buttonRow->addStretch();
    layout1->addLayout(buttonRow);
    Stacked->addWidget(page1);

    parent->setCentralWidget(Stacked);
    parent->setMinimumSize(800, 600);
    Stacked->setCurrentIndex(0);
}

void MainWindowGUI::Loading(bool enabled)
{
    if (!enabled) {
        LoadingLabelM->setVisible(false);
        LoadingLabelP->setVisible(false);
        LoadingGIF->stop();
        return;
    }

    if (Stacked->currentIndex() == 0)
        LoadingLabelM->setVisible(true);
    else
        LoadingLabelP->setVisible(true);

    LoadingGIF->start();
}

void MainWindowGUI::SetEnabled(bool enabled)
{
    Canvas->setEnabled(enabled);
    EncoderPath->setEnabled(enabled);
    DecoderPath->setEnabled(enabled);

    BtnChoose->setEnabled(enabled);
    BtnSegment->setEnabled(enabled);
    BtnReset->setEnabled(enabled);
    BtnSave->setEnabled(enabled);
    BtnStart->setEnabled(enabled);
}
