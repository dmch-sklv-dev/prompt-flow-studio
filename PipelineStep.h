#ifndef PIPELINESTEP_H
#define PIPELINESTEP_H

#include <QString>
#include <QJsonObject>
#include <memory>
#include "ExecutionContext.h"

enum class StepType {
    Prompt,
    Validator,
    Formatter
};

enum class StepStatus {
    Pending,
    Running,
    Success,
    Failed,
    Skipped
};

class PipelineStep {
public:
    PipelineStep(const QString &id, const QString &title, StepType type);
    virtual ~PipelineStep() = default;

    QString getId() const;
    void setId(const QString &id);

    QString getTitle() const;
    void setTitle(const QString &title);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    StepStatus getStatus() const;
    void setStatus(StepStatus status);

    QString getLastOutput() const;
    void setLastOutput(const QString &output);

    QString getLogMessage() const;
    void setLogMessage(const QString &message);

    qint64 getExecutionTimeMs() const;
    void setExecutionTimeMs(qint64 ms);

    int getPromptTokens() const;
    int getCompletionTokens() const;
    double getCost() const;

    virtual StepType getType() const = 0;
    virtual QString getTypeName() const = 0;
    virtual bool execute(ExecutionContext &ctx) = 0;
    virtual int estimateTokens(const ExecutionContext &ctx) const = 0;

    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject &json);

    static QString statusToString(StepStatus status);
    static StepStatus stringToStatus(const QString &str);
    static QString typeToString(StepType type);
    static StepType stringToType(const QString &str);

protected:
    QString m_id;
    QString m_title;
    StepType m_type;
    bool m_enabled;
    StepStatus m_status;
    QString m_lastOutput;
    QString m_logMessage;
    qint64 m_executionTimeMs;

    int m_promptTokens;
    int m_completionTokens;
    double m_cost;
};

#endif // PIPELINESTEP_H
