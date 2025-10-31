# Samsung Modbus Passthrough - REST API Documentation

## Base URL
```
http://localhost:3000/api
```

---

## 1. Status Endpoints

### GET `/api/status`
Get current system status and statistics.

**Response:**
```json
{
  "uptime": 86400,
  "uart1_sent": 15420,
  "uart1_recived": 15418,
  "uart2_sent": 8934,
  "uart2_recived": 8932,
  "eth_status": "connected",
  "eth_ip": "192.168.1.100"
}
```

---

## 2. Interface Configuration Endpoints

### GET `/api/interfaces`
Get current UART interface settings.

**Response:**
```json
{
  "uart1_baud": "115200",
  "uart1_data": "8",
  "uart1_stop": "1",
  "uart1_parity": "0",
  "uart2_baud": "9600",
  "uart2_data": "8",
  "uart2_stop": "1",
  "uart2_parity": "0"
}
```

### POST `/api/interfaces`
Update UART interface settings.

**Request Body:**
```json
{
  "uart1_baud": "115200",
  "uart1_data": "8",
  "uart1_stop": "1",
  "uart1_parity": "0",
  "uart2_baud": "9600",
  "uart2_data": "8",
  "uart2_stop": "1",
  "uart2_parity": "0"
}
```

**Valid Values:**
- `baudrate`: "9600", "19200", "38400", "57600", "115200"
- `data`: "5", "6", "7", "8"
- `stop`: "1", "2"
- `parity`: "0" (None), "1" (Odd), "2" (Even)

**Response:**
```json
{
  "message": "Settings saved successfully",
  "data": { ...updated settings... }
}
```

**Error Response (400):**
```json
{
  "error": "Invalid baudrate"
}
```

---

## 3. Modbus Configuration Endpoints

### GET `/api/modbus/groups`
Get all outdoor devices with their structure (without register values).

**Response:**
```json
[
  {
    "id": 1,
    "name": "Outdoor Device A",
    "registers": [
      { "id": 1, "name": "Temperature" },
      { "id": 2, "name": "Pressure" }
    ],
    "slaves": [
      {
        "id": 1,
        "registers": [
          { "id": 10, "name": "Sensor A1" },
          { "id": 11, "name": "Sensor A2" }
        ]
      }
    ]
  }
]
```

---

### GET `/api/modbus/group?id={outdoor-id}`
Get specific outdoor device with current register values.

**Query Parameters:**
- `id` (required): Outdoor device ID

**Response:**
```json
{
  "id": 1,
  "name": "Outdoor Device A",
  "register": [
    { "id": 1, "name": "Temperature", "value": 100 },
    { "id": 2, "name": "Pressure", "value": 110 }
  ],
  "slaves": [
    {
      "id": 1,
      "register": [
        { "id": 10, "name": "Sensor A1", "value": 101 },
        { "id": 11, "name": "Sensor A2", "value": 111 }
      ]
    }
  ]
}
```

**Error Response (404):**
```json
{
  "error": "Outdoor device not found"
}
```

---

## 4. Outdoor Device Management

### POST `/api/modbus/group/create?id={outdoor-id}&slave={indoor-count}`
Create a new outdoor device.

**Query Parameters:**
- `id` (required): Outdoor device Modbus address (0-255)
- `slave` (required): Number of indoor devices (0-255)

**Response:**
```json
{
  "message": "Outdoor device created successfully",
  "data": {
    "id": 1,
    "name": "Outdoor Device 1",
    "registers": [],
    "slaves": [
      { "id": 1, "registers": [] },
      { "id": 2, "registers": [] }
    ]
  }
}
```

**Error Response (400):**
```json
{
  "error": "Outdoor device with this ID already exists"
}
```

---

### PATCH `/api/modbus/group/update?id={outdoor-id}&slave={indoor-count}`
Update number of indoor devices in an outdoor device.

**Query Parameters:**
- `id` (required): Outdoor device ID
- `slave` (required): New number of indoor devices

**Response:**
```json
{
  "message": "Outdoor device updated successfully"
}
```

**Note:** Decreasing the indoor device count will delete indoor devices and their registers.

**Error Response (404):**
```json
{
  "error": "Outdoor device not found"
}
```

---

### DELETE `/api/modbus/group/delete?id={outdoor-id}`
Delete an outdoor device and all its data.

**Query Parameters:**
- `id` (required): Outdoor device ID

**Response:**
```json
{
  "message": "Outdoor device deleted successfully"
}
```

**Error Response (404):**
```json
{
  "error": "Outdoor device not found"
}
```

---

## 5. Register Management

### POST `/api/modbus/group/update/register?id={outdoor-id}[&slave={indoor-id}]`
Add a new register to outdoor device or indoor device.

