#include "PipelineManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

PipelineManager::PipelineManager(QObject *parent)
    : QObject(parent)
    , m_pipelineName("Default Pipeline")
    , m_pipelineDescription("Multi-step prompt execution and evaluation workflow")
{
}

void PipelineManager::addStep(std::shared_ptr<PipelineStep> step) {
    if (step) {
        m_steps.append(step);
        emit pipelineModified();
    }
}

void PipelineManager::insertStep(int index, std::shared_ptr<PipelineStep> step) {
    if (step && index >= 0 && index <= m_steps.size()) {
        m_steps.insert(index, step);
        emit pipelineModified();
    }
}

void PipelineManager::removeStep(int index) {
    if (index >= 0 && index < m_steps.size()) {
        m_steps.removeAt(index);
        emit pipelineModified();
    }
}

void PipelineManager::moveStep(int fromIndex, int toIndex) {
    if (fromIndex >= 0 && fromIndex < m_steps.size() &&
        toIndex >= 0 && toIndex < m_steps.size() &&
        fromIndex != toIndex) {
        auto step = m_steps.takeAt(fromIndex);
        m_steps.insert(toIndex, step);
        emit pipelineModified();
    }
}

void PipelineManager::clear() {
    m_steps.clear();
    m_context.clearVariables();
    m_context.resetExecution();
    emit pipelineModified();
}

std::shared_ptr<PipelineStep> PipelineManager::getStep(int index) const {
    if (index >= 0 && index < m_steps.size()) {
        return m_steps[index];
    }
    return nullptr;
}

int PipelineManager::stepCount() const {
    return m_steps.size();
}

const QVector<std::shared_ptr<PipelineStep>>& PipelineManager::getAllSteps() const {
    return m_steps;
}

ExecutionContext& PipelineManager::getContext() {
    return m_context;
}

const ExecutionContext& PipelineManager::getContext() const {
    return m_context;
}

QString PipelineManager::getPipelineName() const { return m_pipelineName; }
void PipelineManager::setPipelineName(const QString &name) { m_pipelineName = name; }

QString PipelineManager::getPipelineDescription() const { return m_pipelineDescription; }
void PipelineManager::setPipelineDescription(const QString &desc) { m_pipelineDescription = desc; }

void PipelineManager::resetStepStatuses() {
    for (auto &step : m_steps) {
        step->setStatus(StepStatus::Pending);
        step->setLastOutput("");
        step->setLogMessage("");
    }
    m_context.resetExecution();
    emit pipelineModified();
}

bool PipelineManager::runPipeline() {
    emit pipelineStarted();
    m_context.resetExecution();

    bool overallSuccess = true;

    for (int i = 0; i < m_steps.size(); ++i) {
        auto &step = m_steps[i];

        if (!step->isEnabled()) {
            step->setStatus(StepStatus::Skipped);
            step->setLogMessage("Step skipped by user configuration.");
            emit stepFinished(i, true);
            continue;
        }

        emit stepStarted(i);
        bool stepSuccess = step->execute(m_context);
        emit stepFinished(i, stepSuccess);

        if (!stepSuccess) {
            overallSuccess = false;
            // Mark remaining steps as skipped
            for (int j = i + 1; j < m_steps.size(); ++j) {
                m_steps[j]->setStatus(StepStatus::Skipped);
                m_steps[j]->setLogMessage("Skipped due to upstream failure.");
                emit stepFinished(j, false);
            }
            break;
        }
    }

    emit pipelineFinished(overallSuccess);
    return overallSuccess;
}

bool PipelineManager::runSingleStep(int index) {
    if (index < 0 || index >= m_steps.size()) return false;

    emit stepStarted(index);
    bool ok = m_steps[index]->execute(m_context);
    emit stepFinished(index, ok);
    return ok;
}

bool PipelineManager::saveToFile(const QString &filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QJsonObject root;
    root.insert("name", m_pipelineName);
    root.insert("description", m_pipelineDescription);
    root.insert("context", m_context.toJson());

    QJsonArray stepsArray;
    for (const auto &step : m_steps) {
        stepsArray.append(step->toJson());
    }
    root.insert("steps", stepsArray);

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool PipelineManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    m_pipelineName = root["name"].toString("Imported Pipeline");
    m_pipelineDescription = root["description"].toString();

    m_context.fromJson(root["context"].toObject());
    m_steps.clear();

    if (root.contains("steps") && root["steps"].isArray()) {
        QJsonArray stepsArray = root["steps"].toArray();
        for (const auto &val : stepsArray) {
            if (val.isObject()) {
                auto step = createStepFromJson(val.toObject());
                if (step) {
                    m_steps.append(step);
                }
            }
        }
    }

    emit pipelineModified();
    return true;
}

