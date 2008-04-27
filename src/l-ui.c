/*
** Lua binding: ui
** Generated automatically by tolua 5.0a on Sun Apr 27 06:44:59 2008.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "lua/tolua.h"

/* Exported function */
TOLUA_API int tolua_ui_open (lua_State* tolua_S);

#include "angband.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: notice_stuff */
static int tolua_ui_notice_stuff00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  notice_stuff();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'notice_stuff'.",&tolua_err);
 return 0;
#endif
}

/* function: update_stuff */
static int tolua_ui_update_stuff00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  update_stuff();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_stuff'.",&tolua_err);
 return 0;
#endif
}

/* function: redraw_stuff */
static int tolua_ui_redraw_stuff00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  redraw_stuff();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'redraw_stuff'.",&tolua_err);
 return 0;
#endif
}

/* function: window_stuff */
static int tolua_ui_window_stuff00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  window_stuff();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'window_stuff'.",&tolua_err);
 return 0;
#endif
}

/* function: handle_stuff */
static int tolua_ui_handle_stuff00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  handle_stuff();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'handle_stuff'.",&tolua_err);
 return 0;
#endif
}

/* function: inkey */
static int tolua_ui_inkey00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  char tolua_ret = (char)  inkey();
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'inkey'.",&tolua_err);
 return 0;
#endif
}

#if 0
/* function: bell */
static int tolua_ui_bell00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  cptr reason = ((cptr)  tolua_tostring(tolua_S,1,0));
 {
  bell(reason);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'bell'.",&tolua_err);
 return 0;
#endif
}
#endif

/* function: sound */
static int tolua_ui_sound00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int val = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  sound(val);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'sound'.",&tolua_err);
 return 0;
#endif
}

/* function: msg_print */
static int tolua_ui_msg_print00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  cptr msg = ((cptr)  tolua_tostring(tolua_S,1,0));
 {
  msg_print(msg);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'msg_print'.",&tolua_err);
 return 0;
#endif
}

/* function: message */
static int tolua_ui_message00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  u16b message_type = ((u16b)  tolua_tonumber(tolua_S,1,0));
  s16b extra = ((s16b)  tolua_tonumber(tolua_S,2,0));
  cptr message_text = ((cptr)  tolua_tostring(tolua_S,3,0));
 {
  message(message_type,extra,message_text);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'message'.",&tolua_err);
 return 0;
#endif
}

/* function: message_flush */
static int tolua_ui_message_flush00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  message_flush();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'message_flush'.",&tolua_err);
 return 0;
#endif
}

/* function: screen_save */
static int tolua_ui_screen_save00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  screen_save();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'screen_save'.",&tolua_err);
 return 0;
#endif
}

/* function: screen_load */
static int tolua_ui_screen_load00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  screen_load();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'screen_load'.",&tolua_err);
 return 0;
#endif
}

/* function: c_put_str */
static int tolua_ui_c_put_str00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  byte attr = ((byte)  tolua_tonumber(tolua_S,1,0));
  cptr str = ((cptr)  tolua_tostring(tolua_S,2,0));
  int row = ((int)  tolua_tonumber(tolua_S,3,0));
  int col = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  c_put_str(attr,str,row,col);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'c_put_str'.",&tolua_err);
 return 0;
#endif
}

/* function: put_str */
static int tolua_ui_put_str00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  cptr str = ((cptr)  tolua_tostring(tolua_S,1,0));
  int row = ((int)  tolua_tonumber(tolua_S,2,0));
  int col = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  put_str(str,row,col);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'put_str'.",&tolua_err);
 return 0;
#endif
}

/* function: c_prt */
static int tolua_ui_c_prt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  byte attr = ((byte)  tolua_tonumber(tolua_S,1,0));
  cptr str = ((cptr)  tolua_tostring(tolua_S,2,0));
  int row = ((int)  tolua_tonumber(tolua_S,3,0));
  int col = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  c_prt(attr,str,row,col);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'c_prt'.",&tolua_err);
 return 0;
#endif
}

/* function: prt */
static int tolua_ui_prt00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  cptr str = ((cptr)  tolua_tostring(tolua_S,1,0));
  int row = ((int)  tolua_tonumber(tolua_S,2,0));
  int col = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  prt(str,row,col);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'prt'.",&tolua_err);
 return 0;
