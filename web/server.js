const express = require("express");
const path = require("path");
const app = express();
const PORT = 3000;

app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));

// ============================================================
// MOCK DATA STORAGE
// ============================================================

let mockData = {
	status: {
		uptime: 86400 + 3600 + 300, // 1d 1h 5m
		uart1_sent: 15420,
		uart1_recived: 15418,
		uart2_sent: 8934,
		uart2_recived: 8932,
		eth_status: "connected",
		eth_ip: "192.168.1.100",
	},

	interfaces: {
		uart1_baud: "115200",
		uart1_data: "8",
		uart1_stop: "1",
		uart1_parity: "0",
		uart2_baud: "9600",
		uart2_data: "8",
		uart2_stop: "1",
		uart2_parity: "0",
	},

	modbus: [
		{
			id: 1,
			name: "Outdoor Device A",
			registers: [
				{ id: 1, name: "Temperature" },
				{ id: 2, name: "Pressure" },
				{ id: 3, name: "Humidity" },
			],
			slaves: [
				{
					id: 1,
					registers: [
						{ id: 10, name: "Sensor A1" },
						{ id: 11, name: "Sensor A2" },
						{ id: 12, name: "Sensor A3" },
					],
				},
				{
					id: 2,
					registers: [
						{ id: 20, name: "Sensor B1" },
						{ id: 21, name: "Sensor B2" },
						{ id: 22, name: "Sensor B3" },
					],
				},
			],
		},
		{
			id: 2,
			name: "Outdoor Device B",
			registers: [
				{ id: 4, name: "Voltage" },
				{ id: 5, name: "Current" },
				{ id: 6, name: "Power" },
			],
			slaves: [
				{
					id: 1,
					registers: [
						{ id: 30, name: "Sensor C1" },
						{ id: 31, name: "Sensor C2" },
						{ id: 32, name: "Sensor C3" },
					],
				},
			],
		},
	],

	registerValues: {
		// Outdoor device registers
		"1_group": { 1: 100, 2: 110, 3: 120 },
		"2_group": { 4: 200, 5: 210, 6: 220 },
		// Indoor device registers
		"1_1": { 10: 101, 11: 111, 12: 121 },
		"1_2": { 20: 201, 21: 211, 22: 221 },
		"2_1": { 30: 301, 31: 311, 32: 321 },
	},
};

// ============================================================
// UTILITY FUNCTIONS
// ============================================================

function updateRegisterValues() {
	// Simulate changing register values
	Object.keys(mockData.registerValues).forEach((key) => {
		const registers = mockData.registerValues[key];
		Object.keys(registers).forEach((regId) => {
			registers[regId] += Math.floor(Math.random() * 10 - 5); // Random +/- variation
		});
	});
}

function getGroupRegisters(groupId) {
	const group = mockData.modbus.find((g) => g.id === groupId);
	if (!group) return [];

	const key = `${groupId}_group`;
	const registerValues = mockData.registerValues[key] || {};

	return group.registers.map((reg) => ({
		id: reg.id,
		name: reg.name,
		value: registerValues[reg.id] !== undefined ? registerValues[reg.id] : 0,
	}));
}

function getSlaveRegisters(groupId, slaveId) {
	const group = mockData.modbus.find((g) => g.id === groupId);
	if (!group) return [];

	const slave = group.slaves.find((s) => s.id === slaveId);
	if (!slave) return [];

	const key = `${groupId}_${slaveId}`;
	const registerValues = mockData.registerValues[key] || {};

	return slave.registers.map((reg) => ({
		id: reg.id,
		name: reg.name,
		value: registerValues[reg.id] !== undefined ? registerValues[reg.id] : 0,
	}));
}

// ============================================================
// ROUTES: STATUS
// ============================================================

app.get("/api/status", (req, res) => {
	// Simulate changing values
	mockData.status.uptime += 1;
	mockData.status.uart1_sent += Math.floor(Math.random() * 10);
	mockData.status.uart1_recived += Math.floor(Math.random() * 10);
	mockData.status.uart2_sent += Math.floor(Math.random() * 5);
	mockData.status.uart2_recived += Math.floor(Math.random() * 5);

	res.json(mockData.status);
});

// ============================================================
// ROUTES: INTERFACES
// ============================================================

app.get("/api/interfaces", (req, res) => {
	res.json(mockData.interfaces);
});

