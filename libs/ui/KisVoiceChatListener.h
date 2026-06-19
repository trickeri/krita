/*
 *  Nuldrums: voicechat transcript listener.
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_VOICE_CHAT_LISTENER_H
#define KIS_VOICE_CHAT_LISTENER_H

#include <QByteArray>
#include <QObject>

#include "kritaui_export.h"

class QLocalSocket;
class QTimer;

/**
 * @brief Listens on the voicechat dictation daemon's transcript socket and emits a signal
 * for each finished transcript.
 *
 * voicechat (the headless dictation daemon) broadcasts every finished transcript on a Unix
 * domain socket — @c $VOICECHAT_SOCKET, else @c $XDG_RUNTIME_DIR/voicechat.sock — as one JSON
 * line per transcript: <tt>{"text":"…","app":"…","mode":"…","ts":…}</tt>. When Krita is the
 * focused app, voicechat uses its "emit" mode: it does NOT synthesize a paste, expecting us
 * to consume the dictated text here instead.
 *
 * The listener connects to that socket, reconnects automatically if voicechat isn't running
 * yet or restarts, parses the newline-delimited JSON, and emits transcriptReceived() on the
 * GUI thread. It is read-only and best-effort: if the socket is unavailable it simply keeps
 * retrying and Krita is unaffected. A single instance serves the whole application (see
 * KisMainWindow), dispatching to the active main window.
 */
class KRITAUI_EXPORT KisVoiceChatListener : public QObject
{
    Q_OBJECT
public:
    explicit KisVoiceChatListener(QObject *parent = nullptr);
    ~KisVoiceChatListener() override;

    /** @brief The transcript socket path: @c $VOICECHAT_SOCKET, else @c $XDG_RUNTIME_DIR/voicechat.sock. */
    static QString socketPath();

Q_SIGNALS:
    /** @brief Emitted for each transcript. @p mode is voicechat's routing mode (e.g. "emit"). */
    void transcriptReceived(const QString &text, const QString &app, const QString &mode);

private Q_SLOTS:
    void onReadyRead();
    void scheduleReconnect();
    void tryConnect();

private:
    QLocalSocket *m_socket;
    QTimer *m_reconnect;
    QByteArray m_buffer;
};

#endif // KIS_VOICE_CHAT_LISTENER_H
