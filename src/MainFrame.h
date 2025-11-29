#include <QMainWindow>
#include <QFileInfo>
#include <QWidget>
#include <QFutureWatcher>
#include <QStandardPaths>
#include "Algorithm.h"
#include "MainWindowGUI.h"

class MainFrame : public QMainWindow {
    Q_OBJECT

public:
    MainFrame(QWidget* parent = nullptr);

private:
    Algorithm SAMAlgo;

    MainWindowGUI UI;

    QFutureWatcher<bool>* SetupWatcher;
    QFutureWatcher<bool>* SegmentWatcher;

private:
    void ValidateStart();
    void SegmentButtonClicked();
    void ReadRecentData();
    void ConnectSignals();
    void Segment(const QPoint& pt);
    void OnSetupFinished();
    void OnSegmentFinished();

    QString GetRecentFilePath() const
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(path);
        return path + "/recent.ini";
    }
};
