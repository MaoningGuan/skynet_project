#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>


#include <dbus/dbus.h>

#include "ldbus.h"
#include "watch.h"


static int ldbus_watch_get_unix_fd(lua_State *L) {
	DBusWatch *watch = check_DBusWatch(L, 1);
	int fd;
	if (watch == NULL || (fd = dbus_watch_get_unix_fd(watch)) == -1) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, fd);
	}
	return 1;
}

static int ldbus_watch_get_socket(lua_State *L) {
	DBusWatch *watch = check_DBusWatch(L, 1);
	int fd;
	if (watch == NULL || (fd = dbus_watch_get_socket(watch)) == -1) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, fd);
	}
	return 1;
}

static int ldbus_watch_get_flags(lua_State *L) {
	DBusWatch *watch = check_DBusWatch(L, 1);
	unsigned int flags = (watch == NULL) ? 0 : dbus_watch_get_flags(watch);
	lua_pushinteger(L, flags);
	return 1;
}

static int ldbus_watch_handle(lua_State *L) {
	DBusWatch *watch = check_DBusWatch(L, 1);
	int flags;
	dbus_bool_t ok;
	luaL_argcheck(L, watch != NULL, 1, "watch invalid");
	flags = luaL_checkinteger(L, 2);
	ok = dbus_watch_handle(watch, flags);
	lua_pushboolean(L, ok);
	return 1;
}

static int ldbus_watch_get_enabled(lua_State *L) {
	DBusWatch *watch = check_DBusWatch(L, 1);
	dbus_bool_t enabled = (watch == NULL) ? FALSE : dbus_watch_get_enabled(watch);
	lua_pushboolean(L, enabled);
	return 1;
}

LDBUS_INTERNAL void push_DBusWatch(lua_State *L, DBusWatch *watch) {
	DBusWatch **udata = lua_newuserdata(L, sizeof(DBusWatch*));
	*udata = watch;
	luaL_setmetatable(L, DBUS_WATCH_METATABLE);
}

LDBUS_INTERNAL dbus_bool_t ldbus_watch_add_function(DBusWatch *watch, void *data) {
	lua_State *L = *(lua_State**)data;
	int top = lua_gettop(L);
	if (!lua_checkstack(L, 2)) return FALSE;
	lua_rawgetp(L, LUA_REGISTRYINDEX, data);
	lua_getuservalue(L, -1);
	lua_remove(L, top + 1); /* remove userdata */
	lua_rawgeti(L, -1, DBUS_LUA_FUNC_ADD);
	lua_remove(L, top + 1); /* remove uservalue table */
	push_DBusWatch(L, watch); /* XXX: could throw */
	/* Save DBusWatch in registry */
	lua_pushvalue(L, -1);
	lua_rawsetp(L, LUA_REGISTRYINDEX, watch);
	switch(lua_pcall(L, 1, 0, 0)) {
	case LUA_OK:
		return TRUE;
	case LUA_ERRMEM:
		lua_pop(L, 1);
		return FALSE;
	default:
		/* unhandled error */
		lua_pop(L, 1);
		return TRUE;
	}
}

LDBUS_INTERNAL void ldbus_watch_remove_function(DBusWatch *watch, void *data) {
	lua_State *L = *(lua_State**)data;
	int top = lua_gettop(L);
	DBusWatch **udata;

	/* Grab added watch from registry */
	/* We need to keep a reference to userdata around so we can invalidate it */
	lua_rawgetp(L, LUA_REGISTRYINDEX, watch);
	/* Delete from registry */
	lua_pushnil(L);
	lua_rawsetp(L, LUA_REGISTRYINDEX, watch);

	lua_rawgetp(L, LUA_REGISTRYINDEX, data);
	lua_getuservalue(L, -1);
	lua_remove(L, top + 2); /* remove userdata */
	lua_rawgeti(L, -1, DBUS_LUA_FUNC_REMOVE);
	lua_remove(L, top + 2); /* remove uservalue table */
	lua_pushvalue(L, top + 1); /* push watch as argument */
	if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
		/* unhandled error */
	}

	/* Invalidate watch object */
	udata = lua_touserdata(L, top+1);
	if (udata != NULL) {
		*udata = NULL;
	}

	lua_settop(L, top);
	return;
}

LDBUS_INTERNAL void ldbus_watch_toggled_function(DBusWatch *watch, void *data) {
	lua_State *L = *(lua_State**)data;
	int top = lua_gettop(L);
	lua_rawgetp(L, LUA_REGISTRYINDEX, data);
	lua_getuservalue(L, -1);
	lua_remove(L, top + 1); /* remove userdata */
	lua_rawgeti(L, -1, DBUS_LUA_FUNC_TOGGLE);
	lua_remove(L, top + 1); /* remove uservalue table */
	lua_rawgetp(L, LUA_REGISTRYINDEX, watch); /* grab added watch from registry */
	if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
		/* unhandled error */
		lua_pop(L, 1);
	}
	return;
}

LDBUS_INTERNAL void ldbus_watch_free_data_function(void *data) {
	lua_State *L = *(lua_State**)data;
	lua_pushnil(L);
	lua_rawsetp(L, LUA_REGISTRYINDEX, data);
}

int luaopen_ldbus_watch(lua_State *L) {
	static luaL_Reg const methods [] = {
		{ "get_unix_fd", ldbus_watch_get_unix_fd },
		{ "get_socket", ldbus_watch_get_socket },
		{ "get_flags", ldbus_watch_get_flags },
		{ "handle", ldbus_watch_handle },
		{ "get_enabled", ldbus_watch_get_enabled },
		{ NULL, NULL }
	};

	if (luaL_newmetatable(L, DBUS_WATCH_METATABLE)) {
		luaL_newlib(L, methods);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, tostring);
		lua_setfield(L, -2, "__tostring");

		lua_pushstring(L, "DBusWatch");
		lua_setfield(L, -2, "__udtype");
	}

	lua_createtable(L, 0, 3);
	lua_pushinteger(L, DBUS_WATCH_READABLE); lua_setfield(L, -2, "READABLE");
	lua_pushinteger(L, DBUS_WATCH_WRITABLE); lua_setfield(L, -2, "WRITABLE");
	lua_pushinteger(L, DBUS_WATCH_HANGUP); lua_setfield(L, -2, "HANGUP");
	lua_pushinteger(L, DBUS_WATCH_ERROR); lua_setfield(L, -2, "ERROR");

	return 1;
}
