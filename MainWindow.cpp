#include "MainWindow.h"
#include "StepDialog.h"
#include <QToolBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isUpdatingVariables(false)
{
    setWindowTitle("PromptFlow Studio - AI Workflow & Guardrail Designer");
    resize(1200, 750);

    setupUI();
    createToolbar();

    // Connect PipelineManager signals
    connect(&m_manager, &PipelineManager::stepStarted, this, &MainWindow::onStepStarted);
    connect(&m_manager, &PipelineManager::stepFinished, this, &MainWindow::onStepFinished);
    connect(&m_manager, &PipelineManager::pipelineStarted, this, &MainWindow::onPipelineStarted);
    connect(&m_manager, &PipelineManager::pipelineFinished, this, &MainWindow::onPipelineFinished);
    connect(&m_manager, &PipelineManager::pipelineModified, this, &MainWindow::onPipelineModified);

    // Load initial default preset
    m_manager.loadPreset(0);
    refreshStepTree();
    refreshVariablesTable();
    logMessage("System initialized. Loaded default pipeline preset: " + m_manager.getPipelineName());
}

void MainWindow::setupUI() {
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    auto mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);

    // ================= LEFT PANEL: PIPELINE STEPS =================
    auto leftWidget = new QWidget(mainSplitter);
    auto leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto stepsGroup = new QGroupBox("Pipeline Execution Graph", leftWidget);
    auto stepsGroupLayout = new QVBoxLayout(stepsGroup);

    m_stepTree = new QTreeWidget(stepsGroup);
    m_stepTree->setHeaderLabels({"#", "Status", "Step Title", "Type", "Latency"});
    m_stepTree->setColumnWidth(0, 30);
    m_stepTree->setColumnWidth(1, 85);
    m_stepTree->setColumnWidth(2, 160);
    m_stepTree->setColumnWidth(3, 110);
    m_stepTree->setRootIsDecorated(false);
    m_stepTree->setAlternatingRowColors(true);
    connect(m_stepTree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onStepTreeSelectionChanged);
    stepsGroupLayout->addWidget(m_stepTree);

    auto stepBtnsLayout = new QHBoxLayout();
    m_addStepBtn = new QPushButton("➕ Add", stepsGroup);
    m_editStepBtn = new QPushButton("✏️ Edit", stepsGroup);
    m_removeStepBtn = new QPushButton("🗑️ Remove", stepsGroup);
    m_moveUpBtn = new QPushButton("⬆", stepsGroup);
    m_moveDownBtn = new QPushButton("⬇", stepsGroup);

    stepBtnsLayout->addWidget(m_addStepBtn);
    stepBtnsLayout->addWidget(m_editStepBtn);
    stepBtnsLayout->addWidget(m_removeStepBtn);
    stepBtnsLayout->addWidget(m_moveUpBtn);
    stepBtnsLayout->addWidget(m_moveDownBtn);
    stepsGroupLayout->addLayout(stepBtnsLayout);

    connect(m_addStepBtn, &QPushButton::clicked, this, &MainWindow::onAddStepClicked);
    connect(m_editStepBtn, &QPushButton::clicked, this, &MainWindow::onEditStepClicked);
    connect(m_removeStepBtn, &QPushButton::clicked, this, &MainWindow::onRemoveStepClicked);
    connect(m_moveUpBtn, &QPushButton::clicked, this, &MainWindow::onMoveUpClicked);
    connect(m_moveDownBtn, &QPushButton::clicked, this, &MainWindow::onMoveDownClicked);

    // Preset selector
    auto presetLayout = new QHBoxLayout();
    presetLayout->addWidget(new QLabel("Presets:", stepsGroup));
    m_presetCombo = new QComboBox(stepsGroup);
    m_presetCombo->addItems({
        "1. AI Research Digest & Validator",
        "2. Code Review Assistant & JSON",
        "3. Customer Support Generator"
    });
    m_loadPresetBtn = new QPushButton("Load", stepsGroup);
    presetLayout->addWidget(m_presetCombo, 1);
    presetLayout->addWidget(m_loadPresetBtn);
    stepsGroupLayout->addLayout(presetLayout);

    connect(m_loadPresetBtn, &QPushButton::clicked, this, &MainWindow::onLoadPresetClicked);

    leftLayout->addWidget(stepsGroup);
    mainSplitter->addWidget(leftWidget);

    // ================= RIGHT PANEL: DETAILS & OUTPUT =================
    auto rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);

    // Top: Tabs (Variables & Inspector)
    auto topTabs = new QTabWidget(rightSplitter);

    // Tab 1: Variables Table
    auto varsWidget = new QWidget(topTabs);
    auto varsLayout = new QVBoxLayout(varsWidget);
    varsLayout->setContentsMargins(4, 4, 4, 4);

    m_variablesTable = new QTableWidget(varsWidget);
    m_variablesTable->setColumnCount(2);
    m_variablesTable->setHorizontalHeaderLabels({"Variable Name", "Current Value / Payload"});
    m_variablesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_variablesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_variablesTable->setColumnWidth(0, 160);
    connect(m_variablesTable, &QTableWidget::cellChanged, this, &MainWindow::onVariableCellChanged);
    varsLayout->addWidget(m_variablesTable);

    auto varsBtnLayout = new QHBoxLayout();
    m_addVarBtn = new QPushButton("➕ Add Variable", varsWidget);
    m_removeVarBtn = new QPushButton("➖ Remove Selected", varsWidget);
    varsBtnLayout->addWidget(m_addVarBtn);
    varsBtnLayout->addWidget(m_removeVarBtn);
    varsBtnLayout->addStretch();
    varsLayout->addLayout(varsBtnLayout);

    connect(m_addVarBtn, &QPushButton::clicked, this, &MainWindow::onAddVariableClicked);
    connect(m_removeVarBtn, &QPushButton::clicked, this, &MainWindow::onRemoveVariableClicked);

    topTabs->addTab(varsWidget, "Input Variables ({context})");

    // Tab 2: Step Inspector
    m_stepDetailsBrowser = new QTextBrowser(topTabs);
    m_stepDetailsBrowser->setPlaceholderText("Select a step in the left panel to inspect its configuration and payload.");
    topTabs->addTab(m_stepDetailsBrowser, "Step Configuration Inspector");

    rightSplitter->addWidget(topTabs);

    // Bottom: Tabs (Output, Console, Token Analytics)
    auto bottomTabs = new QTabWidget(rightSplitter);

    m_outputPreview = new QTextBrowser(bottomTabs);
    m_outputPreview->setPlaceholderText("Final pipeline execution results will be rendered here...");
    bottomTabs->addTab(m_outputPreview, "✨ Pipeline Output Preview");

    m_consoleLog = new QTextEdit(bottomTabs);
    m_consoleLog->setReadOnly(true);
    bottomTabs->addTab(m_consoleLog, "📋 Console Log");

    m_analyticsTable = new QTableWidget(bottomTabs);
    m_analyticsTable->setColumnCount(6);
    m_analyticsTable->setHorizontalHeaderLabels({"Step ID", "Title", "Input Tokens", "Output Tokens", "Latency (ms)", "Est. Cost ($)"});
    m_analyticsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bottomTabs->addTab(m_analyticsTable, "📊 Token & Cost Breakdown");

    rightSplitter->addWidget(bottomTabs);
    mainSplitter->addWidget(rightSplitter);

    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 2);
    mainLayout->addWidget(mainSplitter);

    // Status bar
    m_statusLabel = new QLabel("Ready", this);
    m_metricsLabel = new QLabel("Tokens: 0 | Cost: $0.0000", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(150);
    m_progressBar->setVisible(false);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_metricsLabel);
}

