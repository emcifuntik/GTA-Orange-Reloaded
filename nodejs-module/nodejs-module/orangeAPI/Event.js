class Event {
  static on(event, callback) {
    if(this._events == null) {
      this._events = {};
    }
    if(this._events[event] == null) {
      this._events[event] = [];
    }
    let eventIndex = this._events[event].length;
    this._events[event].push(callback);
    let self = this;
    return {
      cancel: function() {
        delete this._events[event][eventIndex];
      }
    };
  }

  static trigger(event, ...args) {
    if(this._events == null) {
      this._events = {};
    }
    if(this._events[event] == null) {
      return;
    }
    for(let i in this._events[event]) {
      this._events[event][i].apply(this, args);
    }
  }

  on(event, callback) {
    if(this._events == null) {
      this._events = {};
    }
    if(this._events[event] == null) {
      this._events[event] = [];
    }
    let eventIndex = this._events[event].length;
    this._events[event].push(callback);
    let self = this;
    return {
      cancel: function() {
        delete this._events[event][eventIndex];
      }
    };
  }

  trigger(event, ...args) {
    if(this._events == null) {
      this._events = {};
    }
    if(this._events[event] == null) {
      return;
    }
    for(let i in this._events[event]) {
      this._events[event][i].apply(this, args);
    }
  }
}

module.exports = Event;