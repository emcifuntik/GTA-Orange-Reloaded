const orange = process.binding('orange');
const Vectors = require("./Vectors.js");

class Player {
  constructor(playerId) {
    this.playerId = parseInt(playerId);
  }

  get position() {
    var pos = orange.getPlayerPosition(this.playerId);
    var playerId = this.playerId;
    var obj = {
      get x() {
        return pos[0];
      },
      get y() {
        return pos[1];
      },
      get z() {
        return pos[2];
      },

      set x(value) {
        pos[0] = value;
        orange.setPlayerPosition(playerId, pos[0], pos[1], pos[2]);
      },
      set y(value) {
        pos[1] = value;
        orange.setPlayerPosition(playerId, pos[0], pos[1], pos[2]);
      },
      set z(value) {
        pos[2] = value;
        orange.setPlayerPosition(playerId, pos[0], pos[1], pos[2]);
      }
    }
    return obj;
  }

  set position(pos) {
    orange.setPlayerPosition(this.playerId, pos.x, pos.y, pos.z);
  }
}

module.exports = Player;