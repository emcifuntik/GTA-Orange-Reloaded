global.orange = process.binding('orange');
const {NodeVM} = require('vm2');
const fs = require("fs");
const path = require("path");
const EventEmitter = require('events');

orange.eventHandler = new EventEmitter();

const Player = require("./orangeAPI/Player.js");
const Vehicle = require("./orangeAPI/Vehicle.js");
const Vectors = require("./orangeAPI/Vectors.js");

const vm = new NodeVM({
  console: 'redirect',
  sandbox: {
    Player: Player,
    Vehicle: Vehicle,
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
  let scriptPath = path.resolve(`./resources/${resource}/main.js`);
  let script = fs.readFileSync(scriptPath);
  try {
    vm.run(script, scriptPath);
  }
  catch(ex) {
    orange.print("Resource " + resource + " load failed! Reason:\n" + ex.stack);
    return;
  }
  orange.print("Resource " + resource + " has been loaded!");
});

orange.onEvent(function(event, args) {
  console.log("Event: ", event, args);
  try {
    orange.eventHandler.emit.apply(orange.eventHandler, [ event, ...args ]);
  }
  catch(ex) {
    //this exceptions is required because default stacktrace shows only some weird informations inside vm2's contextify
    console.log(ex.stack);
  }
});

orange.onPlayerCommand(function(playerId, command) {
  try {
    orange.eventHandler.emit("PlayerCommand", playerId, command);    
  }
  catch(ex) {
    console.log(ex.stack);
  }
});