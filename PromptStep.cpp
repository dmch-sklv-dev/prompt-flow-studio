#include "PromptStep.h"
#include <QElapsedTimer>
#include <QStringList>

PromptStep::PromptStep(const QString &id, const QString &title)
    : PipelineStep(id, title, StepType::Prompt)
    , m_promptTemplate("")
    , m_modelName("gpt-4o-mini")
    , m_temperature(0.7)
    , m_maxTokens(512)
    , m_systemInstruction("You are a helpful and precise AI assistant.")
{
}

StepType PromptStep::getType() const {
    return StepType::Prompt;
}

QString PromptStep::getTypeName() const {
    return "LLM Prompt";
}

int PromptStep::estimateTokens(const ExecutionContext &ctx) const {
    QString finalPrompt = ctx.interpolate(m_promptTemplate);
    return std::max(1, static_cast<int>(finalPrompt.length() / 4));
}

bool PromptStep::execute(ExecutionContext &ctx) {
    QElapsedTimer timer;
    timer.start();
    m_status = StepStatus::Running;

    QString finalPrompt = ctx.interpolate(m_promptTemplate);

    m_promptTokens = std::max(1, static_cast<int>(finalPrompt.length() / 4));

    m_lastOutput = simulateLLMResponse(finalPrompt, ctx);
    m_completionTokens = std::max(1, static_cast<int>(m_lastOutput.length() / 4));

    // Pricing estimation: $0.15/1M input, $0.60/1M output tokens
    m_cost = (m_promptTokens * 0.00000015) + (m_completionTokens * 0.00000060);
    m_executionTimeMs = timer.elapsed() + 120; // Simulated realistic latency

    ctx.setStepOutput(m_id, m_lastOutput);
    ctx.addTokenUsage(m_promptTokens, m_completionTokens, m_cost);

    m_status = StepStatus::Success;
    m_logMessage = QString("Generated %1 completion tokens (%2 ms, model: %3)")
                       .arg(m_completionTokens)
                       .arg(m_executionTimeMs)
                       .arg(m_modelName);

    return true;
}

QString PromptStep::simulateLLMResponse(const QString &prompt, const ExecutionContext &ctx) const {
    QString lowerPrompt = prompt.toLower();

    if (lowerPrompt.contains("summarize") || lowerPrompt.contains("підсумуй") || lowerPrompt.contains("тези")) {
        QString topic = ctx.getVariable("topic", "Target Subject");
        return QString("### Executive Summary: %1\n\n"
                       "1. **Core Insight**: The primary objective demonstrates substantial potential for optimization.\n"
                       "2. **Key Metric**: System performance and reliability have improved significantly through modular design.\n"
                       "3. **Conclusion**: Implementation is recommended to proceed into the integration stage with continuous monitoring.")
            .arg(topic);
    }

    if (lowerPrompt.contains("code review") || lowerPrompt.contains("refactor") || lowerPrompt.contains("код")) {
        return QString("### Automated Code Review Findings\n\n"
                       "- **Architecture**: Good separation of concerns using SOLID principles.\n"
                       "- **Memory Management**: Modern C++ smart pointers (`std::shared_ptr`) ensure zero resource leaks.\n"
                       "- **Suggestion**: Ensure boundary conditions are checked before vector indexing.");
    }

    if (lowerPrompt.contains("support") || lowerPrompt.contains("клієнт") || lowerPrompt.contains("customer")) {
        QString customer = ctx.getVariable("customer_name", "Valued Customer");
        return QString("Dear %1,\n\n"
                       "Thank you for contacting our support team. We have received your inquiry and our engineering specialists "
                       "are currently investigating the matter. You can expect a follow-up resolution within 2-4 business hours.\n\n"
                       "Best regards,\nCustomer Success Team")
            .arg(customer);
    }

    // Default intelligent response synthesis
    return QString("Analysis Result:\n"
                   "Based on the provided input: \"%1\"\n\n"
                   "The pipeline executed step [%2] using model %3 (temperature %4). "
                   "All parameters were successfully parsed and validated.")
        .arg(prompt.left(80) + (prompt.length() > 80 ? "..." : ""))
        .arg(m_title)
        .arg(m_modelName)
        .arg(m_temperature, 0, 'f', 2);
}

QString PromptStep::getPromptTemplate() const { return m_promptTemplate; }
void PromptStep::setPromptTemplate(const QString &templateText) { m_promptTemplate = templateText; }

QString PromptStep::getModelName() const { return m_modelName; }
void PromptStep::setModelName(const QString &model) { m_modelName = model; }

double PromptStep::getTemperature() const { return m_temperature; }
void PromptStep::setTemperature(double temp) { m_temperature = temp; }

int PromptStep::getMaxTokens() const { return m_maxTokens; }
void PromptStep::setMaxTokens(int tokens) { m_maxTokens = tokens; }

QString PromptStep::getSystemInstruction() const { return m_systemInstruction; }
void PromptStep::setSystemInstruction(const QString &instruction) { m_systemInstruction = instruction; }

QJsonObject PromptStep::toJson() const {
    QJsonObject json = PipelineStep::toJson();
    json.insert("promptTemplate", m_promptTemplate);
    json.insert("modelName", m_modelName);
    json.insert("temperature", m_temperature);
    json.insert("maxTokens", m_maxTokens);
    json.insert("systemInstruction", m_systemInstruction);
    return json;
}

void PromptStep::fromJson(const QJsonObject &json) {
    PipelineStep::fromJson(json);
    if (json.contains("promptTemplate")) m_promptTemplate = json["promptTemplate"].toString();
    if (json.contains("modelName")) m_modelName = json["modelName"].toString();
    if (json.contains("temperature")) m_temperature = json["temperature"].toDouble(0.7);
    if (json.contains("maxTokens")) m_maxTokens = json["maxTokens"].toInt(512);
    if (json.contains("systemInstruction")) m_systemInstruction = json["systemInstruction"].toString();
}
