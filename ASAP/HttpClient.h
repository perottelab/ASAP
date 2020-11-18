//
// Created by henryhuang on 11/2/20.
//

#ifndef DIAGPATHOLOGY_HTTPCLIENT_H
#define DIAGPATHOLOGY_HTTPCLIENT_H


#include <QtWidgets/QDialog>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QAuthenticator>
#include <QtCore/QFile>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTableWidget>
#include <QUrlQuery>
#include <QtWidgets/QTextEdit>

class HttpClient : public QDialog {
    Q_OBJECT

public:
    enum WorkMode {
        Download = 0x0001,
        Upload = 0x0002,
        Region = 0x0003
      };

    HttpClient(QWidget *parent, WorkMode workMode, const QByteArray& imgCheckSum, QTableWidget* tableWidget);
    ~HttpClient();

    void startGetRequest(const QUrl &requestedUrl);

public slots:
    void uploadFile();

signals:
    void receivedFileDstChanged(const QString& filePath);
    void uploadStarted(const QString&);

private slots:
    void downloadFile();
    void initUploadFile();
    void cancelDownload();
    void httpFinished();
    void writeHttpReplyToFile();
    void enableConnectButton();
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    std::unique_ptr<QFile> openFile(const QString &fileName, const QIODevice::OpenModeFlag openModeFlag);

    QLabel *statusLabel;
    QLineEdit *urlLineEdit;
    QPushButton *connectButton;
    QCheckBox *launchCheckBox;
    QLineEdit *localFileLineEdit;
    QLineEdit *localDirectoryLineEdit;
    QTextEdit *argsTextEdit;

    QUrl url;
    QUrlQuery urlQuery;
    QByteArray imgCheckSum;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    std::unique_ptr<QFile> file;
    bool httpRequestAborted;

    QMap<WorkMode, QString> titleMap;
    WorkMode workMode;
    QTableWidget* tableWidget;

  QUrl parseUrl();

  void startPostRequest(const QNetworkRequest& networkRequest, const QByteArray& dataArray);

  QUrlQuery constructRegionQuery();
};


#endif //DIAGPATHOLOGY_HTTPCLIENT_H
