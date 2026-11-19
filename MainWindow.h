#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "PipelineManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onRunPipelineClicked();
    void onResetPipelineClicked();
    void onAddStepClicked();
    void onEditStepClicked();
    void onRemoveStepClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onLoadPresetClicked();

    void onAddVariableClicked();
    void onRemoveVariableClicked();
    void onVariableCellChanged(int row, int column);

    void onOpenJsonClicked();
    void onSaveJsonClicked();
    void onAboutClicked();

    void onStepTreeSelectionChanged();

    // PipelineManager signal handlers
    void onStepStarted(int index);
    void onStepFinished(int index, bool success);
    void onPipelineStarted();
    void onPipelineFinished(bool overallSuccess);
    void onPipelineModified();

private:
    void setupUI();
    void createToolbar();
    void refreshStepTree();
    void refreshVariablesTable();
    void refreshAnalyticsTable();
    void updateSelectedStepDetails();
    void logMessage(const QString &msg);

    PipelineManager m_manager;

    // UI elements
    QTreeWidget *m_stepTree;
    QPushButton *m_addStepBtn;
    QPushButton *m_editStepBtn;
    QPushButton *m_removeStepBtn;
    QPushButton *m_moveUpBtn;
    QPushButton *m_moveDownBtn;

    QComboBox *m_presetCombo;
    QPushButton *m_loadPresetBtn;

    QTableWidget *m_variablesTable;
    QPushButton *m_addVarBtn;
    QPushButton *m_removeVarBtn;

    QTextBrowser *m_stepDetailsBrowser;
    QTextBrowser *m_outputPreview;
    QTextEdit *m_consoleLog;
    QTableWidget *m_analyticsTable;

    QLabel *m_statusLabel;
    QLabel *m_metricsLabel;
    QProgressBar *m_progressBar;

    bool m_isUpdatingVariables;
};

#endif // MAINWINDOW_H
