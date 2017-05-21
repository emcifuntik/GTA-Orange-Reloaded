var orange = process.binding('orange');
const {NodeVM} = require('vm2');
var fs = require("fs");
var path = require("path");
var orangeAPI;
var Vectors = require("./orangeAPI/Vectors.js");
try
{
  orangeAPI = require("./orangeAPI/main.js");
  orangeAPI = new orangeAPI();
}
catch(ex)
{
  console.log(ex);
}

const vm = new NodeVM({
  console: 'redirect',
  sandbox: {
    orange: orangeAPI,
    Vector3: Vectors.Vector3
  },
  wrapper: "none",
  nesting: true, //not sure about this
  require: {
    external: false,
    builtin: [],
    root: "./",
  }
});

orange.onResourceLoad(function(resource) {
  var scriptPath = path.resolve(`./resources/${resource}/main.js`);
  var script = fs.readFileSync(scriptPath);
  try {
    vm.run(script, scriptPath);    
  }
  catch(ex) {
    orange.print("NodeJS: Resource " + resource + " load failed! Reason:\n" + ex.stack);
    return;
  }
  orange.print("NodeJS: Loaded resource: " + resource);
});

orange.onEvent(function(event, args) {
  orangeAPI.onEvent(event, args);
  /*if(event == "PlayerConnect") {
    orange.setPlayerPosition(args[0], -1018.46, -2870.77, 13.9516);
    /*var playerId = args[0];
    setInterval(function() {
      var pos = orange.getPlayerPosition(args[0]);
      console.log("Pos: ", pos[0], pos[1], pos[2]);
    }, 3000);
  }*/
});

orange.onPlayerCommand(function(playerId, command) {
  console.log("PlayerID: ", playerId, command);
  orangeAPI.onPlayerCommand(playerId, command);
  /*command = command.split(" ");
  switch(command[0]) {
    case "/isPlayerInRange":
      var ret = orange.isPlayerInRange(playerId, -1018.46, -2870.77, 13.9516, 2);
      orange.sendClientMessage(playerId, "done! Res: " + (ret ? "true" : "false"), 0xFF00FF);
      break;
    case "/heading":
      orange.sendClientMessage(playerId, "done! Heading: " + orange.getPlayerHeading(playerId), 0xFF00FF);
      orange.setPlayerHeading(playerId, parseFloat(command[1]));
      break;
    case "/removeWeapons":
      orange.removePlayerWeapons(playerId);
      break;
    case "/weapons":
      if(isNaN(command[1])) {
        orange.givePlayerWeapon(playerId, command[1], parseInt(command[2]));
        orange.givePlayerAmmo(playerId, command[1], 10);
      }
      else {
        orange.givePlayerWeapon(playerId, parseInt(command[1]), parseInt(command[2]));
        orange.givePlayerAmmo(playerId, parseInt(command[1]), 10);
      }
      break;
    case "/money":
      orange.setPlayerMoney(playerId, parseInt(command[1]));
      orange.givePlayerMoney(playerId, 100);
      orange.sendClientMessage(playerId, "done! Money: " + orange.getPlayerMoney(playerId), 0xFF00FF);
      break;
    case "/model":
      orange.sendClientMessage(playerId, "done! Skin: " + orange.getPlayerModel(playerId), 0xFF00FF);
      orange.setPlayerModel(playerId, parseInt(command[1]));
      break;
    case "/name":
      console.log("Name: " + command[1]);
      orange.setPlayerName(playerId, command[1]);
      orange.sendClientMessage(playerId, "done! Name: " + orange.getPlayerName(playerId), 0xFF00FF);
      break;
    case "/health2":
      orange.setPlayerHealth(playerId, 50);
      break;
    case "/health":
      orange.sendClientMessage(playerId, "done! Health: " + orange.getPlayerHealth(playerId), 0xFF00FF);
      console.log("Health: ", parseFloat(command[1]));
      orange.setPlayerHealth(playerId, parseFloat(command[1]));
      break;
    case "/armor":
      orange.sendClientMessage(playerId, "done! Health: " + orange.getPlayerArmour(playerId), 0xFF00FF);
      orange.setPlayerArmour(playerId, parseInt(command[1]));
      break;
    case "/color":
      orange.sendClientMessage(playerId, "done! Color: " + orange.getPlayerColor(playerId), 0xFF00FF);
      orange.getPlayerColor(playerId, parseInt(command[1]));
      break;
    case "/hud":
      orange.disablePlayerHud(playerId, parseInt(command[1]));
      break;
    case "/guid":
      orange.sendClientMessage(playerId, "done! GUID: " + orange.getPlayerGUID(playerId), 0xFF00FF);
      break;
    case "/veh":
      var pos = orange.getPlayerPosition(playerId);
      orange.createVehicle(parseInt(command[1]), -1018.46, -2870.77, 13.9516, 0.0);
      break;
  }*/
});
