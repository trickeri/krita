/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  Nuldrums: AI "Select Subject" tool. A toolbox selection tool whose Human /
 *  Object option drives the local matte / seg model services through the
 *  external `nulpaint` CLI, which writes the resulting selection back into this
 *  Krita over the loopback bridge. The C++ side stays thin: it just offers the
 *  toolbox entry + options and launches the proven external pipeline.
 */
#ifndef KIS_TOOL_SELECT_SUBJECT_H_
#define KIS_TOOL_SELECT_SUBJECT_H_

#include <QObject>
#include <QPointer>
#include <kis_icon.h>
#include "kis_tool.h"
#include "KisSelectionToolFactoryBase.h"

class QComboBox;
class QLabel;
class QPushButton;
class QProcess;
class QProgressBar;

class KisToolSelectSubject : public KisTool
{
    Q_OBJECT
public:
    explicit KisToolSelectSubject(KoCanvasBase *canvas);
    ~KisToolSelectSubject() override;

    void paint(QPainter &, const KoViewConverter &) override {}
    void beginPrimaryAction(KoPointerEvent *event) override;
    QWidget *createOptionWidget() override;

public Q_SLOTS:
    void deactivate() override;

private Q_SLOTS:
    void runSelection();
    void onFinished(int exitCode);

private:
    // Bottom status-bar feedback while the (multi-second) external job runs: a busy
    // cursor + an indeterminate progress bar + a message. endFeedback() tears it all
    // back down; m_feedbackActive keeps the cursor override/restore balanced.
    void beginFeedback(const QString &message);
    void endFeedback(const QString &message);

    QPointer<QComboBox> m_kind;     // 0 = Human, 1 = Object
    QPointer<QPushButton> m_button;
    QPointer<QLabel> m_status;
    QProcess *m_proc {nullptr};
    QPointer<QProgressBar> m_progress;
    bool m_feedbackActive {false};
};


class KisToolSelectSubjectFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectSubjectFactory()
        : KisSelectionToolFactoryBase("KisToolSelectSubject")
    {
        setToolTip(i18n("Subject Selection Tool (AI)"));
        setSection(ToolBoxSection::Select);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("select"));
        setPriority(6);
    }
    ~KisToolSelectSubjectFactory() override {}

    KoToolBase *createTool(KoCanvasBase *canvas) override
    {
        return new KisToolSelectSubject(canvas);
    }
};

#endif // KIS_TOOL_SELECT_SUBJECT_H_
