#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>

#define WIDTH 1280
#define HEIGHT 720

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
    void processStarted();
    void readyReadStandardOutput();
    void encodingFinished();
    void ReadMediaInfo();

    void on_btnBrowseInputDirectory_clicked();
    void on_btnBrowseVideos_clicked();
    void on_rbtnDirectory_toggled(bool checked);
    void on_rbtnVideos_toggled(bool checked);
    void on_btnAddPreroll_clicked();
    void on_btnClearSelection_clicked();
    void on_btnBrowseOutputDirectory_clicked();
    void on_btnRemoveLastPreroll_clicked();
    void on_btnStart_clicked();

    void on_btnStop_clicked();

private:
    Ui::MainWindow *ui;
    QString LastDirectory;
    QString outputDirectory;
    QStringList prerolls;
    QStringList selectedVideos;
    QProcess *ConversionProcess;
    QProcess *MediaInfoProcess;
    QStringList lastConvertedPath;
    QStringList processQueue;
    QStringList messageQueue;
    QStringList toBeDeleted;
    QString Output;
    QString sampleRate;
    QString fps;
    int videoWidth;
    int videoHeight;
    int currentFileConvertingNumber;
    bool stopped;
    bool WordsCountCheck(QString &s);
};

#endif // MAINWINDOW_H
