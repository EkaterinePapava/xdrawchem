// http.h -- async HTTP helper using QNetworkAccessManager (Qt5)
// Fixes applied:
//   - Base class changed from QDialog to QObject (HTTP is not a dialog)
//   - Removed busy-wait spin loop (now uses QEventLoop)
//   - Default constructor no longer uses uninitialized manager pointer
//   - Old-style SIGNAL/SLOT macros replaced with pointer-to-member syntax
//   - SSL errors properly logged via qWarning, not fprintf(stderr)
//   - Removed <stdio.h>

#ifndef HTTP_H
#define HTTP_H

#include <QObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>
#include <QString>
#include <QUrl>

QT_USE_NAMESPACE

class HTTP : public QObject
{
    Q_OBJECT

public:
    // Default constructor — creates a manager but does not start any request.
    explicit HTTP(QObject *parent = nullptr);

    // Convenience constructor: immediately starts downloading url.
    // Call exec() to block (via QEventLoop) until the reply arrives.
    explicit HTTP(const QString &url, QObject *parent = nullptr);

    // Block until the current download finishes.
    // Uses a local QEventLoop — the Qt event loop keeps running, no spin-wait.
    void exec();

    // Return the body of the last completed reply.
    QString Data() const;

    // Start an async GET for url.
    void doDownload(const QUrl &url);

    // Start download of xurl and block until done (calls exec()).
    void execute(const QString &xurl);

signals:
    void downloadComplete();

public slots:
    void downloadFinished(QNetworkReply *reply);
    void slotError(QNetworkReply::NetworkError e);
    void sslErrors(const QList<QSslError> &errors);

private:
    void init();

    QNetworkAccessManager *manager;
    QList<QNetworkReply *> currentDownloads;
    QString replyText;
    bool finished;
};

#endif // HTTP_H
