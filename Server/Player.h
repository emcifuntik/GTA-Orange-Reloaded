#pragma once

namespace Player
{
	PyObject *setPos(PyObject * self, PyObject* args);
	PyObject *getPos(PyObject * self, PyObject* args);
	PyObject *isInRange(PyObject * self, PyObject* args);
	PyObject *setHeading(PyObject * self, PyObject* args);
	PyObject *giveWeapon(PyObject * self, PyObject* args);
	PyObject *giveAmmo(PyObject * self, PyObject* args);
	PyObject *giveMoney(PyObject * self, PyObject* args);
	PyObject *setMoney(PyObject * self, PyObject* args);
	PyObject *resetMoney(PyObject * self, PyObject* args);
	PyObject *getMoney(PyObject * self, PyObject* args);
	PyObject *setModel(PyObject * self, PyObject* args);
	PyObject *getModel(PyObject * self, PyObject* args);
	PyObject *setName(PyObject * self, PyObject* args);
	PyObject *getName(PyObject * self, PyObject* args);
	PyObject *setHealth(PyObject * self, PyObject* args);
	PyObject *getHealth(PyObject * self, PyObject* args);
	PyObject *setArmour(PyObject * self, PyObject* args);
	PyObject *getArmour(PyObject * self, PyObject* args);
	PyObject *setColor(PyObject * self, PyObject* args);
	PyObject *getColor(PyObject * self, PyObject* args);
	PyObject *getIP(PyObject * self, PyObject* args);
	PyObject *sendClientMessage(PyObject * self, PyObject* args);
	PyObject *broadcastClientMessage(PyObject * self, PyObject* args);

	static PyMethodDef Methods[] = {
		{ "setPos", setPos, METH_VARARGS, "Sets player\'s position" },
		{ "getPos", getPos, METH_VARARGS, "Returns player position" },
		{ "isInRangeOfPoint", isInRange, METH_VARARGS, "Returns bool if player in range of point" },
		{ "setHeading", setHeading, METH_VARARGS, "Sets player heading angle" },
		{ "giveWeapon", giveWeapon, METH_VARARGS, "Give weapon to player" },
		{ "giveAmmo", giveAmmo, METH_VARARGS, "Give ammo to player" },
		{ "giveMoney", giveMoney, METH_VARARGS, "Give money to player" },
		{ "setMoney", setMoney, METH_VARARGS, "Sets player\'s money" },
		{ "resetMoney", resetMoney, METH_VARARGS, "Resets player\'s money" },
		{ "getMoney", getMoney, METH_VARARGS, "Returns player money" },
		{ "setModel", setModel, METH_VARARGS, "Sets player\'s model" },
		{ "getModel", getModel, METH_VARARGS, "Return player model" },
		{ "setName", setName, METH_VARARGS, "Sets player\'s name" },
		{ "getName", getName, METH_VARARGS, "Return player\'s name" },
		{ "setHealth", setHealth, METH_VARARGS, "Sets player\' health" },
		{ "getHealth", getHealth, METH_VARARGS, "Return player health" },
		{ "setArmour", setArmour, METH_VARARGS, "Sets player\'s armour" },
		{ "getArmour", getArmour, METH_VARARGS, "Return player armour" },
		{ "setColor", setColor, METH_VARARGS, "Sets player\'s colour" },
		{ "getColor", getColor, METH_VARARGS, "Return player colour" },
		{ "getIP", getIP, METH_VARARGS, "Return player\'s IP address" },
		{ "sendMessage", sendClientMessage, METH_VARARGS, "Sends message to client" },
		{ "broadcastMessage", broadcastClientMessage, METH_VARARGS, "Sends message to all" },
		{ NULL, NULL, 0, NULL }
	};
};
