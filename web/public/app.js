const applicationState = {
	currentPage: "dashboard",
	dashboardState: {
		refreshInterval: null,
		refreshRate: 3000,
		metricsCache: null,
	},
	modbusState: {
		refreshInterval: null,
		refreshRate: 5000,
		groups: [],
		groupsInitialized: false,
	},
	modalsState: {
		addGroupPending: false,
		editGroupPending: false,
		editGroupId: null,
		deleteGroupPending: false,
		deleteGroupId: null,
	},
	mapState: {
		data: null,
		loading: false,
	},
};

let registerModalState = {
	groupId: null,
	slaveId: null,
	registers: [],
	editMode: false,
};

async function apiCall(method, url, body = null) {
	try {
		const options = {
			method: method,
			headers: { "Content-Type": "application/json" },
		};

		if (body && (method === "POST" || method === "PATCH")) {
			options.body = JSON.stringify(body);
		}

		const response = await fetch(url, options);

		if (!response.ok) {
			const errorData = await response.json().catch(() => ({}));
			const errorMessage = errorData.error || `HTTP ${response.status}: ${response.statusText}`;
			throw new Error(errorMessage);
		}

		return await response.json();
	} catch (error) {
		console.error("API Error:", error);
		throw error;
	}
}

function initializeDashboardMetrics() {
	const metricLabels = [
		"Uptime",
		"UART 1 Sent Packets",
		"UART 1 Received Packets",
		"UART 2 Sent Packets",
		"UART 2 Received Packets",
		"Ethernet Status",
		"Ethernet IP",
	];

	const metricsHtml = metricLabels
		.map(
			(label, index) => `
        <div class="metric_card">
            <div class="metric_label">${label}</div>
            <div class="metric_value" data-metric-index="${index}">--</div>
        </div>
    `,
		)
		.join("");

	document.getElementById("metrics_container").innerHTML = metricsHtml;
	applicationState.dashboardState.metricsCache = {};
}

async function updateDashboardMetrics() {
	try {
		const statusData = await apiCall("GET", "/api/status");

		const metricValues = [
			formatUptime(statusData.uptime),
			String(statusData.uart1_sent),
			String(statusData.uart1_recived),
			String(statusData.uart2_sent),
			String(statusData.uart2_recived),
			statusData.eth_status,
			statusData.eth_ip || "None",
		];

		document.querySelectorAll(".metric_value").forEach((element, index) => {
			element.textContent = metricValues[index];
		});

		clearMessage("dashboard_message");
	} catch (error) {
		showMessage("dashboard_message", "error", `Error: ${error.message}`);
	}
}

function formatUptime(seconds) {
	const days = Math.floor(seconds / 86400);
	const hours = Math.floor((seconds % 86400) / 3600);
	const minutes = Math.floor((seconds % 3600) / 60);
	const secs = seconds % 60;

	if (days > 0) return `${days}d ${hours}h`;
	if (hours > 0) return `${hours}h ${minutes}m`;
	if (minutes > 0) return `${minutes}m ${secs}s`;
	return `${secs}s`;
}

function startDashboardRefresh() {
	if (applicationState.dashboardState.refreshInterval) {
		clearInterval(applicationState.dashboardState.refreshInterval);
	}

	initializeDashboardMetrics();
	updateDashboardMetrics();

	applicationState.dashboardState.refreshInterval = setInterval(() => {
		updateDashboardMetrics();
	}, applicationState.dashboardState.refreshRate);
}

function stopDashboardRefresh() {
	if (applicationState.dashboardState.refreshInterval) {
		clearInterval(applicationState.dashboardState.refreshInterval);
		applicationState.dashboardState.refreshInterval = null;
	}
}

async function loadInterfaces() {
	try {
		showMessage("interfaces_message", "loading", '<span class="loading_spinner"></span> Loading settings...');
		const interfacesData = await apiCall("GET", "/api/interfaces");
		populateInterfacesForm(interfacesData);
		clearMessage("interfaces_message");
	} catch (error) {
		showMessage("interfaces_message", "error", `Error: ${error.message}`);
	}
}