void MainWindow::createToolbar() {
    auto toolbar = addToolBar("Main Controls");
    toolbar->setMovable(false);

    auto runAction = toolbar->addAction("▶ Run Pipeline", this, &MainWindow::onRunPipelineClicked);
    runAction->setShortcut(QKeySequence("Ctrl+R"));
    runAction->setToolTip("Execute all enabled steps in sequential order (Ctrl+R)");

    auto resetAction = toolbar->addAction("🔄 Reset", this, &MainWindow::onResetPipelineClicked);
    resetAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    resetAction->setToolTip("Clear outputs and reset step statuses (Ctrl+Shift+R)");

    toolbar->addSeparator();

    auto openAction = toolbar->addAction("📂 Open JSON...", this, &MainWindow::onOpenJsonClicked);
    openAction->setShortcut(QKeySequence("Ctrl+O"));

    auto saveAction = toolbar->addAction("💾 Save JSON...", this, &MainWindow::onSaveJsonClicked);
    saveAction->setShortcut(QKeySequence("Ctrl+S"));

    toolbar->addSeparator();

    toolbar->addAction("ℹ️ About", this, &MainWindow::onAboutClicked);
}

void MainWindow::refreshStepTree() {
    m_stepTree->clear();
    const auto &steps = m_manager.getAllSteps();

    for (int i = 0; i < steps.size(); ++i) {
        const auto &step = steps[i];
        auto item = new QTreeWidgetItem(m_stepTree);
        item->setText(0, QString::number(i + 1));

        QString statusText = PipelineStep::statusToString(step->getStatus());
        item->setText(1, statusText);
        if (step->getStatus() == StepStatus::Success) {
            item->setForeground(1, QBrush(QColor(40, 160, 40)));
        } else if (step->getStatus() == StepStatus::Failed) {
            item->setForeground(1, QBrush(QColor(220, 50, 50)));
        } else if (step->getStatus() == StepStatus::Running) {
            item->setForeground(1, QBrush(QColor(30, 120, 220)));
        }

        item->setText(2, step->getTitle());
        item->setText(3, step->getTypeName());
        item->setText(4, step->getExecutionTimeMs() > 0 ? QString("%1 ms").arg(step->getExecutionTimeMs()) : "-");

        if (!step->isEnabled()) {
            item->setForeground(2, QBrush(Qt::gray));
        }
    }

    updateSelectedStepDetails();
}

