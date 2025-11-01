#ifndef MODBUS_CONTROLLER_H
#define MODBUS_CONTROLLER_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "../services/ModbusService.h"
#include "../services/RegisterMappingService.h"

/**
 * ModbusController handles /api/modbus/* endpoints
 */
class ModbusController {
public:
    /**
     * Register routes
     */
    static void registerRoutes(AsyncWebServer& server) {
        // GET /api/modbus/groups - Get all groups
        server.on("/api/modbus/groups", HTTP_GET, [](AsyncWebServerRequest *request) {
            handleGetGroups(request);
        });
        
        // GET /api/modbus/group - Get specific group by ID
        server.on("/api/modbus/group", HTTP_GET, [](AsyncWebServerRequest *request) {
            handleGetGroup(request);
        });
        
        // POST /api/modbus/group/create - Create new group
        server.on("/api/modbus/group/create", HTTP_POST, [](AsyncWebServerRequest *request) {
            handleCreateGroup(request);
        });
        
        // PATCH /api/modbus/group/update - Update group
        server.on("/api/modbus/group/update", HTTP_PATCH, [](AsyncWebServerRequest *request) {
            handleUpdateGroup(request);
            RegisterMappingService::buildMapping();
        });
        
        // DELETE /api/modbus/group/delete - Delete group
        server.on("/api/modbus/group/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
            handleDeleteGroup(request);
            RegisterMappingService::buildMapping();
        });
        
        // POST /api/modbus/group/update/register - Add register
        server.on("/api/modbus/group/update/register", HTTP_POST, [](AsyncWebServerRequest *request) {
            handlePostRegister(request);
        }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            handlePostRegisterBody(request, data, len, index, total);
            RegisterMappingService::buildMapping();
        });
        
        // PATCH /api/modbus/group/update/register - Update register
        server.on("/api/modbus/group/update/register", HTTP_PATCH, [](AsyncWebServerRequest *request) {
            handlePatchRegister(request);
        }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            handlePatchRegisterBody(request, data, len, index, total);
            RegisterMappingService::buildMapping();
        });
        
        // DELETE /api/modbus/group/update/register - Delete register
        server.on("/api/modbus/group/update/register", HTTP_DELETE, [](AsyncWebServerRequest *request) {
            handleDeleteRegister(request);
            RegisterMappingService::buildMapping();
        });
    }
    
