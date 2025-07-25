# System Architecture - C++ Terminal Script Runner

## Overview

The Script Runner follows a **Model-View-Controller (MVC)** architecture with **Observer pattern** for real-time updates. This design ensures clean separation of concerns, maintainable code, and efficient resource management.

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                          Application Layer                          │
├─────────────────────────────────────────────────────────────────────┤
│                          ScriptRunner                               │
│                       (Main Controller)                             │
├─────────────────┬─────────────────────────┬─────────────────────────┤
│   UIManager     │    ProcessManager       │       Logger            │
│   (View)        │      (Model)            │   (Cross-cutting)       │
├─────────────────┼─────────────────────────┼─────────────────────────┤
│  ┌─────────────┐│  ┌─────────────────────┐│  ┌─────────────────────┐│
│  │ MainWindow  ││  │  ScriptDiscovery    ││  │    FileLogger       ││
│  │ ScriptList  ││  │  ProcessExecutor    ││  │    DebugLogger      ││
│  │ OutputPanes ││  │  OutputCapture      ││  │    ErrorReporting   ││
│  │ StatusBar   ││  │  ThreadManager      ││  │                     ││
│  └─────────────┘│  └─────────────────────┘│  └─────────────────────┘│
└─────────────────┴─────────────────────────┴─────────────────────────┘
```

## Component Details

### 1. ScriptRunner (Main Controller)

**Responsibilities**:
- Application lifecycle management
- Component coordination and communication
- Event routing between UI and process management
- Global state management
- Error handling and recovery

**Key Patterns**:
- **Mediator**: Coordinates communication between UIManager and ProcessManager
- **Command**: Encapsulates user actions for execution and undo
- **State Machine**: Manages application states (idle, running, error)

**Threading Model**:
```
Main Thread (UI Event Loop)
├── Input Handling
├── Screen Updates
├── Component Coordination
└── Error Recovery

Worker Threads
├── Process 1 Output Reader
├── Process 2 Output Reader
└── Resource Cleanup
```

### 2. UIManager (View Layer)

**Responsibilities**:
- Terminal interface rendering using ncurses
- User input capture and processing
- Real-time display updates
- Layout management and responsiveness
- Visual feedback and status indication

**Design Patterns**:
- **Observer**: Receives notifications from ProcessManager for output updates
- **Composite**: Window hierarchy with MainWindow containing child panes
- **Strategy**: Different rendering strategies for various terminal capabilities

**Window Hierarchy**:
```
MainWindow (stdscr)
├── ScriptListPane
│   ├── Header Window
│   ├── Content Window
│   └── Scrollbar Window
├── OutputPane1
│   ├── Header Window
│   ├── Content Window
│   └── Scrollbar Window
├── OutputPane2
│   ├── Header Window
│   ├── Content Window
│   └── Scrollbar Window
└── StatusBar
    ├── Left Section
    ├── Center Section
    └── Right Section
```

**Color Management**:
```cpp
enum class ColorPair {
    DEFAULT = 1,
    HEADER = 2,
    SELECTED = 3,
    RUNNING = 4,
    ERROR = 5,
    SUCCESS = 6,
    STATUS = 7,
    BORDER = 8
};
```

### 3. ProcessManager (Model Layer)

**Responsibilities**:
- Script discovery and validation
- Process execution and lifecycle management
- Output capture and buffering
- Resource monitoring and cleanup
- Inter-process communication

**Design Patterns**:
- **Factory**: Creates appropriate process handlers for different script types
- **Producer-Consumer**: Output capture threads feed UI update queue
- **Resource Pool**: Manages limited process slots (2 concurrent processes)

**Process Lifecycle**:
```
┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│  IDLE   │───▶│STARTING │───▶│ RUNNING │───▶│FINISHED │───▶│CLEANUP  │
└─────────┘    └─────────┘    └─────────┘    └─────────┘    └─────────┘
     ▲              │              │              │              │
     │              ▼              ▼              ▼              ▼
     └─────────────ERROR◀──────────┴──────────────┴──────────────┘
```

**Output Capture Architecture**:
```cpp
struct OutputBuffer {
    std::queue<std::string> lines;
    std::mutex mutex;
    std::condition_variable cv;
    size_t maxSize;
    bool eof;
};

class OutputCapture {
    pid_t process_pid;
    int stdout_fd, stderr_fd;
    std::thread reader_thread;
    OutputBuffer buffer;
    
    void readerLoop();  // Runs in separate thread
    void notifyUI();    // Thread-safe UI notification
};
```

### 4. Logger (Cross-cutting Concern)

**Responsibilities**:
- Centralized logging for all components
- Multiple output destinations (file, debug console)
- Log level filtering and formatting
- Performance impact minimization
- Thread-safe operation

**Design Patterns**:
- **Singleton**: Global access point for logging
- **Strategy**: Different logging strategies (file, console, network)
- **Decorator**: Log message formatting and enhancement

## Data Flow Architecture

### 1. Script Discovery Flow
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ File System     │───▶│ ProcessManager  │───▶│ ScriptRunner    │
│ (../scripts/)   │    │ .discoverScripts│    │ .refreshList    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
                                               ┌─────────────────┐
                                               │ UIManager       │
                                               │ .updateList     │
                                               └─────────────────┘
```

### 2. Script Execution Flow
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ User Input      │───▶│ UIManager       │───▶│ ScriptRunner    │
│ (Enter key)     │    │ .handleInput    │    │ .executeScript  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
                                               ┌─────────────────┐
                                               │ ProcessManager  │
                                               │ .startScript    │
                                               └─────────────────┘
