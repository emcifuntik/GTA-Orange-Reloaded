const Event = require("./Event.js");

class Player extends Event {
  constructor(playerId) {
    super();
    this.playerId = parseInt(playerId);
  }

  getID() {
    return this.playerId;
  }

  setPosition(x, y, z) {
    orange.setPlayerPosition(this.playerId, x, y, z);
  }
}

module.exports = Player;