#ifndef FORMATTERSTEP_H
#define FORMATTERSTEP_H

#include "PipelineStep.h"

class FormatterStep : public PipelineStep {
public:
    FormatterStep(const QString &id = "", const QString &title = "Output Formatter");

    StepType getType() const override;
    QString getTypeName() const override;

    bool execute(ExecutionContext &ctx) override;
    int estimateTokens(const ExecutionContext &ctx) const override;

    QString getSourceStepId() const;
    void setSourceStepId(const QString &stepId);

    QString getFormatType() const;
    void setFormatType(const QString &type);

    QString getCustomHeader() const;
    void setCustomHeader(const QString &header);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

private:
    QString m_sourceStepId;
    QString m_formatType;   // "MarkdownBlock", "NumberedList", "JsonWrapper", "UpperCase"
    QString m_customHeader;
};

#endif // FORMATTERSTEP_H