function populateInterfacesForm(data) {
	document.getElementById("uart1_baud").value = data.uart1_baud;
	document.getElementById("uart1_data").value = data.uart1_data;
	document.getElementById("uart1_stop").value = data.uart1_stop;
	document.getElementById("uart1_parity").value = data.uart1_parity;

	document.getElementById("uart2_baud").value = data.uart2_baud;
	document.getElementById("uart2_data").value = data.uart2_data;
	document.getElementById("uart2_stop").value = data.uart2_stop;
	document.getElementById("uart2_parity").value = data.uart2_parity;
}

async function saveInterfaces(event) {
	event.preventDefault();

	try {
		showMessage("interfaces_message", "loading", '<span class="loading_spinner"></span> Saving settings...');

		const interfacesData = {
			uart1_baud: document.getElementById("uart1_baud").value,
			uart1_data: document.getElementById("uart1_data").value,
			uart1_stop: document.getElementById("uart1_stop").value,
			uart1_parity: document.getElementById("uart1_parity").value,
			uart2_baud: document.getElementById("uart2_baud").value,
			uart2_data: document.getElementById("uart2_data").value,
			uart2_stop: document.getElementById("uart2_stop").value,
			uart2_parity: document.getElementById("uart2_parity").value,
		};

		await apiCall("POST", "/api/interfaces", interfacesData);
		showMessage("interfaces_message", "success", "Settings saved successfully!");
		setTimeout(() => clearMessage("interfaces_message"), 3000);
	} catch (error) {
		showMessage("interfaces_message", "error", `Error: ${error.message}`);
	}
}

async function loadModbusConfig() {
	try {
		showMessage("modbus_message", "loading", '<span class="loading_spinner"></span> Loading configuration...');
		const groupsData = await apiCall("GET", "/api/modbus/groups");

		applicationState.modbusState.groups = groupsData;
		displayModbusGroups();
		applicationState.modbusState.groupsInitialized = true;
		clearMessage("modbus_message");

		if (applicationState.modbusState.refreshInterval) {
			clearInterval(applicationState.modbusState.refreshInterval);
		}
		startModbusRefresh();
	} catch (error) {
		showMessage("modbus_message", "error", `Error: ${error.message}`);
	}
}

function displayModbusGroups() {
	const container = document.getElementById("groups_container");

	if (applicationState.modbusState.groups.length === 0) {
		container.innerHTML = '<div style="grid-column: 1/-1; text-align: center; padding: 40px; color: #7f8c8d;">No outdoor devices configured</div>';
		return;
	}

	container.innerHTML = applicationState.modbusState.groups.map((group) => createGroupColumn(group)).join("");

	applicationState.modbusState.groups.forEach((group) => {
		updateModbusGroupValues(group.id);
	});
}