#endif
}

/* function: clear_from */
static int tolua_ui_clear_from00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int row = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  clear_from(row);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clear_from'.",&tolua_err);
 return 0;
#endif
}

/* function: pause_line */
static int tolua_ui_pause_line00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int row = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  pause_line(row);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pause_line'.",&tolua_err);
 return 0;
#endif
}

/* function: request_command */
static int tolua_ui_request_command00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool shopping = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  request_command(shopping);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'request_command'.",&tolua_err);
 return 0;
#endif
}

/* function: get_aim_dir */
static int tolua_ui_get_aim_dir00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int dp = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  bool tolua_ret = (bool)  get_aim_dir(&dp);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 tolua_pushnumber(tolua_S,(long)dp);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_aim_dir'.",&tolua_err);
 return 0;
#endif
}

/* function: text_out_to_screen */
static int tolua_ui_text_out_to_screen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  byte a = ((byte)  tolua_tonumber(tolua_S,1,0));
  cptr str = ((cptr)  tolua_tostring(tolua_S,2,0));
 {
  text_out_to_screen(a,str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'text_out'.",&tolua_err);
 return 0;
#endif
}

/* function: Term_clear */
static int tolua_ui_Term_clear00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  errr tolua_ret = (errr)  Term_clear();
 tolua_pushnumber(tolua_S,(long)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Term_clear'.",&tolua_err);
 return 0;
#endif
}

/* function: flush */
static int tolua_ui_flush00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  flush();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'flush'.",&tolua_err);
 return 0;
#endif
}

