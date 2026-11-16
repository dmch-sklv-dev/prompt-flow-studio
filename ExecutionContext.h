#ifndef EXECUTIONCONTEXT_H
#define EXECUTIONCONTEXT_H

#include <QString>
#include <QMap>
#include <QJsonObject>
#include <QDateTime>

class ExecutionContext {
public:
    ExecutionContext();

    void setVariable(const QString &key, const QString &value);
    QString getVariable(const QString &key, const QString &defaultValue = "") const;
    bool hasVariable(const QString &key) const;
    void removeVariable(const QString &key);
    void clearVariables();
    QMap<QString, QString> getAllVariables() const;

    void setStepOutput(const QString &stepId, const QString &output);
    QString getStepOutput(const QString &stepId) const;
    bool hasStepOutput(const QString &stepId) const;
    void setLastOutput(const QString &output);
    QString getLastOutput() const;
    QMap<QString, QString> getAllStepOutputs() const;

    QString interpolate(const QString &templateText) const;

    void addTokenUsage(int promptTokens, int completionTokens, double cost);
    int getTotalPromptTokens() const;
    int getTotalCompletionTokens() const;
    int getTotalTokens() const;
    double getTotalEstimatedCost() const;

    void resetExecution();

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QMap<QString, QString> m_variables;
    QMap<QString, QString> m_stepOutputs;
    QString m_lastOutput;

    int m_totalPromptTokens;
    int m_totalCompletionTokens;
    double m_totalCost;
};

#endif // EXECUTIONCONTEXT_H
