#include "PipelineStep.h"

PipelineStep::PipelineStep(const QString &id, const QString &title, StepType type)
    : m_id(id)
    , m_title(title)
    , m_type(type)
    , m_enabled(true)
    , m_status(StepStatus::Pending)
    , m_lastOutput("")
    , m_logMessage("")
    , m_executionTimeMs(0)
    , m_promptTokens(0)
    , m_completionTokens(0)
    , m_cost(0.0)
{
}

QString PipelineStep::getId() const { return m_id; }
void PipelineStep::setId(const QString &id) { m_id = id; }

QString PipelineStep::getTitle() const { return m_title; }
void PipelineStep::setTitle(const QString &title) { m_title = title; }

bool PipelineStep::isEnabled() const { return m_enabled; }
void PipelineStep::setEnabled(bool enabled) { m_enabled = enabled; }

StepStatus PipelineStep::getStatus() const { return m_status; }
void PipelineStep::setStatus(StepStatus status) { m_status = status; }

QString PipelineStep::getLastOutput() const { return m_lastOutput; }
void PipelineStep::setLastOutput(const QString &output) { m_lastOutput = output; }

QString PipelineStep::getLogMessage() const { return m_logMessage; }
void PipelineStep::setLogMessage(const QString &message) { m_logMessage = message; }

qint64 PipelineStep::getExecutionTimeMs() const { return m_executionTimeMs; }
void PipelineStep::setExecutionTimeMs(qint64 ms) { m_executionTimeMs = ms; }

int PipelineStep::getPromptTokens() const { return m_promptTokens; }
int PipelineStep::getCompletionTokens() const { return m_completionTokens; }
double PipelineStep::getCost() const { return m_cost; }

QJsonObject PipelineStep::toJson() const {
    QJsonObject json;
    json.insert("id", m_id);
    json.insert("title", m_title);
    json.insert("type", typeToString(m_type));
    json.insert("enabled", m_enabled);
    return json;
}

void PipelineStep::fromJson(const QJsonObject &json) {
    if (json.contains("id")) m_id = json["id"].toString();
    if (json.contains("title")) m_title = json["title"].toString();
    if (json.contains("enabled")) m_enabled = json["enabled"].toBool(true);
}

QString PipelineStep::statusToString(StepStatus status) {
    switch (status) {
        case StepStatus::Pending: return "Pending";
        case StepStatus::Running: return "Running";
        case StepStatus::Success: return "Success";
        case StepStatus::Failed:  return "Failed";
        case StepStatus::Skipped: return "Skipped";
    }
    return "Unknown";
}

StepStatus PipelineStep::stringToStatus(const QString &str) {
    if (str == "Running") return StepStatus::Running;
    if (str == "Success") return StepStatus::Success;
    if (str == "Failed")  return StepStatus::Failed;
    if (str == "Skipped") return StepStatus::Skipped;
    return StepStatus::Pending;
}

QString PipelineStep::typeToString(StepType type) {
    switch (type) {
        case StepType::Prompt:    return "Prompt";
        case StepType::Validator: return "Validator";
        case StepType::Formatter: return "Formatter";
    }
    return "Unknown";
}

StepType PipelineStep::stringToType(const QString &str) {
    if (str == "Validator") return StepType::Validator;
    if (str == "Formatter") return StepType::Formatter;
    return StepType::Prompt;
}
