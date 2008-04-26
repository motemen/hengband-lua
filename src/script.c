/* File: script.c */

#include "angband.h"

#include "script.h"

lua_State *L = NULL;

static int __lua_msg_print()
{
	if (lua_gettop(L)) {
		msg_print(lua_tostring(L, 1));
	}

	return 0;
}

errr script_init()
{
	char buf[1024];

	L = lua_open();

	// Export functions
	lua_register(L, "msg_print", __lua_msg_print);

	// Load initial script
	path_build(buf, sizeof(buf), ANGBAND_DIR_SCRIPT, "init.lua");
	luaL_dofile(L, buf);

	return 0;
}
