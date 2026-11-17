#ifndef VALIDATORSTEP_H
#define VALIDATORSTEP_H

#include "PipelineStep.h"

class ValidatorStep : public PipelineStep {
public:
    ValidatorStep(const QString &id = "", const QString &title = "Response Guardrail");

    StepType getType() const override;
    QString getTypeName() const override;

    bool execute(ExecutionContext &ctx) override;
    int estimateTokens(const ExecutionContext &ctx) const override;

    QString getSourceStepId() const;
    void setSourceStepId(const QString &stepId);

    QString getRule() const;
    void setRule(const QString &rule);

    QString getRuleValue() const;
    void setRuleValue(const QString &val);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

private:
    QString m_sourceStepId;
    QString m_rule;      // "NotEmpty", "MinLength", "ContainsKeyword", "ValidJson"
    QString m_ruleValue; // e.g. "50", "summary"
};

#endif // VALIDATORSTEP_H
