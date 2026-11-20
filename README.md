# PromptFlow Studio 🚀
### Desktop AI Workflow, Guardrail Validation & Token Analytics Studio

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6.9-green.svg)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-orange.svg)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)

**PromptFlow Studio** is a modern desktop IDE built with **C++17 and Qt 6** designed for developers, prompt engineers, and AI practitioners. It allows designing, debugging, validating, and chaining multi-step LLM prompts locally with dynamic variable interpolation, strict guardrail validation, and token cost estimation — before deploying to production.

---

## 🌟 Key Features

- **Sequential Pipeline Execution**: Chain prompts, validators, and formatters into automated multi-step workflows.
- **Dynamic Variable Interpolation**: Use `{variable_name}` placeholders that resolve against user inputs or outputs of previous steps.
- **Guardrail Validation (Safety Filters)**:
  - `NotEmpty`: Ensures steps produce substantive content (with whitespace sanitization).
  - `MinLength`: Enforces minimum length constraints.
  - `ContainsKeyword`: Checks presence of critical keywords or structured headers.
  - `ValidJson`: Validates that generated outputs conform to strict JSON syntax.
- **Strategy Output Formatters**:
  - `MarkdownBlock`: Wraps responses in formatted markdown quotes with custom headers.
  - `NumberedList`: Sanitizes and enumerates key takeaway lists.
  - `JsonWrapper`: Packages raw model output into structured JSON payloads.
- **Token & Cost Analytics**:
  - Live estimation of input (prompt) and output (completion) tokens.
  - Real-time cost breakdown per step in USD ($).
- **JSON Project Persistence**:
  - Save and load complete workflows (including variable states and step configurations) using `QJsonDocument`.
- **Built-in Presets**:
  - *AI Research Digest & Fact Validator*
  - *Automated Code Review Assistant & JSON Formatter*
  - *Customer Support Resolution Generator*

---

## 📸 Screenshots

| Pipeline Execution & Results | Token & Cost Analytics Breakdown |
| :---: | :---: |
| ![Pipeline Run](bin/screen2.png) | ![Analytics](bin/screen3.png) |

---

## 🏗️ Architecture & Domain Model

The project strictly follows **SOLID** principles, separating business domain logic from Qt UI presentation widgets.

```
       +--------------------+
       |  ExecutionContext  |<----------------------+
       +--------------------+                       |
                 ^                                  |
                 | uses                             | manages
       +--------------------+       +-----------------------+
       |   PipelineStep     |<------|    PipelineManager    |
       |     (Abstract)     | 1..*  +-----------------------+
       +--------------------+                   ^
         ^        ^        ^                    | (signals/slots)
         |        |        |                    v
+------------+ +-------------+ +---------------+ +-----------------------+
| PromptStep | |ValidatorStep| | FormatterStep | |      MainWindow       |
+------------+ +-------------+ +---------------+ +-----------------------+
```

### Applied Design Patterns
1. **Template Method / Polymorphism**: Base class `PipelineStep` defines the unified execution lifecycle (`execute`, `estimateTokens`, `toJson`, `fromJson`), implemented concretely in `PromptStep`, `ValidatorStep`, and `FormatterStep`.
2. **Repository / Serializer**: JSON serialization via `QJsonDocument` allowing complete import/export of pipeline configurations and states.
3. **Observer**: Qt Signals & Slots (`stepStarted`, `stepFinished`, `pipelineFinished`, `pipelineModified`) keep the UI reactively in sync with asynchronous execution events.
4. **Context Object**: `ExecutionContext` acts as a shared memory store for variables, step outputs, and cumulative token metrics.

---

## 🛠️ Build & Installation

### Prerequisites
- **C++ Compiler**: GCC (MinGW UCRT64 11+) or MSVC (2019+) with C++17 support
- **Qt 6**: `Qt6::Widgets`, `Qt6::Core`
- **CMake**: version 3.16 or newer

### Building with CMake

```bash
# Clone the repository
git clone https://github.com/dmch-sklv-dev/prompt-flow-studio.git
cd prompt-flow-studio

# Configure build directory
cmake -B build -G "MinGW Makefiles"

# Build the project
cmake --build build

# Run application
./build/PromptFlowStudio.exe
```

---

## 🤝 Cross-Review & Bug Hunting (GitHub Issues)

During peer cross-testing, two issues were reported and resolved via Pull Request:

- **[Issue #1](https://github.com/dmch-sklv-dev/prompt-flow-studio/issues/1)**: `ValidatorStep passes validation on whitespace-only output for 'NotEmpty' rule`
- **[Issue #2](https://github.com/dmch-sklv-dev/prompt-flow-studio/issues/2)**: `TokenEstimator does not account for substituted variable length in PromptStep`

---

## 📄 License
This project is open-source under the MIT License.
Developed for Academic Coursework (Semester 3, Software Engineering).
