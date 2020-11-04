//
// Created by henryhuang on 11/2/20.
//

#include <QtWidgets/QFormLayout>
#include <QtCore/QStandardPaths>
#include <QtCore/QFileInfo>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDesktopServices>
#include <QPushButton>
#include <QDir>
#include "HttpClient.h"
#include <iostream>
#include <QtXml/QDomDocument>

const char defaultUrl[] = "http://127.0.0.1:5000/annotation";
const char defaultFileName[] = "annotation.xml";

HttpClient::HttpClient(QWidget *parent, WorkMode workMode, QByteArray imgCheckSum)
        : QDialog(parent)
        , workMode(workMode)
        , imgCheckSum(imgCheckSum)
        , statusLabel(new QLabel(tr("Please enter the URL endpoint of this HTTP Request.\n\n"), this))
        , urlLineEdit(new QLineEdit(defaultUrl))
        , connectButton(new QPushButton(tr("Connect")))
        , launchCheckBox(new QCheckBox("Launch file"))
        , localFileLineEdit(new QLineEdit(defaultFileName))
        , localDirectoryLineEdit(new QLineEdit)
        , reply(nullptr)
        , file(nullptr)
        , httpRequestAborted(false)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    titleMap[WorkMode::Download] = QString("HTTP - Download");
    titleMap[WorkMode::Region] = QString("HTTP - Region Detection");
    titleMap[WorkMode::Upload] = QString("HTTP - Upload");

    setWindowTitle(titleMap[workMode]);

    connect(&qnam, &QNetworkAccessManager::authenticationRequired,
            this, &HttpClient::slotAuthenticationRequired);

    #ifndef QT_NO_SSL
    connect(&qnam, &QNetworkAccessManager::sslErrors,
            this, &HttpClient::sslErrors);
    #endif

    QFormLayout *formLayout = new QFormLayout;
    urlLineEdit->setText(urlLineEdit->text() + "/" + imgCheckSum.toHex());
    urlLineEdit->setClearButtonEnabled(true);
    connect(urlLineEdit, &QLineEdit::textChanged,
            this, &HttpClient::enableConnectButton);
    formLayout->addRow(tr("&URL:"), urlLineEdit);

    QString localDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (localDirectory.isEmpty() || !QFileInfo(localDirectory).isDir())
      localDirectory = QDir::currentPath();
    localDirectoryLineEdit->setText(QDir::toNativeSeparators(localDirectory));
    if (workMode == Upload) {
      // use temp directory to save intermediate files
      localDirectoryLineEdit->setEnabled(false);
      auto now = QDateTime::currentDateTime();
      QString tempFileName = now.toString("yyyyMMdd_hhmmss") + QString(".xml");
      localFileLineEdit->setText(tempFileName);
      localFileLineEdit->setEnabled(false);
    }


    formLayout->addRow(tr("Local directory:"), localDirectoryLineEdit);
    formLayout->addRow(tr("Local file:"), localFileLineEdit);
    launchCheckBox->setChecked(true);
    formLayout->addRow(launchCheckBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);

    mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));

    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);

    // connect button signal-slot allocation
    connectButton->setDefault(true);
    if (workMode == Download)
      connect(connectButton, &QAbstractButton::clicked, this, &HttpClient::downloadFile);
    else if (workMode == Upload)
      connect(connectButton, &QAbstractButton::clicked, this, &HttpClient::initUploadFile);

  QPushButton *quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);

    // buttonBox Layout
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
    mainLayout->addWidget(buttonBox);

    urlLineEdit->setFocus();
}

HttpClient::~HttpClient() = default;

void HttpClient::startGetRequest(const QUrl &requestedUrl)
{
    url = requestedUrl;
    httpRequestAborted = false;

    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &HttpClient::httpFinished);

    if (workMode == Download)
      connect(reply, &QIODevice::readyRead, this, &HttpClient::writeHttpReplyToFile);