private:
    static String postData;
    
    /**
     * GET /api/modbus/groups
     * Returns all groups without register values
     */
    static void handleGetGroups(AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        
        JsonArray root = response->getRoot().to<JsonArray>();
        
        const auto& groups = ModbusService::getGroups();
        for (const auto& group : groups) {
            auto groupObj = root.createNestedObject();
            group.toJson(groupObj, false);  // Without values
        }
        
        response->setLength();
        request->send(response);
    }
    
    /**
     * GET /api/modbus/group?id={group-id}
     * Returns specific group with current register values
     */
    static void handleGetGroup(AsyncWebServerRequest *request) {
        if (!request->hasParam("id")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID is required";
            response->setLength();
            request->send(response);
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        auto* group = ModbusService::getGroup(groupId);
        
        if (!group) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(404);
            response->getRoot()["error"] = "Group not found";
            response->setLength();
            request->send(response);
            return;
        }
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject obj = response->getRoot().as<JsonObject>();
        group->toJson(obj, true);  // With values
        response->setLength();
        request->send(response);
    }
    
    /**
     * POST /api/modbus/group/create?id={group-id}&slave={slave-count}
     * Creates a new group
     */
    static void handleCreateGroup(AsyncWebServerRequest *request) {
        if (!request->hasParam("id")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID is required";
            response->setLength();
            request->send(response);
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        uint8_t slaveCount = 0;
        
        if (request->hasParam("slave")) {
            slaveCount = request->getParam("slave")->value().toInt();
        }
        
        if (!ModbusService::createGroup(groupId, slaveCount)) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Failed to create group (may already exist)";
            response->setLength();
            request->send(response);
            return;
        }
        
        auto* group = ModbusService::getGroup(groupId);
        AsyncJsonResponse* response = new AsyncJsonResponse();
        response->setCode(201);
        JsonObject obj = response->getRoot().as<JsonObject>();
        obj["message"] = "Group created successfully";
        auto dataObj = obj.createNestedObject("data");
        group->toJson(dataObj, true);
        response->setLength();
        request->send(response);
    }
    
    /**
     * PATCH /api/modbus/group/update?id={group-id}&slave={new-slave-count}
     * Updates number of slaves in a group
     */
    static void handleUpdateGroup(AsyncWebServerRequest *request) {
        if (!request->hasParam("id") || !request->hasParam("slave")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID and slave count are required";
            response->setLength();
            request->send(response);
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        uint8_t slaveCount = request->getParam("slave")->value().toInt();
        
        if (!ModbusService::updateGroup(groupId, slaveCount)) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(404);
            response->getRoot()["error"] = "Group not found";
            response->setLength();
            request->send(response);
            return;
        }
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        response->getRoot()["message"] = "Group updated successfully";
        response->setLength();
        request->send(response);
    }
    
    /**
     * DELETE /api/modbus/group/delete?id={group-id}
     * Deletes a group
     */
    static void handleDeleteGroup(AsyncWebServerRequest *request) {
        if (!request->hasParam("id")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID is required";
            response->setLength();
            request->send(response);
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        
        if (!ModbusService::deleteGroup(groupId)) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(404);
            response->getRoot()["error"] = "Group not found";
            response->setLength();
            request->send(response);
            return;
        }
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        response->getRoot()["message"] = "Group deleted successfully";
        response->setLength();
        request->send(response);
    }
    
    /**
     * POST body handler
     */
    static void handlePostRegisterBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            postData = "";
        }
        for (size_t i = 0; i < len; i++) {
            postData += (char)data[i];
        }
    }
    
    /**
     * POST /api/modbus/group/update/register?id={group-id}[&slave={slave-id}]
     * Adds a new register to group or slave
     */
    static void handlePostRegister(AsyncWebServerRequest *request) {
        if (!request->hasParam("id")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID is required";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, postData);
        
        if (error) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Invalid JSON";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        JsonObject docObj = doc.as<JsonObject>();
        if (!docObj.containsKey("id") || !docObj.containsKey("name")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Register ID and name are required";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        Register reg(docObj["id"], docObj["name"].as<String>());
        
        bool success = false;
        if (request->hasParam("slave")) {
            uint8_t slaveId = request->getParam("slave")->value().toInt();
            success = ModbusService::addSlaveRegister(groupId, slaveId, reg);
        } else {
            success = ModbusService::addGroupRegister(groupId, reg);
        }
        
        if (!success) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Failed to add register (duplicate ID or invalid group/slave)";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        response->setCode(201);
        JsonObject obj = response->getRoot().as<JsonObject>();
        obj["message"] = "Register added successfully";
        auto regObj = obj.createNestedObject("register");
        reg.toJson(regObj);
        response->setLength();
        request->send(response);
        postData = "";
    }
    
    /**
     * PATCH body handler
     */
    static void handlePatchRegisterBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            postData = "";
        }
        for (size_t i = 0; i < len; i++) {
            postData += (char)data[i];
        }
    }
    
    /**
     * PATCH /api/modbus/group/update/register?id={group-id}[&slave={slave-id}]
     * Updates an existing register
     */
    static void handlePatchRegister(AsyncWebServerRequest *request) {
        if (!request->hasParam("id")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID is required";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, postData);
        
        if (error) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Invalid JSON";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        JsonObject docObj = doc.as<JsonObject>();
        if (!docObj.containsKey("id") || !docObj.containsKey("name")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Register ID and name are required";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        uint16_t regId = docObj["id"];
        uint16_t newRegId = docObj.containsKey("newId") ? (uint16_t)docObj["newId"] : regId;
        String newName = docObj["name"].as<String>();
        
        bool success = false;
        if (request->hasParam("slave")) {
            uint8_t slaveId = request->getParam("slave")->value().toInt();
            success = ModbusService::updateSlaveRegister(groupId, slaveId, regId, newName, newRegId);
        } else {
            success = ModbusService::updateGroupRegister(groupId, regId, newName, newRegId);
        }
        
        if (!success) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Failed to update register (not found or duplicate ID)";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject obj = response->getRoot().as<JsonObject>();
        obj["message"] = "Register updated successfully";
        auto regObj = obj.createNestedObject("register");
        regObj["id"] = newRegId;
        regObj["name"] = newName;
        response->setLength();
        request->send(response);
        postData = "";
    }
    
    /**
     * DELETE /api/modbus/group/update/register?id={group-id}[&slave={slave-id}]&registerId={register-id}
     * Deletes a register
     */
    static void handleDeleteRegister(AsyncWebServerRequest *request) {
        if (!request->hasParam("id") || !request->hasParam("registerId")) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            response->getRoot()["error"] = "Group ID and Register ID are required";
            response->setLength();
            request->send(response);
            return;
        }
        
        uint8_t groupId = request->getParam("id")->value().toInt();
        uint16_t regId = request->getParam("registerId")->value().toInt();
        
        bool success = false;
        if (request->hasParam("slave")) {
            uint8_t slaveId = request->getParam("slave")->value().toInt();
            success = ModbusService::deleteSlaveRegister(groupId, slaveId, regId);
        } else {
            success = ModbusService::deleteGroupRegister(groupId, regId);
        }
        
        if (!success) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(404);
            response->getRoot()["error"] = "Register not found";
            response->setLength();
            request->send(response);
            return;
        }
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        response->getRoot()["message"] = "Register deleted successfully";
        response->setLength();
        request->send(response);
    }
};

// Static member initialization
String ModbusController::postData;

#endif // MODBUS_CONTROLLER_H
