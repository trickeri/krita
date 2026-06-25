/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolSelectSubject.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include <klocalizedstring.h>

#include <kis_cursor.h>
#include <kis_debug.h>
#include <KoPointerEvent.h>
#include "kis_canvas2.h"
#include "KisViewManager.h"

namespace {

// Locate the external `nulpaint` launcher (owns the venv + sd-cli + services).
QString nulpaintLauncher()
{
    const QString home = QDir::homePath();
    const QStringList candidates = {
        home + "/.local/bin/nulpaint",
        home + "/programming/Krita/nulpaint/nulpaint",
    };
    for (const QString &p : candidates) {
        if (QFileInfo::exists(p)) {
            return p;
        }
    }
    return QStringLiteral("nulpaint");
}

} // namespace

KisToolSelectSubject::KisToolSelectSubject(KoCanvasBase *canvas)
    : KisTool(canvas, KisCursor::arrowCursor())
{
    setObjectName("tool_select_subject");
}

KisToolSelectSubject::~KisToolSelectSubject()
{
    if (m_proc) {
        m_proc->deleteLater();
        m_proc = nullptr;
    }
}

void KisToolSelectSubject::deactivate()
{
    KisTool::deactivate();
}

void KisToolSelectSubject::beginPrimaryAction(KoPointerEvent *event)
{
    // Clicking on the canvas triggers a subject selection — the click position
    // is irrelevant (the matte/seg services work on the whole image), so we just
    // consume the event and fire.
    event->accept();
    runSelection();
}

void KisToolSelectSubject::runSelection()
{
    if (m_proc && m_proc->state() != QProcess::NotRunning) {
        return; // already working
    }

    // Index 0 = Object (segmodel), 1 = Human (mattemodel) — see createOptionWidget.
    const bool isObject = !m_kind || m_kind->currentIndex() == 0;
    QStringList args;
    args << QStringLiteral("select-subject");
    if (isObject) {
        args << QStringLiteral("--object");
    }

    if (!m_proc) {
        m_proc = new QProcess(this);
        // Capture everything the launcher prints (stdout + stderr together) so a
        // failure is never silent — we log it and surface the tail in the options
        // panel. This is the blind spot that made past breakages hard to diagnose.
        m_proc->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_proc, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(onFinished(int)));
        // A start failure (launcher missing) emits errorOccurred but NOT finished —
        // route it through the same cleanup so the cursor/progress never get stuck.
        connect(m_proc, &QProcess::errorOccurred, this, [this]() {
            if (m_proc && m_proc->state() == QProcess::NotRunning) {
                onFinished(-1);
            }
        });
    }

    if (m_button) {
        m_button->setEnabled(false);
    }
    beginFeedback(isObject ? i18n("Selecting object…")
                           : i18n("Selecting person…"));
    m_proc->start(nulpaintLauncher(), args);
}

void KisToolSelectSubject::onFinished(int exitCode)
{
    // Read back what the launcher printed (merged stdout+stderr). The CLI prints a
    // one-line "selected object via …" on success and a traceback / error on
    // failure; either way it's the single most useful diagnostic we have.
    const QString output = m_proc
        ? QString::fromUtf8(m_proc->readAll()).trimmed()
        : QString();

    if (exitCode == 0) {
        dbgTools << "KisToolSelectSubject:" << output;
        endFeedback(i18n("Subject selected ✓"));
    } else {
        warnTools << "KisToolSelectSubject FAILED (exit" << exitCode << "):"
                  << output;
        endFeedback(i18n("Subject selection failed (exit %1)", exitCode));
        // Surface the actual error in the options panel — no terminal required.
        if (m_status && !output.isEmpty()) {
            // Keep the label readable: show the last line, which is where the
            // CLI's error message / exception lands.
            const QString lastLine = output.section('\n', -1).trimmed();
            m_status->setText(lastLine.isEmpty() ? output : lastLine);
        }
    }

    if (m_button) {
        m_button->setEnabled(true);
    }
}

void KisToolSelectSubject::beginFeedback(const QString &message)
{
    if (m_feedbackActive) {
        return;
    }
    m_feedbackActive = true;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2 *>(canvas());
    KisViewManager *vm = kisCanvas ? kisCanvas->viewManager() : nullptr;
    if (vm && vm->qtMainWindow() && vm->qtMainWindow()->statusBar()) {
        QStatusBar *sb = vm->qtMainWindow()->statusBar();
        if (!m_progress) {
            m_progress = new QProgressBar(sb);
            m_progress->setRange(0, 0);          // indeterminate "busy" sweep
            m_progress->setTextVisible(false);
            m_progress->setMaximumWidth(140);
            sb->addPermanentWidget(m_progress);
        }
        m_progress->show();
        sb->showMessage(message);
    }
    if (m_status) {
        m_status->setText(message);
    }
}

void KisToolSelectSubject::endFeedback(const QString &message)
{
    if (!m_feedbackActive) {
        return;
    }
    m_feedbackActive = false;
    QApplication::restoreOverrideCursor();

    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2 *>(canvas());
    KisViewManager *vm = kisCanvas ? kisCanvas->viewManager() : nullptr;
    if (vm && vm->qtMainWindow() && vm->qtMainWindow()->statusBar()) {
        QStatusBar *sb = vm->qtMainWindow()->statusBar();
        if (m_progress) {
            sb->removeWidget(m_progress);
            m_progress->deleteLater();
            m_progress = nullptr;
        }
        sb->showMessage(message, 4000);
    }
    if (m_status) {
        m_status->setText(message);
    }
}

QWidget *KisToolSelectSubject::createOptionWidget()
{
    QWidget *widget = new QWidget();
    widget->setObjectName("KisToolSelectSubject option widget");

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *intro = new QLabel(
        i18n("One-click subject selection powered by the local AI matte / "
             "segmentation models. Choose what to isolate, then press Select "
             "(or click the canvas)."));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    m_kind = new QComboBox();
    // Object first + default: segmodel (BiRefNet) isolates any subject — cartoon,
    // object, or person — whereas Human (mattemodel/RVM) only handles real humans.
    m_kind->addItem(i18n("Object"));  // index 0 → segmodel
    m_kind->addItem(i18n("Human"));   // index 1 → mattemodel
    layout->addWidget(m_kind);

    m_button = new QPushButton(i18n("Select Subject"));
    connect(m_button, SIGNAL(clicked()), this, SLOT(runSelection()));
    layout->addWidget(m_button);

    m_status = new QLabel(QStringLiteral("—"));
    m_status->setWordWrap(true);
    layout->addWidget(m_status);

    layout->addStretch(1);
    return widget;
}
