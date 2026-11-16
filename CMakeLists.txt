cmake_minimum_required(VERSION 3.16)
project(PromptFlowStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

add_executable(PromptFlowStudio
    main.cpp
    ExecutionContext.h
    ExecutionContext.cpp
    PipelineStep.h
    PipelineStep.cpp
    PromptStep.h
    PromptStep.cpp
    ValidatorStep.h
    ValidatorStep.cpp
    FormatterStep.h
    FormatterStep.cpp
    PipelineManager.h
    PipelineManager.cpp
    StepDialog.h
    StepDialog.cpp
    MainWindow.h
    MainWindow.cpp
)

target_link_libraries(PromptFlowStudio PRIVATE Qt6::Widgets Qt6::Core)