function createGroupColumn(group) {
	let groupRegistersHtml = "";

	if (group.registers && group.registers.length > 0) {
		groupRegistersHtml = `
            <div class="registers_section">
                <div class="registers_header">
                    <h4>Outdoor Registers</h4>
                    <button type="button" class="btn_mini" onclick="openRegisterModal(${group.id})">Edit</button>
                </div>
        `;

		group.registers.forEach((register) => {
			const registerId = typeof register === "object" ? register.id : register;
			const registerName = typeof register === "object" ? register.name : "Register";

			groupRegistersHtml += `
                <div class="register_item" data-register-type="group" data-register-id="${registerId}">
                    <span class="register_id">${registerName} (${registerId})</span>
                    <span class="register_value">--</span>
                </div>
            `;
		});

		groupRegistersHtml += "</div>";
	} else {
		groupRegistersHtml = `
            <div class="registers_section">
                <div class="registers_header">
                    <h4>Outdoor Registers</h4>
                    <button type="button" class="btn_mini" onclick="openRegisterModal(${group.id})">Edit</button>
                </div>
                <div class="empty_state">No registers configured</div>
            </div>
        `;
	}

	let slavesHtml = "";

	if (group.slaves && group.slaves.length > 0) {
		group.slaves.forEach((slave) => {
			slavesHtml += `
                <div class="registers_section">
                    <div class="slave_item">
                        <div class="registers_header">
                            <div class="slave_header">Indoor ${slave.id}</div>
                            <button type="button" class="btn_mini" onclick="openRegisterModal(${group.id}, ${slave.id})">Edit</button>
                        </div>
            `;

			if (slave.registers && slave.registers.length > 0) {
				slave.registers.forEach((register) => {
					const registerId = typeof register === "object" ? register.id : register;
					const registerName = typeof register === "object" ? register.name : "Register";

					slavesHtml += `
                        <div class="register_item" data-register-type="slave" data-slave-id="${slave.id}" data-register-id="${registerId}">
                            <span class="register_id">${registerName} (${registerId})</span>
                            <span class="register_value">--</span>
                        </div>
                    `;
				});
			} else {
				slavesHtml += '<div class="empty_state">No registers configured</div>';
			}

			slavesHtml += "</div></div>";
		});
	}

	return `
        <div class="group_column" data-group-id="${group.id}">
            <div class="group_header">
                <h3>${group.name}</h3>
                <div class="group_header_actions">
                    <button type="button" class="btn_secondary" onclick="openEditGroupModal(${group.id})">Edit</button>
                    <button type="button" class="btn_danger" onclick="openDeleteGroupModal(${group.id})">Delete</button>
                </div>
            </div>
            <div class="group_content">
                <div id="group_registers_${group.id}">
                    ${groupRegistersHtml}
                    ${slavesHtml}
                </div>
            </div>
        </div>
    `;
}

async function updateModbusGroupValues(groupId) {
	try {
		const groupData = await apiCall("GET", `/api/modbus/group?id=${groupId}`);
		const groupElement = document.querySelector(`[data-group-id="${groupId}"]`);

		if (!groupElement) return;

		if (groupData.registers && groupData.registers.length > 0) {
			groupData.registers.forEach((register) => {
				const valueElement = groupElement.querySelector(`[data-register-type="group"][data-register-id="${register.id}"] .register_value`);
				if (valueElement) {
					valueElement.textContent = register.value;
				}
			});
		}

		if (groupData.slaves && groupData.slaves.length > 0) {
			groupData.slaves.forEach((slave) => {
				if (slave.registers && slave.registers.length > 0) {
					slave.registers.forEach((register) => {
						const valueElement = groupElement.querySelector(
							`[data-register-type="slave"][data-slave-id="${slave.id}"][data-register-id="${register.id}"] .register_value`,
						);
						if (valueElement) {
							valueElement.textContent = register.value;
						}
					});
				}
			});
		}
	} catch (error) {
		console.error(`Error updating group ${groupId}:`, error);
	}
}

function startModbusRefresh() {
	const refreshRate = parseInt(document.getElementById("refresh_rate").value);
	applicationState.modbusState.refreshRate = refreshRate;

	if (applicationState.modbusState.refreshInterval) {
		clearInterval(applicationState.modbusState.refreshInterval);
	}

	applicationState.modbusState.refreshInterval = setInterval(() => {
		applicationState.modbusState.groups.forEach((group) => {
			updateModbusGroupValues(group.id);
		});
	}, applicationState.modbusState.refreshRate);
}

function stopModbusRefresh() {
	if (applicationState.modbusState.refreshInterval) {
		clearInterval(applicationState.modbusState.refreshInterval);
		applicationState.modbusState.refreshInterval = null;
	}
}

function openAddGroupModal() {
	document.getElementById("add_group_modal").classList.add("active");
	document.getElementById("modal_group_id").value = "";
	document.getElementById("modal_group_slave").value = "";
	clearModalMessage("add_group_modal_message");
	document.getElementById("modal_group_id").focus();
}

function closeAddGroupModal() {
	document.getElementById("add_group_modal").classList.remove("active");
	applicationState.modalsState.addGroupPending = false;
}