/* function: flush_fail */
static int tolua_ui_flush_fail00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  flush_fail();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'flush_fail'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_ui_open (lua_State* tolua_S)
{
#if 0
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
 tolua_constant(tolua_S,"TERM_DARK",TERM_DARK);
 tolua_constant(tolua_S,"TERM_WHITE",TERM_WHITE);
 tolua_constant(tolua_S,"TERM_SLATE",TERM_SLATE);
 tolua_constant(tolua_S,"TERM_ORANGE",TERM_ORANGE);
 tolua_constant(tolua_S,"TERM_RED",TERM_RED);
 tolua_constant(tolua_S,"TERM_GREEN",TERM_GREEN);
 tolua_constant(tolua_S,"TERM_BLUE",TERM_BLUE);
 tolua_constant(tolua_S,"TERM_UMBER",TERM_UMBER);
 tolua_constant(tolua_S,"TERM_L_DARK",TERM_L_DARK);
 tolua_constant(tolua_S,"TERM_L_WHITE",TERM_L_WHITE);
 tolua_constant(tolua_S,"TERM_VIOLET",TERM_VIOLET);
 tolua_constant(tolua_S,"TERM_YELLOW",TERM_YELLOW);
 tolua_constant(tolua_S,"TERM_L_RED",TERM_L_RED);
 tolua_constant(tolua_S,"TERM_L_GREEN",TERM_L_GREEN);
 tolua_constant(tolua_S,"TERM_L_BLUE",TERM_L_BLUE);
 tolua_constant(tolua_S,"TERM_L_UMBER",TERM_L_UMBER);
 tolua_constant(tolua_S,"MSG_GENERIC",MSG_GENERIC);
 tolua_constant(tolua_S,"MSG_HIT",MSG_HIT);
 tolua_constant(tolua_S,"MSG_MISS",MSG_MISS);
 tolua_constant(tolua_S,"MSG_FLEE",MSG_FLEE);
 tolua_constant(tolua_S,"MSG_DROP",MSG_DROP);
 tolua_constant(tolua_S,"MSG_KILL",MSG_KILL);
 tolua_constant(tolua_S,"MSG_LEVEL",MSG_LEVEL);
 tolua_constant(tolua_S,"MSG_DEATH",MSG_DEATH);
 tolua_constant(tolua_S,"MSG_STUDY",MSG_STUDY);
 tolua_constant(tolua_S,"MSG_TELEPORT",MSG_TELEPORT);
 tolua_constant(tolua_S,"MSG_SHOOT",MSG_SHOOT);
 tolua_constant(tolua_S,"MSG_QUAFF",MSG_QUAFF);
 tolua_constant(tolua_S,"MSG_ZAP_ROD",MSG_ZAP_ROD);
 tolua_constant(tolua_S,"MSG_WALK",MSG_WALK);
 tolua_constant(tolua_S,"MSG_TPOTHER",MSG_TPOTHER);
 tolua_constant(tolua_S,"MSG_HITWALL",MSG_HITWALL);
 tolua_constant(tolua_S,"MSG_EAT",MSG_EAT);
 tolua_constant(tolua_S,"MSG_STORE1",MSG_STORE1);
 tolua_constant(tolua_S,"MSG_STORE2",MSG_STORE2);
 tolua_constant(tolua_S,"MSG_STORE3",MSG_STORE3);
 tolua_constant(tolua_S,"MSG_STORE4",MSG_STORE4);
 tolua_constant(tolua_S,"MSG_DIG",MSG_DIG);
 tolua_constant(tolua_S,"MSG_OPENDOOR",MSG_OPENDOOR);
 tolua_constant(tolua_S,"MSG_SHUTDOOR",MSG_SHUTDOOR);
 tolua_constant(tolua_S,"MSG_TPLEVEL",MSG_TPLEVEL);
 tolua_constant(tolua_S,"MSG_BELL",MSG_BELL);
 tolua_constant(tolua_S,"MSG_NOTHING_TO_OPEN",MSG_NOTHING_TO_OPEN);
 tolua_constant(tolua_S,"MSG_LOCKPICK_FAIL",MSG_LOCKPICK_FAIL);
 tolua_constant(tolua_S,"MSG_STAIRS_DOWN",MSG_STAIRS_DOWN);
 tolua_constant(tolua_S,"MSG_HITPOINT_WARN",MSG_HITPOINT_WARN);
 tolua_constant(tolua_S,"MSG_ACT_ARTIFACT",MSG_ACT_ARTIFACT);
 tolua_constant(tolua_S,"MSG_USE_STAFF",MSG_USE_STAFF);
 tolua_constant(tolua_S,"MSG_DESTROY",MSG_DESTROY);
 tolua_constant(tolua_S,"MSG_MON_HIT",MSG_MON_HIT);
 tolua_constant(tolua_S,"MSG_MON_TOUCH",MSG_MON_TOUCH);
 tolua_constant(tolua_S,"MSG_MON_PUNCH",MSG_MON_PUNCH);
 tolua_constant(tolua_S,"MSG_MON_KICK",MSG_MON_KICK);
 tolua_constant(tolua_S,"MSG_MON_CLAW",MSG_MON_CLAW);
 tolua_constant(tolua_S,"MSG_MON_BITE",MSG_MON_BITE);
 tolua_constant(tolua_S,"MSG_MON_STING",MSG_MON_STING);
 tolua_constant(tolua_S,"MSG_MON_BUTT",MSG_MON_BUTT);
 tolua_constant(tolua_S,"MSG_MON_CRUSH",MSG_MON_CRUSH);
 tolua_constant(tolua_S,"MSG_MON_ENGULF",MSG_MON_ENGULF);
 tolua_constant(tolua_S,"MSG_MON_CRAWL",MSG_MON_CRAWL);
 tolua_constant(tolua_S,"MSG_MON_DROOL",MSG_MON_DROOL);
 tolua_constant(tolua_S,"MSG_MON_SPIT",MSG_MON_SPIT);
 tolua_constant(tolua_S,"MSG_MON_GAZE",MSG_MON_GAZE);
 tolua_constant(tolua_S,"MSG_MON_WAIL",MSG_MON_WAIL);
 tolua_constant(tolua_S,"MSG_MON_SPORE",MSG_MON_SPORE);
 tolua_constant(tolua_S,"MSG_MON_BEG",MSG_MON_BEG);
 tolua_constant(tolua_S,"MSG_MON_INSULT",MSG_MON_INSULT);
 tolua_constant(tolua_S,"MSG_MON_MOAN",MSG_MON_MOAN);
 tolua_constant(tolua_S,"MSG_RECOVER",MSG_RECOVER);
 tolua_constant(tolua_S,"MSG_BLIND",MSG_BLIND);
 tolua_constant(tolua_S,"MSG_CONFUSED",MSG_CONFUSED);
 tolua_constant(tolua_S,"MSG_POISONED",MSG_POISONED);
 tolua_constant(tolua_S,"MSG_AFRAID",MSG_AFRAID);
 tolua_constant(tolua_S,"MSG_PARALYZED",MSG_PARALYZED);
 tolua_constant(tolua_S,"MSG_DRUGGED",MSG_DRUGGED);
 tolua_constant(tolua_S,"MSG_SPEED",MSG_SPEED);
 tolua_constant(tolua_S,"MSG_SLOW",MSG_SLOW);
 tolua_constant(tolua_S,"MSG_SHIELD",MSG_SHIELD);
 tolua_constant(tolua_S,"MSG_BLESSED",MSG_BLESSED);
 tolua_constant(tolua_S,"MSG_HERO",MSG_HERO);
 tolua_constant(tolua_S,"MSG_BERSERK",MSG_BERSERK);
 tolua_constant(tolua_S,"MSG_PROT_EVIL",MSG_PROT_EVIL);
 tolua_constant(tolua_S,"MSG_INVULN",MSG_INVULN);
 tolua_constant(tolua_S,"MSG_SEE_INVIS",MSG_SEE_INVIS);
 tolua_constant(tolua_S,"MSG_INFRARED",MSG_INFRARED);
 tolua_constant(tolua_S,"MSG_RES_ACID",MSG_RES_ACID);
 tolua_constant(tolua_S,"MSG_RES_ELEC",MSG_RES_ELEC);
 tolua_constant(tolua_S,"MSG_RES_FIRE",MSG_RES_FIRE);
 tolua_constant(tolua_S,"MSG_RES_COLD",MSG_RES_COLD);
 tolua_constant(tolua_S,"MSG_RES_POIS",MSG_RES_POIS);
 tolua_constant(tolua_S,"MSG_STUN",MSG_STUN);
 tolua_constant(tolua_S,"MSG_CUT",MSG_CUT);
 tolua_constant(tolua_S,"MSG_STAIRS_UP",MSG_STAIRS_UP);
 tolua_constant(tolua_S,"MSG_STORE_ENTER",MSG_STORE_ENTER);
 tolua_constant(tolua_S,"MSG_STORE_LEAVE",MSG_STORE_LEAVE);
 tolua_constant(tolua_S,"MSG_STORE_HOME",MSG_STORE_HOME);
 tolua_constant(tolua_S,"MSG_MONEY1",MSG_MONEY1);
 tolua_constant(tolua_S,"MSG_MONEY2",MSG_MONEY2);
 tolua_constant(tolua_S,"MSG_MONEY3",MSG_MONEY3);
 tolua_constant(tolua_S,"MSG_SHOOT_HIT",MSG_SHOOT_HIT);
 tolua_constant(tolua_S,"MSG_STORE5",MSG_STORE5);
 tolua_constant(tolua_S,"MSG_LOCKPICK",MSG_LOCKPICK);
 tolua_constant(tolua_S,"MSG_DISARM",MSG_DISARM);
 tolua_constant(tolua_S,"MSG_IDENT_BAD",MSG_IDENT_BAD);
 tolua_constant(tolua_S,"MSG_IDENT_EGO",MSG_IDENT_EGO);
 tolua_constant(tolua_S,"MSG_IDENT_ART",MSG_IDENT_ART);
 tolua_constant(tolua_S,"MSG_BR_ELEMENTS",MSG_BR_ELEMENTS);
 tolua_constant(tolua_S,"MSG_BR_FROST",MSG_BR_FROST);
 tolua_constant(tolua_S,"MSG_BR_ELEC",MSG_BR_ELEC);
 tolua_constant(tolua_S,"MSG_BR_ACID",MSG_BR_ACID);
 tolua_constant(tolua_S,"MSG_BR_GAS",MSG_BR_GAS);
 tolua_constant(tolua_S,"MSG_BR_FIRE",MSG_BR_FIRE);
 tolua_constant(tolua_S,"MSG_BR_CONF",MSG_BR_CONF);
 tolua_constant(tolua_S,"MSG_BR_DISENCHANT",MSG_BR_DISENCHANT);
 tolua_constant(tolua_S,"MSG_BR_CHAOS",MSG_BR_CHAOS);
 tolua_constant(tolua_S,"MSG_BR_SHARDS",MSG_BR_SHARDS);
 tolua_constant(tolua_S,"MSG_BR_SOUND",MSG_BR_SOUND);
 tolua_constant(tolua_S,"MSG_BR_LIGHT",MSG_BR_LIGHT);
 tolua_constant(tolua_S,"MSG_BR_DARK",MSG_BR_DARK);
 tolua_constant(tolua_S,"MSG_BR_NETHER",MSG_BR_NETHER);
 tolua_constant(tolua_S,"MSG_BR_NEXUS",MSG_BR_NEXUS);
 tolua_constant(tolua_S,"MSG_BR_TIME",MSG_BR_TIME);
 tolua_constant(tolua_S,"MSG_BR_INERTIA",MSG_BR_INERTIA);
 tolua_constant(tolua_S,"MSG_BR_GRAVITY",MSG_BR_GRAVITY);
 tolua_constant(tolua_S,"MSG_BR_PLASMA",MSG_BR_PLASMA);
 tolua_constant(tolua_S,"MSG_BR_FORCE",MSG_BR_FORCE);
 tolua_constant(tolua_S,"MSG_SUM_MONSTER",MSG_SUM_MONSTER);
 tolua_constant(tolua_S,"MSG_SUM_ANGEL",MSG_SUM_ANGEL);
 tolua_constant(tolua_S,"MSG_SUM_UNDEAD",MSG_SUM_UNDEAD);
 tolua_constant(tolua_S,"MSG_SUM_ANIMAL",MSG_SUM_ANIMAL);
 tolua_constant(tolua_S,"MSG_SUM_SPIDER",MSG_SUM_SPIDER);
 tolua_constant(tolua_S,"MSG_SUM_HOUND",MSG_SUM_HOUND);
 tolua_constant(tolua_S,"MSG_SUM_HYDRA",MSG_SUM_HYDRA);
 tolua_constant(tolua_S,"MSG_SUM_DEMON",MSG_SUM_DEMON);
 tolua_constant(tolua_S,"MSG_SUM_DRAGON",MSG_SUM_DRAGON);
 tolua_constant(tolua_S,"MSG_SUM_HI_UNDEAD",MSG_SUM_HI_UNDEAD);
 tolua_constant(tolua_S,"MSG_SUM_HI_DRAGON",MSG_SUM_HI_DRAGON);
 tolua_constant(tolua_S,"MSG_SUM_HI_DEMON",MSG_SUM_HI_DEMON);
 tolua_constant(tolua_S,"MSG_SUM_WRAITH",MSG_SUM_WRAITH);
 tolua_constant(tolua_S,"MSG_SUM_UNIQUE",MSG_SUM_UNIQUE);
 tolua_constant(tolua_S,"MSG_WIELD",MSG_WIELD);
 tolua_constant(tolua_S,"MSG_CURSED",MSG_CURSED);
 tolua_constant(tolua_S,"MSG_PSEUDOID",MSG_PSEUDOID);
 tolua_constant(tolua_S,"MSG_HUNGRY",MSG_HUNGRY);
 tolua_constant(tolua_S,"MSG_NOTICE",MSG_NOTICE);
 tolua_constant(tolua_S,"MSG_AMBIENT_DAY",MSG_AMBIENT_DAY);
 tolua_constant(tolua_S,"MSG_AMBIENT_NITE",MSG_AMBIENT_NITE);
 tolua_constant(tolua_S,"MSG_AMBIENT_DNG1",MSG_AMBIENT_DNG1);
 tolua_constant(tolua_S,"MSG_AMBIENT_DNG2",MSG_AMBIENT_DNG2);
 tolua_constant(tolua_S,"MSG_AMBIENT_DNG3",MSG_AMBIENT_DNG3);
 tolua_constant(tolua_S,"MSG_AMBIENT_DNG4",MSG_AMBIENT_DNG4);
 tolua_constant(tolua_S,"MSG_AMBIENT_DNG5",MSG_AMBIENT_DNG5);
 tolua_constant(tolua_S,"MSG_CREATE_TRAP",MSG_CREATE_TRAP);
 tolua_constant(tolua_S,"MSG_SHRIEK",MSG_SHRIEK);
 tolua_constant(tolua_S,"MSG_CAST_FEAR",MSG_CAST_FEAR);
 tolua_constant(tolua_S,"MSG_HIT_GOOD",MSG_HIT_GOOD);
 tolua_constant(tolua_S,"MSG_HIT_GREAT",MSG_HIT_GREAT);
 tolua_constant(tolua_S,"MSG_HIT_SUPERB",MSG_HIT_SUPERB);
 tolua_constant(tolua_S,"MSG_HIT_HI_GREAT",MSG_HIT_HI_GREAT);
 tolua_constant(tolua_S,"MSG_HIT_HI_SUPERB",MSG_HIT_HI_SUPERB);
 tolua_constant(tolua_S,"MSG_SPELL",MSG_SPELL);
 tolua_constant(tolua_S,"MSG_PRAYER",MSG_PRAYER);
 tolua_constant(tolua_S,"MSG_KILL_UNIQUE",MSG_KILL_UNIQUE);
 tolua_constant(tolua_S,"MSG_KILL_KING",MSG_KILL_KING);
 tolua_constant(tolua_S,"MSG_DRAIN_STAT",MSG_DRAIN_STAT);
 tolua_constant(tolua_S,"MSG_MULTIPLY",MSG_MULTIPLY);
 tolua_constant(tolua_S,"MSG_MAX",MSG_MAX);
 tolua_constant(tolua_S,"PN_COMBINE",PN_COMBINE);
 tolua_constant(tolua_S,"PN_REORDER",PN_REORDER);
 tolua_constant(tolua_S,"PU_BONUS",PU_BONUS);
 tolua_constant(tolua_S,"PU_TORCH",PU_TORCH);
 tolua_constant(tolua_S,"PU_HP",PU_HP);
 tolua_constant(tolua_S,"PU_MANA",PU_MANA);
 tolua_constant(tolua_S,"PU_SPELLS",PU_SPELLS);
 tolua_constant(tolua_S,"PU_FORGET_VIEW",PU_FORGET_VIEW);
 tolua_constant(tolua_S,"PU_UPDATE_VIEW",PU_UPDATE_VIEW);
 tolua_constant(tolua_S,"PU_FORGET_FLOW",PU_FORGET_FLOW);
 tolua_constant(tolua_S,"PU_UPDATE_FLOW",PU_UPDATE_FLOW);
 tolua_constant(tolua_S,"PU_MONSTERS",PU_MONSTERS);
 tolua_constant(tolua_S,"PU_DISTANCE",PU_DISTANCE);
 tolua_constant(tolua_S,"PU_PANEL",PU_PANEL);
 tolua_constant(tolua_S,"PR_MISC",PR_MISC);
 tolua_constant(tolua_S,"PR_TITLE",PR_TITLE);
 tolua_constant(tolua_S,"PR_LEV",PR_LEV);
 tolua_constant(tolua_S,"PR_EXP",PR_EXP);
 tolua_constant(tolua_S,"PR_STATS",PR_STATS);
 tolua_constant(tolua_S,"PR_ARMOR",PR_ARMOR);
 tolua_constant(tolua_S,"PR_HP",PR_HP);
 tolua_constant(tolua_S,"PR_MANA",PR_MANA);
 tolua_constant(tolua_S,"PR_GOLD",PR_GOLD);
 tolua_constant(tolua_S,"PR_DEPTH",PR_DEPTH);
 tolua_constant(tolua_S,"PR_EQUIPPY",PR_EQUIPPY);
 tolua_constant(tolua_S,"PR_HEALTH",PR_HEALTH);
 tolua_constant(tolua_S,"PR_CUT",PR_CUT);
 tolua_constant(tolua_S,"PR_STUN",PR_STUN);
 tolua_constant(tolua_S,"PR_HUNGER",PR_HUNGER);
 tolua_constant(tolua_S,"PR_BLIND",PR_BLIND);
 tolua_constant(tolua_S,"PR_CONFUSED",PR_CONFUSED);
 tolua_constant(tolua_S,"PR_AFRAID",PR_AFRAID);
 tolua_constant(tolua_S,"PR_POISONED",PR_POISONED);
 tolua_constant(tolua_S,"PR_STATE",PR_STATE);
 tolua_constant(tolua_S,"PR_SPEED",PR_SPEED);
 tolua_constant(tolua_S,"PR_STUDY",PR_STUDY);
 tolua_constant(tolua_S,"PR_MAP",PR_MAP);
 tolua_constant(tolua_S,"PW_INVEN",PW_INVEN);
 tolua_constant(tolua_S,"PW_EQUIP",PW_EQUIP);
 tolua_constant(tolua_S,"PW_PLAYER_0",PW_PLAYER_0);
 tolua_constant(tolua_S,"PW_PLAYER_1",PW_PLAYER_1);
 tolua_constant(tolua_S,"PW_PLAYER_2",PW_PLAYER_2);
 tolua_constant(tolua_S,"PW_MAP",PW_MAP);
 tolua_constant(tolua_S,"PW_MESSAGE",PW_MESSAGE);
 tolua_constant(tolua_S,"PW_OVERHEAD",PW_OVERHEAD);
 tolua_constant(tolua_S,"PW_MONSTER",PW_MONSTER);
 tolua_constant(tolua_S,"PW_OBJECT",PW_OBJECT);
 tolua_constant(tolua_S,"PW_MONLIST",PW_MONLIST);
 tolua_constant(tolua_S,"PW_STATUS",PW_STATUS);
 tolua_constant(tolua_S,"PW_SCRIPT_VARS",PW_SCRIPT_VARS);
 tolua_constant(tolua_S,"PW_SCRIPT_SOURCE",PW_SCRIPT_SOURCE);
 tolua_constant(tolua_S,"PW_BORG_1",PW_BORG_1);
 tolua_constant(tolua_S,"PW_BORG_2",PW_BORG_2);
 tolua_function(tolua_S,"notice_stuff",tolua_ui_notice_stuff00);
 tolua_function(tolua_S,"update_stuff",tolua_ui_update_stuff00);
 tolua_function(tolua_S,"redraw_stuff",tolua_ui_redraw_stuff00);
 tolua_function(tolua_S,"window_stuff",tolua_ui_window_stuff00);
 tolua_function(tolua_S,"handle_stuff",tolua_ui_handle_stuff00);
 tolua_function(tolua_S,"inkey",tolua_ui_inkey00);
 tolua_function(tolua_S,"bell",tolua_ui_bell00);
 tolua_function(tolua_S,"sound",tolua_ui_sound00);
 tolua_function(tolua_S,"msg_print",tolua_ui_msg_print00);
 tolua_function(tolua_S,"message",tolua_ui_message00);
 tolua_function(tolua_S,"message_flush",tolua_ui_message_flush00);
 tolua_function(tolua_S,"screen_save",tolua_ui_screen_save00);
 tolua_function(tolua_S,"screen_load",tolua_ui_screen_load00);
 tolua_function(tolua_S,"c_put_str",tolua_ui_c_put_str00);
 tolua_function(tolua_S,"put_str",tolua_ui_put_str00);
 tolua_function(tolua_S,"c_prt",tolua_ui_c_prt00);
 tolua_function(tolua_S,"prt",tolua_ui_prt00);
 tolua_function(tolua_S,"clear_from",tolua_ui_clear_from00);
 tolua_function(tolua_S,"pause_line",tolua_ui_pause_line00);
 tolua_function(tolua_S,"request_command",tolua_ui_request_command00);
 tolua_function(tolua_S,"get_aim_dir",tolua_ui_get_aim_dir00);
 tolua_function(tolua_S,"text_out",tolua_ui_text_out_to_screen00);
 tolua_function(tolua_S,"Term_clear",tolua_ui_Term_clear00);
 tolua_function(tolua_S,"flush",tolua_ui_flush00);
 tolua_function(tolua_S,"flush_fail",tolua_ui_flush_fail00);
 tolua_endmodule(tolua_S);
#endif
 return 1;
}