//    ProgressDialog *progressDialog = new ProgressDialog(url, this);
//    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
//    connect(progressDialog, &QProgressDialog::canceled, this, &HttpWindow::cancelDownload);
//    connect(reply, &QNetworkReply::downloadProgress, progressDialog, &ProgressDialog::networkReplyProgress);
//    connect(reply, &QNetworkReply::finished, progressDialog, &ProgressDialog::hide);
//    progressDialog->show();

    statusLabel->setText(tr("Connecting to %1...").arg(url.toString()));
}

void HttpClient::startPostRequest(QNetworkRequest networkRequest, QByteArray dataArray)
{
  httpRequestAborted = false;

  reply  = qnam.post(networkRequest, dataArray);
  connect(reply, &QNetworkReply::finished, this, &HttpClient::httpFinished);

  statusLabel->setText(tr("Connecting to %1...").arg(networkRequest.url().toString()));
}

QUrl HttpClient::parseUrl() {
  const QString urlSpec = urlLineEdit->text().trimmed();
  if (urlSpec.isEmpty())
    return QUrl();

  const QUrl newUrl = QUrl::fromUserInput(urlSpec);
  if (!newUrl.isValid()) {
    QMessageBox::information(this, tr("Error"),
                             tr("Invalid URL: %1: %2").arg(urlSpec, newUrl.errorString()));
    return QUrl();
  }
  return newUrl;
}

void HttpClient::downloadFile()
{
    const QUrl newUrl = parseUrl();
    if (newUrl.isEmpty() || !newUrl.isValid())
      return;
    QString fileName = QString();
    if (fileName.isEmpty())
        fileName = localFileLineEdit->text().trimmed();
    if (fileName.isEmpty())
        fileName = defaultFileName;
    QString downloadDirectory = QDir::cleanPath(localDirectoryLineEdit->text().trimmed());
    bool useDirectory = !downloadDirectory.isEmpty() && QFileInfo(downloadDirectory).isDir();
    if (useDirectory)
        fileName.prepend(downloadDirectory + '/');
    if (QFile::exists(fileName)) {
        if (QMessageBox::question(this, tr("Overwrite Existing File"),
                                  tr("There already exists a file called %1%2."
                                     " Overwrite?")
                                          .arg(fileName,
                                               useDirectory
                                               ? QString()
                                               : QStringLiteral(" in the current directory")),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No)
            == QMessageBox::No) {
            return;
        }
        QFile::remove(fileName);
    }

    file = openFile(fileName, QIODevice::WriteOnly);
    if (!file)
        return;

    connectButton->setEnabled(false);

    // schedule the request
  startGetRequest(newUrl);
}

void HttpClient::writeHttpReplyToFile()
{
  // this slot gets called every time the QNetworkReply has new data.
  // We read all of its new data and write it into the file.
  // That way we use less RAM than when reading it at the finished()
  // signal of the QNetworkReply
  if (file)
    file->write(reply->readAll());
}

std::unique_ptr<QFile> HttpClient::openFile(const QString &fileName, const QIODevice::OpenModeFlag modeFlag)
{
    std::unique_ptr<QFile> filePtr(new QFile(fileName));
    if ((modeFlag == QIODevice::WriteOnly || modeFlag == QIODevice::ReadWrite) && !filePtr->open(modeFlag)) {
        QMessageBox::information(this, tr("Error"),
                                 tr("Unable to save the file %1: %2.")
                                         .arg(QDir::toNativeSeparators(fileName),
                                              filePtr->errorString()));
        return nullptr;
    }
    else if ((modeFlag == QIODevice::ReadOnly || modeFlag == QIODevice::ReadWrite) && !filePtr->open(modeFlag)) {
      QMessageBox::information(this, tr("Error"),
                               tr("Unable to read the file %1: %2.")
                                   .arg(QDir::toNativeSeparators(fileName),
                                        filePtr->errorString()));
      return nullptr;
    }
    return filePtr;
}