async function submitAddGroupModal() {
	const groupId = document.getElementById("modal_group_id").value;
	const slaveCount = document.getElementById("modal_group_slave").value;

	if (!groupId || !slaveCount) {
		showModalMessage("add_group_modal_message", "error", "Please enter both Modbus Address and Indoor Device Number");
		return;
	}

	if (applicationState.modalsState.addGroupPending) return;

	try {
		applicationState.modalsState.addGroupPending = true;
		showModalMessage("add_group_modal_message", "loading", '<span class="loading_spinner"></span> Creating group...');

		await apiCall("POST", `/api/modbus/group/create?id=${groupId}&slave=${slaveCount}`);

		closeAddGroupModal();
		await loadModbusConfig();
		showMessage("modbus_message", "success", "Outdoor device created successfully!");
		setTimeout(() => clearMessage("modbus_message"), 3000);
	} catch (error) {
		showModalMessage("add_group_modal_message", "error", `Error: ${error.message}`);
		applicationState.modalsState.addGroupPending = false;
	}
}

function openEditGroupModal(groupId) {
	applicationState.modalsState.editGroupId = groupId;

	const group = applicationState.modbusState.groups.find((g) => g.id === groupId);
	const slaveCount = group ? group.slaves.length : 0;

	document.getElementById("edit_group_modal").classList.add("active");
	document.getElementById("modal_edit_group_slave").value = slaveCount;
	clearModalMessage("edit_group_modal_message");
	document.getElementById("modal_edit_group_slave").focus();
}

function closeEditGroupModal() {
	document.getElementById("edit_group_modal").classList.remove("active");
	applicationState.modalsState.editGroupPending = false;
	applicationState.modalsState.editGroupId = null;
}

async function submitEditGroupModal() {
	const groupId = applicationState.modalsState.editGroupId;
	const slaveCount = document.getElementById("modal_edit_group_slave").value;

	if (!slaveCount) {
		showModalMessage("edit_group_modal_message", "error", "Please enter number of indoor devices");
		return;
	}

	if (applicationState.modalsState.editGroupPending) return;

	try {
		applicationState.modalsState.editGroupPending = true;
		showModalMessage("edit_group_modal_message", "loading", '<span class="loading_spinner"></span> Updating group...');

		await apiCall("PATCH", `/api/modbus/group/update?id=${groupId}&slave=${slaveCount}`);

		closeEditGroupModal();
		await loadModbusConfig();
		showMessage("modbus_message", "success", "Outdoor device updated successfully!");
		setTimeout(() => clearMessage("modbus_message"), 3000);
	} catch (error) {
		showModalMessage("edit_group_modal_message", "error", `Error: ${error.message}`);
		applicationState.modalsState.editGroupPending = false;
	}
}

function openDeleteGroupModal(groupId) {
	applicationState.modalsState.deleteGroupId = groupId;
	document.getElementById("delete_group_modal").classList.add("active");
	clearModalMessage("delete_group_modal_message");
}

function closeDeleteGroupModal() {
	document.getElementById("delete_group_modal").classList.remove("active");
	applicationState.modalsState.deleteGroupPending = false;
	applicationState.modalsState.deleteGroupId = null;
}

async function submitDeleteGroupModal() {
	const groupId = applicationState.modalsState.deleteGroupId;

	if (applicationState.modalsState.deleteGroupPending) return;

	try {
		applicationState.modalsState.deleteGroupPending = true;
		showModalMessage("delete_group_modal_message", "loading", '<span class="loading_spinner"></span> Deleting group...');

		await apiCall("DELETE", `/api/modbus/group/delete?id=${groupId}`);

		closeDeleteGroupModal();
		await loadModbusConfig();
		showMessage("modbus_message", "success", "Outdoor device deleted successfully!");
		setTimeout(() => clearMessage("modbus_message"), 3000);
	} catch (error) {
		showModalMessage("delete_group_modal_message", "error", `Error: ${error.message}`);
		applicationState.modalsState.deleteGroupPending = false;
	}
}