app.post("/api/interfaces", (req, res) => {
	const { uart1_baud, uart1_data, uart1_stop, uart1_parity, uart2_baud, uart2_data, uart2_stop, uart2_parity } = req.body;

	// Validate inputs
	const validBauds = ["9600", "19200", "38400", "57600", "115200"];
	const validDataBits = ["5", "6", "7", "8"];
	const validStopBits = ["1", "2"];
	const validParity = ["0", "1", "2"];

	if (!validBauds.includes(String(uart1_baud)) || !validBauds.includes(String(uart2_baud))) {
		return res.status(400).json({ error: "Invalid baudrate" });
	}

	if (!validDataBits.includes(String(uart1_data)) || !validDataBits.includes(String(uart2_data))) {
		return res.status(400).json({ error: "Invalid data bits" });
	}

	if (!validStopBits.includes(String(uart1_stop)) || !validStopBits.includes(String(uart2_stop))) {
		return res.status(400).json({ error: "Invalid stop bits" });
	}

	if (!validParity.includes(String(uart1_parity)) || !validParity.includes(String(uart2_parity))) {
		return res.status(400).json({ error: "Invalid parity" });
	}

	mockData.interfaces = {
		uart1_baud: String(uart1_baud),
		uart1_data: String(uart1_data),
		uart1_stop: String(uart1_stop),
		uart1_parity: String(uart1_parity),
		uart2_baud: String(uart2_baud),
		uart2_data: String(uart2_data),
		uart2_stop: String(uart2_stop),
		uart2_parity: String(uart2_parity),
	};

	res.json({ message: "Settings saved successfully", data: mockData.interfaces });
});

// ============================================================
// ROUTES: MODBUS
// ============================================================

app.get("/api/modbus/groups", (req, res) => {
	res.json(mockData.modbus);
});

app.post("/api/modbus/group/create", (req, res) => {
	const { id, slave } = req.query;

	if (!id || slave === undefined) {
		return res.status(400).json({ error: "Missing id or slave parameter" });
	}

	const groupId = parseInt(id);
	const slavesCount = parseInt(slave);

	// Check if group already exists
	if (mockData.modbus.some((g) => g.id === groupId)) {
		return res.status(400).json({ error: "Group with this ID already exists" });
	}

	// Create new group
	const newGroup = {
		id: groupId,
		name: `Outdoor Device ${String.fromCharCode(65 + mockData.modbus.length)}`,
		registers: [],
		slaves: [],
	};

	// Create slaves with empty registers
	for (let i = 1; i <= slavesCount; i++) {
		newGroup.slaves.push({
			id: i,
			registers: [],
		});

		// Initialize empty register values for slave
		const key = `${groupId}_${i}`;
		mockData.registerValues[key] = {};
	}

	// Initialize empty register values for group
	const groupKey = `${groupId}_group`;
	mockData.registerValues[groupKey] = {};

	mockData.modbus.push(newGroup);

	res.json({ id: groupId, name: newGroup.name, slave: slavesCount });
});

app.patch("/api/modbus/group/update", (req, res) => {
	const { id, slave } = req.query;

	if (!id || slave === undefined) {
		return res.status(400).json({ error: "Missing id or slave parameter" });
	}

	const groupId = parseInt(id);
	const newSlavesCount = parseInt(slave);

	const groupIndex = mockData.modbus.findIndex((g) => g.id === groupId);
	if (groupIndex === -1) {
		return res.status(404).json({ error: "Group not found" });
	}

	const group = mockData.modbus[groupIndex];
	const currentSlavesCount = group.slaves.length;

	// Add new slaves if needed
	if (newSlavesCount > currentSlavesCount) {
		for (let i = currentSlavesCount + 1; i <= newSlavesCount; i++) {
			group.slaves.push({
				id: i,
				registers: [],
			});

			const key = `${groupId}_${i}`;
			mockData.registerValues[key] = {};
		}
	} else if (newSlavesCount < currentSlavesCount) {
		// Remove slaves
		for (let i = newSlavesCount + 1; i <= currentSlavesCount; i++) {
			const key = `${groupId}_${i}`;
			delete mockData.registerValues[key];
		}
		group.slaves = group.slaves.slice(0, newSlavesCount);
	}

	res.json({ id: groupId, name: group.name, slave: newSlavesCount });
});

app.delete("/api/modbus/group/delete", (req, res) => {
	const { id } = req.query;

	if (!id) {
		return res.status(400).json({ error: "Missing id parameter" });
	}

	const groupId = parseInt(id);
	const groupIndex = mockData.modbus.findIndex((g) => g.id === groupId);

	if (groupIndex === -1) {
		return res.status(404).json({ error: "Group not found" });
	}

	const group = mockData.modbus[groupIndex];

	// Clean up register values
	delete mockData.registerValues[`${groupId}_group`];
	group.slaves.forEach((slave) => {
		delete mockData.registerValues[`${groupId}_${slave.id}`];
	});

	mockData.modbus.splice(groupIndex, 1);

	res.json({ id: groupId });
});