void HttpClient::cancelDownload()
{
    statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    reply->abort();
    connectButton->setEnabled(true);
}

void HttpClient::httpFinished()
{
    QFileInfo fi;
    if (file) {
        fi.setFile(file->fileName());
        file->close();
        file.reset();
    }

    if (httpRequestAborted) {
        reply->deleteLater();
        reply = nullptr;
        return;
    }

    if (reply && reply->error()) {
        QFile::remove(fi.absoluteFilePath());
        statusLabel->setText(tr("HTTP request failed:\n%1.").arg(reply->errorString()));
        connectButton->setEnabled(true);
        reply->deleteLater();
        reply = nullptr;
        return;
    }

    reply->deleteLater();
    reply = nullptr;

    statusLabel->setText(tr("Transmission Finished!\nSummary: %1 KB of file %2 in %3")
                           .arg(QString::number(fi.size() / 1024. / 8, 'f', 2))
                           .arg(fi.fileName())
                           .arg(QDir::toNativeSeparators(fi.absolutePath())));

    if (launchCheckBox->isChecked() && workMode == Download){
        auto dstFilePath = fi.absoluteFilePath();
        emit receivedFileDstChanged(dstFilePath);
    }

    connectButton->setEnabled(true);
}

void HttpClient::enableConnectButton()
{
    connectButton->setEnabled(!urlLineEdit->text().isEmpty());
}

void HttpClient::slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator)
{
//    QDialog authenticationDialog;
//    Ui::Dialog ui;
//    ui.setupUi(&authenticationDialog);
//    authenticationDialog.adjustSize();
//    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm(), url.host()));
//
//    // Did the URL have information? Fill the UI
//    // This is only relevant if the URL-supplied credentials were wrong
//    ui.userEdit->setText(url.userName());
//    ui.passwordEdit->setText(url.password());
//
//    if (authenticationDialog.exec() == QDialog::Accepted) {
//        authenticator->setUser(ui.userEdit->text());
//        authenticator->setPassword(ui.passwordEdit->text());
//    }
}

void HttpClient::initUploadFile() {
  auto now = QDateTime::currentDateTime();
  QString tempFileName = now.toString("yyyyMMdd_hhmmss") + QString(".xml");
//  localFileLineEdit->setEnabled(true);
  localFileLineEdit->setText(tempFileName);
  QString downloadDirectory = QDir::cleanPath(localDirectoryLineEdit->text().trimmed());
  tempFileName.prepend(downloadDirectory + '/');
//  std::cout << tempFileName.toStdString() << std::endl;
  emit uploadStarted(tempFileName);
}

void HttpClient::uploadFile() {
  QFileInfo fi;
  QString downloadDirectory = QDir::cleanPath(localDirectoryLineEdit->text().trimmed());
  QString fileName = localFileLineEdit->text().trimmed();

  std::cout << downloadDirectory.toStdString() + "/" << fileName.toStdString() << std::endl;

  fileName.prepend(downloadDirectory + '/');

  file = openFile(fileName, QIODevice::ReadOnly);

  if (file)
    fi.setFile(file -> fileName());

  statusLabel->setText(tr("Uploading \"%1\" from %2;\nSize: %3 KB.")
                        .arg(fi.fileName())
                        .arg(fi.filePath())
                        .arg(QString::number(fi.size() / 1024. / 8, 'f', 2)));

  QDomDocument doc("xml_doc");
  if (!doc.setContent(&(*file))) {
    std::cout << "Cannot load xml file from " + file->fileName().toStdString() + "." << std::endl;
  }

  auto dataArray = doc.toByteArray();

  QNetworkRequest request;
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
  request.setUrl(parseUrl());

  startPostRequest(request, dataArray);

}

#ifndef QT_NO_SSL
void HttpClient::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }

    if (QMessageBox::warning(this, tr("SSL Errors"),
                             tr("One or more SSL errors has occurred:\n%1").arg(errorString),
                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
        reply->ignoreSslErrors();
    }
}
#endif