function openRegisterModal(groupId, slaveId = null) {
	registerModalState.groupId = groupId;
	registerModalState.slaveId = slaveId;
	registerModalState.editMode = false;

	const group = applicationState.modbusState.groups.find((g) => g.id === groupId);
	if (!group) return;

	if (slaveId !== null) {
		const slave = group.slaves.find((s) => s.id === slaveId);
		if (!slave) return;

		registerModalState.registers = JSON.parse(JSON.stringify(slave.registers || []));
		document.getElementById("register_modal_title").textContent = `Manage Registers - ${group.name} - Indoor ${slaveId}`;
	} else {
		registerModalState.registers = JSON.parse(JSON.stringify(group.registers || []));
		document.getElementById("register_modal_title").textContent = `Manage Registers - ${group.name}`;
	}

	renderRegisterList();
	document.getElementById("register_modal").classList.add("active");
}

function closeRegisterModal() {
	document.getElementById("register_modal").classList.remove("active");
	registerModalState = { groupId: null, slaveId: null, registers: [], editMode: false };
}

function renderRegisterList() {
	const listElement = document.getElementById("register_list");
	let html = "";

	registerModalState.registers.forEach((register, index) => {
		if (registerModalState.editMode) {
			html += `
                <div class="register_row" data-index="${index}">
                    <input type="number" class="register_input" value="${register.id}" data-field="id" />
                    <input type="text" class="register_input" value="${register.name}" data-field="name" />
                    <button type="button" class="btn_icon" onclick="deleteRegisterRow(${index})" title="Delete">×</button>
                </div>
            `;
		} else {
			html += `
                <div class="register_row register_row_view">
                    <span class="register_display">${register.id}</span>
                    <span class="register_display">${register.name}</span>
                    <span class="register_spacer"></span>
                </div>
            `;
		}
	});

	html += `
        <div class="register_row register_row_new">
            <input type="number" class="register_input" id="new_register_id" placeholder="Address" />
            <input type="text" class="register_input" id="new_register_name" placeholder="Name" />
            <button type="button" class="btn_primary btn_sm" onclick="addNewRegister()">Add</button>
        </div>
    `;

	if (registerModalState.editMode) {
		html += `
            <div class="register_actions">
                <button type="button" class="btn_primary" onclick="saveRegisterChanges()">Save</button>
                <button type="button" class="btn_secondary" onclick="cancelRegisterEdit()">Cancel</button>
            </div>
        `;
	} else {
		html += `
            <div class="register_actions">
                <button type="button" class="btn_secondary" onclick="enableRegisterEdit()">Edit Registers</button>
            </div>
        `;
	}

	listElement.innerHTML = html;
}

function enableRegisterEdit() {
	registerModalState.editMode = true;
	renderRegisterList();
}

function cancelRegisterEdit() {
	registerModalState.editMode = false;
	openRegisterModal(registerModalState.groupId, registerModalState.slaveId);
}

async function saveRegisterChanges() {
	try {
		clearModalMessage("register_modal_message");

		const registerRows = document.querySelectorAll(".register_row[data-index]");
		const updatedRegisters = [];

		registerRows.forEach((row) => {
			const index = parseInt(row.getAttribute("data-index"));
			const idInput = row.querySelector('[data-field="id"]');
			const nameInput = row.querySelector('[data-field="name"]');
			const registerId = parseInt(idInput.value);
			const registerName = nameInput.value.trim();

			if (!registerName) {
				throw new Error("Register name cannot be empty");
			}

			updatedRegisters.push({
				id: registerId,
				name: registerName,
				originalIndex: index,
			});
		});

		const registerIds = updatedRegisters.map((r) => r.id);
		if (new Set(registerIds).size !== registerIds.length) {
			throw new Error("Duplicate register IDs found");
		}

		showModalMessage("register_modal_message", "loading", '<span class="loading_spinner"></span> Saving changes...');

		for (let i = 0; i < updatedRegisters.length; i++) {
			const updated = updatedRegisters[i];
			const original = registerModalState.registers[updated.originalIndex];

			if (original.id !== updated.id || original.name !== updated.name) {
				const url =
					registerModalState.slaveId !== null
						? `/api/modbus/group/update/register?id=${registerModalState.groupId}&slave=${registerModalState.slaveId}`
						: `/api/modbus/group/update/register?id=${registerModalState.groupId}`;

				await apiCall("PATCH", url, {
					id: original.id,
					newId: updated.id,
					name: updated.name,
				});
			}
		}

		registerModalState.registers = updatedRegisters.map((r) => ({ id: r.id, name: r.name }));
		registerModalState.editMode = false;

		await loadModbusConfig();
		renderRegisterList();
		showModalMessage("register_modal_message", "success", "Registers updated successfully!");
		setTimeout(() => clearModalMessage("register_modal_message"), 2000);
	} catch (error) {
		showModalMessage("register_modal_message", "error", `Error: ${error.message}`);
	}
}