void MainWindow::refreshVariablesTable() {
    m_isUpdatingVariables = true;
    m_variablesTable->setRowCount(0);

    const auto vars = m_manager.getContext().getAllVariables();
    int row = 0;
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        m_variablesTable->insertRow(row);
        auto keyItem = new QTableWidgetItem(it.key());
        auto valItem = new QTableWidgetItem(it.value());
        m_variablesTable->setItem(row, 0, keyItem);
        m_variablesTable->setItem(row, 1, valItem);
        row++;
    }
    m_isUpdatingVariables = false;
}

void MainWindow::refreshAnalyticsTable() {
    m_analyticsTable->setRowCount(0);
    const auto &steps = m_manager.getAllSteps();

    int row = 0;
    for (const auto &step : steps) {
        m_analyticsTable->insertRow(row);
        m_analyticsTable->setItem(row, 0, new QTableWidgetItem(step->getId()));
        m_analyticsTable->setItem(row, 1, new QTableWidgetItem(step->getTitle()));
        m_analyticsTable->setItem(row, 2, new QTableWidgetItem(QString::number(step->getPromptTokens())));
        m_analyticsTable->setItem(row, 3, new QTableWidgetItem(QString::number(step->getCompletionTokens())));
        m_analyticsTable->setItem(row, 4, new QTableWidgetItem(QString::number(step->getExecutionTimeMs())));
        m_analyticsTable->setItem(row, 5, new QTableWidgetItem(QString("$%1").arg(step->getCost(), 0, 'f', 6)));
        row++;
    }

    const auto &ctx = m_manager.getContext();
    m_metricsLabel->setText(QString("Tokens: %1 | Est. Cost: $%2")
                                .arg(ctx.getTotalTokens())
                                .arg(ctx.getTotalEstimatedCost(), 0, 'f', 6));
}

