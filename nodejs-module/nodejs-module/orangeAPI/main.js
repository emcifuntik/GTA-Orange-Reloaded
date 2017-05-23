'use strict';

const orange = process.binding('orange');
const {EventEmitter} = require('events');
const Player = require("./Player.js");

class orangeAPI extends EventEmitter {
  constructor() {
    super();
    this.players = {};
  }

  print(message) {
    orange.print(message);
  }

  onEvent(name, args) {
    switch(name) {
      case "PlayerConnect":
        var playerId = parseInt(args[0]);
        this.players[playerId] = new Player(playerId);
        this.emit("playerConnect", this.players[playerId], args[1]);
        break;
    }
  }

  onPlayerCommand(playerId, command) {
    this.emit("playerCommand", this.players[playerId], command);
  }
}

module.exports = orangeAPI;