#include "stdafx.h"

namespace Player
{
	PyObject *setPos(PyObject * self, PyObject* args)
	{
		long playerID;
		float x, y, z;
		if (PyTuple_Check(args) && PyTuple_Size(args) == 4)
		{
			playerID = PyLong_AsLong(PyTuple_GetItem(args, 0));
			x = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 1));
			y = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 2));
			z = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 3));
		}
		else
		{
			log << "setPos args count not equal 4" << std::endl;
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetPosition(CVector3(x, y, z));
		return PyBool_FromLong(1L);
	}
	PyObject * getPos(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		CVector3 vecPos;
		player->GetPosition(vecPos);
		return Py_BuildValue("{s:f,s:f,s:f}", "x", vecPos.fX, "y", vecPos.fY, "z", vecPos.fZ);
	}
	PyObject * isInRange(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		float x, y, z, range;
		if (!PyArg_ParseTuple(args, "Iffff", &playerID, &x, &y, &z, &range))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		CVector3 vecPos, vecNextPos(x, y, z);
		player->GetPosition(vecPos);
		if((vecNextPos - vecPos).Length() <= range)
			return PyBool_FromLong(1L);
		return PyBool_FromLong(0L);
	}
	PyObject * setHeading(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		float a;
		if (!PyArg_ParseTuple(args, "If", &playerID, &a))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetHeading(a);
		return PyBool_FromLong(1L);
	}
	PyObject * giveWeapon(PyObject * self, PyObject * args)
	{
		unsigned int playerID, weapon, ammo;
		
		if (!PyArg_ParseTuple(args, "III", &playerID, &weapon, &ammo))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->GiveWeapon(weapon, ammo);
		return PyBool_FromLong(1L);
	}
	PyObject * giveAmmo(PyObject * self, PyObject * args)
	{
		unsigned int playerID, weapon, ammo;

		if (!PyArg_ParseTuple(args, "III", &playerID, &weapon, &ammo))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->GiveAmmo(weapon, ammo);
		return PyBool_FromLong(1L);
	}
	PyObject * giveMoney(PyObject * self, PyObject * args)
	{
		unsigned int playerID, money;

		if (!PyArg_ParseTuple(args, "Ii", &playerID, &money))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->GiveMoney(money);
		return PyBool_FromLong(1L);
	}
	PyObject * setMoney(PyObject * self, PyObject * args)
	{
		unsigned int playerID, money;

		if (!PyArg_ParseTuple(args, "Ii", &playerID, &money))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetMoney(money);
		return PyBool_FromLong(1L);
	}
	PyObject * resetMoney(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetMoney(0);
		return PyBool_FromLong(1L);
	}
	PyObject * getMoney(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyInt_FromLong(0L);
		size_t money = player->GetMoney();
		return PyInt_FromSize_t(money);
	}
	PyObject * setModel(PyObject * self, PyObject * args)
	{
		unsigned int playerID, model;

		if (!PyArg_ParseTuple(args, "II", &playerID, &model))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetModel(model);
		return PyBool_FromLong(1L);
	}
	PyObject * getModel(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		return PyBool_FromLong(player->GetModel());
	}
	PyObject * setName(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		char name[32];

		if (!PyArg_ParseTuple(args, "Is", &playerID, name))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetName(name);
		return PyBool_FromLong(1L);
	}
	PyObject * getName(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		return PyString_FromString(player->GetName().c_str());
	}
	PyObject * setHealth(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		float health;

		if (!PyArg_ParseTuple(args, "If", &playerID, &health))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetHealth(health);
		return PyBool_FromLong(1L);
	}
	PyObject * getHealth(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyFloat_FromDouble(0.0);
		return PyFloat_FromDouble(player->GetHealth());
	}
	PyObject * setArmour(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		float armour;

		if (!PyArg_ParseTuple(args, "If", &playerID, &armour))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetArmour(armour);
		return PyBool_FromLong(1L);
	}
	PyObject * getArmour(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyFloat_FromDouble(0.0);
		return PyFloat_FromDouble(player->GetArmour());
	}
	PyObject * setColor(PyObject * self, PyObject * args)
	{
		unsigned int playerID, color;

		if (!PyArg_ParseTuple(args, "II", &playerID, &color))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
		player->SetColor(color);
		return PyBool_FromLong(1L);
	}
	PyObject * getColor(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyLong_FromLong(0L);
		color_t playerColor = player->GetColor();
		return PyLong_FromLong(((playerColor.red & 0xff) << 24) + ((playerColor.green & 0xff) << 16) + ((playerColor.blue & 0xff) << 8) + (playerColor.alpha & 0xff));
	}
	PyObject * getIP(PyObject * self, PyObject * args)
	{
		unsigned int playerID;

		if (!PyArg_ParseTuple(args, "I", &playerID))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyString_FromString("0.0.0.0");
		//color_t playerColor = player->GetIP();
		return PyString_FromString("0.0.0.0");
	}
	PyObject * sendClientMessage(PyObject * self, PyObject * args)
	{
		unsigned int playerID;
		char* message;
		unsigned int color;

		if (!PyArg_ParseTuple(args, "IsI", &playerID, &message, &color))
		{
			PyErr_Print();
			return NULL;
		}
		auto player = CNetworkPlayer::GetByID(playerID);
		if (!player)
			return PyBool_FromLong(0L);
//		player->SendTextMessage(message, color);
		return PyBool_FromLong(1L);
	}
	PyObject * broadcastClientMessage(PyObject * self, PyObject * args)
	{
		char* message;
		unsigned int color;

		if (!PyArg_ParseTuple(args, "sI", &message, &color))
		{
			PyErr_Print();
			return NULL;
		}
		RakNet::BitStream bsOut;
		RakNet::RakString msg(message);
		bsOut.Write(msg);
		color_t col;
		col.red = (BYTE)((color >> 24) & 0xFF);  // Extract the RR byte
		col.green = (BYTE)((color >> 16) & 0xFF);   // Extract the GG byte
		col.blue = (BYTE)((color >> 8) & 0xFF);   // Extract the GG byte
		col.alpha = (BYTE)((color) & 0xFF);        // Extract the BB byte
		bsOut.Write(col);
		CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
		return PyBool_FromLong(1L);
	}
};