std::shared_ptr<PipelineStep> PipelineManager::createStepFromJson(const QJsonObject &json) const {
    QString typeStr = json["type"].toString();
    StepType type = PipelineStep::stringToType(typeStr);

    std::shared_ptr<PipelineStep> step = nullptr;
    if (type == StepType::Prompt) {
        step = std::make_shared<PromptStep>();
    } else if (type == StepType::Validator) {
        step = std::make_shared<ValidatorStep>();
    } else if (type == StepType::Formatter) {
        step = std::make_shared<FormatterStep>();
    }

    if (step) {
        step->fromJson(json);
    }
    return step;
}

void PipelineManager::loadPreset(int presetId) {
    clear();

    if (presetId == 0) {
        // Preset: AI Research Digest & Fact Validator
        m_pipelineName = "AI Research Digest & Fact Validator";
        m_pipelineDescription = "Extracts structured highlights from text, validates length, and formats as markdown.";

        m_context.setVariable("topic", "Autonomous Agent Systems in 2026");
        m_context.setVariable("tone", "Analytical and Concise");
        m_context.setVariable("source_text", "Recent developments in agentic workflows show a 40% increase in developer velocity.");

        auto step1 = std::make_shared<PromptStep>("step_1", "Draft Executive Summary");
        step1->setPromptTemplate("Summarize the key insights regarding {topic} with a {tone} tone. Source data: {source_text}");
        step1->setModelName("gpt-4o-mini");
        step1->setTemperature(0.6);
        addStep(step1);

        auto step2 = std::make_shared<ValidatorStep>("step_2", "Guardrail: Content Not Empty");
        step2->setSourceStepId("step_1");
        step2->setRule("NotEmpty");
        addStep(step2);

        auto step3 = std::make_shared<ValidatorStep>("step_3", "Guardrail: Min Length Threshold");
        step3->setSourceStepId("step_1");
        step3->setRule("MinLength");
        step3->setRuleValue("50");
        addStep(step3);

        auto step4 = std::make_shared<FormatterStep>("step_4", "Format As Executive Highlights");
        step4->setSourceStepId("step_1");
        step4->setFormatType("NumberedList");
        step4->setCustomHeader("Key Strategic Takeaways");
        addStep(step4);
    } else if (presetId == 1) {
        // Preset: Code Review Assistant & JSON Formatter
        m_pipelineName = "Automated Code Review Assistant";
        m_pipelineDescription = "Inspects C++ code snippet for architecture standards and formats output as JSON.";

        m_context.setVariable("language", "C++17 / Qt6");
        m_context.setVariable("code_snippet", "class TaskManager { QVector<std::shared_ptr<Task>> tasks; ... };");

        auto step1 = std::make_shared<PromptStep>("step_1", "Code Review & Static Analysis");
        step1->setPromptTemplate("Perform a comprehensive code review for {language}:\n{code_snippet}\nHighlight architecture and safety.");
        step1->setModelName("claude-3-haiku");
        step1->setTemperature(0.2);
        addStep(step1);

        auto step2 = std::make_shared<ValidatorStep>("step_2", "Guardrail: Key Word Check");
        step2->setSourceStepId("step_1");
        step2->setRule("ContainsKeyword");
        step2->setRuleValue("Architecture");
        addStep(step2);

        auto step3 = std::make_shared<FormatterStep>("step_3", "Format As JSON Audit Payload");
        step3->setSourceStepId("step_1");
        step3->setFormatType("JsonWrapper");
        step3->setCustomHeader("CodeAuditLog");
        addStep(step3);
    } else {
        // Preset: Customer Support Generator
        m_pipelineName = "Customer Support Resolution Pipeline";
        m_pipelineDescription = "Composes an empathetic customer support resolution and validates tone.";

        m_context.setVariable("customer_name", "Dmitro Sokolov");
        m_context.setVariable("issue_type", "Account Authorization Timeout");

        auto step1 = std::make_shared<PromptStep>("step_1", "Generate Support Response");
        step1->setPromptTemplate("Compose a resolution for customer {customer_name} reporting {issue_type}. Be helpful and professional.");
        step1->setModelName("gemini-1.5-flash");
        step1->setTemperature(0.5);
        addStep(step1);

        auto step2 = std::make_shared<ValidatorStep>("step_2", "Guardrail: Not Empty");
        step2->setSourceStepId("step_1");
        step2->setRule("NotEmpty");
        addStep(step2);

        auto step3 = std::make_shared<FormatterStep>("step_3", "Blockquote Formatting");
        step3->setSourceStepId("step_1");
        step3->setFormatType("MarkdownBlock");
        step3->setCustomHeader("Official Support Resolution");
        addStep(step3);
    }

    emit pipelineModified();
}
