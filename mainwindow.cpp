#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QStringList>
#include <QScrollBar>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(geometry().size());

    ConversionProcess = new QProcess(this);
    MediaInfoProcess = new QProcess(this);

    connect(MediaInfoProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(ReadMediaInfo()));

    connect(ConversionProcess, SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
    connect(ConversionProcess, SIGNAL(finished(int)), this, SLOT(encodingFinished()));

    if(!QFile(QDir::currentPath() + "/ffmpeg.exe").exists())
    {
        QMessageBox::information(this, tr("ffmpeg"),tr("ffmpeg.exe not found in current directory"));
        return;
    }
    if(!QFile(QDir::currentPath() + "/MediaInfo.exe").exists())
    {
        QMessageBox::information(this, tr("MediaInfo"),tr("MediaInfo.exe not found in current directory"));
        return;
    }

    LastDirectory = QDir::currentPath();
    ui->editInputDirectory->setText(LastDirectory + "/Downloads");
    QDir dir(LastDirectory + "/Downloads", "*.mp4");
    QFileInfoList files = dir.entryInfoList();
    foreach (QFileInfo file, files) {
        selectedVideos.append(file.absoluteFilePath());
    }

    QString text = "The following videos do not have " + QString::number(WIDTH) + "x" + QString::number(HEIGHT) + " resolution\n";
    bool removed = FALSE;
    foreach (QString video, selectedVideos)
    {
        MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Video;Resolution %Width% %Height%\" \"" + video + "\"");
        MediaInfoProcess->waitForFinished();
        if(videoWidth != WIDTH || videoHeight != HEIGHT)
        {
            text += QFileInfo(video).fileName() + "\n";
            selectedVideos.removeOne(video);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Resolution"), text);

    text = "The following videos have more than 7 words in their names\n";
    removed = FALSE;
    for(int i = 0; i < selectedVideos.length(); i++)
    {
        if(!WordsCountCheck(selectedVideos[i]))
        {
            text += QFileInfo(selectedVideos[i]).fileName() + "\n";
            selectedVideos.removeOne(selectedVideos[i]);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Name"), text);


    ui->lblSelectedVideosCount->setText(QString::number(selectedVideos.count()) + " selected videos");
    foreach (QString file, selectedVideos) {
        ui->txtSelectedVideos->append(QFileInfo(file).fileName());
    }
    on_rbtnDirectory_toggled(TRUE);

    files = QDir(QDir::currentPath() + "/Prerolls to use", "*.mp4").entryInfoList();
    foreach (QFileInfo file, files) {
        prerolls.append(file.absoluteFilePath());
    }

    text = "The following prerolls do not have " + QString::number(WIDTH) + "x" + QString::number(HEIGHT) + " resolution\n";
    removed = FALSE;
    foreach (QString video, prerolls)
    {
        MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Video;Resolution %Width% %Height%\" \"" + video + "\"");
        MediaInfoProcess->waitForFinished();
        if(videoWidth != WIDTH || videoHeight != HEIGHT)
        {
            text += QFileInfo(video).fileName() + "\n";
            prerolls.removeOne(video);
            removed = TRUE;
        }
        WordsCountCheck(video);
    }
    if(removed)
        QMessageBox::information(this, tr("Resolution"), text);

    text = "The following videos have more than 7 words in their names\n";
    removed = FALSE;
    for(int i = 0; i < prerolls.length(); i++)
    {
        if(!WordsCountCheck(prerolls[i]))
        {
            text += QFileInfo(prerolls[i]).fileName() + "\n";
            prerolls.removeOne(prerolls[i]);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Name"), text);

    foreach (QString file, prerolls) {
        ui->txtPrerollsList->append(QFileInfo(file).fileName());
    }

    ui->lblStatus->setText("Ready");

    outputDirectory = QDir::currentPath() + "/Joined";
    ui->editOutputDirectory->setText(outputDirectory);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processStarted()
{
    ui->lblStatus->setText(messageQueue.back());
    ui->lblStatus->repaint();
    messageQueue.pop_back();
}

void MainWindow::readyReadStandardOutput()
{
    QString output = ConversionProcess->readAllStandardOutput();
    if(output == "") return;
    ui->txtStandardOutput->append(output);

    ui->txtStandardOutput->verticalScrollBar()->setSliderPosition(ui->txtStandardOutput->verticalScrollBar()->maximum());
}

void MainWindow::encodingFinished()
{
    // Set the encoding status by checking output file's existence

    if (QFile::exists(lastConvertedPath.back()))
        ui->txtStandardOutput->append("Conversion Status: Successful!\n\n");
    else
    {
        ui->txtStandardOutput ->append("Conversion Status: Failed!\n\n");
        on_btnStop_clicked();
    }
    lastConvertedPath.pop_back();

    QString lastDeleted;
    if(!processQueue.isEmpty() && processQueue.back() == "delete")
    {
        lastDeleted = toBeDeleted.back();
        if(QFile(lastDeleted).exists())
            QFile(lastDeleted).remove();
        toBeDeleted.pop_back();
        processQueue.pop_back();
    }

    if(!processQueue.isEmpty())
    {
        processStarted();
        ConversionProcess->start(processQueue.back());
        processQueue.pop_back();
    }
    else
    {
        ui->btnStart->setEnabled(TRUE);
        ui->btnBrowseOutputDirectory->setEnabled(TRUE);
        ui->btnAddPreroll->setEnabled(TRUE);
        ui->btnRemoveLastPreroll->setEnabled(TRUE);
        if(ui->rbtnDirectory->isChecked())
            ui->btnBrowseInputDirectory->setEnabled(TRUE);
        if(ui->rbtnVideos->isChecked())
        {
            ui->btnClearSelection->setEnabled(TRUE);
            ui->btnBrowseVideos->setEnabled(TRUE);
        }

        foreach (QString file, toBeDeleted) {
            if(QFile(file).exists())
                QFile(file).remove();
        }

        QDir dir(QFileInfo(lastDeleted).path());
        if(dir.exists())
            dir.rmdir(dir.path());

        if(stopped == TRUE)
        {
            ui->lblStatus->setText("Stopped");
            ui->lblStatus->repaint();
            return;
        }

        ui->lblStatus->setText("Finished");
        ui->lblStatus->repaint();
    }
}

void MainWindow::ReadMediaInfo()
{
    QString output = MediaInfoProcess->readAllStandardOutput();
    if(output.startsWith("SampleRate"))
    {
        sampleRate = output.mid(10, 5);
    }
    if(output.startsWith("Resolution"))
    {
        QStringList temp = output.split(' ');
        if(temp.count() < 3) return;
        videoWidth = temp.at(1).toInt();
        videoHeight = temp.at(2).left(temp.at(2).indexOf('\\')).toInt();
    }
}

void MainWindow::on_btnBrowseInputDirectory_clicked()
{
    QFileDialog browse;
    QString filePath = browse.getExistingDirectory();
    if(filePath.isEmpty()) return;
    LastDirectory = filePath;
    ui->editInputDirectory->setText(filePath);

    ui->txtSelectedVideos->clear();
    QDir dir(filePath, "*.mp4");
    QFileInfoList files = dir.entryInfoList();
    selectedVideos.clear();
    foreach (QFileInfo file, files) {
        selectedVideos.append(file.absoluteFilePath());
    }

    QString text = "The following videos do not have " + QString::number(WIDTH) + "x" + QString::number(HEIGHT) + " resolution\n";
    bool removed = FALSE;
    foreach (QString video, selectedVideos)
    {
        MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Video;Resolution %Width% %Height%\" \"" + video + "\"");
        MediaInfoProcess->waitForFinished();
        if(videoWidth != WIDTH || videoHeight != HEIGHT)
        {
            text += QFileInfo(video).fileName() + "\n";
            selectedVideos.removeOne(video);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Resolution"), text);

    text = "The following videos have more than 7 words in their names\n";
    removed = FALSE;
    for(int i = 0; i < selectedVideos.length(); i++)
    {
        if(!WordsCountCheck(selectedVideos[i]))
        {
            text += QFileInfo(selectedVideos[i]).fileName() + "\n";
            selectedVideos.removeOne(selectedVideos[i]);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Name"), text);

    ui->lblSelectedVideosCount->setText(QString::number(selectedVideos.count()) + " selected videos");
    ui->txtSelectedVideos->clear();
    foreach (QString file, selectedVideos) {
        ui->txtSelectedVideos->append(QFileInfo(file).fileName());
    }
}

void MainWindow::on_btnBrowseVideos_clicked()
{
    QFileDialog browse;
    selectedVideos += browse.getOpenFileNames(this, "Select Videos", LastDirectory, "Video Files (*.mp4)");
    selectedVideos.removeDuplicates();
    foreach (QString file, selectedVideos) {
        if(!file.endsWith(".mp4")) selectedVideos.removeOne(file);
    }
    if(selectedVideos.isEmpty())
    {
        ui->lblSelectedVideosCount->setText("0 selected videos");
        return;
    }
    LastDirectory = QFileInfo(selectedVideos.at(selectedVideos.length() - 1)).path();
    ui->txtSelectedVideos->clear();

    QString text = "The following videos do not have " + QString::number(WIDTH) + "x" + QString::number(HEIGHT) + " resolution\n";
    bool removed = FALSE;
    foreach (QString video, selectedVideos)
    {
        MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Video;Resolution %Width% %Height%\" \"" + video + "\"");
        MediaInfoProcess->waitForFinished();
        if(videoWidth != WIDTH || videoHeight != HEIGHT)
        {
            text += QFileInfo(video).fileName() + "\n";
            selectedVideos.removeOne(video);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Resolution"), text);

    text = "The following videos have more than 7 words in their names\n";
    removed = FALSE;
    for(int i = 0; i < selectedVideos.length(); i++)
    {
        if(!WordsCountCheck(selectedVideos[i]))
        {
            text += QFileInfo(selectedVideos[i]).fileName() + "\n";
            selectedVideos.removeOne(selectedVideos[i]);
            removed = TRUE;
        }
    }
    if(removed)
        QMessageBox::information(this, tr("Name"), text);

    foreach (QString file, selectedVideos) {
        ui->txtSelectedVideos->append(QFileInfo(file).fileName());
    }

    ui->lblSelectedVideosCount->setText(QString::number(selectedVideos.count()) + " selected videos");
}

void MainWindow::on_rbtnDirectory_toggled(bool checked)
{
    if(checked)
    {
        ui->editInputDirectory->setEnabled(TRUE);
        ui->btnBrowseInputDirectory->setEnabled(TRUE);
        ui->btnClearSelection->setEnabled(FALSE);
        ui->btnBrowseVideos->setEnabled(FALSE);
    }
}

void MainWindow::on_rbtnVideos_toggled(bool checked)
{
    if(checked)
    {
        ui->editInputDirectory->setEnabled(FALSE);
        ui->btnBrowseInputDirectory->setEnabled(FALSE);
        ui->btnClearSelection->setEnabled(TRUE);
        ui->btnBrowseVideos->setEnabled(TRUE);
    }
}

void MainWindow::on_btnAddPreroll_clicked()
{
    QFileDialog browse;
    QString filePath = browse.getOpenFileName(this, "Select Preroll", LastDirectory, "Video Files (*.mp4)");
    if(filePath.isEmpty()) return;
    LastDirectory = QFileInfo(filePath).path();

    MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Video;Resolution %Width% %Height%\" \"" + filePath + "\"");
    MediaInfoProcess->waitForFinished();
    if(videoWidth != WIDTH || videoHeight != HEIGHT)
    {
        QMessageBox::information(this, tr("Resolution"), "The selected video doesn't have " + QString::number(WIDTH) + "x" + QString::number(HEIGHT) + " resolution\n");
        return;
    }


    prerolls.append(filePath);
    prerolls.removeDuplicates();
    foreach (QString file, prerolls) {
        if(!file.endsWith(".mp4"))
            prerolls.removeOne(file);
    }

    ui->txtPrerollsList->clear();
    foreach (QString file, prerolls) {
        ui->txtPrerollsList->append(QFileInfo(file).fileName());
    }
}

void MainWindow::on_btnClearSelection_clicked()
{
    selectedVideos.clear();
    ui->lblSelectedVideosCount->setText("0 selected videos");
    ui->txtSelectedVideos->clear();
}

void MainWindow::on_btnBrowseOutputDirectory_clicked()
{
    QFileDialog browse;
    QString filePath = browse.getExistingDirectory(this, "Select Output Folder", LastDirectory);
    if(filePath.isEmpty()) return;
    outputDirectory = filePath;
    LastDirectory = outputDirectory;
    ui->editOutputDirectory->setText(outputDirectory);
}

void MainWindow::on_btnRemoveLastPreroll_clicked()
{
    if(prerolls.isEmpty()) return;
    prerolls.removeLast();
    ui->txtPrerollsList->clear();
    foreach (QString file, prerolls) {
        ui->txtPrerollsList->append(QFileInfo(file).fileName());
    }
}

void MainWindow::on_btnStart_clicked()
{
    if(!QFile(QDir::currentPath() + "/ffmpeg.exe").exists())
    {
        QMessageBox::information(this, tr("ffmpeg"),tr("ffmpeg.exe not found in current directory"));
        return;
    }
    if(!QFile(QDir::currentPath() + "/MediaInfo.exe").exists())
    {
        QMessageBox::information(this, tr("MediaInfo"),tr("MediaInfo.exe not found in current directory"));
        return;
    }
    if(selectedVideos.isEmpty())
    {
        QMessageBox::information(this, tr("Videos"),tr("No videos selected"));
        return;
    }
    if(prerolls.isEmpty())
    {
        QMessageBox::information(this, tr("Prerolls"),tr("No prerolls selected"));
        return;
    }

    quint64 size = 0;
    foreach (QString video, selectedVideos) {
        size += QFileInfo(video).size();
    }
    foreach (QString preroll, prerolls) {
        size += QFileInfo(preroll).size()*2*selectedVideos.count();
    }
    quint64 availableBytes = QStorageInfo(outputDirectory).bytesAvailable();
    size *= 1.2;
    if(availableBytes < size)
    {
        QMessageBox::information(this, tr("Size"),"The joined videos will occupy approximately " + QString::number(size/1024/1024/1024.0) + "GB. Make sure you have enough free space.\n"
                                                         + "You have " + QString::number(availableBytes/1024/1024/1024.0) + "GB available.");
        return;
    }

    //ffmpeg -i input1.mp4 -c copy -bsf:v h264_mp4toannexb -f mpegts intermediate1.ts
    //ffmpeg -i input2.mp4 -c copy -bsf:v h264_mp4toannexb -f mpegts intermediate2.ts
    //ffmpeg -i "concat:intermediate1.ts|intermediate2.ts" -c copy -bsf:a aac_adtstoasc output.mp4

    stopped = FALSE;
    currentFileConvertingNumber = 0;

    lastConvertedPath.clear();
    processQueue.clear();
    toBeDeleted.clear();
    messageQueue.clear();
    ui->txtStandardOutput->clear();
    ui->lblStatus->setText("Starting...");
    ui->lblStatus->repaint();

    ui->btnStart->setEnabled(FALSE);
    ui->btnAddPreroll->setEnabled(FALSE);
    ui->btnBrowseInputDirectory->setEnabled(FALSE);
    ui->btnBrowseOutputDirectory->setEnabled(FALSE);
    ui->btnClearSelection->setEnabled(FALSE);
    ui->btnRemoveLastPreroll->setEnabled(FALSE);
    ui->btnBrowseVideos->setEnabled(FALSE);



    QStringList frontDeleted;
    QString inputFirst = selectedVideos.at(0);
    QString intermediateFirst = QDir::currentPath() + "/Temp/" + QFileInfo(inputFirst).baseName() + ".ts";

    QDir dir(QFileInfo(intermediateFirst).path());
    if (!dir.exists())
        dir.mkpath(dir.path());
    dir.setPath(QDir::currentPath() + "/Prerolls");
    if (!dir.exists())
        dir.mkpath(dir.path());
    dir.setPath(outputDirectory);
    if (!dir.exists())
        dir.mkpath(dir.path());

    ConversionProcess->setProcessChannelMode(QProcess::MergedChannels);

    QString command = "ffmpeg -i \"" + inputFirst + "\" -c copy -bsf:v h264_mp4toannexb -f mpegts -y \"" + intermediateFirst + "\"";
    lastConvertedPath.push_front(intermediateFirst);


    QString input1, input2, intermediate1, intermediate2;

    foreach (QString video, selectedVideos)
    {
        input1 = video;
        intermediate1 = QDir::currentPath() + "/Temp/" + QFileInfo(input1).baseName() + ".ts";
        if(video != selectedVideos.at(0))
        {
            processQueue.push_front("ffmpeg -i \"" + input1 + "\" -c copy -bsf:v h264_mp4toannexb -f mpegts -y \"" + intermediate1 + "\"");
            lastConvertedPath.push_front(intermediate1);
        }

        messageQueue.push_front("(" + QString::number(++currentFileConvertingNumber) + "/"
                                                       +  QString::number(selectedVideos.count()) + ") 1.Converting:  "
                                                       + QFileInfo(intermediate1).fileName());

        QString videoSampleRate, prerollSampleRate;

        MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Audio;SampleRate%SamplingRate%\" \"" + video + "\"");
        MediaInfoProcess->waitForFinished();
        ui->txtStandardOutput->append(sampleRate + "Hz - " + QFileInfo(video).fileName());
        ui->txtStandardOutput->repaint();
        videoSampleRate = sampleRate;

        foreach (QString preroll, prerolls)
        {
            input2 = preroll;
            intermediate2 = QDir::currentPath() + "/Prerolls/" + QFileInfo(input2).baseName() + ".ts";

            MediaInfoProcess->start("MediaInfo --Language=raw --Full --Inform=\"Audio;SampleRate%SamplingRate%\" \"" + preroll + "\"");
            MediaInfoProcess->waitForFinished();
            ui->txtStandardOutput->append(sampleRate + "Hz - " + QFileInfo(preroll).fileName());
            ui->txtStandardOutput->repaint();
            prerollSampleRate = sampleRate;

            if(videoSampleRate != prerollSampleRate)
            {
                QString temp = QDir::currentPath() + "/Prerolls/" + QFileInfo(input2).baseName() + " " + videoSampleRate;
                if(!QFile(temp + ".ts").exists())
                {
                    ui->txtStandardOutput->append("Different Audio Sample Rates. Converting Preroll with video's Sample Rate: " + videoSampleRate + " Hz");
                    ui->txtStandardOutput->repaint();
                    processQueue.push_front("ffmpeg -i \"" + input2 + "\" -ar " + videoSampleRate + " -n \"" + temp + ".mp4" + "\"");
                    lastConvertedPath.push_front(temp + ".mp4");
                    messageQueue.push_front("(" + QString::number(currentFileConvertingNumber) + "/"
                                                                   +  QString::number(selectedVideos.count()) + ") 2.Additional.Converting:  "
                                                                   + QFileInfo(temp + ".mp4").fileName());
                    frontDeleted.push_front(temp + ".mp4");
                }
                input2 = temp + ".mp4";
                intermediate2 = temp + ".ts";
            }

            processQueue.push_front("ffmpeg -i \"" + input2 + "\" -c copy -bsf:v h264_mp4toannexb -f mpegts -n \"" + intermediate2 + "\"");
            lastConvertedPath.push_front(intermediate2);
            messageQueue.push_front("(" + QString::number(currentFileConvertingNumber) + "/"
                                                               + QString::number(selectedVideos.count()) + ") 2.Converting:  "
                                                               + QFileInfo(intermediate2).fileName());

            QString newOutputDirectory = outputDirectory + "/" + QFileInfo(preroll).baseName();
            QDir dir(newOutputDirectory);
            if (!dir.exists())
                dir.mkpath(newOutputDirectory);
            processQueue.push_front("ffmpeg -i \"concat:" + intermediate2 + "|" + intermediate1 + "|" + intermediate2 + "\" "
                                    + " -c copy -bsf:a aac_adtstoasc -y \""
                                    + newOutputDirectory + "/" + QFileInfo(input1).fileName() + "\"");
            lastConvertedPath.push_front(newOutputDirectory + "/" + QFileInfo(input1).fileName());
            messageQueue.push_front("(" + QString::number(currentFileConvertingNumber) + "/"
                                                           +  QString::number(selectedVideos.count()) + ") 3.Joining:   "
                                                           + QFileInfo(input1).fileName());
        }
        processQueue.push_front("delete");
        toBeDeleted.push_front(intermediate1);
    }
    foreach (QString file, frontDeleted) {
        toBeDeleted.push_front(file);
    }
    processStarted();
    ConversionProcess->start(command);
}


void MainWindow::on_btnStop_clicked()
{
    stopped = TRUE;
    processQueue.clear();
    foreach (QString file, toBeDeleted) {
        if(QFile(file).exists())
            QFile(file).remove();
    }

    if(!toBeDeleted.isEmpty())
    {
        QDir dir(QFileInfo(toBeDeleted.first()).path());
        if(dir.exists())
            dir.rmdir(dir.path());
    }
    if(ui->lblStatus->text() == "Ready" || ui->lblStatus->text() == "Finished" || ui->lblStatus->text() == "Stopped") return;
    ui->lblStatus->setText("Process will stop after completing last conversion process");
    ui->lblStatus->repaint();
}

bool MainWindow::WordsCountCheck(QString &s)
{
    QString name = QFileInfo(s).fileName();
    name = name.left(name.length() - QString(".mp4").length());
    QStringList temp = name.split(QRegExp("(\\ |\\,|\\.|\\:)"), QString::SkipEmptyParts);
    if(temp.count() > 7) return FALSE;
    name.clear();
    foreach (QString word, temp) {
        if(word[0].isLower())
            word[0] = word[0].toUpper();
        name += word + " ";
    }
    name.resize(name.length() - 1);
    name = QFileInfo(s).path() + "/" + name + "." + QFileInfo(s).suffix();
    QFile(s).rename(name);
    s = name;
    return TRUE;
}
