const applicationState = {
	currentPage: "dashboard",
	dashboardState: { refreshInterval: null, refreshRate: 3e3, metricsCache: null },
	modbusState: { refreshInterval: null, refreshRate: 5e3, groups: [], groupsInitialized: !1 },
	modalsState: { addGroupPending: !1, editGroupPending: !1, editGroupId: null, deleteGroupPending: !1, deleteGroupId: null },
	mapState: { data: null, loading: !1 },
};
let registerModalState = { groupId: null, slaveId: null, registers: [], editMode: !1 };
async function apiCall(e, t, a = null) {
	try {
		const s = { method: e, headers: { "Content-Type": "application/json" } };
		!a || ("POST" !== e && "PATCH" !== e) || (s.body = JSON.stringify(a));
		const r = await fetch(t, s);
		if (!r.ok) {
			const e = (await r.json().catch(() => ({}))).error || `HTTP ${r.status}: ${r.statusText}`;
			throw new Error(e);
		}
		return await r.json();
	} catch (e) {
		throw (console.error("API Error:", e), e);
	}
}
function initializeDashboardMetrics() {
	const e = ["Uptime", "UART 1 Sent Packets", "UART 1 Received Packets", "UART 2 Sent Packets", "UART 2 Received Packets", "Ethernet Status", "Ethernet IP"]
		.map(
			(e, t) =>
				`\n        <div class="metric_card">\n            <div class="metric_label">${e}</div>\n            <div class="metric_value" data-metric-index="${t}">--</div>\n        </div>\n    `,
		)
		.join("");
	(document.getElementById("metrics_container").innerHTML = e), (applicationState.dashboardState.metricsCache = {});
}
async function updateDashboardMetrics() {
	try {
		const e = await apiCall("GET", "/api/status"),
			t = [
				formatUptime(e.uptime),
				String(e.uart1_sent),
				String(e.uart1_recived),
				String(e.uart2_sent),
				String(e.uart2_recived),
				e.eth_status,
				e.eth_ip || "None",
			];
		document.querySelectorAll(".metric_value").forEach((e, a) => {
			e.textContent = t[a];
		}),
			clearMessage("dashboard_message");
	} catch (e) {
		showMessage("dashboard_message", "error", `Error: ${e.message}`);
	}
}
function formatUptime(e) {
	const t = Math.floor(e / 86400),
		a = Math.floor((e % 86400) / 3600),
		s = Math.floor((e % 3600) / 60),
		r = e % 60;
	return t > 0 ? `${t}d ${a}h` : a > 0 ? `${a}h ${s}m` : s > 0 ? `${s}m ${r}s` : `${r}s`;
}
function startDashboardRefresh() {
	applicationState.dashboardState.refreshInterval && clearInterval(applicationState.dashboardState.refreshInterval),
		initializeDashboardMetrics(),
		updateDashboardMetrics(),
		(applicationState.dashboardState.refreshInterval = setInterval(() => {
			updateDashboardMetrics();
		}, applicationState.dashboardState.refreshRate));
}
function stopDashboardRefresh() {
	applicationState.dashboardState.refreshInterval &&
		(clearInterval(applicationState.dashboardState.refreshInterval), (applicationState.dashboardState.refreshInterval = null));
}
async function loadInterfaces() {
	try {
		showMessage("interfaces_message", "loading", '<span class="loading_spinner"></span> Loading settings...');
		populateInterfacesForm(await apiCall("GET", "/api/interfaces")), clearMessage("interfaces_message");
	} catch (e) {
		showMessage("interfaces_message", "error", `Error: ${e.message}`);
	}
}
function populateInterfacesForm(e) {
	(document.getElementById("uart1_baud").value = e.uart1_baud),
		(document.getElementById("uart1_data").value = e.uart1_data),
		(document.getElementById("uart1_stop").value = e.uart1_stop),
		(document.getElementById("uart1_parity").value = e.uart1_parity),
		(document.getElementById("uart2_baud").value = e.uart2_baud),
		(document.getElementById("uart2_data").value = e.uart2_data),
		(document.getElementById("uart2_stop").value = e.uart2_stop),
		(document.getElementById("uart2_parity").value = e.uart2_parity);
}
async function saveInterfaces(e) {
	e.preventDefault();
	try {
		showMessage("interfaces_message", "loading", '<span class="loading_spinner"></span> Saving settings...');
		const e = {
			uart1_baud: document.getElementById("uart1_baud").value,
			uart1_data: document.getElementById("uart1_data").value,
			uart1_stop: document.getElementById("uart1_stop").value,
			uart1_parity: document.getElementById("uart1_parity").value,
			uart2_baud: document.getElementById("uart2_baud").value,
			uart2_data: document.getElementById("uart2_data").value,
			uart2_stop: document.getElementById("uart2_stop").value,
			uart2_parity: document.getElementById("uart2_parity").value,
		};
		await apiCall("POST", "/api/interfaces", e),
			showMessage("interfaces_message", "success", "Settings saved successfully!"),
			setTimeout(() => clearMessage("interfaces_message"), 3e3);
	} catch (e) {
		showMessage("interfaces_message", "error", `Error: ${e.message}`);
	}
}
async function loadModbusConfig() {
	try {
		showMessage("modbus_message", "loading", '<span class="loading_spinner"></span> Loading configuration...');
		const e = await apiCall("GET", "/api/modbus/groups");
		(applicationState.modbusState.groups = e),
			displayModbusGroups(),
			(applicationState.modbusState.groupsInitialized = !0),
			clearMessage("modbus_message"),
			applicationState.modbusState.refreshInterval && clearInterval(applicationState.modbusState.refreshInterval),
			startModbusRefresh();
	} catch (e) {
		showMessage("modbus_message", "error", `Error: ${e.message}`);
	}
}
function displayModbusGroups() {
	const e = document.getElementById("groups_container");
	0 !== applicationState.modbusState.groups.length
		? ((e.innerHTML = applicationState.modbusState.groups.map((e) => createGroupColumn(e)).join("")),
		  applicationState.modbusState.groups.forEach((e) => {
				updateModbusGroupValues(e.id);
		  }))
		: (e.innerHTML = '<div style="grid-column: 1/-1; text-align: center; padding: 40px; color: #7f8c8d;">No outdoor devices configured</div>');
}
function createGroupColumn(e) {
	let t = "";
	e.registers && e.registers.length > 0
		? ((t = `\n            <div class="registers_section">\n                <div class="registers_header">\n                    <h4>Outdoor Registers</h4>\n                    <button type="button" class="btn_mini" onclick="openRegisterModal(${e.id})">Edit</button>\n                </div>\n        `),
		  e.registers.forEach((e) => {
				const a = "object" == typeof e ? e.id : e,
					s = "object" == typeof e ? e.name : "Register";
				t += `\n                <div class="register_item" data-register-type="group" data-register-id="${a}">\n                    <span class="register_id">${s} (${a})</span>\n                    <span class="register_value">--</span>\n                </div>\n            `;
		  }),
		  (t += "</div>"))
		: (t = `\n            <div class="registers_section">\n                <div class="registers_header">\n                    <h4>Outdoor Registers</h4>\n                    <button type="button" class="btn_mini" onclick="openRegisterModal(${e.id})">Edit</button>\n                </div>\n                <div class="empty_state">No registers configured</div>\n            </div>\n        `);
	let a = "";
	return (
		e.slaves &&
			e.slaves.length > 0 &&
			e.slaves.forEach((t) => {
				(a += `\n                <div class="registers_section">\n                    <div class="slave_item">\n                        <div class="registers_header">\n                            <div class="slave_header">Indoor ${t.id}</div>\n                            <button type="button" class="btn_mini" onclick="openRegisterModal(${e.id}, ${t.id})">Edit</button>\n                        </div>\n            `),
					t.registers && t.registers.length > 0
						? t.registers.forEach((e) => {
								const s = "object" == typeof e ? e.id : e,
									r = "object" == typeof e ? e.name : "Register";
								a += `\n                        <div class="register_item" data-register-type="slave" data-slave-id="${t.id}" data-register-id="${s}">\n                            <span class="register_id">${r} (${s})</span>\n                            <span class="register_value">--</span>\n                        </div>\n                    `;
						  })
						: (a += '<div class="empty_state">No registers configured</div>'),
					(a += "</div></div>");
			}),
		`\n        <div class="group_column" data-group-id="${e.id}">\n            <div class="group_header">\n                <h3>${e.name}</h3>\n                <div class="group_header_actions">\n                    <button type="button" class="btn_secondary" onclick="openEditGroupModal(${e.id})">Edit</button>\n                    <button type="button" class="btn_danger" onclick="openDeleteGroupModal(${e.id})">Delete</button>\n                </div>\n            </div>\n            <div class="group_content">\n                <div id="group_registers_${e.id}">\n                    ${t}\n                    ${a}\n                </div>\n            </div>\n        </div>\n    `
	);
}
async function updateModbusGroupValues(e) {
	try {
		const t = await apiCall("GET", `/api/modbus/group?id=${e}`),
			a = document.querySelector(`[data-group-id="${e}"]`);
		if (!a) return;
		t.registers &&
			t.registers.length > 0 &&
			t.registers.forEach((e) => {
				const t = a.querySelector(`[data-register-type="group"][data-register-id="${e.id}"] .register_value`);
				t && (t.textContent = e.value);
			}),
			t.slaves &&
				t.slaves.length > 0 &&
				t.slaves.forEach((e) => {
					e.registers &&
						e.registers.length > 0 &&
						e.registers.forEach((t) => {
							const s = a.querySelector(`[data-register-type="slave"][data-slave-id="${e.id}"][data-register-id="${t.id}"] .register_value`);
							s && (s.textContent = t.value);
						});
				});
	} catch (t) {
		console.error(`Error updating group ${e}:`, t);
	}
}
function startModbusRefresh() {
	const e = parseInt(document.getElementById("refresh_rate").value);
	(applicationState.modbusState.refreshRate = e),
		applicationState.modbusState.refreshInterval && clearInterval(applicationState.modbusState.refreshInterval),
		(applicationState.modbusState.refreshInterval = setInterval(() => {
			applicationState.modbusState.groups.forEach((e) => {
				updateModbusGroupValues(e.id);
			});
		}, applicationState.modbusState.refreshRate));
}
function stopModbusRefresh() {
	applicationState.modbusState.refreshInterval &&
		(clearInterval(applicationState.modbusState.refreshInterval), (applicationState.modbusState.refreshInterval = null));
}
function openAddGroupModal() {
	document.getElementById("add_group_modal").classList.add("active"),
		(document.getElementById("modal_group_id").value = ""),
		(document.getElementById("modal_group_slave").value = ""),
		clearModalMessage("add_group_modal_message"),
		document.getElementById("modal_group_id").focus();
}
function closeAddGroupModal() {
	document.getElementById("add_group_modal").classList.remove("active"), (applicationState.modalsState.addGroupPending = !1);
}
async function submitAddGroupModal() {
	const e = document.getElementById("modal_group_id").value,
		t = document.getElementById("modal_group_slave").value;
	if (e && t) {
		if (!applicationState.modalsState.addGroupPending)
			try {
				(applicationState.modalsState.addGroupPending = !0),
					showModalMessage("add_group_modal_message", "loading", '<span class="loading_spinner"></span> Creating group...'),
					await apiCall("POST", `/api/modbus/group/create?id=${e}&slave=${t}`),
					closeAddGroupModal(),
					await loadModbusConfig(),
					showMessage("modbus_message", "success", "Outdoor device created successfully!"),
					setTimeout(() => clearMessage("modbus_message"), 3e3);
			} catch (e) {
				showModalMessage("add_group_modal_message", "error", `Error: ${e.message}`), (applicationState.modalsState.addGroupPending = !1);
			}
	} else showModalMessage("add_group_modal_message", "error", "Please enter both Modbus Address and Indoor Device Number");
}
function openEditGroupModal(e) {
	applicationState.modalsState.editGroupId = e;
	const t = applicationState.modbusState.groups.find((t) => t.id === e),
		a = t ? t.slaves.length : 0;
	document.getElementById("edit_group_modal").classList.add("active"),
		(document.getElementById("modal_edit_group_slave").value = a),
		clearModalMessage("edit_group_modal_message"),
		document.getElementById("modal_edit_group_slave").focus();
}
function closeEditGroupModal() {
	document.getElementById("edit_group_modal").classList.remove("active"),
		(applicationState.modalsState.editGroupPending = !1),
		(applicationState.modalsState.editGroupId = null);
}
async function submitEditGroupModal() {
	const e = applicationState.modalsState.editGroupId,
		t = document.getElementById("modal_edit_group_slave").value;
	if (t) {
		if (!applicationState.modalsState.editGroupPending)
			try {
				(applicationState.modalsState.editGroupPending = !0),
					showModalMessage("edit_group_modal_message", "loading", '<span class="loading_spinner"></span> Updating group...'),
					await apiCall("PATCH", `/api/modbus/group/update?id=${e}&slave=${t}`),
					closeEditGroupModal(),
					await loadModbusConfig(),
					showMessage("modbus_message", "success", "Outdoor device updated successfully!"),
					setTimeout(() => clearMessage("modbus_message"), 3e3);
			} catch (e) {
				showModalMessage("edit_group_modal_message", "error", `Error: ${e.message}`), (applicationState.modalsState.editGroupPending = !1);
			}
	} else showModalMessage("edit_group_modal_message", "error", "Please enter number of indoor devices");
}
function openDeleteGroupModal(e) {
	(applicationState.modalsState.deleteGroupId = e),
		document.getElementById("delete_group_modal").classList.add("active"),
		clearModalMessage("delete_group_modal_message");
}
function closeDeleteGroupModal() {
	document.getElementById("delete_group_modal").classList.remove("active"),
		(applicationState.modalsState.deleteGroupPending = !1),
		(applicationState.modalsState.deleteGroupId = null);
}
async function submitDeleteGroupModal() {
	const e = applicationState.modalsState.deleteGroupId;
	if (!applicationState.modalsState.deleteGroupPending)
		try {
			(applicationState.modalsState.deleteGroupPending = !0),
				showModalMessage("delete_group_modal_message", "loading", '<span class="loading_spinner"></span> Deleting group...'),
				await apiCall("DELETE", `/api/modbus/group/delete?id=${e}`),
				closeDeleteGroupModal(),
				await loadModbusConfig(),
				showMessage("modbus_message", "success", "Outdoor device deleted successfully!"),
				setTimeout(() => clearMessage("modbus_message"), 3e3);
		} catch (e) {
			showModalMessage("delete_group_modal_message", "error", `Error: ${e.message}`), (applicationState.modalsState.deleteGroupPending = !1);
		}
}
function openRegisterModal(e, t = null) {
	(registerModalState.groupId = e), (registerModalState.slaveId = t), (registerModalState.editMode = !1);
	const a = applicationState.modbusState.groups.find((t) => t.id === e);
	if (a) {
		if (null !== t) {
			const e = a.slaves.find((e) => e.id === t);
			if (!e) return;
			(registerModalState.registers = JSON.parse(JSON.stringify(e.registers || []))),
				(document.getElementById("register_modal_title").textContent = `Manage Registers - ${a.name} - Indoor ${t}`);
		} else
			(registerModalState.registers = JSON.parse(JSON.stringify(a.registers || []))),
				(document.getElementById("register_modal_title").textContent = `Manage Registers - ${a.name}`);
		renderRegisterList(), document.getElementById("register_modal").classList.add("active");
	}
}
function closeRegisterModal() {
	document.getElementById("register_modal").classList.remove("active"), (registerModalState = { groupId: null, slaveId: null, registers: [], editMode: !1 });
}
function renderRegisterList() {
	const e = document.getElementById("register_list");
	let t = "";
	registerModalState.registers.forEach((e, a) => {
		registerModalState.editMode
			? (t += `\n                <div class="register_row" data-index="${a}">\n                    <input type="number" class="register_input" value="${e.id}" data-field="id" />\n                    <input type="text" class="register_input" value="${e.name}" data-field="name" />\n                    <button type="button" class="btn_icon" onclick="deleteRegisterRow(${a})" title="Delete">×</button>\n                </div>\n            `)
			: (t += `\n                <div class="register_row register_row_view">\n                    <span class="register_display">${e.id}</span>\n                    <span class="register_display">${e.name}</span>\n                    <span class="register_spacer"></span>\n                </div>\n            `);
	}),
		(t +=
			'\n        <div class="register_row register_row_new">\n            <input type="number" class="register_input" id="new_register_id" placeholder="Address" />\n            <input type="text" class="register_input" id="new_register_name" placeholder="Name" />\n            <button type="button" class="btn_primary btn_sm" onclick="addNewRegister()">Add</button>\n        </div>\n    '),
		registerModalState.editMode
			? (t +=
					'\n            <div class="register_actions">\n                <button type="button" class="btn_primary" onclick="saveRegisterChanges()">Save</button>\n                <button type="button" class="btn_secondary" onclick="cancelRegisterEdit()">Cancel</button>\n            </div>\n        ')
			: (t +=
					'\n            <div class="register_actions">\n                <button type="button" class="btn_secondary" onclick="enableRegisterEdit()">Edit Registers</button>\n            </div>\n        '),
		(e.innerHTML = t);
}
function enableRegisterEdit() {
	(registerModalState.editMode = !0), renderRegisterList();
}
function cancelRegisterEdit() {
	(registerModalState.editMode = !1), openRegisterModal(registerModalState.groupId, registerModalState.slaveId);
}
async function saveRegisterChanges() {
	try {
		clearModalMessage("register_modal_message");
		const e = document.querySelectorAll(".register_row[data-index]"),
			t = [];
		e.forEach((e) => {
			const a = parseInt(e.getAttribute("data-index")),
				s = e.querySelector('[data-field="id"]'),
				r = e.querySelector('[data-field="name"]'),
				o = parseInt(s.value),
				d = r.value.trim();
			if (!d) throw new Error("Register name cannot be empty");
			t.push({ id: o, name: d, originalIndex: a });
		});
		const a = t.map((e) => e.id);
		if (new Set(a).size !== a.length) throw new Error("Duplicate register IDs found");
		showModalMessage("register_modal_message", "loading", '<span class="loading_spinner"></span> Saving changes...');
		for (let e = 0; e < t.length; e++) {
			const a = t[e],
				s = registerModalState.registers[a.originalIndex];
			if (s.id !== a.id || s.name !== a.name) {
				const e =
					null !== registerModalState.slaveId
						? `/api/modbus/group/update/register?id=${registerModalState.groupId}&slave=${registerModalState.slaveId}`
						: `/api/modbus/group/update/register?id=${registerModalState.groupId}`;
				await apiCall("PATCH", e, { id: s.id, newId: a.id, name: a.name });
			}
		}
		(registerModalState.registers = t.map((e) => ({ id: e.id, name: e.name }))),
			(registerModalState.editMode = !1),
			await loadModbusConfig(),
			renderRegisterList(),
			showModalMessage("register_modal_message", "success", "Registers updated successfully!"),
			setTimeout(() => clearModalMessage("register_modal_message"), 2e3);
	} catch (e) {
		showModalMessage("register_modal_message", "error", `Error: ${e.message}`);
	}
}
async function addNewRegister() {
	try {
		clearModalMessage("register_modal_message");
		const e = document.getElementById("new_register_id"),
			t = document.getElementById("new_register_name"),
			a = parseInt(e.value),
			s = t.value.trim();
		if (isNaN(a) || !s) return void showModalMessage("register_modal_message", "error", "Please enter both Address and Name");
		if (registerModalState.registers.some((e) => e.id === a))
			return void showModalMessage("register_modal_message", "error", "Register with this Address already exists");
		showModalMessage("register_modal_message", "loading", '<span class="loading_spinner"></span> Adding register...');
		const r =
			null !== registerModalState.slaveId
				? `/api/modbus/group/update/register?id=${registerModalState.groupId}&slave=${registerModalState.slaveId}`
				: `/api/modbus/group/update/register?id=${registerModalState.groupId}`;
		await apiCall("POST", r, { id: a, name: s }),
			registerModalState.registers.push({ id: a, name: s }),
			(e.value = ""),
			(t.value = ""),
			await loadModbusConfig(),
			renderRegisterList(),
			showModalMessage("register_modal_message", "success", "Register added successfully!"),
			setTimeout(() => clearModalMessage("register_modal_message"), 2e3);
	} catch (e) {
		showModalMessage("register_modal_message", "error", `Error: ${e.message}`);
	}
}
async function deleteRegisterRow(e) {
	try {
		const t = registerModalState.registers[e];
		showModalMessage("register_modal_message", "loading", '<span class="loading_spinner"></span> Deleting register...');
		const a =
			null !== registerModalState.slaveId
				? `/api/modbus/group/update/register?id=${registerModalState.groupId}&slave=${registerModalState.slaveId}&registerId=${t.id}`
				: `/api/modbus/group/update/register?id=${registerModalState.groupId}&registerId=${t.id}`;
		await apiCall("DELETE", a),
			registerModalState.registers.splice(e, 1),
			await loadModbusConfig(),
			renderRegisterList(),
			showModalMessage("register_modal_message", "success", "Register deleted successfully!"),
			setTimeout(() => clearModalMessage("register_modal_message"), 2e3);
	} catch (e) {
		showModalMessage("register_modal_message", "error", `Error: ${e.message}`);
	}
}
function showMessage(e, t, a) {
	const s = document.getElementById(e);
	(s.className = `status_message status_${t}`), (s.innerHTML = a);
}
function clearMessage(e) {
	const t = document.getElementById(e);
	(t.innerHTML = ""), (t.className = "");
}
function showModalMessage(e, t, a) {
	const s = document.getElementById(e);
	(s.className = `modal_message ${t}`), (s.innerHTML = a);
}
function clearModalMessage(e) {
	const t = document.getElementById(e);
	(t.innerHTML = ""), (t.className = "");
}
function navigateToPage(e) {
	stopDashboardRefresh(),
		stopModbusRefresh(),
		document.querySelectorAll(".page").forEach((e) => {
			e.classList.remove("active");
		}),
		document.querySelectorAll(".nav_link").forEach((e) => {
			e.classList.remove("active");
		}),
		document.getElementById(e).classList.add("active"),
		document.querySelector(`[data-page="${e}"]`).classList.add("active"),
		(applicationState.currentPage = e),
		"dashboard" === e
			? startDashboardRefresh()
			: "interfaces" === e
			? loadInterfaces()
			: "modbus" === e
			? loadModbusConfig()
			: "map" === e && loadMapData();
}
async function loadMapData() {
	if (!applicationState.mapState.loading)
		try {
			(applicationState.mapState.loading = !0), showMessage("map_message", "Loading register map...", "info");
			const e = await apiCall("GET", "/api/map");
			(applicationState.mapState.data = e), renderMapData(e), updateMapStatus(e), clearMessage("map_message");
		} catch (e) {
			showMessage("map_message", `Failed to load map: ${e.message}`, "error");
		} finally {
			applicationState.mapState.loading = !1;
		}
}
function updateMapStatus(e) {
	const t = document.getElementById("map_status");
	e.initialized
		? ((t.textContent = `Initialized • ${e.total_groups} groups mapped`), t.classList.add("initialized"))
		: ((t.textContent = "Not initialized"), t.classList.remove("initialized"));
}
function renderMapData(e) {
	const t = document.getElementById("map_container");
	if (!e.groups || 0 === e.groups.length)
		return void (t.innerHTML = '<div class="map_empty">No groups configured. Add groups in the Modbus Config page.</div>');
	const a = e.groups
		.map((e) => {
			const t = e.registers
				.map((e) => {
					const t =
							"group" === e.type
								? '<span class="map_type_badge map_type_group">Group</span>'
								: `<span class="map_type_badge map_type_slave">Slave ${e.slave_id}</span>`,
						a = "slave" === e.type ? `<div class="map_device_info">${"Indoor " + e.slave_id}</div>` : "Outdoor Device";
					return `\n\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t<td class="map_address">${e.address}</td>\n\t\t\t\t\t\t\t<td>${e.id}</td>\n\t\t\t\t\t\t\t<td>${e.name}</td>\n\t\t\t\t\t\t\t<td>${t}</td>\n\t\t\t\t\t\t\t<td>${a}</td>\n\t\t\t\t\t\t</tr>\n\t\t\t\t\t`;
				})
				.join("");
			return `\n\t\t\t\t<div class="map_group">\n\t\t\t\t\t<div class="map_group_header">\n\t\t\t\t\t\t<div>\n\t\t\t\t\t\t\t<div class="map_group_title">${e.name}</div>\n\t\t\t\t\t\t</div>\n\t\t\t\t\t\t<div class="map_group_badge">${e.total_registers} registers</div>\n\t\t\t\t\t</div>\n\t\t\t\t\t<table class="map_table">\n\t\t\t\t\t\t<thead>\n\t\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t\t<th>Address</th>\n\t\t\t\t\t\t\t\t<th>Remote Address</th>\n\t\t\t\t\t\t\t\t<th>Name</th>\n\t\t\t\t\t\t\t\t<th>Type</th>\n\t\t\t\t\t\t\t\t<th>Device</th>\n\t\t\t\t\t\t\t</tr>\n\t\t\t\t\t\t</thead>\n\t\t\t\t\t\t<tbody>\n\t\t\t\t\t\t\t${t}\n\t\t\t\t\t\t</tbody>\n\t\t\t\t\t</table>\n\t\t\t\t</div>\n\t\t\t`;
		})
		.join("");
	t.innerHTML = a;
}
document.addEventListener("DOMContentLoaded", function () {
	document.querySelectorAll(".nav_link").forEach((e) => {
		e.addEventListener("click", function () {
			navigateToPage(this.getAttribute("data-page"));
		});
	}),
		document.getElementById("interfaces_form").addEventListener("submit", saveInterfaces),
		document.getElementById("add_group_btn").addEventListener("click", openAddGroupModal),
		document.getElementById("refresh_rate").addEventListener("change", function () {
			(applicationState.modbusState.refreshRate = parseInt(this.value)), "modbus" === applicationState.currentPage && startModbusRefresh();
		}),
		document.getElementById("add_group_modal").addEventListener("click", function (e) {
			e.target === this && closeAddGroupModal();
		}),
		document.getElementById("edit_group_modal").addEventListener("click", function (e) {
			e.target === this && closeEditGroupModal();
		}),
		document.getElementById("delete_group_modal").addEventListener("click", function (e) {
			e.target === this && closeDeleteGroupModal();
		}),
		navigateToPage("dashboard");
});