async function addNewRegister() {
	try {
		clearModalMessage("register_modal_message");

		const idInput = document.getElementById("new_register_id");
		const nameInput = document.getElementById("new_register_name");
		const registerId = parseInt(idInput.value);
		const registerName = nameInput.value.trim();

        if (isNaN(registerId) || !registerName) {
            showModalMessage("register_modal_message", "error", "Please enter both Address and Name");
            return;
        }

		if (registerModalState.registers.some((r) => r.id === registerId)) {
			showModalMessage("register_modal_message", "error", "Register with this Address already exists");
			return;
		}

		showModalMessage("register_modal_message", "loading", '<span class="loading_spinner"></span> Adding register...');

		const url =
			registerModalState.slaveId !== null
				? `/api/modbus/group/update/register?id=${registerModalState.groupId}&slave=${registerModalState.slaveId}`
				: `/api/modbus/group/update/register?id=${registerModalState.groupId}`;

		await apiCall("POST", url, { id: registerId, name: registerName });

		registerModalState.registers.push({ id: registerId, name: registerName });
		idInput.value = "";
		nameInput.value = "";

		await loadModbusConfig();
		renderRegisterList();
		showModalMessage("register_modal_message", "success", "Register added successfully!");
		setTimeout(() => clearModalMessage("register_modal_message"), 2000);
	} catch (error) {
		showModalMessage("register_modal_message", "error", `Error: ${error.message}`);
	}
}

async function deleteRegisterRow(index) {
	try {
		const register = registerModalState.registers[index];

		showModalMessage("register_modal_message", "loading", '<span class="loading_spinner"></span> Deleting register...');

		const url =
			registerModalState.slaveId !== null
				? `/api/modbus/group/update/register?id=${registerModalState.groupId}&slave=${registerModalState.slaveId}&registerId=${register.id}`
				: `/api/modbus/group/update/register?id=${registerModalState.groupId}&registerId=${register.id}`;

		await apiCall("DELETE", url);

		registerModalState.registers.splice(index, 1);

		await loadModbusConfig();
		renderRegisterList();
		showModalMessage("register_modal_message", "success", "Register deleted successfully!");
		setTimeout(() => clearModalMessage("register_modal_message"), 2000);
	} catch (error) {
		showModalMessage("register_modal_message", "error", `Error: ${error.message}`);
	}
}

function showMessage(elementId, type, message) {
	const element = document.getElementById(elementId);
	element.className = `status_message status_${type}`;
	element.innerHTML = message;
}

function clearMessage(elementId) {
	const element = document.getElementById(elementId);
	element.innerHTML = "";
	element.className = "";
}

function showModalMessage(elementId, type, message) {
	const element = document.getElementById(elementId);
	element.className = `modal_message ${type}`;
	element.innerHTML = message;
}

function clearModalMessage(elementId) {
	const element = document.getElementById(elementId);
	element.innerHTML = "";
	element.className = "";
}