void MainWindow::updateSelectedStepDetails() {
    auto currentItem = m_stepTree->currentItem();
    if (!currentItem) {
        m_stepDetailsBrowser->setHtml("<p style='color:gray;'>Select a step to inspect details.</p>");
        return;
    }

    int idx = m_stepTree->indexOfTopLevelItem(currentItem);
    auto step = m_manager.getStep(idx);
    if (!step) return;

    QString html = QString("<h3>Step [%1]: %2</h3>").arg(step->getId()).arg(step->getTitle());
    html += QString("<p><b>Type:</b> %1 | <b>Status:</b> %2 | <b>Enabled:</b> %3</p>")
                .arg(step->getTypeName())
                .arg(PipelineStep::statusToString(step->getStatus()))
                .arg(step->isEnabled() ? "Yes" : "No");

    if (auto prompt = std::dynamic_pointer_cast<PromptStep>(step)) {
        html += QString("<p><b>Target Model:</b> %1 | <b>Temperature:</b> %2 | <b>Max Tokens:</b> %3</p>")
                    .arg(prompt->getModelName())
                    .arg(prompt->getTemperature())
                    .arg(prompt->getMaxTokens());
        html += QString("<h4>Prompt Template:</h4><pre style='background:#f4f4f4; padding:8px; border-radius:4px;'>%1</pre>")
                    .arg(prompt->getPromptTemplate().toHtmlEscaped());
    } else if (auto val = std::dynamic_pointer_cast<ValidatorStep>(step)) {
        html += QString("<p><b>Rule:</b> %1 | <b>Parameter:</b> %2 | <b>Source Step:</b> %3</p>")
                    .arg(val->getRule())
                    .arg(val->getRuleValue())
                    .arg(val->getSourceStepId().isEmpty() ? "Previous Step" : val->getSourceStepId());
    } else if (auto fmt = std::dynamic_pointer_cast<FormatterStep>(step)) {
        html += QString("<p><b>Format Type:</b> %1 | <b>Header:</b> %2 | <b>Source Step:</b> %3</p>")
                    .arg(fmt->getFormatType())
                    .arg(fmt->getCustomHeader())
                    .arg(fmt->getSourceStepId().isEmpty() ? "Previous Step" : fmt->getSourceStepId());
    }

    if (!step->getLastOutput().isEmpty()) {
        html += QString("<h4>Last Generated Output:</h4><pre style='background:#eef6ff; padding:8px; border-radius:4px;'>%1</pre>")
                    .arg(step->getLastOutput().toHtmlEscaped());
    }

    m_stepDetailsBrowser->setHtml(html);
}

void MainWindow::onStepTreeSelectionChanged() {
    updateSelectedStepDetails();
}

void MainWindow::onRunPipelineClicked() {
    logMessage("--- Starting Pipeline Execution ---");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, m_manager.stepCount());
    m_progressBar->setValue(0);
    m_statusLabel->setText("Running pipeline...");

    bool success = m_manager.runPipeline();

    m_progressBar->setVisible(false);
    refreshStepTree();
    refreshAnalyticsTable();

    QString lastOutput = m_manager.getContext().getLastOutput();
    m_outputPreview->setMarkdown(lastOutput);

    if (success) {
        m_statusLabel->setText("Pipeline completed successfully.");
        logMessage("Pipeline execution completed successfully.");
    } else {
        m_statusLabel->setText("Pipeline execution failed at guardrail step.");
        logMessage("Pipeline execution aborted due to step failure.");
    }
}

void MainWindow::onResetPipelineClicked() {
    m_manager.resetStepStatuses();
    m_outputPreview->clear();
    refreshStepTree();
    refreshAnalyticsTable();
    m_statusLabel->setText("Pipeline state reset.");
    logMessage("Pipeline reset to pending state.");
}

void MainWindow::onAddStepClicked() {
    StepDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        auto step = dlg.getStep();
        if (step) {
            m_manager.addStep(step);
            logMessage(QString("Added step '%1' [%2]").arg(step->getTitle()).arg(step->getId()));
        }
    }
}

void MainWindow::onEditStepClicked() {
    auto currentItem = m_stepTree->currentItem();
    if (!currentItem) return;

    int idx = m_stepTree->indexOfTopLevelItem(currentItem);
    auto step = m_manager.getStep(idx);
    if (!step) return;

    StepDialog dlg(this, step);
    if (dlg.exec() == QDialog::Accepted) {
        refreshStepTree();
        logMessage(QString("Updated step '%1' [%2]").arg(step->getTitle()).arg(step->getId()));
    }
}

void MainWindow::onRemoveStepClicked() {
    auto currentItem = m_stepTree->currentItem();
    if (!currentItem) return;

    int idx = m_stepTree->indexOfTopLevelItem(currentItem);
    m_manager.removeStep(idx);
    logMessage(QString("Removed step at index %1").arg(idx + 1));
}

void MainWindow::onMoveUpClicked() {
    auto currentItem = m_stepTree->currentItem();
    if (!currentItem) return;

    int idx = m_stepTree->indexOfTopLevelItem(currentItem);
    if (idx > 0) {
        m_manager.moveStep(idx, idx - 1);
        m_stepTree->setCurrentItem(m_stepTree->topLevelItem(idx - 1));
    }
}

