const Event = require("./Event.js");
const Vectors = require("./Vectors.js");

const _playersPool = [];

class Player extends Event {
  constructor(playerId, ip) {
    super();
    this._playerId = parseInt(playerId);
    this._ip = ip.replace("|", ":");
    this._HUDvisible = true;
  }

  get id() {
    return this._playerId;
  }

  get ip() {
    return this._ip;
  }

  kick() {
    orange.kickPlayer(this.id);
  }

  get position() {
    let pos = orange.getPlayerPosition(this.id);
    let playerId = this.id;
    let vector = new Vectors.Vector3(pos.x, pos.y, pos.z);
    vector.afterChangeCallback = function(changedVector) {
      orange.setPlayerPosition(playerId, changedVector.x, changedVector.y, changedVector.z);
    };
    return vector;
  }

  set position(pos) {
    orange.setPlayerPosition(this.id, pos.x, pos.y, pos.z);
  }

  //isInRange

  get heading() {
    return orange.getPlayerHeading(this.id);
  }

  set heading(value) {
    orange.setPlayerHeading(this.id, value);
  }

  giveWeapon(weapon, ammo) {
    orange.givePlayerWeapon(this.id, weapon, ammo);
  }

  giveAmmo(weapon, ammo) {
    orange.givePlayerAmmo(this.id, weapon, ammo);
  }

  //TODO: Set Ammo, Check if player have weapon, remove weapon

  get money() {
    return orange.getPlayerMoney(this.id);
  }

  set money(money) {
    orange.setPlayerMoney(this.id, money);
  }

  get model() {
    return orange.getPlayerModel(this.id);
  }

  set model(hash) {
    orange.setPlayerModel(this.id, hash);
  }

  get name() {
    return orange.getPlayerName(this.id);
  }

  set name(name) {
    orange.setPlayerName(this.id, name);
  }

  get health() {
    return orange.getPlayerHealth(this.id);
  }

  set health(value) {
    orange.setPlayerHealth(this.id, value);
  }

  get armour() {
    return orange.getPlayerArmour(this.id);
  }

  set armour(value) {
    orange.setPlayerArmour(this.id);
  }

  get color() {
    return orange.getPlayerColor(this.id);
  }

  set color(value) {
    orange.setPlayerColor(this.id, value);
  }

  sendMessage(message, color) {
    orange.sendClientMessage(this.id, message, color);
  }

  putIntoVehicle(vehicle) {
    orange.setPlayerIntoVehicle(this.id, vehicle.id);
  }

  get HUDvisible() {
    return this._HUDvisible;
  }

  set HUDvisible(value) {
    this._HUDvisible = value;
    orange.disablePlayerHud(!this._HUDvisible);
  }

  static broadcastMessage(message, color) {
    orange.broadcastClientMessage(message, color);
  }

  static getByID(playerId) {
    return _playersPool[playerId];
  }
}

function _onPlayerConnect(playerId, ip) {
  let player = new Player(playerId, ip);
  Player.trigger("connect", player, ip);
  player.trigger("connect", ip);
  _playersPool[playerId] = player;
};

function _onPlayerCommand(playerId, command) {
  let player = Player.getByID(playerId);
  Player.trigger("command", player, command);
  player.trigger("command", command);
}

function _onPlayerDisconnect(playerId, reason) {
  let player = Player.getByID(playerId);
  Player.trigger("disconnect", player, reason);
  player.trigger("disconnect", reason);
  delete _playersPool[playerId];
}

function _onPlayerDeath(playerId, killerId, weapon) {
  let player = Player.getByID(playerId);
  let killer = Player.getByID(killerId);

  Player.trigger("death", player, killer, weapon);
  player.trigger("death", killer, weapon);
}

function _onPlayerSpawn(playerId, x, y, z) {
  let player = Player.getByID(playerId);

  Player.trigger("spawn", player, x, y, z);
  player.trigger("spawn", x, y, z);
}

function _onPlayerPressKey(playerId, keyId) {
  let player = Player.getByID(playerId);

  Player.trigger("pressKey", player, keyId);
  player.trigger("pressKey", keyId);
}

function _onPlayerClientEvent(playerId, eventName, ...args) {
  let player = Player.getByID(playerId);
  Player.trigger.apply(Player, [ player, eventName, ...args ]);
  player.trigger.apply(player, [ eventName, ...args ]);
}

function _onPlayerEnterVehicle(playerId, vehicleId) {
  let player = Player.getByID(playerId);
  let vehicle = Vehicle.getByID(vehicleId);

  Player.trigger("enterVehicle", player, vehicle);
  player.trigger("enterVehicle", vehicle);
}

function _onPlayerLeftVehicle(playerId, vehicleId) {
  let player = Player.getByID(playerId);
  let vehicle = Vehicle.getByID(vehicleId);

  Player.trigger("leftVehicle", player, vehicle);
  player.trigger("leftVehicle", vehicle);
}


orange.eventHandler.on("PlayerConnect", _onPlayerConnect);
orange.eventHandler.on("PlayerDisconnect", _onPlayerDisconnect);
orange.eventHandler.on("PlayerDeath", _onPlayerDeath);
orange.eventHandler.on("PlayerSpawn", _onPlayerSpawn);
orange.eventHandler.on("pressKey", _onPlayerPressKey);
orange.eventHandler.on("PlayerClientEvent", _onPlayerClientEvent);
orange.eventHandler.on("PlayerCommand", _onPlayerCommand);
orange.eventHandler.on("EnterVehicle", _onPlayerEnterVehicle);
orange.eventHandler.on("LeftVehicle", _onPlayerLeftVehicle);

module.exports = Player;