#ifndef STEPDIALOG_H
#define STEPDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QStackedWidget>
#include <memory>
#include "PipelineStep.h"
#include "PromptStep.h"
#include "ValidatorStep.h"
#include "FormatterStep.h"

class StepDialog : public QDialog {
    Q_OBJECT
public:
    explicit StepDialog(QWidget *parent = nullptr, std::shared_ptr<PipelineStep> existingStep = nullptr);

    std::shared_ptr<PipelineStep> getStep() const;

private slots:
    void onTypeChanged(int index);
    void validateAndAccept();

private:
    void setupUI();
    void loadFromExisting();

    std::shared_ptr<PipelineStep> m_existingStep;
    std::shared_ptr<PipelineStep> m_resultStep;

    QLineEdit *m_idEdit;
    QLineEdit *m_titleEdit;
    QComboBox *m_typeCombo;
    QCheckBox *m_enabledCheck;
    QStackedWidget *m_stackedWidget;

    // Prompt page
    QPlainTextEdit *m_promptTemplateEdit;
    QComboBox *m_modelCombo;
    QDoubleSpinBox *m_tempSpin;
    QSpinBox *m_maxTokensSpin;

    // Validator page
    QLineEdit *m_valSourceIdEdit;
    QComboBox *m_ruleCombo;
    QLineEdit *m_ruleValueEdit;

    // Formatter page
    QLineEdit *m_fmtSourceIdEdit;
    QComboBox *m_formatTypeCombo;
    QLineEdit *m_customHeaderEdit;
};

#endif // STEPDIALOG_H
