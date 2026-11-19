#include "StepDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

StepDialog::StepDialog(QWidget *parent, std::shared_ptr<PipelineStep> existingStep)
    : QDialog(parent)
    , m_existingStep(existingStep)
    , m_resultStep(nullptr)
{
    setupUI();
    if (m_existingStep) {
        setWindowTitle("Edit Pipeline Step");
        loadFromExisting();
    } else {
        setWindowTitle("Add New Pipeline Step");
    }
}

void StepDialog::setupUI() {
    setMinimumWidth(500);
    auto mainLayout = new QVBoxLayout(this);

    auto formLayout = new QFormLayout();

    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText("e.g. step_1");
    formLayout->addRow("Step Identifier (ID):", m_idEdit);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("e.g. Generate Summary");
    formLayout->addRow("Step Title:", m_titleEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("LLM Prompt Generation", static_cast<int>(StepType::Prompt));
    m_typeCombo->addItem("Guardrail Validator", static_cast<int>(StepType::Validator));
    m_typeCombo->addItem("Output Formatter", static_cast<int>(StepType::Formatter));
    formLayout->addRow("Step Type:", m_typeCombo);

    m_enabledCheck = new QCheckBox("Step is enabled", this);
    m_enabledCheck->setChecked(true);
    formLayout->addRow("", m_enabledCheck);

    mainLayout->addLayout(formLayout);

    // Stacked widget for type-specific options
    m_stackedWidget = new QStackedWidget(this);

    // Page 0: Prompt
    auto promptPage = new QWidget(this);
    auto promptLayout = new QFormLayout(promptPage);
    m_promptTemplateEdit = new QPlainTextEdit(promptPage);
    m_promptTemplateEdit->setPlaceholderText("Enter template, e.g.: Summarize {topic} for {audience}...");
    m_promptTemplateEdit->setMinimumHeight(100);
    promptLayout->addRow("Prompt Template:", m_promptTemplateEdit);

    m_modelCombo = new QComboBox(promptPage);
    m_modelCombo->addItems({"gpt-4o-mini", "claude-3-haiku", "gemini-1.5-flash", "llama-3.1-8b"});
    promptLayout->addRow("Target Model:", m_modelCombo);

    m_tempSpin = new QDoubleSpinBox(promptPage);
    m_tempSpin->setRange(0.0, 1.0);
    m_tempSpin->setSingleStep(0.05);
    m_tempSpin->setValue(0.7);
    promptLayout->addRow("Temperature:", m_tempSpin);

    m_maxTokensSpin = new QSpinBox(promptPage);
    m_maxTokensSpin->setRange(32, 4096);
    m_maxTokensSpin->setValue(512);
    promptLayout->addRow("Max Tokens:", m_maxTokensSpin);
    m_stackedWidget->addWidget(promptPage);

    // Page 1: Validator
    auto validatorPage = new QWidget(this);
    auto valLayout = new QFormLayout(validatorPage);
    m_valSourceIdEdit = new QLineEdit(validatorPage);
    m_valSourceIdEdit->setPlaceholderText("Source Step ID (empty = previous step)");
    valLayout->addRow("Source Step ID:", m_valSourceIdEdit);

    m_ruleCombo = new QComboBox(validatorPage);
    m_ruleCombo->addItems({"NotEmpty", "MinLength", "ContainsKeyword", "ValidJson"});
    valLayout->addRow("Validation Rule:", m_ruleCombo);

    m_ruleValueEdit = new QLineEdit(validatorPage);
    m_ruleValueEdit->setPlaceholderText("Rule parameter (e.g. min length 50, keyword)");
    valLayout->addRow("Rule Parameter:", m_ruleValueEdit);
    m_stackedWidget->addWidget(validatorPage);

    // Page 2: Formatter
    auto formatterPage = new QWidget(this);
    auto fmtLayout = new QFormLayout(formatterPage);
    m_fmtSourceIdEdit = new QLineEdit(formatterPage);
    m_fmtSourceIdEdit->setPlaceholderText("Source Step ID (empty = previous step)");
    fmtLayout->addRow("Source Step ID:", m_fmtSourceIdEdit);

    m_formatTypeCombo = new QComboBox(formatterPage);
    m_formatTypeCombo->addItems({"MarkdownBlock", "NumberedList", "JsonWrapper", "UpperCase"});
    fmtLayout->addRow("Format Strategy:", m_formatTypeCombo);

    m_customHeaderEdit = new QLineEdit(formatterPage);
    m_customHeaderEdit->setPlaceholderText("Optional header or title");
    fmtLayout->addRow("Section Header:", m_customHeaderEdit);
    m_stackedWidget->addWidget(formatterPage);

    mainLayout->addWidget(m_stackedWidget);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StepDialog::onTypeChanged);

    // Button box
    auto btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto saveBtn = new QPushButton("Save Step", this);
    saveBtn->setDefault(true);
    auto cancelBtn = new QPushButton("Cancel", this);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &StepDialog::validateAndAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void StepDialog::onTypeChanged(int index) {
    m_stackedWidget->setCurrentIndex(index);
}

void StepDialog::loadFromExisting() {
    if (!m_existingStep) return;

    m_idEdit->setText(m_existingStep->getId());
    m_titleEdit->setText(m_existingStep->getTitle());
    m_enabledCheck->setChecked(m_existingStep->isEnabled());

    int typeIdx = static_cast<int>(m_existingStep->getType());
    m_typeCombo->setCurrentIndex(typeIdx);
    m_typeCombo->setEnabled(false); // cannot change base type of existing step
    m_stackedWidget->setCurrentIndex(typeIdx);

    if (auto prompt = std::dynamic_pointer_cast<PromptStep>(m_existingStep)) {
        m_promptTemplateEdit->setPlainText(prompt->getPromptTemplate());
        m_modelCombo->setCurrentText(prompt->getModelName());
        m_tempSpin->setValue(prompt->getTemperature());
        m_maxTokensSpin->setValue(prompt->getMaxTokens());
    } else if (auto val = std::dynamic_pointer_cast<ValidatorStep>(m_existingStep)) {
        m_valSourceIdEdit->setText(val->getSourceStepId());
        m_ruleCombo->setCurrentText(val->getRule());
        m_ruleValueEdit->setText(val->getRuleValue());
    } else if (auto fmt = std::dynamic_pointer_cast<FormatterStep>(m_existingStep)) {
        m_fmtSourceIdEdit->setText(fmt->getSourceStepId());
        m_formatTypeCombo->setCurrentText(fmt->getFormatType());
        m_customHeaderEdit->setText(fmt->getCustomHeader());
    }
}

void StepDialog::validateAndAccept() {
    QString id = m_idEdit->text().trimmed();
    QString title = m_titleEdit->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Step ID cannot be empty.");
        m_idEdit->setFocus();
        return;
    }

    if (title.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Step Title cannot be empty.");
        m_titleEdit->setFocus();
        return;
    }

    StepType type = static_cast<StepType>(m_typeCombo->currentIndex());

    if (m_existingStep) {
        m_resultStep = m_existingStep;
        m_resultStep->setId(id);
        m_resultStep->setTitle(title);
        m_resultStep->setEnabled(m_enabledCheck->isChecked());
    } else {
        if (type == StepType::Prompt) {
            m_resultStep = std::make_shared<PromptStep>(id, title);
        } else if (type == StepType::Validator) {
            m_resultStep = std::make_shared<ValidatorStep>(id, title);
        } else {
            m_resultStep = std::make_shared<FormatterStep>(id, title);
        }
        m_resultStep->setEnabled(m_enabledCheck->isChecked());
    }

    if (type == StepType::Prompt) {
        auto prompt = std::dynamic_pointer_cast<PromptStep>(m_resultStep);
        if (prompt) {
            prompt->setPromptTemplate(m_promptTemplateEdit->toPlainText());
            prompt->setModelName(m_modelCombo->currentText());
            prompt->setTemperature(m_tempSpin->value());
            prompt->setMaxTokens(m_maxTokensSpin->value());
        }
    } else if (type == StepType::Validator) {
        auto val = std::dynamic_pointer_cast<ValidatorStep>(m_resultStep);
        if (val) {
            val->setSourceStepId(m_valSourceIdEdit->text().trimmed());
            val->setRule(m_ruleCombo->currentText());
            val->setRuleValue(m_ruleValueEdit->text().trimmed());
        }
    } else if (type == StepType::Formatter) {
        auto fmt = std::dynamic_pointer_cast<FormatterStep>(m_resultStep);
        if (fmt) {
            fmt->setSourceStepId(m_fmtSourceIdEdit->text().trimmed());
            fmt->setFormatType(m_formatTypeCombo->currentText());
            fmt->setCustomHeader(m_customHeaderEdit->text().trimmed());
        }
    }

    accept();
}

std::shared_ptr<PipelineStep> StepDialog::getStep() const {
    return m_resultStep;
}