**Query Parameters:**
- `id` (required): Outdoor device ID
- `slave` (optional): Indoor device ID (if adding to indoor device)

**Request Body:**
```json
{
  "id": 1,
  "name": "Temperature"
}
```

**Response:**
```json
{
  "message": "Register added successfully",
  "register": {
    "id": 1,
    "name": "Temperature"
  }
}
```

**Error Responses:**

*400 - Missing parameters:*
```json
{
  "error": "Register ID and name are required"
}
```

*400 - Duplicate register:*
```json
{
  "error": "Register with this ID already exists in this scope"
}
```

*404 - Outdoor device not found:*
```json
{
  "error": "Outdoor device not found"
}
```

*404 - Indoor device not found:*
```json
{
  "error": "Indoor device not found"
}
```

---

### PATCH `/api/modbus/group/update/register?id={outdoor-id}[&slave={indoor-id}]`
Edit an existing register (can change both ID and name).

**Query Parameters:**
- `id` (required): Outdoor device ID
- `slave` (optional): Indoor device ID (if editing indoor register)

**Request Body:**
```json
{
  "id": 1,
  "newId": 2,
  "name": "New Temperature"
}
```

**Fields:**
- `id` (required): Current register ID
- `newId` (optional): New register ID (if changing ID)
- `name` (required): New register name

**Response:**
```json
{
  "message": "Register updated successfully",
  "register": {
    "id": 2,
    "name": "New Temperature"
  }
}
```

**Error Responses:**

*400 - Missing parameters:*
```json
{
  "error": "Register ID and name are required"
}
```

*400 - Duplicate ID:*
```json
{
  "error": "Register with new ID already exists in this scope"
}
```

*404 - Register not found:*
```json
{
  "error": "Register not found"
}
```

---

### DELETE `/api/modbus/group/update/register?id={outdoor-id}[&slave={indoor-id}]&registerId={register-id}`
Delete a register from outdoor or indoor device.

**Query Parameters:**
- `id` (required): Outdoor device ID
- `slave` (optional): Indoor device ID (if deleting from indoor device)
- `registerId` (required): Register ID to delete

**Response:**
```json
{
  "message": "Register deleted successfully"
}
```

**Error Responses:**

*400 - Missing register ID:*
```json
{
  "error": "Register ID is required"
}
```

*404 - Outdoor device not found:*
```json
{
  "error": "Outdoor device not found"
}
```

*404 - Indoor device not found:*
```json
{
  "error": "Indoor device not found"
}
```

*404 - Register not found:*
```json
{
  "error": "Register not found"
}
```

---

## Data Models

### Outdoor Device
```typescript
{
  id: number,              // Modbus address (0-255)
  name: string,            // Auto-generated: "Outdoor Device {id}"
  registers: Register[],   // Outdoor-level registers
  slaves: Indoor[]         // Indoor devices
}
```

### Indoor Device
```typescript
{
  id: number,              // Indoor device ID (1-N)
  registers: Register[]    // Indoor-level registers
}
```

### Register
```typescript
{
  id: number,              // Register address (user-defined)
  name: string             // Register name (user-defined)
}
```

### Register with Value (in GET responses)
```typescript
{
  id: number,
  name: string,
  value: number            // Current register value
}
```

---

## Error Handling

All endpoints return standard HTTP status codes:

- **200 OK**: Successful GET request
- **201 Created**: Successful POST request
- **400 Bad Request**: Invalid parameters or validation error
- **404 Not Found**: Resource not found
- **500 Internal Server Error**: Server error

Error response format:
```json
{
  "error": "Error message description"
}
```

---

## Architecture Notes

### Terminology
- **Outdoor Device**: Represents a Modbus outdoor unit (historically called "group" in code)
- **Indoor Device**: Represents an indoor unit connected to an outdoor device (historically called "slave" in code)
- **Register**: A Modbus register with address (id), name, and value

### Data Hierarchy
```
Outdoor Device (id: 1)
├── Registers (outdoor-level)
│   ├── Register 1: Temperature
│   └── Register 2: Pressure
└── Indoor Devices
    ├── Indoor 1
    │   ├── Register 10: Sensor A1
    │   └── Register 11: Sensor A2
    └── Indoor 2
        ├── Register 20: Sensor B1
        └── Register 21: Sensor B2
```

### Register Scope
Registers can exist at two levels:
1. **Outdoor Device Level**: Shared registers for the outdoor unit
2. **Indoor Device Level**: Specific registers for each indoor unit

When using register management endpoints:
- Without `slave` parameter → operates on outdoor device registers
- With `slave` parameter → operates on specific indoor device registers

### Value Updates
Register values are simulated and auto-increment every 5 seconds in the mock implementation. In production, these would reflect actual Modbus register values.
