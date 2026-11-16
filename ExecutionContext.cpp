#include "ExecutionContext.h"
#include <QRegularExpression>
#include <QJsonArray>

ExecutionContext::ExecutionContext()
    : m_totalPromptTokens(0)
    , m_totalCompletionTokens(0)
    , m_totalCost(0.0)
{
}

void ExecutionContext::setVariable(const QString &key, const QString &value) {
    m_variables[key.trimmed()] = value;
}

QString ExecutionContext::getVariable(const QString &key, const QString &defaultValue) const {
    return m_variables.value(key.trimmed(), defaultValue);
}

bool ExecutionContext::hasVariable(const QString &key) const {
    return m_variables.contains(key.trimmed());
}

void ExecutionContext::removeVariable(const QString &key) {
    m_variables.remove(key.trimmed());
}

void ExecutionContext::clearVariables() {
    m_variables.clear();
}

QMap<QString, QString> ExecutionContext::getAllVariables() const {
    return m_variables;
}

void ExecutionContext::setStepOutput(const QString &stepId, const QString &output) {
    m_stepOutputs[stepId] = output;
    m_lastOutput = output;
}

QString ExecutionContext::getStepOutput(const QString &stepId) const {
    return m_stepOutputs.value(stepId, "");
}

bool ExecutionContext::hasStepOutput(const QString &stepId) const {
    return m_stepOutputs.contains(stepId);
}

void ExecutionContext::setLastOutput(const QString &output) {
    m_lastOutput = output;
}

QString ExecutionContext::getLastOutput() const {
    return m_lastOutput;
}

QMap<QString, QString> ExecutionContext::getAllStepOutputs() const {
    return m_stepOutputs;
}

QString ExecutionContext::interpolate(const QString &templateText) const {
    QString result = templateText;
    static const QRegularExpression regex(R"(\{([a-zA-Z0-9_\-\.]+)\})");

    auto matches = regex.globalMatch(templateText);
    while (matches.hasNext()) {
        auto match = matches.next();
        QString fullPlaceholder = match.captured(0);
        QString varName = match.captured(1);

        if (m_variables.contains(varName)) {
            result.replace(fullPlaceholder, m_variables.value(varName));
        } else if (m_stepOutputs.contains(varName)) {
            result.replace(fullPlaceholder, m_stepOutputs.value(varName));
        }
    }

    return result;
}

void ExecutionContext::addTokenUsage(int promptTokens, int completionTokens, double cost) {
    m_totalPromptTokens += promptTokens;
    m_totalCompletionTokens += completionTokens;
    m_totalCost += cost;
}

int ExecutionContext::getTotalPromptTokens() const {
    return m_totalPromptTokens;
}

int ExecutionContext::getTotalCompletionTokens() const {
    return m_totalCompletionTokens;
}

int ExecutionContext::getTotalTokens() const {
    return m_totalPromptTokens + m_totalCompletionTokens;
}

double ExecutionContext::getTotalEstimatedCost() const {
    return m_totalCost;
}

void ExecutionContext::resetExecution() {
    m_stepOutputs.clear();
    m_lastOutput.clear();
    m_totalPromptTokens = 0;
    m_totalCompletionTokens = 0;
    m_totalCost = 0.0;
}

QJsonObject ExecutionContext::toJson() const {
    QJsonObject json;
    QJsonObject varsObj;
    for (auto it = m_variables.begin(); it != m_variables.end(); ++it) {
        varsObj.insert(it.key(), it.value());
    }
    json.insert("variables", varsObj);
    return json;
}

void ExecutionContext::fromJson(const QJsonObject &json) {
    m_variables.clear();
    if (json.contains("variables") && json["variables"].isObject()) {
        QJsonObject varsObj = json["variables"].toObject();
        for (auto it = varsObj.begin(); it != varsObj.end(); ++it) {
            m_variables.insert(it.key(), it.value().toString());
        }
    }
}