void MainWindow::onMoveDownClicked() {
    auto currentItem = m_stepTree->currentItem();
    if (!currentItem) return;

    int idx = m_stepTree->indexOfTopLevelItem(currentItem);
    if (idx < m_manager.stepCount() - 1) {
        m_manager.moveStep(idx, idx + 1);
        m_stepTree->setCurrentItem(m_stepTree->topLevelItem(idx + 1));
    }
}

void MainWindow::onLoadPresetClicked() {
    int presetIdx = m_presetCombo->currentIndex();
    m_manager.loadPreset(presetIdx);
    refreshStepTree();
    refreshVariablesTable();
    refreshAnalyticsTable();
    m_outputPreview->clear();
    logMessage(QString("Loaded preset: %1").arg(m_manager.getPipelineName()));
}

void MainWindow::onAddVariableClicked() {
    int row = m_variablesTable->rowCount();
    m_variablesTable->insertRow(row);
    m_variablesTable->setItem(row, 0, new QTableWidgetItem("new_variable"));
    m_variablesTable->setItem(row, 1, new QTableWidgetItem("value"));
    m_manager.getContext().setVariable("new_variable", "value");
}

void MainWindow::onRemoveVariableClicked() {
    int row = m_variablesTable->currentRow();
    if (row >= 0) {
        QString key = m_variablesTable->item(row, 0)->text();
        m_manager.getContext().removeVariable(key);
        m_variablesTable->removeRow(row);
    }
}

void MainWindow::onVariableCellChanged(int row, int column) {
    if (m_isUpdatingVariables) return;
    Q_UNUSED(column);

    auto keyItem = m_variablesTable->item(row, 0);
    auto valItem = m_variablesTable->item(row, 1);
    if (keyItem && valItem) {
        m_manager.getContext().setVariable(keyItem->text(), valItem->text());
    }
}

void MainWindow::onOpenJsonClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Open Pipeline JSON", "", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        if (m_manager.loadFromFile(path)) {
            refreshStepTree();
            refreshVariablesTable();
            refreshAnalyticsTable();
            logMessage(QString("Successfully loaded pipeline from %1").arg(path));
        } else {
            QMessageBox::critical(this, "Load Error", "Failed to parse pipeline JSON file.");
        }
    }
}

void MainWindow::onSaveJsonClicked() {
    QString path = QFileDialog::getSaveFileName(this, "Save Pipeline JSON", "pipeline.json", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        if (m_manager.saveToFile(path)) {
            logMessage(QString("Saved pipeline configuration to %1").arg(path));
        } else {
            QMessageBox::critical(this, "Save Error", "Failed to write JSON file.");
        }
    }
}

void MainWindow::onAboutClicked() {
    QMessageBox::about(this, "About PromptFlow Studio",
                       "<b>PromptFlow Studio v1.0.0</b><br>"
                       "Developed as a Semester 3 OOP final project.<br><br>"
                       "A multi-step LLM prompt chaining, guardrail validation, and formatting desktop studio.<br>"
                       "Built with C++17 and Qt 6 Widgets.");
}

void MainWindow::onStepStarted(int index) {
    auto step = m_manager.getStep(index);
    if (step) {
        logMessage(QString("[RUNNING] Step %1: %2").arg(index + 1).arg(step->getTitle()));
    }
}

void MainWindow::onStepFinished(int index, bool success) {
    auto step = m_manager.getStep(index);
    if (step) {
        QString tag = success ? "[SUCCESS]" : "[FAILED]";
        logMessage(QString("%1 Step %2: %3 -> %4")
                       .arg(tag)
                       .arg(index + 1)
                       .arg(step->getTitle())
                       .arg(step->getLogMessage()));
    }
    m_progressBar->setValue(index + 1);
}

void MainWindow::onPipelineStarted() {
    m_consoleLog->clear();
    m_statusLabel->setText("Pipeline running...");
}

void MainWindow::onPipelineFinished(bool overallSuccess) {
    m_statusLabel->setText(overallSuccess ? "Pipeline completed successfully." : "Pipeline execution halted.");
}

void MainWindow::onPipelineModified() {
    refreshStepTree();
}

void MainWindow::logMessage(const QString &msg) {
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_consoleLog->append(QString("[%1] %2").arg(timeStr).arg(msg));
}
