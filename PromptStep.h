#ifndef PROMPTSTEP_H
#define PROMPTSTEP_H

#include "PipelineStep.h"

class PromptStep : public PipelineStep {
public:
    PromptStep(const QString &id = "", const QString &title = "LLM Prompt Generation");

    StepType getType() const override;
    QString getTypeName() const override;

    bool execute(ExecutionContext &ctx) override;
    int estimateTokens(const ExecutionContext &ctx) const override;

    QString getPromptTemplate() const;
    void setPromptTemplate(const QString &templateText);

    QString getModelName() const;
    void setModelName(const QString &model);

    double getTemperature() const;
    void setTemperature(double temp);

    int getMaxTokens() const;
    void setMaxTokens(int tokens);

    QString getSystemInstruction() const;
    void setSystemInstruction(const QString &instruction);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

private:
    QString m_promptTemplate;
    QString m_modelName;
    double m_temperature;
    int m_maxTokens;
    QString m_systemInstruction;

    QString simulateLLMResponse(const QString &prompt, const ExecutionContext &ctx) const;
};

#endif // PROMPTSTEP_H