app.get("/api/modbus/group", (req, res) => {
	const { id } = req.query;

	if (!id) {
		return res.status(400).json({ error: "Missing id parameter" });
	}

	const groupId = parseInt(id);
	const group = mockData.modbus.find((g) => g.id === groupId);

	if (!group) {
		return res.status(404).json({ error: "Group not found" });
	}

	// Update register values (simulate data changes)
	updateRegisterValues();

	const response = {
		id: group.id,
		registers: getGroupRegisters(group.id),
		slaves: group.slaves.map((slave) => ({
			id: slave.id,
			registers: getSlaveRegisters(group.id, slave.id),
		})),
	};

	res.json(response);
});

// ============================================================
// ROUTES: REGISTER MANAGEMENT
// ============================================================

app.post("/api/modbus/group/update/register", (req, res) => {
	const { id, slave } = req.query;
	const { id: registerId, name: registerName } = req.body;

	if (!id) {
		return res.status(400).json({ error: "Missing group id parameter" });
	}

	if (registerId === undefined || !registerName) {
		return res.status(400).json({ error: "Missing register id or name in body" });
	}

	const groupId = parseInt(id);
	const regId = parseInt(registerId);
	const group = mockData.modbus.find((g) => g.id === groupId);

	if (!group) {
		return res.status(404).json({ error: "Outdoor device not found" });
	}

	if (slave !== undefined) {
		// Add register to indoor device
		const slaveId = parseInt(slave);
		const slaveObj = group.slaves.find((s) => s.id === slaveId);

		if (!slaveObj) {
			return res.status(404).json({ error: "Indoor device not found" });
		}

		// Check for duplicate register ID in indoor device scope
		if (slaveObj.registers.some((r) => r.id === regId)) {
			return res.status(400).json({ error: "Register with this ID already exists in this indoor device" });
		}

		slaveObj.registers.push({ id: regId, name: registerName });

		// Initialize register value
		const key = `${groupId}_${slaveId}`;
		if (!mockData.registerValues[key]) {
			mockData.registerValues[key] = {};
		}
		mockData.registerValues[key][regId] = Math.floor(Math.random() * 1000);

		return res.json({ id: regId, name: registerName });
	} else {
		// Add register to outdoor device
		if (group.registers.some((r) => r.id === regId)) {
			return res.status(400).json({ error: "Register with this ID already exists in this outdoor device" });
		}

		group.registers.push({ id: regId, name: registerName });

		// Initialize register value
		const key = `${groupId}_group`;
		if (!mockData.registerValues[key]) {
			mockData.registerValues[key] = {};
		}
		mockData.registerValues[key][regId] = Math.floor(Math.random() * 1000);

		return res.json({ id: regId, name: registerName });
	}
});

app.patch("/api/modbus/group/update/register", (req, res) => {
	const { id, slave } = req.query;
	const { id: oldRegisterId, newId, name: registerName } = req.body;

	if (!id) {
		return res.status(400).json({ error: "Missing group id parameter" });
	}

	if (oldRegisterId === undefined || newId === undefined || !registerName) {
		return res.status(400).json({ error: "Missing register id, newId, or name in body" });
	}

	const groupId = parseInt(id);
	const oldRegId = parseInt(oldRegisterId);
	const newRegId = parseInt(newId);
	const group = mockData.modbus.find((g) => g.id === groupId);

	if (!group) {
		return res.status(404).json({ error: "Outdoor device not found" });
	}

	if (slave !== undefined) {
		// Edit register in indoor device
		const slaveId = parseInt(slave);
		const slaveObj = group.slaves.find((s) => s.id === slaveId);

		if (!slaveObj) {
			return res.status(404).json({ error: "Indoor device not found" });
		}

		const registerIndex = slaveObj.registers.findIndex((r) => r.id === oldRegId);
		if (registerIndex === -1) {
			return res.status(404).json({ error: "Register not found" });
		}

		// Check for duplicate register ID if ID is changing
		if (oldRegId !== newRegId && slaveObj.registers.some((r) => r.id === newRegId)) {
			return res.status(400).json({ error: "Register with this ID already exists in this indoor device" });
		}

		slaveObj.registers[registerIndex] = { id: newRegId, name: registerName };

		// Update register value if ID changed
		const key = `${groupId}_${slaveId}`;
		if (oldRegId !== newRegId && mockData.registerValues[key]) {
			const value = mockData.registerValues[key][oldRegId];
			delete mockData.registerValues[key][oldRegId];
			mockData.registerValues[key][newRegId] = value !== undefined ? value : Math.floor(Math.random() * 1000);
		}

		return res.json({ id: newRegId, name: registerName });
	} else {
		// Edit register in outdoor device
		const registerIndex = group.registers.findIndex((r) => r.id === oldRegId);
		if (registerIndex === -1) {
			return res.status(404).json({ error: "Register not found" });
		}

		// Check for duplicate register ID if ID is changing
		if (oldRegId !== newRegId && group.registers.some((r) => r.id === newRegId)) {
			return res.status(400).json({ error: "Register with this ID already exists in this outdoor device" });
		}

		group.registers[registerIndex] = { id: newRegId, name: registerName };

		// Update register value if ID changed
		const key = `${groupId}_group`;
		if (oldRegId !== newRegId && mockData.registerValues[key]) {
			const value = mockData.registerValues[key][oldRegId];
			delete mockData.registerValues[key][oldRegId];
			mockData.registerValues[key][newRegId] = value !== undefined ? value : Math.floor(Math.random() * 1000);
		}

		return res.json({ id: newRegId, name: registerName });
	}
});

