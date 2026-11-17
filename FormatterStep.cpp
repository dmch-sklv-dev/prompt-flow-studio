#include "FormatterStep.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QStringList>

FormatterStep::FormatterStep(const QString &id, const QString &title)
    : PipelineStep(id, title, StepType::Formatter)
    , m_sourceStepId("")
    , m_formatType("MarkdownBlock")
    , m_customHeader("")
{
}

StepType FormatterStep::getType() const {
    return StepType::Formatter;
}

QString FormatterStep::getTypeName() const {
    return "Output Formatter";
}

int FormatterStep::estimateTokens(const ExecutionContext &ctx) const {
    Q_UNUSED(ctx);
    return 0; // Local transformation step
}

bool FormatterStep::execute(ExecutionContext &ctx) {
    QElapsedTimer timer;
    timer.start();
    m_status = StepStatus::Running;

    QString sourceText;
    if (!m_sourceStepId.isEmpty() && ctx.hasStepOutput(m_sourceStepId)) {
        sourceText = ctx.getStepOutput(m_sourceStepId);
    } else {
        sourceText = ctx.getLastOutput();
    }

    QString formatted;
    if (m_formatType == "MarkdownBlock") {
        if (!m_customHeader.isEmpty()) {
            formatted = QString("## %1\n\n> %2")
                            .arg(m_customHeader)
                            .arg(sourceText.split("\n").join("\n> "));
        } else {
            formatted = QString("> %1").arg(sourceText.split("\n").join("\n> "));
        }
    } else if (m_formatType == "NumberedList") {
        QStringList lines = sourceText.split("\n", Qt::SkipEmptyParts);
        QStringList formattedLines;
        int idx = 1;
        for (const QString &line : lines) {
            QString cleanLine = line.trimmed();
            if (cleanLine.startsWith("- ") || cleanLine.startsWith("* ")) {
                cleanLine = cleanLine.mid(2).trimmed();
            }
            formattedLines.append(QString("%1. %2").arg(idx++).arg(cleanLine));
        }
        if (!m_customHeader.isEmpty()) {
            formatted = QString("### %1\n\n%2").arg(m_customHeader).arg(formattedLines.join("\n"));
        } else {
            formatted = formattedLines.join("\n");
        }
    } else if (m_formatType == "JsonWrapper") {
        QJsonObject root;
        root.insert("status", "success");
        root.insert("header", m_customHeader.isEmpty() ? "PipelineResult" : m_customHeader);
        root.insert("payload", sourceText);
        QJsonDocument doc(root);
        formatted = doc.toJson(QJsonDocument::Indented);
    } else if (m_formatType == "UpperCase") {
        formatted = sourceText.toUpper();
    } else {
        formatted = sourceText;
    }

    m_executionTimeMs = timer.elapsed() + 4;
    m_lastOutput = formatted;
    m_status = StepStatus::Success;
    m_logMessage = QString("Formatted using '%1' (%2 ms)").arg(m_formatType).arg(m_executionTimeMs);

    ctx.setStepOutput(m_id, m_lastOutput);
    return true;
}

QString FormatterStep::getSourceStepId() const { return m_sourceStepId; }
void FormatterStep::setSourceStepId(const QString &stepId) { m_sourceStepId = stepId; }

QString FormatterStep::getFormatType() const { return m_formatType; }
void FormatterStep::setFormatType(const QString &type) { m_formatType = type; }

QString FormatterStep::getCustomHeader() const { return m_customHeader; }
void FormatterStep::setCustomHeader(const QString &header) { m_customHeader = header; }

QJsonObject FormatterStep::toJson() const {
    QJsonObject json = PipelineStep::toJson();
    json.insert("sourceStepId", m_sourceStepId);
    json.insert("formatType", m_formatType);
    json.insert("customHeader", m_customHeader);
    return json;
}

void FormatterStep::fromJson(const QJsonObject &json) {
    PipelineStep::fromJson(json);
    if (json.contains("sourceStepId")) m_sourceStepId = json["sourceStepId"].toString();
    if (json.contains("formatType")) m_formatType = json["formatType"].toString("MarkdownBlock");
    if (json.contains("customHeader")) m_customHeader = json["customHeader"].toString();
}
