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

%naturalvar LuaFunction;
%typemap(in) LuaFunction
%{
if (lua_isfunction(L, $input)) {
    lua_pushvalue(L, $input);
    $1 = luaL_ref(L, LUA_REGISTRYINDEX);
} else {
    $1 = 0;
}
%}

%typemap(out) LuaFunction
%{
lua_rawgeti(L, LUA_REGISTRYINDEX, $1);
SWIG_arg ++;
%}

// binding of QString to lua basic type string
// -------------------------------------------

%naturalvar QString;

%typemap(in, checkfn = "lua_isstring") QString
%{ $1 = lua_tostring(L, $input); %}

%typemap(out) QString
%{ lua_pushstring(L, $1.toUtf8()); SWIG_arg++; %}

// -------------------------------------------

%naturalvar QStringList;

%typemap(in, checkfn = "lua_istable") QStringList
%{
for (size_t i = 0; i < lua_rawlen(L, $input); ++i) {
    lua_rawgeti(L, $input, i + 1);
    const char *elem = luaL_checkstring(L, -1);
    $1 << elem;
    lua_pop(L, 1);
}
%}

%typemap(out) QStringList
%{
lua_createtable(L, $1.length(), 0);

for (int i = 0; i < $1.length(); ++i) {
    QString str = $1.at(i);
    lua_pushstring(L, str.toUtf8());
    lua_rawseti(L, -2, i + 1);
}

SWIG_arg++;
%}