```

### 3. Output Update Flow
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Process Output  │───▶│ OutputCapture   │───▶│ ProcessManager  │
│ (stdout/stderr) │    │ .readerThread   │    │ .outputBuffer   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
                                               ┌─────────────────┐
                                               │ ScriptRunner    │───┐
                                               │ .handleOutput   │   │
                                               └─────────────────┘   │
                                                        │             │
                                                        ▼             │
                                               ┌─────────────────┐   │
                                               │ UIManager       │   │
                                               │ .updateOutput   │   │
                                               └─────────────────┘   │
                                                                     │
                                                Logger ◀─────────────┘
                                                (All events logged)
```

## Threading Architecture

### Thread Responsibilities

**Main Thread (UI)**:
- ncurses event loop
- Keyboard input processing
- Screen rendering and updates
- Component coordination
- Error handling and recovery

**Output Reader Threads (2x)**:
- Non-blocking read from process pipes
- Buffer management and overflow handling
- EOF detection and cleanup
- Thread-safe communication with main thread

**Cleanup Thread**:
- Periodic resource garbage collection
- Memory usage monitoring
- Process zombie cleanup
- Log file rotation

### Synchronization Strategy

**Mutexes**:
```cpp
class ProcessManager {
private:
    std::mutex m_processListMutex;     // Protects running processes list
    std::mutex m_outputBufferMutex[2]; // Per-process output buffers
    std::mutex m_statusMutex;          // Process status updates
};
```

**Condition Variables**:
```cpp
class OutputCapture {
private:
    std::condition_variable m_outputReady;  // Signals new output available
    std::condition_variable m_shutdownComplete; // Signals thread cleanup
};
```

**Atomic Variables**:
```cpp
std::atomic<bool> m_running{true};          // Application state
std::atomic<int> m_activeProcesses{0};      // Process counter
std::atomic<ProcessStatus> m_status[2];     // Per-process status
```

## Memory Management

### RAII Principles
- **Smart Pointers**: All dynamic allocations use `std::unique_ptr` or `std::shared_ptr`
- **Automatic Cleanup**: Destructors handle all resource deallocation
- **Exception Safety**: Strong exception guarantee for all operations

### Resource Ownership
```cpp
class ScriptRunner {
private:
    std::unique_ptr<UIManager> m_uiManager;       // Owns UI resources
    std::unique_ptr<ProcessManager> m_processManager; // Owns processes
    std::unique_ptr<Logger> m_logger;             // Owns log files
};
```

### Buffer Management
- **Circular Buffers**: Fixed-size output history to prevent memory growth
- **Copy Avoidance**: Move semantics for large data transfers
- **Pool Allocation**: Pre-allocated buffers for high-frequency operations

## Error Handling Architecture

### Exception Hierarchy
```cpp
ScriptRunnerException
├── UIException
│   ├── TerminalInitializationException
│   ├── WindowCreationException
│   └── RenderingException
├── ProcessException
│   ├── ScriptNotFoundException
│   ├── ExecutionFailedException
│   └── ProcessTerminationException
└── FileSystemException
    ├── DirectoryAccessException
    ├── PermissionDeniedException
    └── IOException
```

### Recovery Strategies

**UI Corruption Recovery**:
1. Detect corruption (rendering failures, input problems)
2. Save current state (selected script, scroll positions)
3. Reinitialize ncurses subsystem
4. Restore saved state
5. Log recovery event

**Process Failure Recovery**:
1. Detect process failure (exit code, signal)
2. Clean up process resources (pipes, threads)
3. Update UI with error status
4. Log failure details
5. Allow user to retry or select different script

**Memory Pressure Recovery**:
1. Monitor memory usage periodically
2. Reduce buffer sizes when threshold exceeded
3. Force garbage collection
4. Warn user of performance degradation
5. Graceful degradation of features

## Performance Considerations

### UI Responsiveness
- **Frame Rate**: Target 60 FPS (16ms per frame)
- **Input Latency**: < 10ms from keypress to response
- **Partial Updates**: Only redraw changed screen regions
- **Buffered Output**: Accumulate output updates for batch rendering

### Memory Efficiency
- **Buffer Limits**: Configurable maximum output history
- **Lazy Allocation**: Create resources only when needed
- **Resource Pooling**: Reuse expensive objects
- **Copy Elimination**: Move semantics and reference passing

### CPU Optimization
- **Idle Detection**: Reduce polling frequency when inactive
- **Thread Affinity**: Pin threads to specific CPU cores if beneficial
- **Cache Optimization**: Structure data for cache locality
- **Algorithm Selection**: Choose appropriate data structures and algorithms

## Extension Points

### Plugin Architecture (Future)
```cpp
class ScriptRunnerPlugin {
public:
    virtual void initialize(ScriptRunner* runner) = 0;
    virtual void handleEvent(const Event& event) = 0;
    virtual void cleanup() = 0;
};
```

### Configuration System (Future)
```cpp
class Configuration {
public:
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path);
    
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue) const;
    
    template<typename T>
    void setValue(const std::string& key, const T& value);
};
```

### Remote Monitoring (Future)
```cpp
class RemoteMonitor {
public:
    void startServer(int port);
    void sendStatus(const SystemStatus& status);
    void handleRemoteCommand(const Command& command);
};
```

This architecture provides a solid foundation for the Script Runner application, with clear separation of concerns, robust error handling, and efficient resource management. The design is extensible and maintainable, suitable for Copilot-driven development.
