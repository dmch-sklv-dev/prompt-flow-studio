#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H

#include <QObject>
#include <QVector>
#include <memory>
#include "PipelineStep.h"
#include "PromptStep.h"
#include "ValidatorStep.h"
#include "FormatterStep.h"
#include "ExecutionContext.h"

class PipelineManager : public QObject {
    Q_OBJECT
public:
    explicit PipelineManager(QObject *parent = nullptr);

    void addStep(std::shared_ptr<PipelineStep> step);
    void insertStep(int index, std::shared_ptr<PipelineStep> step);
    void removeStep(int index);
    void moveStep(int fromIndex, int toIndex);
    void clear();

    std::shared_ptr<PipelineStep> getStep(int index) const;
    int stepCount() const;
    const QVector<std::shared_ptr<PipelineStep>>& getAllSteps() const;

    ExecutionContext& getContext();
    const ExecutionContext& getContext() const;

    QString getPipelineName() const;
    void setPipelineName(const QString &name);

    QString getPipelineDescription() const;
    void setPipelineDescription(const QString &desc);

    bool runPipeline();
    bool runSingleStep(int index);
    void resetStepStatuses();

    bool saveToFile(const QString &filePath) const;
    bool loadFromFile(const QString &filePath);

    void loadPreset(int presetId);

signals:
    void stepStarted(int index);
    void stepFinished(int index, bool success);
    void pipelineStarted();
    void pipelineFinished(bool overallSuccess);
    void pipelineModified();

private:
    QString m_pipelineName;
    QString m_pipelineDescription;
    QVector<std::shared_ptr<PipelineStep>> m_steps;
    ExecutionContext m_context;

    std::shared_ptr<PipelineStep> createStepFromJson(const QJsonObject &json) const;
};

#endif // PIPELINEMANAGER_H
