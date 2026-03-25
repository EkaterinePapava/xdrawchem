// http.cpp -- async HTTP helper using QNetworkAccessManager (Qt5)
// Fixes applied:
//   - Removed busy-wait spin loop: execute() now uses a local QEventLoop so
//     the Qt event loop keeps running while waiting for the reply.
//   - Default constructor no longer calls connect() on an uninitialized pointer;
//     init() is shared by both constructors.
//   - Old-style SIGNAL/SLOT macros replaced with pointer-to-member syntax.
//   - SSL errors logged via qWarning() instead of fprintf(stderr).
//   - downloadFinished() emits downloadComplete() instead of calling accept()
//     (HTTP is no longer a QDialog).
//   - Removed #include <cstdio> (no longer needed).

#include <QEventLoop>
#include <QDebug>

#include "http.h"

// ── Shared initialisation ─────────────────────────────────────────────────────
void HTTP::init()
{
    finished = false;
    manager  = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this,    &HTTP::downloadFinished);
}

// ── Default constructor ───────────────────────────────────────────────────────
HTTP::HTTP(QObject *parent)
    : QObject(parent)
{
    init();
}

// ── URL constructor ───────────────────────────────────────────────────────────
HTTP::HTTP(const QString &url, QObject *parent)
    : QObject(parent)
{
    init();
    qDebug() << "HTTP:" << url;
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::errorOccurred,
            this,  &HTTP::slotError);
#ifndef QT_NO_SSL
    connect(reply, &QNetworkReply::sslErrors,
            this,  &HTTP::sslErrors);
#endif
}

// ── doDownload ────────────────────────────────────────────────────────────────
void HTTP::doDownload(const QUrl &url)
{
    qDebug() << "doDownload:" << url;
    QNetworkReply *reply = manager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::errorOccurred,
            this,  &HTTP::slotError);
#ifndef QT_NO_SSL
    connect(reply, &QNetworkReply::sslErrors,
            this,  &HTTP::sslErrors);
#endif
    currentDownloads.append(reply);
}

// ── execute ───────────────────────────────────────────────────────────────────
// Starts the download and blocks using a local QEventLoop until the reply
// arrives.  The Qt event loop continues to run, so timers, other signals,
// and UI repaints are not starved.
void HTTP::execute(const QString &xurl)
{
    QEventLoop loop;
    connect(this, &HTTP::downloadComplete, &loop, &QEventLoop::quit);
    doDownload(QUrl::fromUserInput(xurl));
    loop.exec();
}

// ── exec ──────────────────────────────────────────────────────────────────────
// Blocks until downloadComplete is emitted (companion to the URL constructor).
void HTTP::exec()
{
    if (finished)
        return;
    QEventLoop loop;
    connect(this, &HTTP::downloadComplete, &loop, &QEventLoop::quit);
    loop.exec();
}

// ── Slots ─────────────────────────────────────────────────────────────────────
void HTTP::slotError(QNetworkReply::NetworkError e)
{
    qWarning() << "HTTP network error:" << e;
}

void HTTP::sslErrors(const QList<QSslError> &errors)
{
#ifndef QT_NO_SSL
    for (const QSslError &error : errors)
        qWarning() << "SSL error:" << error.errorString();
#else
    Q_UNUSED(errors);
#endif
}

void HTTP::downloadFinished(QNetworkReply *reply)
{
    replyText = reply->readAll();
    qDebug() << "downloadFinished:" << replyText;
    currentDownloads.removeAll(reply);
    reply->deleteLater();
    finished = true;
    emit downloadComplete();
}

// ── Accessors ─────────────────────────────────────────────────────────────────
QString HTTP::Data() const
{
    return replyText;
}
