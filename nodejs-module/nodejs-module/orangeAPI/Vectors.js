//Maybe after every getter fetch new value

class Vector3 {
  constructor(x, y, z) {
    this.prepared = false;
    this._x = x;
    this._y = y;
    this._z = z;
    this.afterChangeCallback = null;
    this.prepared = true;
  }

  get x() {
    return this._x;
  }

  get y() {
    return this._y;
  }

  get z() {
    return this._z;
  }

  set x(value) {
    if(this.prepared == false) return;
    this._x = value;
    if(this.afterChangeCallback !== null) {
      this.afterChangeCallback(this);
    }
  }

  set y(value) {
    if(this.prepared == false) return;
    this._y = value;
    if(this.afterChangeCallback !== null) {
      this.afterChangeCallback(this);
    }
  }

  set z(value) {
    if(this.prepared == false) return;
    this._z = value;
    if(this.afterChangeCallback !== null) {
      this.afterChangeCallback(this);
    }
  }
}

module.exports = {
  Vector3
}