app.delete("/api/modbus/group/update/register", (req, res) => {
	const { id, slave, registerId } = req.query;

	if (!id) {
		return res.status(400).json({ error: "Missing group id parameter" });
	}

	if (registerId === undefined) {
		return res.status(400).json({ error: "Missing register id parameter" });
	}

	const groupId = parseInt(id);
	const regId = parseInt(registerId);
	const group = mockData.modbus.find((g) => g.id === groupId);

	if (!group) {
		return res.status(404).json({ error: "Outdoor device not found" });
	}

	if (slave !== undefined) {
		// Delete register from indoor device
		const slaveId = parseInt(slave);
		const slaveObj = group.slaves.find((s) => s.id === slaveId);

		if (!slaveObj) {
			return res.status(404).json({ error: "Indoor device not found" });
		}

		const registerIndex = slaveObj.registers.findIndex((r) => r.id === regId);
		if (registerIndex === -1) {
			return res.status(404).json({ error: "Register not found" });
		}

		const deletedRegister = slaveObj.registers[registerIndex];
		slaveObj.registers.splice(registerIndex, 1);

		// Clean up register value
		const key = `${groupId}_${slaveId}`;
		if (mockData.registerValues[key]) {
			delete mockData.registerValues[key][regId];
		}

		return res.json({ id: deletedRegister.id, name: deletedRegister.name });
	} else {
		// Delete register from outdoor device
		const registerIndex = group.registers.findIndex((r) => r.id === regId);
		if (registerIndex === -1) {
			return res.status(404).json({ error: "Register not found" });
		}

		const deletedRegister = group.registers[registerIndex];
		group.registers.splice(registerIndex, 1);

		// Clean up register value
		const key = `${groupId}_group`;
		if (mockData.registerValues[key]) {
			delete mockData.registerValues[key][regId];
		}

		return res.json({ id: deletedRegister.id, name: deletedRegister.name });
	}
});

// ============================================================
// CORS MIDDLEWARE
// ============================================================

app.use((req, res, next) => {
	res.header("Access-Control-Allow-Origin", "*");
	res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
	res.header("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS");
	next();
});

app.options("*", (req, res) => {
	res.sendStatus(200);
});

// ============================================================
// ERROR HANDLING & STARTUP
// ============================================================

app.use((err, req, res, next) => {
	console.error(err.stack);
	res.status(500).json({ error: "Internal server error", message: err.message });
});

app.listen(PORT, () => {
	console.log(`Mock backend server running at http://localhost:${PORT}`);
	console.log("Open http://localhost:3000 in your browser");
	console.log("\nAvailable API endpoints:");
	console.log("  GET    /api/status");
	console.log("  GET    /api/interfaces");
	console.log("  POST   /api/interfaces");
	console.log("  GET    /api/modbus/groups");
	console.log("  POST   /api/modbus/group/create?id={id}&slave={slave}");
	console.log("  PATCH  /api/modbus/group/update?id={id}&slave={slave}");
	console.log("  DELETE /api/modbus/group/delete?id={id}");
	console.log("  POST   /api/modbus/group/update/register?id={id}&slave={slave}&registerId={registerId}");
	console.log("  PATCH  /api/modbus/group/update/register?id={id}&slave={slave}&registerId={registerId}");
	console.log("  DELETE /api/modbus/group/update/register?id={id}&slave={slave}&registerId={registerId}");
	console.log("  GET    /api/modbus/group?id={id}");
});
