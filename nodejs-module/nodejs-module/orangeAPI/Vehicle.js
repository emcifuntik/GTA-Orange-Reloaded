const Event = require("./Event.js");
const Vectors = require("./Vectors.js");

const _vehiclesPool = [];

class Vehicle extends Event {
  constructor(model, x, y, z, heading) {
    super();
    this._vehicleId = orange.createVehicle(model, x, y, z, heading);
    this._model = model;
    _vehiclesPool[this._vehicleId] = this;
  }

  get id() {
    return this._vehicleId;
  }

  get model() {
    return model;
  }

  get position() {
    let pos = orange.getVehiclePosition(this.id);
    let vehicleId = this.id;
    let vector = new Vectors.Vector3(pos.x, pos.y, pos.z);
    vector.afterChangeCallback = function(changedVector) {
      orange.setVehiclePosition(vehicleId, changedVector.x, changedVector.y, changedVector.z);
    };
    return vector;
  }

  set position(pos) {
    orange.setVehiclePosition(this.id, pos.x, pos.y, pos.z);
  }

  get rotation() {
    let pos = orange.getVehicleRotation(this.id);
    let vehicleId = this.id;
    let vector = new Vectors.Vector3(pos.x, pos.y, pos.z);
    vector.afterChangeCallback = function(changedVector) {
      orange.setVehicleRotation(vehicleId, changedVector.x, changedVector.y, changedVector.z);
    };
    return vector;
  }

  set rotation(rot) {
    orange.setVehicleRotation(this.id, rot.x, rot.y, rot.z);
  }

  get primaryColor() {
    return orange.getVehicleColours(this.id).primaryColor;
  }

  set primaryColor(color) {
    let colors = orange.getVehicleColours(this.id);
    orange.setVehicleColours(this.id, color, colors.secondary);
  }

  get secondaryColor() {
    return orange.getVehicleColours(this.id).secondaryColor;
  }

  set secondaryColor(color) {
    let colors = orange.getVehicleColours(this.id);
    orange.setVehicleColours(this.id, colors.primary, color);
  }

  get isTyresBulletproof() {
    return orange.getVehicleTyresBulletproof(this.id);
  }

  set isTyresBulletproof(state) {
    orange.setVehicleTyresBulletproof(this.id, state);
  }

  get isEngineRunning() {
    return orange.getVehicleEngineStatus(this.id);
  }

  set isEngineRunning(state) {
    return orange.setVehicleEngineStatus(this.id, state);
  }

  get isLocked() {
    return orange.isVehicleLocked(this.id);
  }

  set isLocked(state) {
    return orange.setVehicleLocked(this.id, state);
  }

  get health() {
    let healths = orange.getVehicleHealth(this.id);
    let vehicleId = this.id;
    let obj = {
      get body() {
        return healths.body;
      },
      get engine() {
        return healths.engine;
      },
      get tank() {
        return healths.tank;
      },
      set body(value) {
        orange.setVehicleBodyHealth(vehicleId, value);
      },
      set engine(value) {
        orange.setVehicleEngineHealth(vehicleId, value);
      },
      set tank(value) {
        orange.setVehicleTankHealth(vehicleId, value);
      }
    };
    return obj;
  }

  get isSirenTurnedOn() {
    return orange.getVehicleSirenState(this.id);
  }

  set isSirenTurnedOn(state) {
    return orange.setVehicleSirenState(this.id, state);
  }

  get driver() {
    //TODO, ability to kick?
    return orange.getVehicleDriver(this.id);
  }

  get passangers() {
    return orange.getVehiclePassengers(this.id);
  }
}

module.exports = Vehicle;