# Register GUI - Technical Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Architecture Overview](#architecture-overview)
3. [Core Components](#core-components)
4. [Data Flow](#data-flow)
5. [μHAL Integration](#μhal-integration)
6. [Configuration](#configuration)
7. [Development Guide](#development-guide)

---

## Introduction

### Purpose

Register GUI provides a graphical interface for interacting with FPGA hardware registers via the IPbus protocol (μHAL). It enables physicists and engineers at the CERN AMBER experiment to configure detector electronics without writing low-level code.

### Key Capabilities

- Parse μHAL connection XML files to discover devices
- Display devices in an interactive, draggable diagram
- Read all registers from a device with batch dispatch
- Write to individual registers with validation
- Continuous polling with configurable intervals
- Filter and search registers by any column

---

## Architecture Overview

### Layer Architecture

```
┌─────────────────────────────────────────────────────┐
│                 Presentation Layer                   │
│  MainWindow, LoginWindow, DraggableView, Dialogs    │
├─────────────────────────────────────────────────────┤
│                 Controller Layer                     │
│    RegisterController, RegisterTreeViewHandler       │
├─────────────────────────────────────────────────────┤
│                   Data Layer                         │
│         DeviceManager, DeviceInfo, DeviceEntry       │
├─────────────────────────────────────────────────────┤
│              Hardware Abstraction Layer              │
│               μHAL (uhal::HwInterface)               │
└─────────────────────────────────────────────────────┘
                         │
                         ▼
                  ┌─────────────┐
                  │ FPGA Device │
                  │  (IPbus)    │
                  └─────────────┘
```

---

## Core Components

### MainWindow

The central application window managing all UI components and user interactions.

**Key Responsibilities:**
- Setup and manage Connection and Data tabs
- Handle device selection and display
- Coordinate continuous reading with `QtConcurrent`
- Manage view menu actions (zoom, pan, layout)
- Save/restore user settings via `QSettings`

**Important Members:**
| Member | Type | Purpose |
|--------|------|---------|
| `tWidget` | QTabWidget* | Connection/Data tab container |
| `diagramScene` | QGraphicsScene* | Device block diagram |
| `diagramView` | DraggableView* | Zoomable/pannable view |
| `tData` | QTreeWidget* | Register tree display |
| `deviceInfoMap` | QMap<QString, DeviceInfo> | Device metadata cache |
| `registerController` | RegisterController* | Hardware operations |

### RegisterController

Handles all μHAL hardware communication with optimized batched reads.

**Key Methods:**

```cpp
void readNodes(uhal::HwInterface& hw);
// Batch-reads all registers using dispatch() for efficiency

void writeNode(uhal::HwInterface& hw, const QString& reg, const QString& val);
// Writes a single register after permission check

void performRead(bool resetFilters, bool clearTree, ...);
// Orchestrates full read cycle with UI updates

void ensureCachedHw(const QString& deviceId, ...);
// Maintains cached HwInterface to avoid reconnection overhead
```

**Caching Strategy:**
- `cachedHw`: Reuses μHAL HwInterface across reads
- `nodeMetadataCache`: Caches address/mask/permission per node

### DeviceManager

Parses XML connection files and manages device lifecycle.

**Key Methods:**

```cpp
bool loadDeviceListFromXml(const QString& xmlPath,
                           QList<DeviceEntry>& deviceList,
                           QMap<QString, DeviceInfo>& deviceInfoMap,
                           QString& errorMessage);

DeviceInfo parseUr[118;1:3ui(const QString& uri);
// Supports both chtcp and ipbusudp URI formats

ClickableRectItem* createDeviceBlock(const QString& deviceId, ...);
// Creates visual device representation with saved positions
```

### RegisterTreeViewHandler

Manages the hierarchical tree display of registers.

**Key Methods:**

```cpp
void populateTree(const std::string& node, const QString& addr,
                  const QString& dec, const QString& hex,
                  const QString& mask, const QString& permission);
// Updates or creates tree items with register data

void filterTree(const QVector<QLineEdit*>& filters);
// Applies regex filters across all columns
```

**Node Caching:**
Uses `nodeCache` (QMap<QString, QTreeWidgetItem*>) to avoid recreating tree items on every read cycle.

### Data Structures

**DeviceInfo:**
```cpp
struct DeviceInfo {
    QString connectionType;    // "chtcp" or "ipbusudp"
    QString connectionVersion; // "2.0"
    QString gatewayHost;       // ControlHub hostname
    int gatewayPort;           // ControlHub port
    QString ip;                // Device IP
    int port;                  // Device port
    QString deviceXmlPath;     // Address table path
    QString uri;               // Full URI string
};
```

**NodeMetadata:**
```cpp
struct NodeMetadata {
    uint32_t address;    // Register address
    uint32_t mask;       // Bit mask
    int permission;      // 1=R, 2=W, 3=R/W
};
```

---

## Data Flow

### Register Read Cycle

1. User clicks "Read" or continuous timer triggers
2. `MainWindow::performRead()` called
3. `RegisterController::ensureCachedHw()` creates/reuses HwInterface
4. `RegisterController::readNodes()`:
   - Iterates all nodes, queues reads
   - Calls `hw.dispatch()` (single network transaction)
   - Updates tree via `RegisterTreeViewHandler::populateTree()`
5. UI refreshes with new values

### Register Write Cycle

1. User clicks writable register cell
2. `MainWindow::openWriteDialog()` shows dialog
3. User enters hex/decimal value (auto-synced)
4. On confirm: `RegisterController::writeNode()`:
   - Validates permission
   - Converts value to uint32_t
   - Calls `hw.getNode().write()` + `dispatch()`
5. Tree item updated with new value

---

## μHAL Integration

### Connection URI Formats

**Direct IPbus (UDP):**
```
ipbusudp-2.0://192.168.1.100:50001
```

**Via ControlHub:**
```
chtcp-2.0://controlhub.cern.ch:10203?target=192.168.1.100:50001
```

### Batched Read Pattern

```cpp
// Queue all reads
std::vector<uhal::ValWord<uint32_t>> values(nodes.size());
for (size_t i = 0; i < nodes.size(); ++i) {
    values[i] = hw.getNode(nodes[i]).read();
}

// Single dispatch (one network round-trip)
hw.dispatch();

// Access values
for (size_t i = 0; i < nodes.size(); ++i) {
    uint32_t val = values[i].value();
}
```

### Permission Handling

| Value | Constant | Meaning |
|-------|----------|---------|
| 1 | uhal::defs::READ | Read-only |
| 2 | uhal::defs::WRITE | Write-only |
| 3 | uhal::defs::READWRITE | Read/Write |

---

## Configuration

### QSettings Storage

Application settings stored under organization "CERN", application "Register GUI":

| Key | Description |
|-----|-------------|
| `xmlPath` | Last used connection XML path |
| `readSpeed` | Continuous read interval (ms) |
| `DevicePositions/<id>/x` | Saved X position of device block |
| `DevicePositions/<id>/y` | Saved Y position of device block |

### Connection XML Schema

```xml
<?xml version="1.0" encoding="UTF-8"?>
<connections>
    <connection id="DEVICE_ID"
                uri="PROTOCOL://HOST:PORT[?target=IP:PORT]"
                address_table="file://PATH/TO/ADDRESS_TABLE.xml"/>
</connections>
```

### Address Table Schema (μHAL standard)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<node id="TOP">
    <node id="REG1" address="0x0000" permission="rw" mask="0xFFFFFFFF"/>
    <node id="REG2" address="0x0001" permission="r"/>
    <node id="BLOCK">
        <node id="SUBREG1" address="0x0100" permission="rw"/>
    </node>
</node>
```

---

## Development Guide

### Adding a New Feature

1. **UI Changes**: Modify `MainWindow::setup*()` methods
2. **Hardware Logic**: Extend `RegisterController`
3. **Data Handling**: Update `DeviceManager` or add new structures

### Code Style

- Qt naming conventions (camelCase methods, m_ prefix for members)
- Use `QSettings` for persistent configuration
- Leverage `QtConcurrent::run()` for background operations
- Always use `QMetaObject::invokeMethod()` to update UI from threads

### Building Debug Version

```bash
qmake RegisterEditor.pro CONFIG+=debug
make
```

### Testing Without Hardware

Modify `main.cpp` to bypass login:
```cpp
MainWindow mainWin(0, "TestUser");
mainWin.show();
```

Use μHAL dummy connections for testing:
```xml
<connection id="dummy" uri="ipbusdummy://" address_table="..."/>
```

---

## Appendix: Class Reference

| Class | File | Description |
|-------|------|-------------|
| MainWindow | mainwindow.h/cpp | Main application window |
| LoginWindow | loginwindow.h/cpp | Authentication dialog |
| DeviceManager | devicemanager.h/cpp | XML parsing, device creation |
| RegisterController | registercontroller.h/cpp | μHAL operations |
| RegisterTreeViewHandler | registertreeviewhandler.h/cpp | Tree widget logic |
| ClickableRectItem | clickablerectitem.h/cpp | Interactive device block |
| DraggableView | draggableview.h/cpp | Zoomable graphics view |
