global.orange = process.binding('orange');
const {NodeVM} = require('vm2');
var fs = require("fs");
var path = require("path");

var Player;
try
{
  Player = require("./orangeAPI/Player.js");
}
catch(ex)
{
  console.log(ex);
}

const vm = new NodeVM({
  console: 'redirect',
  sandbox: {
    Player
  },
  wrapper: "none",
  nesting: true, //not sure about this
  require: {
    external: false,
    builtin: [],
    root: "./",
  }
});

vm.on("console.log", (...args) => {
  orange.print(args.map(function(object) {
    if(Array.isArray(object)) {
      return JSON.stringify(object, null, 2);
    }
    else if(object instanceof Object) {
      return object.toString() + " " + JSON.stringify(object, null, 2);
    }
    else {
      return object.toString();
    }
  }).join(" "));
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
  orangeAPI.orange.onEvent(event, args);
});

orange.onPlayerCommand(function(playerId, command) {
  orangeAPI.onPlayerCommand(playerId, command);

});
