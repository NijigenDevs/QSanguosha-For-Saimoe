/********************************************************************
    Copyright (c) 2013-2015 - Mogara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  Mogara
*********************************************************************/

%{

#include "settings.h"

#include <QMessageBox>

%}

%native(GetFileNames) int GetFileNames(lua_State *lua);
%native(Print) int Print(lua_State *lua);
%native(AddTranslationEntry) int AddTranslationEntry(lua_State *lua);
%native(GetConfig) int GetConfig(lua_State *lua);
%native(SetConfig) int SetConfig(lua_State *lua);
%native(GetProperty) int GetProperty(lua_State *lua);
%native(Alert) int Alert(lua_State *lua);

%{

static int GetFileNames(lua_State *lua)
{
    const char *dirname = luaL_checkstring(lua, 1);
    QDir dir(dirname);
    QStringList filenames = dir.entryList(QDir::Files);

    lua_createtable(lua, filenames.length(), 0);

    for (int i = 0; i < filenames.length(); ++i) {
        lua_pushstring(lua, filenames.at(i).toLatin1());
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}

static int Print(lua_State *lua)
{
    const char *msg = luaL_checkstring(lua, 1);
    qDebug("%s", msg);

    return 0;
}

static int AddTranslationEntry(lua_State *lua)
{
    const char *key = luaL_checkstring(lua, 1);
    const char *value = luaL_checkstring(lua, 2);

    Sanguosha->addTranslationEntry(key, value);

    return 0;
}

static int GetConfig(lua_State *lua)
{
    const char *key = luaL_checkstring(lua, 1);
    int type = lua_type(lua, 2);
    switch (type) {
        case LUA_TNUMBER: {
#if LUA_VERSION_NUM >= 503
            int n = luaL_checkinteger(lua, 2);
#else
            int n = luaL_checkint(lua, 2);
#endif
            lua_pushinteger(lua, Config.value(key, n).toInt());

            break;
        }
        case LUA_TBOOLEAN: {
            bool b = lua_toboolean(lua, 2);
            lua_pushboolean(lua, Config.value(key, b).toBool());

            break;
        }
        case LUA_TSTRING: {
            const char *str = luaL_checkstring(lua, 2);
            QString qstr = Config.value(key, str).toString();
            lua_pushstring(lua, qstr.toUtf8().constData());

            break;
        }
        default:
            luaL_error(lua, "The second argument of %s should be a number, boolean or a string", __FUNCTION__);
    }

    return 1;
}

static int SetConfig(lua_State *lua)
{
    const char *key = luaL_checkstring(lua, 1);
    int type = lua_type(lua, 2);

    switch (type) {
        case LUA_TNUMBER: {
#if LUA_VERSION_NUM >= 503
            int n = luaL_checkinteger(lua, 2);
#else
            int n = luaL_checkint(lua, 2);
#endif
            Config.setValue(key, n);

            break;
        }
        case LUA_TBOOLEAN: {
            bool b = lua_toboolean(lua, 2);
            Config.setValue(key, b);

            break;
        }
        case LUA_TSTRING: {
            const char *str = luaL_checkstring(lua, 2);
            Config.setValue(key, str);

            break;
        }
        default:
            luaL_error(lua, "The second argument of %s should be a number, boolean or a string", __FUNCTION__);
    }

    return 0;
}

static int GetProperty(lua_State *lua)
{
    void *udata;
    int result = SWIG_ConvertPtr(lua, 1, &udata, SWIGTYPE_p_QObject, 0);
    luaL_argcheck(lua, SWIG_IsOK(result), 1, "QObject *");

    QObject *obj = static_cast<QObject *>(udata);
    const char *property_name = luaL_checkstring(lua, 2);
    QVariant value = obj->property(property_name);

    switch (value.type()) {
        case QMetaType::Int: {
            lua_pushinteger(lua, value.toInt());
            break;
        }
        case QMetaType::Bool: {
            lua_pushboolean(lua, value.toBool());
            break;
        }
        case QMetaType::QString: {
            lua_pushstring(lua, value.toString().toUtf8().constData());
            break;
        }
        default:
            lua_pushnil(lua);
    }

    return 1;
}

static int Alert(lua_State *lua)
{
    const char *msg = luaL_checkstring(lua, 1);

    QMessageBox::warning(NULL, "Lua warning", msg);

    return 0;
}

%}