function navigateToPage(pageName) {
	stopDashboardRefresh();
	stopModbusRefresh();

	document.querySelectorAll(".page").forEach((page) => {
		page.classList.remove("active");
	});

	document.querySelectorAll(".nav_link").forEach((link) => {
		link.classList.remove("active");
	});

	document.getElementById(pageName).classList.add("active");
	document.querySelector(`[data-page="${pageName}"]`).classList.add("active");

	applicationState.currentPage = pageName;

	if (pageName === "dashboard") {
		startDashboardRefresh();
	} else if (pageName === "interfaces") {
		loadInterfaces();
	} else if (pageName === "modbus") {
		loadModbusConfig();
	} else if (pageName === "map") {
		loadMapData();
	}
}

// Register Map Functions
async function loadMapData() {
	if (applicationState.mapState.loading) return;

	try {
		applicationState.mapState.loading = true;
		showMessage("map_message", "Loading register map...", "info");

		const data = await apiCall("GET", "/api/map");
		applicationState.mapState.data = data;

		renderMapData(data);
		updateMapStatus(data);
		clearMessage("map_message");
	} catch (error) {
		showMessage("map_message", `Failed to load map: ${error.message}`, "error");
	} finally {
		applicationState.mapState.loading = false;
	}
}

function updateMapStatus(data) {
	const statusElement = document.getElementById("map_status");
	if (data.initialized) {
		statusElement.textContent = `Initialized • ${data.total_groups} groups mapped`;
		statusElement.classList.add("initialized");
	} else {
		statusElement.textContent = "Not initialized";
		statusElement.classList.remove("initialized");
	}
}

function renderMapData(data) {
	const container = document.getElementById("map_container");

	if (!data.groups || data.groups.length === 0) {
		container.innerHTML = '<div class="map_empty">No groups configured. Add groups in the Modbus Config page.</div>';
		return;
	}

	const groupsHtml = data.groups
		.map((group) => {
			const registersHtml = group.registers
				.map((reg) => {
					const typeBadge = reg.type === "group" 
						? '<span class="map_type_badge map_type_group">Group</span>'
						: `<span class="map_type_badge map_type_slave">Slave ${reg.slave_id}</span>`;
					
					const deviceInfo = reg.type === "slave" 
						? `<div class="map_device_info">${'Indoor ' + reg.slave_id}</div>`
						: '<div class="map_device_info">Outdoor Device</div>';

					return `
						<tr>
							<td class="map_address">${reg.address}</td>
							<td>${reg.id}</td>
							<td>${reg.name}</td>
							<td>${typeBadge}</td>
							<td>${deviceInfo}</td>
						</tr>
					`;
				})
				.join("");

			return `
				<div class="map_group">
					<div class="map_group_header">
						<div>
							<div class="map_group_title">${group.name}</div>
						</div>
						<div class="map_group_badge">${group.total_registers} registers</div>
					</div>
					<table class="map_table">
						<thead>
							<tr>
								<th>Address</th>
								<th>Remote Address</th>
								<th>Name</th>
								<th>Type</th>
								<th>Device</th>
							</tr>
						</thead>
						<tbody>
							${registersHtml}
						</tbody>
					</table>
				</div>
			`;
		})
		.join("");

	container.innerHTML = groupsHtml;
}

document.addEventListener("DOMContentLoaded", function () {
	document.querySelectorAll(".nav_link").forEach((link) => {
		link.addEventListener("click", function () {
			navigateToPage(this.getAttribute("data-page"));
		});
	});

	document.getElementById("interfaces_form").addEventListener("submit", saveInterfaces);
	document.getElementById("add_group_btn").addEventListener("click", openAddGroupModal);

	document.getElementById("refresh_rate").addEventListener("change", function () {
		applicationState.modbusState.refreshRate = parseInt(this.value);
		if (applicationState.currentPage === "modbus") {
			startModbusRefresh();
		}
	});

	document.getElementById("add_group_modal").addEventListener("click", function (event) {
		if (event.target === this) {
			closeAddGroupModal();
		}
	});

	document.getElementById("edit_group_modal").addEventListener("click", function (event) {
		if (event.target === this) {
			closeEditGroupModal();
		}
	});

	document.getElementById("delete_group_modal").addEventListener("click", function (event) {
		if (event.target === this) {
			closeDeleteGroupModal();
		}
	});

	navigateToPage("dashboard");
});
