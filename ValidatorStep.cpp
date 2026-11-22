#include "ValidatorStep.h"
#include <QJsonDocument>
#include <QElapsedTimer>

ValidatorStep::ValidatorStep(const QString &id, const QString &title)
    : PipelineStep(id, title, StepType::Validator)
    , m_sourceStepId("")
    , m_rule("NotEmpty")
    , m_ruleValue("")
{
}

StepType ValidatorStep::getType() const {
    return StepType::Validator;
}

QString ValidatorStep::getTypeName() const {
    return "Guardrail Validator";
}

int ValidatorStep::estimateTokens(const ExecutionContext &ctx) const {
    Q_UNUSED(ctx);
    return 0; // Validation is a local guardrail step
}

bool ValidatorStep::execute(ExecutionContext &ctx) {
    QElapsedTimer timer;
    timer.start();
    m_status = StepStatus::Running;

    QString sourceText;
    if (!m_sourceStepId.isEmpty() && ctx.hasStepOutput(m_sourceStepId)) {
        sourceText = ctx.getStepOutput(m_sourceStepId);
    } else {
        sourceText = ctx.getLastOutput();
    }

    bool passed = true;
    QString errorMsg;

    if (m_rule == "NotEmpty") {
        if (sourceText.trimmed().isEmpty()) {
            passed = false;
            errorMsg = "Validation failed: source output is empty or contains only whitespace.";
        }
    } else if (m_rule == "MinLength") {
        int minLen = m_ruleValue.toInt();
        if (sourceText.length() < minLen) {
            passed = false;
            errorMsg = QString("Validation failed: text length (%1) is less than required minimum (%2).")
                           .arg(sourceText.length())
                           .arg(minLen);
        }
    } else if (m_rule == "ContainsKeyword") {
        if (!sourceText.contains(m_ruleValue, Qt::CaseInsensitive)) {
            passed = false;
            errorMsg = QString("Validation failed: text does not contain required keyword '%1'.")
                           .arg(m_ruleValue);
        }
    } else if (m_rule == "ValidJson") {
        QJsonParseError parseErr;
        QJsonDocument::fromJson(sourceText.toUtf8(), &parseErr);
        if (parseErr.error != QJsonParseError::NoError) {
            passed = false;
            errorMsg = QString("Validation failed: output is not valid JSON (%1).")
                           .arg(parseErr.errorString());
        }
    }

    m_executionTimeMs = timer.elapsed() + 5;
    m_lastOutput = sourceText; // passes through source text

    if (passed) {
        m_status = StepStatus::Success;
        m_logMessage = QString("Passed rule '%1' (%2 ms)").arg(m_rule).arg(m_executionTimeMs);
        ctx.setStepOutput(m_id, sourceText);
        return true;
    } else {
        m_status = StepStatus::Failed;
        m_logMessage = errorMsg;
        return false; // halts subsequent pipeline execution
    }
}

QString ValidatorStep::getSourceStepId() const { return m_sourceStepId; }
void ValidatorStep::setSourceStepId(const QString &stepId) { m_sourceStepId = stepId; }

QString ValidatorStep::getRule() const { return m_rule; }
void ValidatorStep::setRule(const QString &rule) { m_rule = rule; }

QString ValidatorStep::getRuleValue() const { return m_ruleValue; }
void ValidatorStep::setRuleValue(const QString &val) { m_ruleValue = val; }

QJsonObject ValidatorStep::toJson() const {
    QJsonObject json = PipelineStep::toJson();
    json.insert("sourceStepId", m_sourceStepId);
    json.insert("rule", m_rule);
    json.insert("ruleValue", m_ruleValue);
    return json;
}

void ValidatorStep::fromJson(const QJsonObject &json) {
    PipelineStep::fromJson(json);
    if (json.contains("sourceStepId")) m_sourceStepId = json["sourceStepId"].toString();
    if (json.contains("rule")) m_rule = json["rule"].toString("NotEmpty");
    if (json.contains("ruleValue")) m_ruleValue = json["ruleValue"].toString();
}
