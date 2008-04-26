/* File: spells1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

/* Purpose: Spell projection */

#include "angband.h"

/* ToDo: Make this global */
/* 1/x chance of reducing stats (for elemental attacks) */
#define HURT_CHANCE 16


static int rakubadam_m;
static int rakubadam_p;

int project_length = 0;

/*
 * Get another mirror. for SEEKER 
 */
static void next_mirror( int* next_y , int* next_x , int cury, int curx)
{
	int mirror_x[10],mirror_y[10]; /* ���Ϥ�äȾ��ʤ� */
	int mirror_num=0;              /* ���ο� */
	int x,y;
	int num;

	for( x=0 ; x < cur_wid ; x++ )
	{
		for( y=0 ; y < cur_hgt ; y++ )
		{
			if( is_mirror_grid(&cave[y][x])){
				mirror_y[mirror_num]=y;
				mirror_x[mirror_num]=x;
				mirror_num++;
			}
		}
	}
	if( mirror_num )
	{
		num=randint0(mirror_num);
		*next_y=mirror_y[num];
		*next_x=mirror_x[num];
		return;
	}
	*next_y=cury+randint0(5)-2;
	*next_x=curx+randint0(5)-2;
	return;
}
		
/*
 * Get a legal "multi-hued" color for drawing "spells"
 */
static byte mh_attr(int max)
{
	switch (randint1(max))
	{
		case  1: return (TERM_RED);
		case  2: return (TERM_GREEN);
		case  3: return (TERM_BLUE);
		case  4: return (TERM_YELLOW);
		case  5: return (TERM_ORANGE);
		case  6: return (TERM_VIOLET);
		case  7: return (TERM_L_RED);
		case  8: return (TERM_L_GREEN);
		case  9: return (TERM_L_BLUE);
		case 10: return (TERM_UMBER);
		case 11: return (TERM_L_UMBER);
		case 12: return (TERM_SLATE);
		case 13: return (TERM_WHITE);
		case 14: return (TERM_L_WHITE);
		case 15: return (TERM_L_DARK);
	}

	return (TERM_WHITE);
}


/*
 * Return a color to use for the bolt/ball spells
 */
static byte spell_color(int type)
{
	/* Check if A.B.'s new graphics should be used (rr9) */
	if (streq(ANGBAND_GRAF, "new"))
	{
		/* Analyze */
		switch (type)
		{
			case GF_PSY_SPEAR:      return (0x06);
			case GF_MISSILE:        return (0x0F);
			case GF_ACID:           return (0x04);
			case GF_ELEC:           return (0x02);
			case GF_FIRE:           return (0x00);
			case GF_COLD:           return (0x01);
			case GF_POIS:           return (0x03);
			case GF_HOLY_FIRE:      return (0x00);
			case GF_HELL_FIRE:      return (0x00);
			case GF_MANA:           return (0x0E);
			  /* by henkma */
			case GF_SEEKER:         return (0x0E);
			case GF_SUPER_RAY:      return (0x0E);

			case GF_ARROW:          return (0x0F);
			case GF_WATER:          return (0x04);
			case GF_NETHER:         return (0x07);
			case GF_CHAOS:          return (mh_attr(15));
			case GF_DISENCHANT:     return (0x05);
			case GF_NEXUS:          return (0x0C);
			case GF_CONFUSION:      return (mh_attr(4));
			case GF_SOUND:          return (0x09);
			case GF_SHARDS:         return (0x08);
			case GF_FORCE:          return (0x09);
			case GF_INERTIA:        return (0x09);
			case GF_GRAVITY:        return (0x09);
			case GF_TIME:           return (0x09);
			case GF_LITE_WEAK:      return (0x06);
			case GF_LITE:           return (0x06);
			case GF_DARK_WEAK:      return (0x07);
			case GF_DARK:           return (0x07);
			case GF_PLASMA:         return (0x0B);
			case GF_METEOR:         return (0x00);
			case GF_ICE:            return (0x01);
			case GF_ROCKET:         return (0x0F);
			case GF_DEATH_RAY:      return (0x07);
			case GF_NUKE:           return (mh_attr(2));
			case GF_DISINTEGRATE:   return (0x05);
			case GF_PSI:
			case GF_PSI_DRAIN:
			case GF_TELEKINESIS:
			case GF_DOMINATION:
			case GF_DRAIN_MANA:
			case GF_MIND_BLAST:
			case GF_BRAIN_SMASH:
						return (0x09);
			case GF_CAUSE_1:
			case GF_CAUSE_2:
			case GF_CAUSE_3:
			case GF_CAUSE_4:        return (0x0E);
			case GF_HAND_DOOM:      return (0x07);
			case GF_CAPTURE  :      return (0x0E);
			case GF_IDENTIFY:       return (0x01);
			case GF_ATTACK:        return (0x0F);
			case GF_PHOTO   :      return (0x06);
		}
	}
	/* Normal tiles or ASCII */
	else
	{
		byte a;
		char c;

		/* Lookup the default colors for this type */
		cptr s = quark_str(gf_color[type]);

		/* Oops */
		if (!s) return (TERM_WHITE);

		/* Pick a random color */
		c = s[randint0(strlen(s))];

		/* Lookup this color */
		a = my_strchr(color_char, c) - color_char;

		/* Invalid color (note check for < 0 removed, gave a silly
		 * warning because bytes are always >= 0 -- RG) */
		if (a > 15) return (TERM_WHITE);

		/* Use this color */
		return (a);
	}

	/* Standard "color" */
	return (TERM_WHITE);
}


/*
 * Find the attr/char pair to use for a spell effect
 *
 * It is moving (or has moved) from (x,y) to (nx,ny).
 *
 * If the distance is not "one", we (may) return "*".
 */
u16b bolt_pict(int y, int x, int ny, int nx, int typ)
{
	int base;

	byte k;

	byte a;
	char c;

	/* No motion (*) */
	if ((ny == y) && (nx == x)) base = 0x30;

	/* Vertical (|) */
	else if (nx == x) base = 0x40;

	/* Horizontal (-) */
	else if (ny == y) base = 0x50;

	/* Diagonal (/) */
	else if ((ny - y) == (x - nx)) base = 0x60;

	/* Diagonal (\) */
	else if ((ny - y) == (nx - x)) base = 0x70;

	/* Weird (*) */
	else base = 0x30;

	/* Basic spell color */
	k = spell_color(typ);

	/* Obtain attr/char */
	a = misc_to_attr[base + k];
	c = misc_to_char[base + k];

	/* Create pict */
	return (PICT(a, c));
}


/*
 * Determine the path taken by a projection.
 *
 * The projection will always start from the grid (y1,x1), and will travel
 * towards the grid (y2,x2), touching one grid per unit of distance along
 * the major axis, and stopping when it enters the destination grid or a
 * wall grid, or has travelled the maximum legal distance of "range".
 *
 * Note that "distance" in this function (as in the "update_view()" code)
 * is defined as "MAX(dy,dx) + MIN(dy,dx)/2", which means that the player
 * actually has an "octagon of projection" not a "circle of projection".
 *
 * The path grids are saved into the grid array pointed to by "gp", and
 * there should be room for at least "range" grids in "gp".  Note that
 * due to the way in which distance is calculated, this function normally
 * uses fewer than "range" grids for the projection path, so the result
 * of this function should never be compared directly to "range".  Note
 * that the initial grid (y1,x1) is never saved into the grid array, not
 * even if the initial grid is also the final grid.  XXX XXX XXX
 *
 * The "flg" flags can be used to modify the behavior of this function.
 *
 * In particular, the "PROJECT_STOP" and "PROJECT_THRU" flags have the same
 * semantics as they do for the "project" function, namely, that the path
 * will stop as soon as it hits a monster, or that the path will continue
 * through the destination grid, respectively.
 *
 * The "PROJECT_JUMP" flag, which for the "project()" function means to
 * start at a special grid (which makes no sense in this function), means
 * that the path should be "angled" slightly if needed to avoid any wall
 * grids, allowing the player to "target" any grid which is in "view".
 * This flag is non-trivial and has not yet been implemented, but could
 * perhaps make use of the "vinfo" array (above).  XXX XXX XXX
 *
 * This function returns the number of grids (if any) in the path.  This
 * function will return zero if and only if (y1,x1) and (y2,x2) are equal.
 *
 * This algorithm is similar to, but slightly different from, the one used
 * by "update_view_los()", and very different from the one used by "los()".
 */
sint project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg)
{
	int y, x;

	int n = 0;
	int k = 0;

	/* Absolute */
	int ay, ax;

	/* Offsets */
	int sy, sx;

	/* Fractions */
	int frac;

	/* Scale factors */
	int full, half;

	/* Slope */
	int m;

	/* No path necessary (or allowed) */
	if ((x1 == x2) && (y1 == y2)) return (0);


	/* Analyze "dy" */
	if (y2 < y1)
	{
		ay = (y1 - y2);
		sy = -1;
	}
	else
	{
		ay = (y2 - y1);
		sy = 1;
	}

	/* Analyze "dx" */
	if (x2 < x1)
	{
		ax = (x1 - x2);
		sx = -1;
	}
	else
	{
		ax = (x2 - x1);
		sx = 1;
	}


	/* Number of "units" in one "half" grid */
	half = (ay * ax);

	/* Number of "units" in one "full" grid */
	full = half << 1;

	/* Vertical */
	if (ay > ax)
	{
		/* Let m = ((dx/dy) * full) = (dx * dx * 2) */
		m = ax * ax * 2;

		/* Start */
		y = y1 + sy;
		x = x1;

		frac = m;

		if (frac > half)
		{
			/* Advance (X) part 2 */
			x += sx;

			/* Advance (X) part 3 */
			frac -= full;

			/* Track distance */
			k++;
		}

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			if (flg & (PROJECT_DISI))
			{
				if ((n > 0) && cave_stop_disintegration(y, x)) break;
			}
			else if (flg & (PROJECT_LOS))
			{
				if ((n > 0) && !cave_los_bold(y, x)) break;
			}
			else if (!(flg & (PROJECT_PATH)))
			{
				/* Always stop at non-initial wall grids */
				if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) break;
			}

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) &&
				    (player_bold(y, x) || cave[y][x].m_idx != 0))
					break;
			}

			if (!in_bounds(y, x)) break;

			/* Slant */
			if (m)
			{
				/* Advance (X) part 1 */
				frac += m;

				/* Horizontal change */
				if (frac > half)
				{
					/* Advance (X) part 2 */
					x += sx;

					/* Advance (X) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (Y) */
			y += sy;
		}
	}

	/* Horizontal */
	else if (ax > ay)
	{
		/* Let m = ((dy/dx) * full) = (dy * dy * 2) */
		m = ay * ay * 2;

		/* Start */
		y = y1;
		x = x1 + sx;

		frac = m;

		/* Vertical change */
		if (frac > half)
		{
			/* Advance (Y) part 2 */
			y += sy;

			/* Advance (Y) part 3 */
			frac -= full;

			/* Track distance */
			k++;
		}

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			if (flg & (PROJECT_DISI))
			{
				if ((n > 0) && cave_stop_disintegration(y, x)) break;
			}
			else if (flg & (PROJECT_LOS))
			{
				if ((n > 0) && !cave_los_bold(y, x)) break;
			}
			else if (!(flg & (PROJECT_PATH)))
			{
				/* Always stop at non-initial wall grids */
				if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) break;
			}

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) &&
				    (player_bold(y, x) || cave[y][x].m_idx != 0))
					break;
			}

			if (!in_bounds(y, x)) break;

			/* Slant */
			if (m)
			{
				/* Advance (Y) part 1 */
				frac += m;

				/* Vertical change */
				if (frac > half)
				{
					/* Advance (Y) part 2 */
					y += sy;

					/* Advance (Y) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (X) */
			x += sx;
		}
	}

	/* Diagonal */
	else
	{
		/* Start */
		y = y1 + sy;
		x = x1 + sx;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if ((n + (n >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			if (flg & (PROJECT_DISI))
			{
				if ((n > 0) && cave_stop_disintegration(y, x)) break;
			}
			else if (flg & (PROJECT_LOS))
			{
				if ((n > 0) && !cave_los_bold(y, x)) break;
			}
			else if (!(flg & (PROJECT_PATH)))
			{
				/* Always stop at non-initial wall grids */
				if ((n > 0) && !cave_have_flag_bold(y, x, FF_PROJECT)) break;
			}

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) &&
				    (player_bold(y, x) || cave[y][x].m_idx != 0))
					break;
			}

			if (!in_bounds(y, x)) break;

			/* Advance (Y) */
			y += sy;

			/* Advance (X) */
			x += sx;
		}
	}

	/* Length */
	return (n);
}



/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;
/* Mega-Hack -- monsters target */
static s16b monster_target_x;
static s16b monster_target_y;


/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * XXX XXX XXX Perhaps we should affect doors?
 */
static bool project_f(int who, int r, int y, int x, int dam, int typ)
{
	cave_type       *c_ptr = &cave[y][x];
	feature_type    *f_ptr = &f_info[c_ptr->feat];

	bool obvious = FALSE;
	bool known = player_has_los_bold(y, x);


	/* XXX XXX XXX */
	who = who ? who : 0;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	if (have_flag(f_ptr->flags, FF_TREE))
	{
		cptr message;
		switch (typ)
		{
		case GF_POIS:
		case GF_NUKE:
		case GF_DEATH_RAY:
#ifdef JP
			message = "�Ϥ줿";break;
#else
			message = "was blasted.";break;
#endif
		case GF_TIME:
#ifdef JP
			message = "�̤��";break;
#else
			message = "shrank.";break;
#endif
		case GF_ACID:
#ifdef JP
			message = "�Ϥ���";break;
#else
			message = "melted.";break;
#endif
		case GF_COLD:
		case GF_ICE:
#ifdef JP
			message = "��ꡢ�դ����ä�";break;
#else
			message = "was frozen and smashed.";break;
#endif
		case GF_FIRE:
		case GF_ELEC:
		case GF_PLASMA:
#ifdef JP
			message = "ǳ����";break;
#else
			message = "burns up!";break;
#endif
		case GF_METEOR:
		case GF_CHAOS:
		case GF_MANA:
		case GF_SEEKER:
		case GF_SUPER_RAY:
		case GF_SHARDS:
		case GF_ROCKET:
		case GF_SOUND:
		case GF_DISENCHANT:
		case GF_FORCE:
		case GF_GRAVITY:
#ifdef JP
			message = "ʴ�դ��줿";break;
#else
			message = "was crushed.";break;
#endif
		default:
			message = NULL;break;
		}
		if (message)
		{
#ifdef JP
			msg_format("�ڤ�%s��", message);
#else
			msg_format("A tree %s", message);
#endif
			cave_set_feat(y, x, one_in_(3) ? feat_brake : feat_grass);

			/* Observe */
			if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;
		}
	}

	/* Analyze the type */
	switch (typ)
	{
		/* Ignore most effects */
		case GF_CAPTURE:
		case GF_HAND_DOOM:
		case GF_CAUSE_1:
		case GF_CAUSE_2:
		case GF_CAUSE_3:
		case GF_CAUSE_4:
		case GF_MIND_BLAST:
		case GF_BRAIN_SMASH:
		case GF_DRAIN_MANA:
		case GF_PSY_SPEAR:
		case GF_FORCE:
		case GF_HOLY_FIRE:
		case GF_HELL_FIRE:
		case GF_PSI:
		case GF_PSI_DRAIN:
		case GF_TELEKINESIS:
		case GF_DOMINATION:
		case GF_IDENTIFY:
		case GF_ATTACK:
		case GF_ACID:
		case GF_ELEC:
		case GF_COLD:
		case GF_ICE:
		case GF_FIRE:
		case GF_PLASMA:
		case GF_METEOR:
		case GF_CHAOS:
		case GF_MANA:
		case GF_SEEKER:
		case GF_SUPER_RAY:
		{
			break;
		}

		/* Destroy Traps (and Locks) */
		case GF_KILL_TRAP:
		{
			/* Reveal secret doors */
			if (is_hidden_door(c_ptr))
			{
				/* Pick a door */
				disclose_grid(y, x);

				/* Check line of sight */
				if (known)
				{
					obvious = TRUE;
				}
			}

			/* Destroy traps */
			if (is_trap(c_ptr->feat))
			{
				/* Check line of sight */
				if (known)
				{
#ifdef JP
					msg_print("�ޤФ椤���������ä���");
#else
					msg_print("There is a bright flash of light!");
#endif

					obvious = TRUE;
				}

				/* Destroy the trap */
				cave_alter_feat(y, x, FF_DISARM);
			}

			/* Locked doors are unlocked */
			if (is_closed_door(c_ptr->feat) && f_ptr->power && have_flag(f_ptr->flags, FF_OPEN))
			{
				s16b old_feat = c_ptr->feat;

				/* Unlock the door */
				cave_alter_feat(y, x, FF_DISARM);

				/* Check line of sound */
				if (known && (old_feat != c_ptr->feat))
				{
#ifdef JP
					msg_print("�����äȲ���������");
#else
					msg_print("Click!");
#endif

					obvious = TRUE;
				}
			}

			/* Remove "unsafe" flag if player is not blind */
			if (!p_ptr->blind && player_has_los_bold(y, x))
			{
				c_ptr->info &= ~(CAVE_UNSAFE);

				/* Redraw */
				lite_spot(y, x);

				obvious = TRUE;
			}

			break;
		}

		/* Destroy Doors (and traps) */
		case GF_KILL_DOOR:
		{
			/* Destroy all doors and traps */
			if (is_trap(c_ptr->feat) || have_flag(f_ptr->flags, FF_DOOR))
			{
				/* Check line of sight */
				if (known)
				{
					/* Message */
#ifdef JP
					msg_print("�ޤФ椤���������ä���");
#else
					msg_print("There is a bright flash of light!");
#endif

					obvious = TRUE;
				}

				/* Destroy the feature */
				cave_alter_feat(y, x, FF_TUNNEL);
			}

			/* Remove "unsafe" flag if player is not blind */
			if (!p_ptr->blind && player_has_los_bold(y, x))
			{
				c_ptr->info &= ~(CAVE_UNSAFE);

				/* Redraw */
				lite_spot(y, x);

				obvious = TRUE;
			}

			break;
		}

		case GF_JAM_DOOR: /* Jams a door (as if with a spike) */
		{
			if (have_flag(f_ptr->flags, FF_SPIKE))
			{
				s16b old_mimic = c_ptr->mimic;
				feature_type *mimic_f_ptr = &f_info[get_feat_mimic(c_ptr)];

				cave_alter_feat(y, x, FF_SPIKE);

				c_ptr->mimic = old_mimic;

				/* Notice */
				note_spot(y, x);

				/* Redraw */
				lite_spot(y, x);

				/* Check line of sight */
				if (known && have_flag(mimic_f_ptr->flags, FF_OPEN))
				{
					/* Message */
#ifdef JP
					msg_format("%s�˲������Ĥä����Ƴ����ʤ��ʤä���", f_name + mimic_f_ptr->name);
#else
					msg_format("The %s seems stuck.", f_name + mimic_f_ptr->name);
#endif

					obvious = TRUE;
				}
			}
			break;
		}

		/* Destroy walls (and doors) */
		case GF_KILL_WALL:
		{
			if (have_flag(f_ptr->flags, FF_HURT_ROCK))
			{
				/* Message */
				if (known && (c_ptr->info & (CAVE_MARK)))
				{
#ifdef JP
					msg_format("%s���Ϥ���ť�ˤʤä���", f_name + f_info[get_feat_mimic(c_ptr)].name);
#else
					msg_format("The %s turns into mud!", f_name + f_info[get_feat_mimic(c_ptr)].name);
#endif

					obvious = TRUE;
				}

				/* Destroy the wall */
				cave_alter_feat(y, x, FF_HURT_ROCK);

				/* Update some things */
				p_ptr->update |= (PU_FLOW);
			}

			break;
		}

		/* Make doors */
		case GF_MAKE_DOOR:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Not on the player */
			if (player_bold(y, x)) break;

			/* Create a closed door */
			cave_set_feat(y, x, feat_door[DOOR_DOOR].closed);

			/* Observe */
			if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;

			break;
		}

		/* Make traps */
		case GF_MAKE_TRAP:
		{
			/* Place a trap */
			place_trap(y, x);

			break;
		}

		/* Make doors */
		case GF_MAKE_TREE:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Not on the player */
			if (player_bold(y, x)) break;

			/* Create a closed door */
			cave_set_feat(y, x, feat_tree);

			/* Observe */
			if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;


			break;
		}

		case GF_MAKE_GLYPH:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Create a glyph */
			c_ptr->info |= CAVE_OBJECT;
			c_ptr->mimic = feat_glyph;

			/* Notice */
			note_spot(y, x);

			/* Redraw */
			lite_spot(y, x);

			break;
		}

		case GF_STONE_WALL:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Not on the player */
			if (player_bold(y, x)) break;

			/* Place a wall */
			cave_set_feat(y, x, feat_granite);

			break;
		}


		case GF_LAVA_FLOW:
		{
			/* Ignore permanent grid */
			if (have_flag(f_ptr->flags, FF_PERMANENT)) break;

			/* Shallow Lava */
			if (dam == 1)
			{
				/* Ignore grid without enough space */
				if (!have_flag(f_ptr->flags, FF_FLOOR)) break;

				/* Place a shallow lava */
				cave_set_feat(y, x, feat_shallow_lava);
			}
			/* Deep Lava */
			else if (dam)
			{
				/* Place a deep lava */
				cave_set_feat(y, x, feat_deep_lava);
			}
			break;
		}

		case GF_WATER_FLOW:
		{
			/* Ignore permanent grid */
			if (have_flag(f_ptr->flags, FF_PERMANENT)) break;

			/* Shallow Water */
			if (dam == 1)
			{
				/* Ignore grid without enough space */
				if (!have_flag(f_ptr->flags, FF_FLOOR)) break;

				/* Place a shallow water */
				cave_set_feat(y, x, feat_shallow_water);
			}
			/* Deep Water */
			else if (dam)
			{
				/* Place a deep water */
				cave_set_feat(y, x, feat_deep_water);
			}
			break;
		}

		/* Lite up the grid */
		case GF_LITE_WEAK:
		case GF_LITE:
		{
			/* Turn on the light */
			if (!(d_info[dungeon_type].flags1 & DF1_DARKNESS))
			{
				c_ptr->info |= (CAVE_GLOW);

				/* Notice */
				note_spot(y, x);

				/* Redraw */
				lite_spot(y, x);

				update_local_illumination(y, x);

				/* Observe */
				if (player_can_see_bold(y, x)) obvious = TRUE;

				/* Mega-Hack -- Update the monster in the affected grid */
				/* This allows "spear of light" (etc) to work "correctly" */
				if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

				if (p_ptr->special_defense & NINJA_S_STEALTH)
				{
					if (player_bold(y, x)) set_superstealth(FALSE);
				}
			}

			break;
		}

		/* Darken the grid */
		case GF_DARK_WEAK:
		case GF_DARK:
		{
			bool do_dark = !p_ptr->inside_battle && !is_mirror_grid(c_ptr);
			int j;

			/* Turn off the light. */
			if (do_dark)
			{
				if (dun_level || !is_daytime())
				{
					for (j = 0; j < 9; j++)
					{
						int by = y + ddy_ddd[j];
						int bx = x + ddx_ddd[j];

						if (in_bounds2(by, bx))
						{
							cave_type *cc_ptr = &cave[by][bx];

							if (have_flag(f_info[get_feat_mimic(cc_ptr)].flags, FF_GLOW))
							{
								do_dark = FALSE;
								break;
							}
						}
					}

					if (!do_dark) break;
				}

				c_ptr->info &= ~(CAVE_GLOW);

				/* Hack -- Forget "boring" grids */
				if (!have_flag(f_ptr->flags, FF_REMEMBER))
				{
					/* Forget */
					c_ptr->info &= ~(CAVE_MARK);

					/* Notice */
					note_spot(y, x);
				}

				/* Redraw */
				lite_spot(y, x);

				update_local_illumination(y, x);

				/* Notice */
				if (player_can_see_bold(y, x)) obvious = TRUE;

				/* Mega-Hack -- Update the monster in the affected grid */
				/* This allows "spear of light" (etc) to work "correctly" */
				if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);
			}

			/* All done */
			break;
		}

		case GF_SHARDS:
		case GF_ROCKET:
		{
			if (is_mirror_grid(c_ptr))
			{
#ifdef JP
				msg_print("������줿��");
#else
				msg_print("The mirror was crashed!");
#endif
				sound(SOUND_GLASS);
				remove_mirror(y, x);
				project(0, 2, y, x, p_ptr->lev / 2 + 5, GF_SHARDS, (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP | PROJECT_NO_HANGEKI), -1);
			}

			if (have_flag(f_ptr->flags, FF_GLASS) && !have_flag(f_ptr->flags, FF_PERMANENT) && (dam >= 50))
			{
				/* Message */
				if (known && (c_ptr->info & CAVE_MARK))
				{
#ifdef JP
					msg_format("%s����줿��", f_name + f_info[get_feat_mimic(c_ptr)].name);
#else
					msg_format("The %s was crashed!", f_name + f_info[get_feat_mimic(c_ptr)].name);
#endif
					sound(SOUND_GLASS);
				}

				/* Destroy the wall */
				cave_alter_feat(y, x, FF_HURT_ROCK);

				/* Update some things */
				p_ptr->update |= (PU_FLOW);
			}
			break;
		}

		case GF_SOUND:
		{
			if (is_mirror_grid(c_ptr) && p_ptr->lev < 40)
			{
#ifdef JP
				msg_print("������줿��");
#else
				msg_print("The mirror was crashed!");
#endif
				sound(SOUND_GLASS);
				remove_mirror(y, x);
				project(0, 2, y, x, p_ptr->lev / 2 + 5, GF_SHARDS, (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP | PROJECT_NO_HANGEKI), -1);
			}

			if (have_flag(f_ptr->flags, FF_GLASS) && !have_flag(f_ptr->flags, FF_PERMANENT) && (dam >= 200))
			{
				/* Message */
				if (known && (c_ptr->info & CAVE_MARK))
				{
#ifdef JP
					msg_format("%s����줿��", f_name + f_info[get_feat_mimic(c_ptr)].name);
#else
					msg_format("The %s was crashed!", f_name + f_info[get_feat_mimic(c_ptr)].name);
#endif
					sound(SOUND_GLASS);
				}

				/* Destroy the wall */
				cave_alter_feat(y, x, FF_HURT_ROCK);

				/* Update some things */
				p_ptr->update |= (PU_FLOW);
			}
			break;
		}

		case GF_DISINTEGRATE:
		{
			/* Destroy mirror/glyph */
			if (is_mirror_grid(c_ptr) || is_glyph_grid(c_ptr) || is_explosive_rune_grid(c_ptr))
				remove_mirror(y, x);

			/* Permanent features don't get effect */
			/* But not protect monsters and other objects */
			if (have_flag(f_ptr->flags, FF_HURT_DISI) && !have_flag(f_ptr->flags, FF_PERMANENT))
			{
				cave_alter_feat(y, x, FF_HURT_DISI);

				/* Update some things -- similar to GF_KILL_WALL */
				p_ptr->update |= (PU_FLOW);
			}
			break;
		}
	}

	lite_spot(y, x);
	/* Return "Anything seen?" */
	return (obvious);
}



/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
static bool project_o(int who, int r, int y, int x, int dam, int typ)
{
	cave_type *c_ptr = &cave[y][x];

	s16b this_o_idx, next_o_idx = 0;

	bool obvious = FALSE;
	bool known = player_has_los_bold(y, x);

	u32b flgs[TR_FLAG_SIZE];

	char o_name[MAX_NLEN];

	int k_idx = 0;
	bool is_potion = FALSE;


	/* XXX XXX XXX */
	who = who ? who : 0;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Scan all objects in the grid */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		bool is_art = FALSE;
		bool ignore = FALSE;
		bool do_kill = FALSE;

		cptr note_kill = NULL;

#ifndef JP
		/* Get the "plural"-ness */
		bool plural = (o_ptr->number > 1);
#endif

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Extract the flags */
		object_flags(o_ptr, flgs);

		/* Check for artifact */
		if (object_is_artifact(o_ptr)) is_art = TRUE;

		/* Analyze the type */
		switch (typ)
		{
			/* Acid -- Lots of things */
			case GF_ACID:
			{
				if (hates_acid(o_ptr))
				{
					do_kill = TRUE;
#ifdef JP
note_kill = "ͻ���Ƥ��ޤä���";
#else
					note_kill = (plural ? " melt!" : " melts!");
#endif

					if (have_flag(flgs, TR_IGNORE_ACID)) ignore = TRUE;
				}
				break;
			}

			/* Elec -- Rings and Wands */
			case GF_ELEC:
			{
				if (hates_elec(o_ptr))
				{
					do_kill = TRUE;
#ifdef JP
note_kill = "����Ƥ��ޤä���";
#else
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
#endif

					if (have_flag(flgs, TR_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire -- Flammable objects */
			case GF_FIRE:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
#ifdef JP
note_kill = "ǳ���Ƥ��ޤä���";
#else
					note_kill = (plural ? " burn up!" : " burns up!");
#endif

					if (have_flag(flgs, TR_IGNORE_FIRE)) ignore = TRUE;
				}
				break;
			}

			/* Cold -- potions and flasks */
			case GF_COLD:
			{
				if (hates_cold(o_ptr))
				{
#ifdef JP
note_kill = "�դ����äƤ��ޤä���";
#else
					note_kill = (plural ? " shatter!" : " shatters!");
#endif

					do_kill = TRUE;
					if (have_flag(flgs, TR_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Elec */
			case GF_PLASMA:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
#ifdef JP
note_kill = "ǳ���Ƥ��ޤä���";
#else
					note_kill = (plural ? " burn up!" : " burns up!");
#endif

					if (have_flag(flgs, TR_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_elec(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
#ifdef JP
note_kill = "����Ƥ��ޤä���";
#else
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
#endif

					if (have_flag(flgs, TR_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Cold */
			case GF_METEOR:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
#ifdef JP
note_kill = "ǳ���Ƥ��ޤä���";
#else
					note_kill = (plural ? " burn up!" : " burns up!");
#endif

					if (have_flag(flgs, TR_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_cold(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
#ifdef JP
note_kill = "�դ����äƤ��ޤä���";
#else
					note_kill = (plural ? " shatter!" : " shatters!");
#endif

					if (have_flag(flgs, TR_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Hack -- break potions and such */
			case GF_ICE:
			case GF_SHARDS:
			case GF_FORCE:
			case GF_SOUND:
			{
				if (hates_cold(o_ptr))
				{
#ifdef JP
note_kill = "�դ����äƤ��ޤä���";
#else
					note_kill = (plural ? " shatter!" : " shatters!");
#endif

					do_kill = TRUE;
				}
				break;
			}

			/* Mana and Chaos -- destroy everything */
			case GF_MANA:
			case GF_SEEKER:
			case GF_SUPER_RAY:
			{
				do_kill = TRUE;
#ifdef JP
note_kill = "����Ƥ��ޤä���";
#else
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
#endif

				break;
			}

			case GF_DISINTEGRATE:
			{
				do_kill = TRUE;
#ifdef JP
note_kill = "��ȯ���Ƥ��ޤä���";
#else
				note_kill = (plural ? " evaporate!" : " evaporates!");
#endif

				break;
			}

			case GF_CHAOS:
			{
				do_kill = TRUE;
#ifdef JP
note_kill = "����Ƥ��ޤä���";
#else
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
#endif

				if (have_flag(flgs, TR_RES_CHAOS)) ignore = TRUE;
				else if ((o_ptr->tval == TV_SCROLL) && (o_ptr->sval == SV_SCROLL_CHAOS)) ignore = TRUE;
				break;
			}

			/* Holy Fire and Hell Fire -- destroys cursed non-artifacts */
			case GF_HOLY_FIRE:
			case GF_HELL_FIRE:
			{
				if (object_is_cursed(o_ptr))
				{
					do_kill = TRUE;
#ifdef JP
note_kill = "����Ƥ��ޤä���";
#else
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
#endif

				}
				break;
			}

			case GF_IDENTIFY:
			{
				identify_item(o_ptr);

				/* Auto-inscription */
				autopick_alter_item((-this_o_idx), FALSE);
				break;
			}

			/* Unlock chests */
			case GF_KILL_TRAP:
			case GF_KILL_DOOR:
			{
				/* Chests are noticed only if trapped or locked */
				if (o_ptr->tval == TV_CHEST)
				{
					/* Disarm/Unlock traps */
					if (o_ptr->pval > 0)
					{
						/* Disarm or Unlock */
						o_ptr->pval = (0 - o_ptr->pval);

						/* Identify */
						object_known(o_ptr);

						/* Notice */
						if (known && (o_ptr->marked & OM_FOUND))
						{
#ifdef JP
msg_print("�����äȲ���������");
#else
							msg_print("Click!");
#endif

							obvious = TRUE;
						}
					}
				}

				break;
			}
			case GF_ANIM_DEAD:
			{
				if (o_ptr->tval == TV_CORPSE)
				{
					int i;
					u32b mode = 0L;

					if (!who || is_pet(&m_list[who]))
						mode |= PM_FORCE_PET;

					for (i = 0; i < o_ptr->number ; i++)
					{
						if (((o_ptr->sval == SV_CORPSE) && (randint1(100) > 80)) ||
						    ((o_ptr->sval == SV_SKELETON) && (randint1(100) > 60)))
						{
							if (!note_kill)
							{
#ifdef JP
note_kill = "���ˤʤä���";
#else
					note_kill = (plural ? " become dust." : " becomes dust.");
#endif
							}
							continue;
						}
						else if (summon_named_creature(who, y, x, o_ptr->pval, mode))
						{
#ifdef JP
note_kill = "�����֤ä���";
#else
					note_kill = " revived.";
#endif
						}
						else if (!note_kill)
						{
#ifdef JP
note_kill = "���ˤʤä���";
#else
							note_kill = (plural ? " become dust." : " becomes dust.");
#endif
						}
					}
					do_kill = TRUE;
					obvious = TRUE;
				}
				break;
			}
		}


		/* Attempt to destroy the object */
		if (do_kill)
		{
			/* Effect "observed" */
			if (known && (o_ptr->marked & OM_FOUND))
			{
				obvious = TRUE;
				object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
			}

			/* Artifacts, and other objects, get to resist */
			if (is_art || ignore)
			{
				/* Observe the resist */
				if (known && (o_ptr->marked & OM_FOUND))
				{
#ifdef JP
msg_format("%s�ϱƶ�������ʤ���",
   o_name);
#else
					msg_format("The %s %s unaffected!",
							o_name, (plural ? "are" : "is"));
#endif

				}
			}

			/* Kill it */
			else
			{
				/* Describe if needed */
				if (known && (o_ptr->marked & OM_FOUND) && note_kill)
				{
#ifdef JP
msg_format("%s��%s", o_name, note_kill);
#else
					msg_format("The %s%s", o_name, note_kill);
#endif

				}

				k_idx = o_ptr->k_idx;
				is_potion = object_is_potion(o_ptr);


				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Potions produce effects when 'shattered' */
				if (is_potion)
				{
					(void)potion_smash_effect(who, y, x, k_idx);
				}

				/* Redraw */
				lite_spot(y, x);
			}
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint1(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint1(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint1(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint1(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint1(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 */
/* "flg" was added. */
static bool project_m(int who, int r, int y, int x, int dam, int typ, int flg, bool see_s_msg)
{
	int tmp;

	cave_type *c_ptr = &cave[y][x];

	monster_type *m_ptr = &m_list[c_ptr->m_idx];
	monster_type *caster_ptr = (who > 0) ? &m_list[who] : NULL;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	char killer[80];

	/* Is the monster "seen"? */
	bool seen = m_ptr->ml;
	bool seen_msg = is_seen(m_ptr);

	bool slept = (bool)MON_CSLEEP(m_ptr);

	/* Were the effects "obvious" (if seen)? */
	bool obvious = FALSE;

	/* Can the player know about this effect? */
	bool known = ((m_ptr->cdis <= MAX_SIGHT) || p_ptr->inside_battle);

	/* Were the effects "irrelevant"? */
	bool skipped = FALSE;

	/* Gets the monster angry at the source of the effect? */
	bool get_angry = FALSE;

	/* Polymorph setting (true or false) */
	bool do_poly = FALSE;

	/* Teleport setting (max distance) */
	int do_dist = 0;

	/* Confusion setting (amount to confuse) */
	int do_conf = 0;

	/* Stunning setting (amount to stun) */
	int do_stun = 0;

	/* Sleep amount (amount to sleep) */
	int do_sleep = 0;

	/* Fear amount (amount to fear) */
	int do_fear = 0;

	/* Time amount (amount to time) */
	int do_time = 0;

	bool heal_leper = FALSE;

	/* Hold the monster name */
	char m_name[80];

#ifndef JP
	char m_poss[10];
#endif

	int photo = 0;

	/* Assume no note */
	cptr note = NULL;

	/* Assume a default death */
	cptr note_dies = extract_note_dies(real_r_ptr(m_ptr));

	int ty = m_ptr->fy;
	int tx = m_ptr->fx;

	int caster_lev = (who > 0) ? r_info[caster_ptr->r_idx].level : (p_ptr->lev * 2);

	/* Nobody here */
	if (!c_ptr->m_idx) return (FALSE);

	/* Never affect projector */
	if (who && (c_ptr->m_idx == who)) return (FALSE);
	if ((c_ptr->m_idx == p_ptr->riding) && !who && !(typ == GF_OLD_HEAL) && !(typ == GF_OLD_SPEED) && !(typ == GF_STAR_HEAL)) return (FALSE);
	if (sukekaku && ((m_ptr->r_idx == MON_SUKE) || (m_ptr->r_idx == MON_KAKU))) return FALSE;

	/* Don't affect already death monsters */
	/* Prevents problems with chain reactions of exploding monsters */
	if (m_ptr->hp < 0) return (FALSE);

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Get the monster name (BEFORE polymorphing) */
	monster_desc(m_name, m_ptr, 0);

#ifndef JP
	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);
#endif


	if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) disturb(1, 0);

	/* Analyze the damage type */
	switch (typ)
	{
		/* Magic Missile -- pure damage */
		case GF_MISSILE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			break;
		}

		/* Acid */
		case GF_ACID:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_IM_ACID)
			{
#ifdef JP
note = "�ˤϤ��ʤ����������롪";
#else
				note = " resists a lot.";
#endif

				dam /= 9;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_ACID);
			}
			break;
		}

		/* Electricity */
		case GF_ELEC:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_IM_ELEC)
			{
#ifdef JP
note = "�ˤϤ��ʤ����������롪";
#else
				note = " resists a lot.";
#endif

				dam /= 9;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_ELEC);
			}
			break;
		}

		/* Fire damage */
		case GF_FIRE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_IM_FIRE)
			{
#ifdef JP
note = "�ˤϤ��ʤ����������롪";
#else
				note = " resists a lot.";
#endif

				dam /= 9;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_FIRE);
			}
			else if (r_ptr->flags3 & (RF3_HURT_FIRE))
			{
#ifdef JP
note = "�ϤҤɤ��˼�򤦤�����";
#else
				note = " is hit hard.";
#endif

				dam *= 2;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_FIRE);
			}
			break;
		}

		/* Cold */
		case GF_COLD:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_IM_COLD)
			{
#ifdef JP
note = "�ˤϤ��ʤ����������롪";
#else
				note = " resists a lot.";
#endif

				dam /= 9;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_COLD);
			}
			else if (r_ptr->flags3 & (RF3_HURT_COLD))
			{
#ifdef JP
note = "�ϤҤɤ��˼�򤦤�����";
#else
				note = " is hit hard.";
#endif

				dam *= 2;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_COLD);
			}
			break;
		}

		/* Poison */
		case GF_POIS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_IM_POIS)
			{
#ifdef JP
note = "�ˤϤ��ʤ����������롪";
#else
				note = " resists a lot.";
#endif

				dam /= 9;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_POIS);
			}
			break;
		}

		/* Nuclear waste */
		case GF_NUKE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_IM_POIS)
			{
#ifdef JP
note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_POIS);
			}
			else if (one_in_(3)) do_poly = TRUE;
			break;
		}

		/* Hellfire -- hurts Evil */
		case GF_HELL_FIRE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags3 & RF3_GOOD)
			{
				dam *= 2;
#ifdef JP
note = "�ϤҤɤ��˼���������";
#else
				note = " is hit hard.";
#endif

				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_GOOD);
			}
			break;
		}

		/* Holy Fire -- hurts Evil, Good are immune, others _resist_ */
		case GF_HOLY_FIRE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags3 & RF3_GOOD)
			{
				dam = 0;
#ifdef JP
note = "�ˤϴ��������������롣";
#else
				note = " is immune.";
#endif

				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_GOOD;
			}
			else if (r_ptr->flags3 & RF3_EVIL)
			{
				dam *= 2;
#ifdef JP
note = "�ϤҤɤ��˼���������";
#else
				note = " is hit hard.";
#endif

				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= RF3_EVIL;
			}
			else
			{
#ifdef JP
note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
			}
			break;
		}

		/* Arrow -- XXX no defense */
		case GF_ARROW:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			break;
		}

		/* Plasma -- XXX perhaps check ELEC or FIRE */
		case GF_PLASMA:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_PLAS)
			{
#ifdef JP
note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_PLAS);
			}
			break;
		}

		/* Nether -- see above */
		case GF_NETHER:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_NETH)
			{
				if (r_ptr->flags3 & RF3_UNDEAD)
				{
#ifdef JP
					note = "�ˤϴ��������������롣";
#else
					note = " is immune.";
#endif

					dam = 0;
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);
				}
				else
				{
#ifdef JP
					note = "�ˤ����������롣";
#else
					note = " resists.";
#endif

					dam *= 3; dam /= randint1(6) + 6;
				}
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_NETH);
			}
			else if (r_ptr->flags3 & RF3_EVIL)
			{
				dam /= 2;
#ifdef JP
				note = "�Ϥ����餫�����򼨤�����";
#else
				note = " resists somewhat.";
#endif

				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);
			}
			break;
		}

		/* Water (acid) damage -- Water spirits/elementals are immune */
		case GF_WATER:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_WATE)
			{
				if ((m_ptr->r_idx == MON_WATER_ELEM) || (m_ptr->r_idx == MON_UNMAKER))
				{
#ifdef JP
					note = "�ˤϴ��������������롣";
#else
					note = " is immune.";
#endif

					dam = 0;
				}
				else
				{
#ifdef JP
					note = "�ˤ����������롣";
#else
					note = " resists.";
#endif

					dam *= 3; dam /= randint1(6) + 6;
				}
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_WATE);
			}
			break;
		}

		/* Chaos -- Chaos breathers resist */
		case GF_CHAOS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_CHAO)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_CHAO);
			}
			else if ((r_ptr->flags3 & RF3_DEMON) && one_in_(3))
			{
#ifdef JP
				note = "�Ϥ����餫�����򼨤�����";
#else
				note = " resists somewhat.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_DEMON);
			}
			else
			{
				do_poly = TRUE;
				do_conf = (5 + randint1(11) + r) / (r + 1);
			}
			break;
		}

		/* Shards -- Shard breathers resist */
		case GF_SHARDS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_SHAR)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_SHAR);
			}
			break;
		}

		/* Rocket: Shard resistance helps */
		case GF_ROCKET:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_SHAR)
			{
#ifdef JP
				note = "�Ϥ����餫�����򼨤�����";
#else
				note = " resists somewhat.";
#endif

				dam /= 2;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_SHAR);
			}
			break;
		}


		/* Sound -- Sound breathers resist */
		case GF_SOUND:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_SOUN)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 2; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_SOUN);
			}
			else do_stun = (10 + randint1(15) + r) / (r + 1);
			break;
		}

		/* Confusion */
		case GF_CONFUSION:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags3 & RF3_NO_CONF)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
			}
			else do_conf = (10 + randint1(15) + r) / (r + 1);
			break;
		}

		/* Disenchantment -- Breathers and Disenchanters resist */
		case GF_DISENCHANT:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_DISE)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_DISE);
			}
			break;
		}

		/* Nexus -- Breathers and Existers resist */
		case GF_NEXUS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_NEXU)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_NEXU);
			}
			break;
		}

		/* Force */
		case GF_FORCE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_WALL)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_WALL);
			}
			else do_stun = (randint1(15) + r) / (r + 1);
			break;
		}

		/* Inertia -- breathers resist */
		case GF_INERTIA:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_INER)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_INER);
			}
			else
			{
				/* Powerful monsters can resist */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					obvious = FALSE;
				}
				/* Normal monsters slow down */
				else
				{
					if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
					{
#ifdef JP
						note = "��ư�����٤��ʤä���";
#else
						note = " starts moving slower.";
#endif
					}
				}
			}
			break;
		}

		/* Time -- breathers resist */
		case GF_TIME:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_TIME)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_TIME);
			}
			else do_time = (dam + 1) / 2;
			break;
		}

		/* Gravity -- breathers resist */
		case GF_GRAVITY:
		{
			bool resist_tele = FALSE;

			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_TELE)
			{
				if (r_ptr->flags1 & (RF1_UNIQUE))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					resist_tele = TRUE;
				}
				else if (r_ptr->level > randint1(100))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
note = "�ˤ����������롪";
#else
					note = " resists!";
#endif

					resist_tele = TRUE;
				}
			}

			if (!resist_tele) do_dist = 10;
			else do_dist = 0;
			if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_dist = 0;

			if (r_ptr->flagsr & RFR_RES_GRAV)
			{
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif

				dam *= 3; dam /= randint1(6) + 6;
				do_dist = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_GRAV);
			}
			else
			{
				/* 1. slowness */
				/* Powerful monsters can resist */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					obvious = FALSE;
				}
				/* Normal monsters slow down */
				else
				{
					if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
					{
#ifdef JP
						note = "��ư�����٤��ʤä���";
#else
						note = " starts moving slower.";
#endif
					}
				}

				/* 2. stun */
				do_stun = damroll((caster_lev / 20) + 3 , (dam)) + 1;

				/* Attempt a saving throw */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					/* Resist */
					do_stun = 0;
					/* No obvious effect */
#ifdef JP
					note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
				}
			}
			break;
		}

		/* Pure damage */
		case GF_MANA:
		case GF_SEEKER:
		case GF_SUPER_RAY:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			break;
		}


		/* Pure damage */
		case GF_DISINTEGRATE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags3 & RF3_HURT_ROCK)
			{
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_ROCK);
#ifdef JP
note = "�����椬�����줿��";
note_dies = "�Ͼ�ȯ������";
#else
				note = " loses some skin!";
				note_dies = " evaporates!";
#endif

				dam *= 2;
			}
			break;
		}

		case GF_PSI:
		{
			if (seen) obvious = TRUE;

			/* PSI only works if the monster can see you! -- RG */
			if (!(los(m_ptr->fy, m_ptr->fx, py, px)))
			{
#ifdef JP
				if (seen_msg) msg_format("%s�Ϥ��ʤ��������ʤ��ΤǱƶ�����ʤ���", m_name);
#else
				if (seen_msg) msg_format("%^s can't see you, and isn't affected!", m_name);
#endif
				skipped = TRUE;
				break;
			}

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				dam = 0;
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune!";
#endif
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);

			}
			else if ((r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
			         (r_ptr->flags3 & RF3_ANIMAL) ||
			         (r_ptr->level > randint1(3 * dam)))
			{
				dam /= 3;
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif


				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
				    (r_ptr->level > p_ptr->lev / 2) &&
				    one_in_(2))
				{
					note = NULL;
#ifdef JP
					msg_format("%^s������������Ϲ����ķ���֤�����", m_name);
#else
					msg_format("%^s%s corrupted mind backlashes your attack!",
					    m_name, (seen ? "'s" : "s"));
#endif

					/* Saving throw */
					if ((randint0(100 + r_ptr->level / 2) < p_ptr->skill_sav) && !CHECK_MULTISHADOW())
					{
#ifdef JP
						msg_print("���������Ϥ�ķ���֤�����");
#else
						msg_print("You resist the effects!");
#endif

					}
					else
					{
						/* Injure +/- confusion */
						monster_desc(killer, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
						take_hit(DAMAGE_ATTACK, dam, killer, -1);  /* has already been /3 */
						if (one_in_(4) && !CHECK_MULTISHADOW())
						{
							switch (randint1(4))
							{
								case 1:
									set_confused(p_ptr->confused + 3 + randint1(dam));
									break;
								case 2:
									set_stun(p_ptr->stun + randint1(dam));
									break;
								case 3:
								{
									if (r_ptr->flags3 & RF3_NO_FEAR)
#ifdef JP
										note = "�ˤϸ��̤��ʤ��ä���";
#else
										note = " is unaffected.";
#endif

									else
										set_afraid(p_ptr->afraid + 3 + randint1(dam));
									break;
								}
								default:
									if (!p_ptr->free_act)
										(void)set_paralyzed(p_ptr->paralyzed + randint1(dam));
									break;
							}
						}
					}
					dam = 0;
				}
			}

			if ((dam > 0) && one_in_(4))
			{
				switch (randint1(4))
				{
					case 1:
						do_conf = 3 + randint1(dam);
						break;
					case 2:
						do_stun = 3 + randint1(dam);
						break;
					case 3:
						do_fear = 3 + randint1(dam);
						break;
					default:
#ifdef JP
						note = "��̲�����Ǥ��ޤä���";
#else
						note = " falls asleep!";
#endif

						do_sleep = 3 + randint1(dam);
						break;
				}
			}

#ifdef JP
			note_dies = "�������������������Τ�ȴ���̤Ȥʤä���";
#else
			note_dies = " collapses, a mindless husk.";
#endif

			break;
		}

		case GF_PSI_DRAIN:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				dam = 0;
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune!";
#endif

			}
			else if ((r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
			         (r_ptr->flags3 & RF3_ANIMAL) ||
			         (r_ptr->level > randint1(3 * dam)))
			{
				dam /= 3;
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif


				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
				     (r_ptr->level > p_ptr->lev / 2) &&
				     (one_in_(2)))
				{
					note = NULL;
#ifdef JP
					msg_format("%^s������������Ϲ����ķ���֤�����", m_name);
#else
					msg_format("%^s%s corrupted mind backlashes your attack!",
					    m_name, (seen ? "'s" : "s"));
#endif

					/* Saving throw */
					if ((randint0(100 + r_ptr->level / 2) < p_ptr->skill_sav) && !CHECK_MULTISHADOW())
					{
#ifdef JP
						msg_print("���ʤ��ϸ��Ϥ�ķ���֤�����");
#else
						msg_print("You resist the effects!");
#endif
					}
					else
					{
						/* Injure + mana drain */
						monster_desc(killer, m_ptr, MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
						if (!CHECK_MULTISHADOW())
						{
#ifdef JP
							msg_print("Ķǽ�ϥѥ��ۤ��Ȥ�줿��");
#else
							msg_print("Your psychic energy is drained!");
#endif

							p_ptr->csp -= damroll(5, dam) / 2;
							if (p_ptr->csp < 0) p_ptr->csp = 0;
							p_ptr->redraw |= PR_MANA;
							p_ptr->window |= (PW_SPELL);
						}
						take_hit(DAMAGE_ATTACK, dam, killer, -1);  /* has already been /3 */
					}
					dam = 0;
				}
			}
			else if (dam > 0)
			{
				int b = damroll(5, dam) / 4;
#ifdef JP
				msg_format("���ʤ���%s�ζ��ˤ�Ķǽ�ϥѥ���Ѵ�������", m_name);
#else
				msg_format("You convert %s%s pain into psychic energy!",
				    m_name, (seen ? "'s" : "s"));
#endif

				b = MIN(p_ptr->msp, p_ptr->csp + b);
				p_ptr->csp = b;
				p_ptr->redraw |= PR_MANA;
				p_ptr->window |= (PW_SPELL);
			}

#ifdef JP
			note_dies = "�������������������Τ�ȴ���̤Ȥʤä���";
#else
			note_dies = " collapses, a mindless husk.";
#endif

			break;
		}

		case GF_TELEKINESIS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (one_in_(4))
			{
				if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_dist = 0;
				else do_dist = 7;
			}

			/* 1. stun */
			do_stun = damroll((caster_lev / 20) + 3 , dam) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			    (r_ptr->level > 5 + randint1(dam)))
			{
				/* Resist */
				do_stun = 0;
				/* No obvious effect */
				obvious = FALSE;
			}
			break;
		}

		/* Psycho-spear -- powerful magic missile */
		case GF_PSY_SPEAR:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			break;
		}

		/* Meteor -- powerful magic missile */
		case GF_METEOR:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			break;
		}

		case GF_DOMINATION:
		{
			if (!is_hostile(m_ptr)) break;

			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) ||
			    (r_ptr->flags3 & RF3_NO_CONF) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & RF3_NO_CONF)
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
				    (r_ptr->level > p_ptr->lev / 2) &&
				    (one_in_(2)))
				{
					note = NULL;
#ifdef JP
					msg_format("%^s������������Ϲ����ķ���֤�����", m_name);
#else
					msg_format("%^s%s corrupted mind backlashes your attack!",
					    m_name, (seen ? "'s" : "s"));
#endif

					/* Saving throw */
					if (randint0(100 + r_ptr->level/2) < p_ptr->skill_sav)
					{
#ifdef JP
						msg_print("���������Ϥ�ķ���֤�����");
#else
						msg_print("You resist the effects!");
#endif

					}
					else
					{
						/* Confuse, stun, terrify */
						switch (randint1(4))
						{
							case 1:
								set_stun(p_ptr->stun + dam / 2);
								break;
							case 2:
								set_confused(p_ptr->confused + dam / 2);
								break;
							default:
							{
								if (r_ptr->flags3 & RF3_NO_FEAR)
#ifdef JP
									note = "�ˤϸ��̤��ʤ��ä���";
#else
									note = " is unaffected.";
#endif

								else
									set_afraid(p_ptr->afraid + dam);
							}
						}
					}
				}
				else
				{
					/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
				}
			}
			else
			{
				if ((dam > 29) && (randint1(100) < dam))
				{
#ifdef JP
note = "�����ʤ�����°������";
#else
					note = " is in your thrall!";
#endif

					set_pet(m_ptr);
				}
				else
				{
					switch (randint1(4))
					{
						case 1:
							do_stun = dam / 2;
							break;
						case 2:
							do_conf = dam / 2;
							break;
						default:
							do_fear = dam;
					}
				}
			}

			/* No "real" damage */
			dam = 0;
			break;
		}



		/* Ice -- Cold + Cuts + Stun */
		case GF_ICE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			do_stun = (randint1(15) + 1) / (r + 1);
			if (r_ptr->flagsr & RFR_IM_COLD)
			{
#ifdef JP
				note = "�ˤϤ��ʤ����������롣";
#else
				note = " resists a lot.";
#endif

				dam /= 9;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_IM_COLD);
			}
			else if (r_ptr->flags3 & (RF3_HURT_COLD))
			{
#ifdef JP
				note = "�ϤҤɤ��˼�򤦤�����";
#else
				note = " is hit hard.";
#endif

				dam *= 2;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_COLD);
			}
			break;
		}


		/* Drain Life */
		case GF_OLD_DRAIN:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (!monster_living(r_ptr))
			{
				if (is_original_ap_and_seen(m_ptr))
				{
					if (r_ptr->flags3 & RF3_DEMON) r_ptr->r_flags3 |= (RF3_DEMON);
					if (r_ptr->flags3 & RF3_UNDEAD) r_ptr->r_flags3 |= (RF3_UNDEAD);
					if (r_ptr->flags3 & RF3_NONLIVING) r_ptr->r_flags3 |= (RF3_NONLIVING);
				}

#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
				dam = 0;
			}
			else do_time = (dam+7)/8;

			break;
		}

		/* Death Ray */
		case GF_DEATH_RAY:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (!monster_living(r_ptr))
			{
				if (is_original_ap_and_seen(m_ptr))
				{
					if (r_ptr->flags3 & RF3_DEMON) r_ptr->r_flags3 |= (RF3_DEMON);
					if (r_ptr->flags3 & RF3_UNDEAD) r_ptr->r_flags3 |= (RF3_UNDEAD);
					if (r_ptr->flags3 & RF3_NONLIVING) r_ptr->r_flags3 |= (RF3_NONLIVING);
				}

#ifdef JP
				note = "�ˤϴ��������������롣";
#else
				note = " is immune.";
#endif

				obvious = FALSE;
				dam = 0;
			}
			else if (((r_ptr->flags1 & RF1_UNIQUE) &&
				 (randint1(888) != 666)) ||
				 (((r_ptr->level + randint1(20)) > randint1((caster_lev / 2) + randint1(10))) &&
				 randint1(100) != 66))
			{
#ifdef JP
note = "�ˤ����������롪";
#else
				note = " resists!";
#endif

				obvious = FALSE;
				dam = 0;
			}

			break;
		}

		/* Polymorph monster (Use "dam" as "power") */
		case GF_OLD_POLY:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Attempt to polymorph (see below) */
			do_poly = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			    (r_ptr->flags1 & RF1_QUESTOR) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				do_poly = FALSE;
				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Clone monsters (Ignore "dam") */
		case GF_OLD_CLONE:
		{
			if (seen) obvious = TRUE;

			if (is_pet(m_ptr) || (r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) || (r_ptr->flags7 & (RF7_NAZGUL | RF7_UNIQUE2)))
			{
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
 note = " is unaffected!";
#endif
			}
			else
			{
				/* Heal fully */
				m_ptr->hp = m_ptr->maxhp;

				/* Attempt to clone. */
				if (multiply_monster(c_ptr->m_idx, TRUE, 0L))
				{
#ifdef JP
note = "��ʬ��������";
#else
					note = " spawns!";
#endif

				}
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Heal Monster (use "dam" as amount of healing) */
		case GF_STAR_HEAL:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			(void)set_monster_csleep(c_ptr->m_idx, 0);

			if (m_ptr->maxhp < m_ptr->max_maxhp)
			{
#ifdef JP
				if (seen_msg) msg_format("%^s�ζ�������ä���", m_name);
#else
				if (seen_msg) msg_format("%^s recovers %s vitality.", m_name, m_poss);
#endif
				m_ptr->maxhp = m_ptr->max_maxhp;
			}

			if (!dam)
			{
				/* Redraw (later) if needed */
				if (p_ptr->health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);
				if (p_ptr->riding == c_ptr->m_idx) p_ptr->redraw |= (PR_UHEALTH);
				break;
			}

			/* Fall through */
		}
		case GF_OLD_HEAL:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			(void)set_monster_csleep(c_ptr->m_idx, 0);
			if (MON_STUNNED(m_ptr))
			{
#ifdef JP
				if (seen_msg) msg_format("%^s��ۯ۰���֤���Ω��ľ�ä���", m_name);
#else
				if (seen_msg) msg_format("%^s is no longer stunned.", m_name);
#endif
				(void)set_monster_stunned(c_ptr->m_idx, 0);
			}
			if (MON_CONFUSED(m_ptr))
			{
#ifdef JP
				if (seen_msg) msg_format("%^s�Ϻ��𤫤�Ω��ľ�ä���", m_name);
#else
				if (seen_msg) msg_format("%^s is no longer confused.", m_name);
#endif
				(void)set_monster_confused(c_ptr->m_idx, 0);
			}
			if (MON_MONFEAR(m_ptr))
			{
#ifdef JP
				if (seen_msg) msg_format("%^s��ͦ�������ᤷ����", m_name);
#else
				if (seen_msg) msg_format("%^s recovers %s courage.", m_name, m_poss);
#endif
				(void)set_monster_monfear(c_ptr->m_idx, 0);
			}

			/* Heal */
			if (m_ptr->hp < 30000) m_ptr->hp += dam;

			/* No overflow */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			if (!who)
			{
				chg_virtue(V_VITALITY, 1);

				if (r_ptr->flags1 & RF1_UNIQUE)
					chg_virtue(V_INDIVIDUALISM, 1);

				if (is_friendly(m_ptr))
					chg_virtue(V_HONOUR, 1);
				else if (!(r_ptr->flags3 & RF3_EVIL))
				{
					if (r_ptr->flags3 & RF3_GOOD)
						chg_virtue(V_COMPASSION, 2);
					else
						chg_virtue(V_COMPASSION, 1);
				}

				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(V_NATURE, 1);
			}

			if (m_ptr->r_idx == MON_LEPER)
			{
				heal_leper = TRUE;
				if (!who) chg_virtue(V_COMPASSION, 5);
			}

			/* Redraw (later) if needed */
			if (p_ptr->health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);
			if (p_ptr->riding == c_ptr->m_idx) p_ptr->redraw |= (PR_UHEALTH);

			/* Message */
#ifdef JP
			note = "�����Ϥ���������褦����";
#else
			note = " looks healthier.";
#endif

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Speed Monster (Ignore "dam") */
		case GF_OLD_SPEED:
		{
			if (seen) obvious = TRUE;

			/* Speed up */
			if (set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100))
			{
#ifdef JP
				note = "��ư����®���ʤä���";
#else
				note = " starts moving faster.";
#endif
			}

			if (!who)
			{
				if (r_ptr->flags1 & RF1_UNIQUE)
					chg_virtue(V_INDIVIDUALISM, 1);
				if (is_friendly(m_ptr))
					chg_virtue(V_HONOUR, 1);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Slow Monster (Use "dam" as "power") */
		case GF_OLD_SLOW:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
			}

			/* Normal monsters slow down */
			else
			{
				if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
				{
#ifdef JP
					note = "��ư�����٤��ʤä���";
#else
					note = " starts moving slower.";
#endif
				}
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Sleep (Use "dam" as "power") */
		case GF_OLD_SLEEP:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			    (r_ptr->flags3 & RF3_NO_SLEEP) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & RF3_NO_SLEEP)
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_SLEEP);
				}

				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
			}
			else
			{
				/* Go to sleep (much) later */
#ifdef JP
note = "��̲�����Ǥ��ޤä���";
#else
				note = " falls asleep!";
#endif

				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Sleep (Use "dam" as "power") */
		case GF_STASIS_EVIL:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			    !(r_ptr->flags3 & RF3_EVIL) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
			}
			else
			{
				/* Go to sleep (much) later */
#ifdef JP
note = "��ư���ʤ��ʤä���";
#else
				note = " is suspended!";
#endif

				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Sleep (Use "dam" as "power") */
		case GF_STASIS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
			}
			else
			{
				/* Go to sleep (much) later */
#ifdef JP
note = "��ư���ʤ��ʤä���";
#else
				note = " is suspended!";
#endif

				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Charm monster */
		case GF_CHARM:
		{
			int vir;
			dam += (adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1);
			vir = virtue_number(V_HARMONY);
			if (vir)
			{
				dam += p_ptr->virtues[vir-1]/10;
			}

			vir = virtue_number(V_INDIVIDUALISM);
			if (vir)
			{
				dam -= p_ptr->virtues[vir-1]/20;
			}

			if (seen) obvious = TRUE;

			if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
				dam = dam * 2 / 3;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_QUESTOR) ||
			    (r_ptr->flags3 & RF3_NO_CONF) ||
			    (m_ptr->mflag2 & MFLAG2_NOPET) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 5))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & RF3_NO_CONF)
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (p_ptr->cursed & TRC_AGGRAVATE)
			{
#ifdef JP
note = "�Ϥ��ʤ���Ũ�դ������Ƥ��롪";
#else
				note = " hates you too much!";
#endif

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
#ifdef JP
note = "������ͧ��Ū�ˤʤä��褦����";
#else
				note = " suddenly seems friendly!";
#endif

				set_pet(m_ptr);

				chg_virtue(V_INDIVIDUALISM, -1);
				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(V_NATURE, 1);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Control undead */
		case GF_CONTROL_UNDEAD:
		{
			int vir;
			if (seen) obvious = TRUE;

			vir = virtue_number(V_UNLIFE);
			if (vir)
			{
				dam += p_ptr->virtues[vir-1]/10;
			}

			vir = virtue_number(V_INDIVIDUALISM);
			if (vir)
			{
				dam -= p_ptr->virtues[vir-1]/20;
			}

			if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
				dam = dam * 2 / 3;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_QUESTOR) ||
			  (!(r_ptr->flags3 & RF3_UNDEAD)) ||
			    (m_ptr->mflag2 & MFLAG2_NOPET) ||
				 (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (p_ptr->cursed & TRC_AGGRAVATE)
			{
#ifdef JP
note = "�Ϥ��ʤ���Ũ�դ������Ƥ��롪";
#else
				note = " hates you too much!";
#endif

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
#ifdef JP
note = "�ϴ��ˤ��ʤ����������";
#else
				note = " is in your thrall!";
#endif

				set_pet(m_ptr);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Control demon */
		case GF_CONTROL_DEMON:
		{
			int vir;
			if (seen) obvious = TRUE;

			vir = virtue_number(V_UNLIFE);
			if (vir)
			{
				dam += p_ptr->virtues[vir-1]/10;
			}

			vir = virtue_number(V_INDIVIDUALISM);
			if (vir)
			{
				dam -= p_ptr->virtues[vir-1]/20;
			}

			if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
				dam = dam * 2 / 3;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_QUESTOR) ||
			  (!(r_ptr->flags3 & RF3_DEMON)) ||
			    (m_ptr->mflag2 & MFLAG2_NOPET) ||
				 (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (p_ptr->cursed & TRC_AGGRAVATE)
			{
#ifdef JP
note = "�Ϥ��ʤ���Ũ�դ������Ƥ��롪";
#else
				note = " hates you too much!";
#endif

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
#ifdef JP
note = "�ϴ��ˤ��ʤ����������";
#else
				note = " is in your thrall!";
#endif

				set_pet(m_ptr);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Tame animal */
		case GF_CONTROL_ANIMAL:
		{
			int vir;

			if (seen) obvious = TRUE;

			vir = virtue_number(V_NATURE);
			if (vir)
			{
				dam += p_ptr->virtues[vir-1]/10;
			}

			vir = virtue_number(V_INDIVIDUALISM);
			if (vir)
			{
				dam -= p_ptr->virtues[vir-1]/20;
			}

			if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
				dam = dam * 2 / 3;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_QUESTOR)) ||
			  (!(r_ptr->flags3 & (RF3_ANIMAL))) ||
			    (m_ptr->mflag2 & MFLAG2_NOPET) ||
				 (r_ptr->flags3 & (RF3_NO_CONF)) ||
				 (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (p_ptr->cursed & TRC_AGGRAVATE)
			{
#ifdef JP
note = "�Ϥ��ʤ���Ũ�դ������Ƥ��롪";
#else
				note = " hates you too much!";
#endif

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
#ifdef JP
note = "�ϤʤĤ�����";
#else
				note = " is tamed!";
#endif

				set_pet(m_ptr);

				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(V_NATURE, 1);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Tame animal */
		case GF_CONTROL_LIVING:
		{
			int vir;

			vir = virtue_number(V_UNLIFE);
			if (seen) obvious = TRUE;

			dam += (adj_chr_chm[p_ptr->stat_ind[A_CHR]]);
			vir = virtue_number(V_UNLIFE);
			if (vir)
			{
				dam -= p_ptr->virtues[vir-1]/10;
			}

			vir = virtue_number(V_INDIVIDUALISM);
			if (vir)
			{
				dam -= p_ptr->virtues[vir-1]/20;
			}

			if (r_ptr->flags3 & (RF3_NO_CONF)) dam -= 30;
			if (dam < 1) dam = 1;
#ifdef JP
msg_format("%s�򸫤Ĥ᤿��",m_name);
#else
			msg_format("You stare into %s.", m_name);
#endif
			if ((r_ptr->flagsr & RFR_RES_ALL) || p_ptr->inside_arena)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags7 & RF7_NAZGUL))
				dam = dam * 2 / 3;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_QUESTOR)) ||
			    (m_ptr->mflag2 & MFLAG2_NOPET) ||
				 !monster_living(r_ptr) ||
				 ((r_ptr->level+10) > randint1(dam)))
			{
				/* Resist */
				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (p_ptr->cursed & TRC_AGGRAVATE)
			{
#ifdef JP
note = "�Ϥ��ʤ���Ũ�դ������Ƥ��롪";
#else
				note = " hates you too much!";
#endif

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
#ifdef JP
note = "����ۤ�����";
#else
				note = " is tamed!";
#endif

				set_pet(m_ptr);

				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(V_NATURE, 1);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Confusion (Use "dam" as "power") */
		case GF_OLD_CONF:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			/* Get confused later */
			do_conf = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			    (r_ptr->flags3 & (RF3_NO_CONF)) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		case GF_STUN:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			do_stun = damroll((caster_lev / 20) + 3 , (dam)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				do_stun = 0;

				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}




		/* Lite, but only hurts susceptible creatures */
		case GF_LITE_WEAK:
		{
			if (!dam)
			{
				skipped = TRUE;
				break;
			}
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				dam = 0;
				break;
			}
			/* Hurt by light */
			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				/* Obvious effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_LITE);

				/* Special effect */
#ifdef JP
note = "�ϸ��˿Ȥ򤹤��᤿��";
note_dies = "�ϸ�������Ƥ��ܤ�Ǥ��ޤä���";
#else
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
#endif

			}

			/* Normally no damage */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}



		/* Lite -- opposite of Dark */
		case GF_LITE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_LITE)
			{
#ifdef JP
				note = "�ˤ����������롪";
#else
				note = " resists.";
#endif

				dam *= 2; dam /= (randint1(6)+6);
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_LITE);
			}
			else if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_LITE);
#ifdef JP
				note = "�ϸ��˿Ȥ򤹤��᤿��";
				note_dies = "�ϸ�������Ƥ��ܤ�Ǥ��ޤä���";
#else
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
#endif

				dam *= 2;
			}
			break;
		}


		/* Dark -- opposite of Lite */
		case GF_DARK:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flagsr & RFR_RES_DARK)
			{
#ifdef JP
				note = "�ˤ����������롪";
#else
				note = " resists.";
#endif

				dam *= 2; dam /= (randint1(6)+6);
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_DARK);
			}
			break;
		}


		/* Stone to Mud */
		case GF_KILL_WALL:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				dam = 0;
				break;
			}
			/* Hurt by rock remover */
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
			{
				/* Notice effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_ROCK);

				/* Cute little message */
#ifdef JP
note = "�����椬�����줿��";
note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
				note = " loses some skin!";
				note_dies = " dissolves!";
#endif

			}

			/* Usually, ignore the effects */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Teleport undead (Use "dam" as "power") */
		case GF_AWAY_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				bool resists_tele = FALSE;

				if (r_ptr->flagsr & RFR_RES_TELE)
				{
					if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
					{
						if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
						note = " is unaffected!";
#endif

						resists_tele = TRUE;
					}
					else if (r_ptr->level > randint1(100))
					{
						if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
note = "�ˤ����������롪";
#else
						note = " resists!";
#endif

						resists_tele = TRUE;
					}
				}

				if (!resists_tele)
				{
					if (seen) obvious = TRUE;
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);
					do_dist = dam;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport evil (Use "dam" as "power") */
		case GF_AWAY_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				bool resists_tele = FALSE;

				if (r_ptr->flagsr & RFR_RES_TELE)
				{
					if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
					{
						if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
						note = " is unaffected!";
#endif

						resists_tele = TRUE;
					}
					else if (r_ptr->level > randint1(100))
					{
						if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
note = "�ˤ����������롪";
#else
						note = " resists!";
#endif

						resists_tele = TRUE;
					}
				}

				if (!resists_tele)
				{
					if (seen) obvious = TRUE;
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);
					do_dist = dam;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport monster (Use "dam" as "power") */
		case GF_AWAY_ALL:
		{
			bool resists_tele = FALSE;
			if (r_ptr->flagsr & RFR_RES_TELE)
			{
				if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
					note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					resists_tele = TRUE;
				}
				else if (r_ptr->level > randint1(100))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
#ifdef JP
					note = "�ˤ����������롪";
#else
					note = " resists!";
#endif

					resists_tele = TRUE;
				}
			}

			if (!resists_tele)
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Prepare to teleport */
				do_dist = dam;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn undead (Use "dam" as "power") */
		case GF_TURN_UNDEAD:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				break;
			}
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					/* No obvious effect */
#ifdef JP
					note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
					do_fear = 0;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn evil (Use "dam" as "power") */
		case GF_TURN_EVIL:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				break;
			}
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					/* No obvious effect */
#ifdef JP
					note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
					do_fear = 0;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn monster (Use "dam" as "power") */
		case GF_TURN_ALL:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				break;
			}
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Apply some fear */
			do_fear = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			    (r_ptr->flags3 & (RF3_NO_FEAR)) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif

				obvious = FALSE;
				do_fear = 0;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Dispel undead */
		case GF_DISP_UNDEAD:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				dam = 0;
				break;
			}
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);

				/* Message */
#ifdef JP
				note = "�Ͽȿ̤�������";
				note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
				note = " shudders.";
				note_dies = " dissolves!";
#endif
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel evil */
		case GF_DISP_EVIL:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				dam = 0;
				break;
			}
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);

				/* Message */
#ifdef JP
				note = "�Ͽȿ̤�������";
				note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
				note = " shudders.";
				note_dies = " dissolves!";
#endif
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel good */
		case GF_DISP_GOOD:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				dam = 0;
				break;
			}
			/* Only affect good */
			if (r_ptr->flags3 & (RF3_GOOD))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_GOOD);

				/* Message */
#ifdef JP
				note = "�Ͽȿ̤�������";
				note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
				note = " shudders.";
				note_dies = " dissolves!";
#endif
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel living */
		case GF_DISP_LIVING:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				dam = 0;
				break;
			}
			/* Only affect non-undead */
			if (monster_living(r_ptr))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
#ifdef JP
note = "�Ͽȿ̤�������";
note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
				note = " shudders.";
				note_dies = " dissolves!";
#endif

			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel demons */
		case GF_DISP_DEMON:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				dam = 0;
				break;
			}
			/* Only affect demons */
			if (r_ptr->flags3 & (RF3_DEMON))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_DEMON);

				/* Message */
#ifdef JP
				note = "�Ͽȿ̤�������";
				note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
				note = " shudders.";
				note_dies = " dissolves!";
#endif
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel monster */
		case GF_DISP_ALL:
		{
			if (r_ptr->flagsr & RFR_RES_ALL)
			{
				skipped = TRUE;
				dam = 0;
				break;
			}
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Message */
#ifdef JP
note = "�Ͽȿ̤�������";
note_dies = "�ϥɥ�ɥ���Ϥ�����";
#else
			note = " shudders.";
			note_dies = " dissolves!";
#endif


			break;
		}

		/* Drain mana */
		case GF_DRAIN_MANA:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if ((r_ptr->flags4 & ~(RF4_NOMAGIC_MASK)) || (r_ptr->flags5 & ~(RF5_NOMAGIC_MASK)) || (r_ptr->flags6 & ~(RF6_NOMAGIC_MASK)))
			{
				if (who > 0)
				{
					/* Heal the monster */
					if (caster_ptr->hp < caster_ptr->maxhp)
					{
						/* Heal */
						caster_ptr->hp += 6 * dam;
						if (caster_ptr->hp > caster_ptr->maxhp) caster_ptr->hp = caster_ptr->maxhp;

						/* Redraw (later) if needed */
						if (p_ptr->health_who == who) p_ptr->redraw |= (PR_HEALTH);
						if (p_ptr->riding == who) p_ptr->redraw |= (PR_UHEALTH);

						/* Special message */
						if (see_s_msg)
						{
							/* Get the monster name */
							monster_desc(killer, caster_ptr, 0);
#ifdef JP
							msg_format("%^s�ϵ�ʬ���ɤ���������", killer);
#else
							msg_format("%^s appears healthier.", killer);
#endif
						}
					}
				}
				else
				{
					/* Message */
#ifdef JP
					msg_format("%s�����������ͥ륮����ۤ��Ȥä���", m_name);
#else
					msg_format("You draw psychic energy from %s.", m_name);
#endif

					(void)hp_player(dam);
				}
			}
			else
			{
#ifdef JP
				if (see_s_msg) msg_format("%s�ˤϸ��̤��ʤ��ä���", m_name);
#else
				if (see_s_msg) msg_format("%s is unaffected.", m_name);
#endif
			}
			dam = 0;
			break;
		}

		/* Mind blast */
		case GF_MIND_BLAST:
		{
			if (seen) obvious = TRUE;
			/* Message */
#ifdef JP
			if (!who) msg_format("%s�򤸤ä��ˤ����", m_name);
#else
			if (!who) msg_format("You gaze intently at %s.", m_name);
#endif

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				 (r_ptr->flags3 & RF3_NO_CONF) ||
				 (r_ptr->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune!";
#endif
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_WEIRD_MIND)
			{
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif
				dam /= 3;
			}
			else
			{
#ifdef JP
				note = "����������򿩤�ä���";
				note_dies = "�������������������Τ�ȴ���̤Ȥʤä���";
#else
				note = " is blasted by psionic energy.";
				note_dies = " collapses, a mindless husk.";
#endif

				if (who > 0) do_conf = randint0(4) + 4;
				else do_conf = randint0(8) + 8;
			}
			break;
		}

		/* Brain smash */
		case GF_BRAIN_SMASH:
		{
			if (seen) obvious = TRUE;
			/* Message */
#ifdef JP
			if (!who) msg_format("%s�򤸤ä��ˤ����", m_name);
#else
			if (!who) msg_format("You gaze intently at %s.", m_name);
#endif

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				 (r_ptr->flags3 & RF3_NO_CONF) ||
				 (r_ptr->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune!";
#endif
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_WEIRD_MIND)
			{
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
#ifdef JP
				note = "�ˤ����������롣";
#else
				note = " resists.";
#endif
				dam /= 3;
			}
			else
			{
#ifdef JP
				note = "����������򿩤�ä���";
				note_dies = "�������������������Τ�ȴ���̤Ȥʤä���";
#else
				note = " is blasted by psionic energy.";
				note_dies = " collapses, a mindless husk.";
#endif

				if (who > 0)
				{
					do_conf = randint0(4) + 4;
					do_stun = randint0(4) + 4;
				}
				else
				{
					do_conf = randint0(8) + 8;
					do_stun = randint0(8) + 8;
				}
				(void)set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 10);
			}
			break;
		}

		/* CAUSE_1 */
		case GF_CAUSE_1:
		{
			if (seen) obvious = TRUE;
			/* Message */
#ifdef JP
			if (!who) msg_format("%s��غ����Ƽ����򤫤�����", m_name);
#else
			if (!who) msg_format("You point at %s and curse.", m_name);
#endif

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			break;
		}

		/* CAUSE_2 */
		case GF_CAUSE_2:
		{
			if (seen) obvious = TRUE;
			/* Message */
#ifdef JP
			if (!who) msg_format("%s��غ����ƶ������˼����򤫤�����", m_name);
#else
			if (!who) msg_format("You point at %s and curse horribly.", m_name);
#endif

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			break;
		}

		/* CAUSE_3 */
		case GF_CAUSE_3:
		{
			if (seen) obvious = TRUE;
			/* Message */
#ifdef JP
			if (!who) msg_format("%s��غ������������˼�ʸ�򾧤�����", m_name);
#else
			if (!who) msg_format("You point at %s, incanting terribly!", m_name);
#endif

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			break;
		}

		/* CAUSE_4 */
		case GF_CAUSE_4:
		{
			if (seen) obvious = TRUE;
			/* Message */
#ifdef JP
			if (!who) msg_format("%s���빦���ͤ��ơ��֤����ϴ��˻��Ǥ���פȶ������", m_name);
#else
			if (!who) msg_format("You point at %s, screaming the word, 'DIE!'.", m_name);
#endif

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if ((randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35)) && ((who <= 0) || (caster_ptr->r_idx != MON_KENSHIROU)))
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			break;
		}

		/* HAND_DOOM */
		case GF_HAND_DOOM:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			if (r_ptr->flags1 & RF1_UNIQUE)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			else
			{
				if ((who > 0) ? ((caster_lev + randint1(dam)) > (r_ptr->level + 10 + randint1(20))) :
				   (((caster_lev / 2) + randint1(dam)) > (r_ptr->level + randint1(200))))
				{
					dam = ((40 + randint1(20)) * m_ptr->hp) / 100;

					if (m_ptr->hp < dam) dam = m_ptr->hp - 1;
				}
				else
				{
#ifdef JP
					note = "����������äƤ��롪";
#else
					note = "resists!";
#endif
					dam = 0;
				}
			}
			break;
		}

		/* Capture monster */
		case GF_CAPTURE:
		{
			int nokori_hp;
			if ((p_ptr->inside_quest && (quest[p_ptr->inside_quest].type == QUEST_TYPE_KILL_ALL) && !is_pet(m_ptr)) ||
			    (r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flags7 & (RF7_NAZGUL)) || (r_ptr->flags7 & (RF7_UNIQUE2)) || (r_ptr->flags1 & RF1_QUESTOR) || m_ptr->parent_m_idx)
			{
#ifdef JP
				msg_format("%s�ˤϸ��̤��ʤ��ä���",m_name);
#else
				msg_format("%^s is unaffected.", m_name);
#endif
				skipped = TRUE;
				break;
			}

			if (is_pet(m_ptr)) nokori_hp = m_ptr->maxhp * 4L;
			else if ((p_ptr->pclass == CLASS_BEASTMASTER) && monster_living(r_ptr))
				nokori_hp = m_ptr->maxhp * 3 / 10;
			else
				nokori_hp = m_ptr->maxhp * 3 / 20;

			if (m_ptr->hp >= nokori_hp)
			{
#ifdef JP
				msg_format("��äȼ�餻�ʤ��ȡ�");
#else
				msg_format("You need to weaken %s more.", m_name);
#endif
				skipped = TRUE;
			}
			else if (m_ptr->hp < randint0(nokori_hp))
			{
				if (m_ptr->mflag2 & MFLAG2_CHAMELEON) choose_new_monster(c_ptr->m_idx, FALSE, MON_CHAMELEON);
#ifdef JP
				msg_format("%s���ᤨ����",m_name);
#else
				msg_format("You capture %^s!", m_name);
#endif
				cap_mon = m_ptr->r_idx;
				cap_mspeed = m_ptr->mspeed;
				cap_hp = m_ptr->hp;
				cap_maxhp = m_ptr->max_maxhp;
				cap_nickname = m_ptr->nickname; /* Quark transfer */
				if (c_ptr->m_idx == p_ptr->riding)
				{
					if (rakuba(-1, FALSE))
					{
#ifdef JP
						msg_print("���̤���Ȥ��줿��");
#else
						msg_format("You have fallen from %s.", m_name);
#endif
					}
				}

				delete_monster_idx(c_ptr->m_idx);

				return (TRUE);
			}
			else
			{
#ifdef JP
msg_format("���ޤ���ޤ����ʤ��ä���");
#else
				msg_format("You failed to capture %s.", m_name);
#endif
				skipped = TRUE;
			}
			break;
		}

		/* Attack (Use "dam" as attack type) */
		case GF_ATTACK:
		{
			/* Return this monster's death */
			return py_attack(y, x, dam);
		}

		/* Sleep (Use "dam" as "power") */
		case GF_ENGETSU:
		{
			int effect = 0;
			bool done = TRUE;

			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune!";
#endif
				dam = 0;
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
				break;
			}
			if (MON_CSLEEP(m_ptr))
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune!";
#endif
				dam = 0;
				skipped = TRUE;
				break;
			}

			if (one_in_(5)) effect = 1;
			else if (one_in_(4)) effect = 2;
			else if (one_in_(3)) effect = 3;
			else done = FALSE;

			if (effect == 1)
			{
				/* Powerful monsters can resist */
				if ((r_ptr->flags1 & RF1_UNIQUE) ||
				    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
#ifdef JP
					note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
				}

				/* Normal monsters slow down */
				else
				{
					if (set_monster_slow(c_ptr->m_idx, MON_SLOW(m_ptr) + 50))
					{
#ifdef JP
						note = "��ư�����٤��ʤä���";
#else
						note = " starts moving slower.";
#endif
					}
				}
			}

			else if (effect == 2)
			{
				do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

				/* Attempt a saving throw */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					/* Resist */
					do_stun = 0;

					/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
				}
			}

			else if (effect == 3)
			{
				/* Attempt a saving throw */
				if ((r_ptr->flags1 & RF1_UNIQUE) ||
				    (r_ptr->flags3 & RF3_NO_SLEEP) ||
				    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					/* Memorize a flag */
					if (r_ptr->flags3 & RF3_NO_SLEEP)
					{
						if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_SLEEP);
					}

					/* No obvious effect */
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
					note = " is unaffected!";
#endif

					obvious = FALSE;
				}
				else
				{
					/* Go to sleep (much) later */
#ifdef JP
note = "��̲�����Ǥ��ޤä���";
#else
					note = " falls asleep!";
#endif

					do_sleep = 500;
				}
			}

			if (!done)
			{
#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune!";
#endif
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* GENOCIDE */
		case GF_GENOCIDE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

#ifdef JP
			if (genocide_aux(c_ptr->m_idx, dam, !who, (r_ptr->level + 1) / 2, "��󥹥�������"))
#else
			if (genocide_aux(c_ptr->m_idx, dam, !who, (r_ptr->level + 1) / 2, "Genocide One"))
#endif
			{
#ifdef JP
				if (seen_msg) msg_format("%s�Ͼ��Ǥ�����", m_name);
#else
				if (seen_msg) msg_format("%^s disappered!", m_name);
#endif
				chg_virtue(V_VITALITY, -1);
				return TRUE;
			}

			skipped = TRUE;
			break;
		}

		case GF_PHOTO:
		{
#ifdef JP
			if (!who) msg_format("%s��̿��˻��ä���", m_name);
#else
			if (!who) msg_format("You take a photograph of %s.", m_name);
#endif
			/* Hurt by light */
			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				/* Obvious effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_LITE);

				/* Special effect */
#ifdef JP
				note = "�ϸ��˿Ȥ򤹤��᤿��";
				note_dies = "�ϸ�������Ƥ��ܤ�Ǥ��ޤä���";
#else
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
#endif
			}

			/* Normally no damage */
			else
			{
				/* No damage */
				dam = 0;
			}

			photo = m_ptr->r_idx;

			break;
		}


		/* blood curse */
		case GF_BLOOD_CURSE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				dam = 0;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}
			break;
		}

		case GF_CRUSADE:
		{
			bool success = FALSE;
			if (seen) obvious = TRUE;

			if ((r_ptr->flags3 & (RF3_GOOD)) && !p_ptr->inside_arena)
			{
				if (r_ptr->flags3 & (RF3_NO_CONF)) dam -= 50;
				if (dam < 1) dam = 1;

				/* No need to tame your pet */
				if (is_pet(m_ptr))
				{
#ifdef JP
					note = "��ư����®���ʤä���";
#else
					note = " starts moving faster.";
#endif

					(void)set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100);
					success = TRUE;
				}

				/* Attempt a saving throw */
				else if ((r_ptr->flags1 & (RF1_QUESTOR)) ||
				    (r_ptr->flags1 & (RF1_UNIQUE)) ||
				    (m_ptr->mflag2 & MFLAG2_NOPET) ||
				    (p_ptr->cursed & TRC_AGGRAVATE) ||
					 ((r_ptr->level+10) > randint1(dam)))
				{
					/* Resist */
					if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
				}
				else
				{
#ifdef JP
					note = "����ۤ�����";
#else
					note = " is tamed!";
#endif

					set_pet(m_ptr);
					(void)set_monster_fast(c_ptr->m_idx, MON_FAST(m_ptr) + 100);

					/* Learn about type */
					if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_GOOD);
					success = TRUE;
				}
			}

			if (!success)
			{
				if (!(r_ptr->flags3 & RF3_NO_FEAR))
				{
					do_fear = randint1(90)+10;
				}
				else if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flags3 |= (RF3_NO_FEAR);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		case GF_WOUNDS:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_ALL)
			{
#ifdef JP
				note = "�ˤϴ��������������롪";
#else
				note = " is immune.";
#endif
				skipped = TRUE;
				if (is_original_ap_and_seen(m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
				break;
			}

			/* Attempt a saving throw */
			if (randint0(100 + dam) < (r_ptr->level + 50))
			{

#ifdef JP
note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = "is unaffected!";
#endif
				dam = 0;
			}
			break;
		}

		/* Default */
		default:
		{
			/* Irrelevant */
			skipped = TRUE;

			/* No damage */
			dam = 0;

			break;
		}
	}


	/* Absolutely no effect */
	if (skipped) return (FALSE);

	/* "Unique" monsters cannot be polymorphed */
	if (r_ptr->flags1 & (RF1_UNIQUE)) do_poly = FALSE;

	/* Quest monsters cannot be polymorphed */
	if (r_ptr->flags1 & RF1_QUESTOR) do_poly = FALSE;

	if (p_ptr->riding && (c_ptr->m_idx == p_ptr->riding)) do_poly = FALSE;

	/* "Unique" and "quest" monsters can only be "killed" by the player. */
	if (((r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) || (r_ptr->flags7 & RF7_NAZGUL)) && !p_ptr->inside_battle)
	{
		if (who && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}

	if (!who && slept)
	{
		if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) chg_virtue(V_COMPASSION, -1);
		if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) chg_virtue(V_HONOUR, -1);
	}

	/* Modify the damage */
	tmp = dam;
	dam = mon_damage_mod(m_ptr, dam, (bool)(typ == GF_PSY_SPEAR));
#ifdef JP
	if ((tmp > 0) && (dam == 0)) note = "�ϥ��᡼��������Ƥ��ʤ���";
#else
	if ((tmp > 0) && (dam == 0)) note = " is unharmed.";
#endif

	/* Check for death */
	if (dam > m_ptr->hp)
	{
		/* Extract method of death */
		note = note_dies;
	}
	else
	{
		/* Sound and Impact resisters never stun */
		if (do_stun &&
		    !(r_ptr->flagsr & (RFR_RES_SOUN | RFR_RES_WALL)) &&
		    !(r_ptr->flags3 & RF3_NO_STUN))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Get stunned */
			if (MON_STUNNED(m_ptr))
			{
#ifdef JP
				note = "�ϤҤɤ��⤦���Ȥ�����";
#else
				note = " is more dazed.";
#endif

				tmp = MON_STUNNED(m_ptr) + (do_stun / 2);
			}
			else
			{
#ifdef JP
				note = "�Ϥ⤦���Ȥ�����";
#else
				note = " is dazed.";
#endif

				tmp = do_stun;
			}

			/* Apply stun */
			(void)set_monster_stunned(c_ptr->m_idx, tmp);

			/* Get angry */
			get_angry = TRUE;
		}

		/* Confusion and Chaos resisters (and sleepers) never confuse */
		if (do_conf &&
			 !(r_ptr->flags3 & RF3_NO_CONF) &&
			 !(r_ptr->flagsr & RFR_EFF_RES_CHAO_MASK))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Already partially confused */
			if (MON_CONFUSED(m_ptr))
			{
#ifdef JP
				note = "�Ϥ���˺��𤷤��褦����";
#else
				note = " looks more confused.";
#endif

				tmp = MON_CONFUSED(m_ptr) + (do_conf / 2);
			}

			/* Was not confused */
			else
			{
#ifdef JP
				note = "�Ϻ��𤷤��褦����";
#else
				note = " looks confused.";
#endif

				tmp = do_conf;
			}

			/* Apply confusion */
			(void)set_monster_confused(c_ptr->m_idx, tmp);

			/* Get angry */
			get_angry = TRUE;
		}

		if (do_time)
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			if (do_time >= m_ptr->maxhp) do_time = m_ptr->maxhp - 1;

			if (do_time)
			{
#ifdef JP
				note = "�ϼ夯�ʤä��褦����";
#else
				note = " seems weakened.";
#endif
				m_ptr->maxhp -= do_time;
				if ((m_ptr->hp - dam) > m_ptr->maxhp) dam = m_ptr->hp - m_ptr->maxhp;
			}
			get_angry = TRUE;
		}

		/* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
		if (do_poly && (randint1(90) > r_ptr->level))
		{
			if (polymorph_monster(y, x))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Monster polymorphs */
#ifdef JP
				note = "���ѿȤ�����";
#else
				note = " changes!";
#endif

				/* Turn off the damage */
				dam = 0;
			}
			else
			{
				/* No polymorph */
#ifdef JP
				note = "�ˤϸ��̤��ʤ��ä���";
#else
				note = " is unaffected!";
#endif
			}

			/* Hack -- Get new monster */
			m_ptr = &m_list[c_ptr->m_idx];

			/* Hack -- Get new race */
			r_ptr = &r_info[m_ptr->r_idx];
		}

		/* Handle "teleport" */
		if (do_dist)
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Message */
#ifdef JP
			note = "���ä���ä���";
#else
			note = " disappears!";
#endif

			if (!who) chg_virtue(V_VALOUR, -1);

			/* Teleport */
			teleport_away(c_ptr->m_idx, do_dist,
						(!who ? TELEPORT_DEC_VALOUR : 0L) | TELEPORT_PASSIVE);

			/* Hack -- get new location */
			y = m_ptr->fy;
			x = m_ptr->fx;

			/* Hack -- get new grid */
			c_ptr = &cave[y][x];
		}

		/* Fear */
		if (do_fear)
		{
			/* Set fear */
			(void)set_monster_monfear(c_ptr->m_idx, MON_MONFEAR(m_ptr) + do_fear);

			/* Get angry */
			get_angry = TRUE;
		}
	}

	if (typ == GF_DRAIN_MANA)
	{
		/* Drain mana does nothing */
	}

	/* If another monster did the damage, hurt the monster by hand */
	else if (who)
	{
		/* Redraw (later) if needed */
		if (p_ptr->health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);
		if (p_ptr->riding == c_ptr->m_idx) p_ptr->redraw |= (PR_UHEALTH);

		/* Wake the monster up */
		(void)set_monster_csleep(c_ptr->m_idx, 0);

		/* Hurt the monster */
		m_ptr->hp -= dam;

		/* Dead monster */
		if (m_ptr->hp < 0)
		{
			bool sad = FALSE;

			if (is_pet(m_ptr) && !(m_ptr->ml))
				sad = TRUE;

			/* Give detailed messages if destroyed */
			if (known && note)
			{
				monster_desc(m_name, m_ptr, MD_TRUE_NAME);
				if (see_s_msg)
				{
					msg_format("%^s%s", m_name, note);
				}
				else
				{
					mon_fight = TRUE;
				}
			}

			monster_gain_exp(who, m_ptr->r_idx);

			/* Generate treasure, etc */
			monster_death(c_ptr->m_idx, FALSE);

			/* Delete the monster */
			delete_monster_idx(c_ptr->m_idx);

			if (sad)
			{
#ifdef JP
				msg_print("�����ᤷ����ʬ��������");
#else
				msg_print("You feel sad for a moment.");
#endif
			}
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen_msg) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (see_s_msg)
			{
				message_pain(c_ptr->m_idx, dam);
			}
			else
			{
				mon_fight = TRUE;
			}

			/* Hack -- handle sleep */
			if (do_sleep) (void)set_monster_csleep(c_ptr->m_idx, do_sleep);
		}
	}

	else if (heal_leper)
	{
#ifdef JP
		if (seen_msg) msg_print("�Է���¿ͤ��µ������ä���");
#else
		if (seen_msg) msg_print("The Mangy looking leper is healed!");
#endif

		if (record_named_pet && is_pet(m_ptr) && m_ptr->nickname)
		{
			char m2_name[80];

			monster_desc(m2_name, m_ptr, MD_INDEF_VISIBLE);
			do_cmd_write_nikki(NIKKI_NAMED_PET, RECORD_NAMED_PET_HEAL_LEPER, m2_name);
		}

		delete_monster_idx(c_ptr->m_idx);
	}

	/* If the player did it, give him experience, check fear */
	else
	{
		bool fear = FALSE;

		/* Hurt the monster, check for fear and death */
		if (mon_take_hit(c_ptr->m_idx, dam, &fear, note_dies))
		{
			/* Dead monster */
		}

		/* Damaged monster */
		else
		{
			/* HACK - anger the monster before showing the sleep message */
			if (do_sleep) anger_monster(m_ptr);

			/* Give detailed messages if visible or destroyed */
			if (note && seen_msg)
#ifdef JP
				msg_format("%s%s", m_name, note);
#else
				msg_format("%^s%s", m_name, note);
#endif


			/* Hack -- Pain message */
			else if (known && (dam || !do_fear))
			{
				message_pain(c_ptr->m_idx, dam);
			}

			/* Anger monsters */
			if (((dam > 0) || get_angry) && !do_sleep)
				anger_monster(m_ptr);

			/* Take note */
			if ((fear || do_fear) && seen)
			{
				/* Sound */
				sound(SOUND_FLEE);

				/* Message */
#ifdef JP
				msg_format("%^s�϶��ݤ���ƨ���Ф�����", m_name);
#else
				msg_format("%^s flees in terror!", m_name);
#endif
			}

			/* Hack -- handle sleep */
			if (do_sleep) (void)set_monster_csleep(c_ptr->m_idx, do_sleep);
		}
	}

	if ((typ == GF_BLOOD_CURSE) && one_in_(4))
	{
		int curse_flg = (PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
		int count = 0;
		do
		{
			switch (randint1(28))
			{
			case 1: case 2:
				if (!count)
				{
#ifdef JP
msg_print("���̤��ɤ줿...");
#else
					msg_print("The ground trembles...");
#endif

					earthquake(ty, tx, 4 + randint0(4));
					if (!one_in_(6)) break;
				}
			case 3: case 4: case 5: case 6:
				if (!count)
				{
					int dam = damroll(10, 10);
#ifdef JP
msg_print("�������Ϥμ����ؤ��⤬��������");
#else
					msg_print("A portal opens to a plane of raw mana!");
#endif

					project(0, 8, ty,tx, dam, GF_MANA, curse_flg, -1);
					if (!one_in_(6)) break;
				}
			case 7: case 8:
				if (!count)
				{
#ifdef JP
msg_print("���֤��Ĥ����");
#else
					msg_print("Space warps about you!");
#endif

					if (m_ptr->r_idx) teleport_away(c_ptr->m_idx, damroll(10, 10), TELEPORT_PASSIVE);
					if (one_in_(13)) count += activate_hi_summon(ty, tx, TRUE);
					if (!one_in_(6)) break;
				}
			case 9: case 10: case 11:
#ifdef JP
msg_print("���ͥ륮���Τ��ͤ�򴶤�����");
#else
				msg_print("You feel a surge of energy!");
#endif

				project(0, 7, ty, tx, 50, GF_DISINTEGRATE, curse_flg, -1);
				if (!one_in_(6)) break;
			case 12: case 13: case 14: case 15: case 16:
				aggravate_monsters(0);
				if (!one_in_(6)) break;
			case 17: case 18:
				count += activate_hi_summon(ty, tx, TRUE);
				if (!one_in_(6)) break;
			case 19: case 20: case 21: case 22:
			{
				bool pet = !one_in_(3);
				u32b mode = PM_ALLOW_GROUP;

				if (pet) mode |= PM_FORCE_PET;
				else mode |= (PM_NO_PET | PM_FORCE_FRIENDLY);

				count += summon_specific((pet ? -1 : 0), py, px, (pet ? p_ptr->lev*2/3+randint1(p_ptr->lev/2) : dun_level), 0, mode);
				if (!one_in_(6)) break;
			}
			case 23: case 24: case 25:
				if (p_ptr->hold_life && (randint0(100) < 75)) break;
#ifdef JP
msg_print("��̿�Ϥ��Τ���ۤ����줿�������롪");
#else
				msg_print("You feel your life draining away...");
#endif

				if (p_ptr->hold_life) lose_exp(p_ptr->exp / 160);
				else lose_exp(p_ptr->exp / 16);
				if (!one_in_(6)) break;
			case 26: case 27: case 28:
			{
				int i = 0;
				if (one_in_(13))
				{
					while (i < 6)
					{
						do
						{
							(void)do_dec_stat(i);
						}
						while (one_in_(2));

						i++;
					}
				}
				else
				{
					(void)do_dec_stat(randint0(6));
				}
				break;
			}
			}
		}
		while (one_in_(5));
	}

	if (p_ptr->inside_battle)
	{
		p_ptr->health_who = c_ptr->m_idx;
		p_ptr->redraw |= (PR_HEALTH);
		redraw_stuff();
	}

	/* XXX XXX XXX Verify this code */

	/* Update the monster */
	if (m_ptr->r_idx) update_mon(c_ptr->m_idx, FALSE);

	/* Redraw the monster grid */
	lite_spot(y, x);


	/* Update monster recall window */
	if ((p_ptr->monster_race_idx == m_ptr->r_idx) && (seen || !m_ptr->r_idx))
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}

	if ((dam > 0) && !is_pet(m_ptr) && !is_friendly(m_ptr))
	{
		if (!who)
		{
			if (!(flg & PROJECT_NO_HANGEKI))
			{
				set_target(m_ptr, monster_target_y, monster_target_x);
			}
		}
		else if ((who > 0) && is_pet(caster_ptr) && !player_bold(m_ptr->target_y, m_ptr->target_x))
		{
			set_target(m_ptr, caster_ptr->fy, caster_ptr->fx);
		}
	}

	if (p_ptr->riding && (p_ptr->riding == c_ptr->m_idx) && (dam > 0))
	{
		if (m_ptr->hp > m_ptr->maxhp/3) dam = (dam + 1) / 2;
		rakubadam_m = (dam > 200) ? 200 : dam;
	}


	if (photo)
	{
		object_type *q_ptr;
		object_type forge;

		/* Get local object */
		q_ptr = &forge;

		/* Prepare to make a Blade of Chaos */
		object_prep(q_ptr, lookup_kind(TV_STATUE, SV_PHOTO));

		q_ptr->pval = photo;

		/* Mark the item as fully known */
		q_ptr->ident |= (IDENT_MENTAL);

		/* Drop it in the dungeon */
		(void)drop_near(q_ptr, -1, py, px);
	}

	/* Track it */
	project_m_n++;
	project_m_x = x;
	project_m_y = y;

	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * NOTE (Zangband): 'Bolt' attacks can be reflected back, so we need
 * to know if this is actually a ball or a bolt spell
 *
 *
 * We return "TRUE" if any "obvious" effects were observed.  XXX XXX Actually,
 * we just assume that the effects were obvious, for historical reasons.
 */
static bool project_p(int who, cptr who_name, int r, int y, int x, int dam, int typ, int flg, int monspell)
{
	int k = 0;
	int rlev = 0;

	/* Hack -- assume obvious */
	bool obvious = TRUE;

	/* Player blind-ness */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Player needs a "description" (he is blind) */
	bool fuzzy = FALSE;

	/* Source monster */
	monster_type *m_ptr = NULL;

	/* Monster name (for attacks) */
	char m_name[80];

	/* Monster name (for damage) */
	char killer[80];

	/* Hack -- messages */
	cptr act = NULL;

	int get_damage = 0;


	/* Player is not here */
	if (!player_bold(y, x)) return (FALSE);

	if ((p_ptr->special_defense & NINJA_KAWARIMI) && dam && (randint0(55) < (p_ptr->lev*3/5+20)) && who && (who != p_ptr->riding))
	{
		if (kawarimi(TRUE)) return FALSE;
	}

	/* Player cannot hurt himself */
	if (!who) return (FALSE);
	if (who == p_ptr->riding) return (FALSE);

	if ((p_ptr->reflect || ((p_ptr->special_defense & KATA_FUUJIN) && !p_ptr->blind)) && (flg & PROJECT_REFLECTABLE) && !one_in_(10))
	{
		byte t_y, t_x;
		int max_attempts = 10;

#ifdef JP
		if (blind) msg_print("������ķ���֤ä���");
		else if (p_ptr->special_defense & KATA_FUUJIN) msg_print("����ǡ�����򿶤�ä��Ƥ��֤�����");
		else msg_print("���⤬ķ���֤ä���");
#else
		if (blind) msg_print("Something bounces!");
		else msg_print("The attack bounces!");
#endif


		/* Choose 'new' target */
		if (who > 0)
		{
			do
			{
				t_y = m_list[who].fy - 1 + randint1(3);
				t_x = m_list[who].fx - 1 + randint1(3);
				max_attempts--;
			}
			while (max_attempts && in_bounds2u(t_y, t_x) && !projectable(py, px, t_y, t_x));

			if (max_attempts < 1)
			{
				t_y = m_list[who].fy;
				t_x = m_list[who].fx;
			}
		}
		else
		{
			t_y = py - 1 + randint1(3);
			t_x = px - 1 + randint1(3);
		}

		project(0, 0, t_y, t_x, dam, typ, (PROJECT_STOP|PROJECT_KILL|PROJECT_REFLECTABLE), monspell);

		disturb(1, 0);
		return TRUE;
	}

	/* XXX XXX XXX */
	/* Limit maximum damage */
	if (dam > 1600) dam = 1600;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* If the player is blind, be more descriptive */
	if (blind) fuzzy = TRUE;


	if (who > 0)
	{
		/* Get the source monster */
		m_ptr = &m_list[who];
		/* Extract the monster level */
		rlev = (((&r_info[m_ptr->r_idx])->level >= 1) ? (&r_info[m_ptr->r_idx])->level : 1);

		/* Get the monster name */
		monster_desc(m_name, m_ptr, 0);

		/* Get the monster's real name (gotten before polymorph!) */
		strcpy(killer, who_name);
	}
	else
	{
		switch (who)
		{
		case PROJECT_WHO_UNCTRL_POWER:
#ifdef JP
			strcpy(killer, "����Ǥ��ʤ��Ϥ���ή");
#else
			strcpy(killer, "uncontrollable power storm");
#endif
			break;

		case PROJECT_WHO_GLASS_SHARDS:
#ifdef JP
			strcpy(killer, "���饹������");
#else
			strcpy(killer, "shards of glass");
#endif
			break;

		default:
#ifdef JP
			strcpy(killer, "�");
#else
			strcpy(killer, "a trap");
#endif
			break;
		}

		/* Paranoia */
		strcpy(m_name, killer);
	}

	/* Analyze the damage */
	switch (typ)
	{
		/* Standard damage -- hurts inventory too */
		case GF_ACID:
		{
#ifdef JP
			if (fuzzy) msg_print("���ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by acid!");
#endif
			
			get_damage = acid_dam(dam, killer, monspell);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_FIRE:
		{
#ifdef JP
			if (fuzzy) msg_print("�б�ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by fire!");
#endif

			get_damage = fire_dam(dam, killer, monspell);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_COLD:
		{
#ifdef JP
			if (fuzzy) msg_print("�䵤�ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by cold!");
#endif

			get_damage = cold_dam(dam, killer, monspell);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_ELEC:
		{
#ifdef JP
			if (fuzzy) msg_print("�ŷ�ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by lightning!");
#endif

			get_damage = elec_dam(dam, killer, monspell);
			break;
		}

		/* Standard damage -- also poisons player */
		case GF_POIS:
		{
			bool double_resist = IS_OPPOSE_POIS();
#ifdef JP
			if (fuzzy) msg_print("�Ǥǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by poison!");
#endif

			if (p_ptr->resist_pois) dam = (dam + 2) / 3;
			if (double_resist) dam = (dam + 2) / 3;

			if ((!(double_resist || p_ptr->resist_pois)) &&
			     one_in_(HURT_CHANCE) && !CHECK_MULTISHADOW())
			{
				do_dec_stat(A_CON);
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);

			if (!(double_resist || p_ptr->resist_pois) && !CHECK_MULTISHADOW())
			{
				set_poisoned(p_ptr->poisoned + randint0(dam) + 10);
			}
			break;
		}

		/* Standard damage -- also poisons / mutates player */
		case GF_NUKE:
		{
			bool double_resist = IS_OPPOSE_POIS();
#ifdef JP
			if (fuzzy) msg_print("����ǽ�ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by radiation!");
#endif

			if (p_ptr->resist_pois) dam = (2 * dam + 2) / 5;
			if (double_resist) dam = (2 * dam + 2) / 5;
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			if (!(double_resist || p_ptr->resist_pois) && !CHECK_MULTISHADOW())
			{
				set_poisoned(p_ptr->poisoned + randint0(dam) + 10);

				if (one_in_(5)) /* 6 */
				{
#ifdef JP
					msg_print("���Ū���ѿȤ�뤲����");
#else
					msg_print("You undergo a freakish metamorphosis!");
#endif

					if (one_in_(4)) /* 4 */
						do_poly_self();
					else
						mutate_player();
				}

				if (one_in_(6))
				{
					inven_damage(set_acid_destroy, 2);
				}
			}
			break;
		}

		/* Standard damage */
		case GF_MISSILE:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something!");
#endif

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Holy Orb -- Player only takes partial damage */
		case GF_HOLY_FIRE:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something!");
#endif

			if (p_ptr->align > 10)
				dam /= 2;
			else if (p_ptr->align < -10)
				dam *= 2;
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		case GF_HELL_FIRE:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something!");
#endif

			if (p_ptr->align > 10)
				dam *= 2;
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Arrow -- XXX no dodging */
		case GF_ARROW:
		{
#ifdef JP
			if (fuzzy) msg_print("�����Ԥ���Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something sharp!");
#endif

			else if ((inventory[INVEN_RARM].name1 == ART_ZANTETSU) || (inventory[INVEN_LARM].name1 == ART_ZANTETSU))
			{
#ifdef JP
				msg_print("���¤�ΤƤ���");
#else
				msg_print("You cut down the arrow!");
#endif
				break;
			}
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Plasma -- XXX No resist */
		case GF_PLASMA:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ȤƤ�Ǯ����Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something *HOT*!");
#endif

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);

			if (!p_ptr->resist_sound && !CHECK_MULTISHADOW())
			{
				int k = (randint1((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}

			if (!(p_ptr->resist_fire ||
			      IS_OPPOSE_FIRE() ||
			      p_ptr->immune_fire))
			{
				inven_damage(set_acid_destroy, 3);
			}

			break;
		}

		/* Nether -- drain experience */
		case GF_NETHER:
		{
#ifdef JP
			if (fuzzy) msg_print("�Ϲ����Ϥǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by nether forces!");
#endif

			if (p_ptr->resist_neth)
			{
				if (!prace_is_(RACE_SPECTRE))
					dam *= 6; dam /= (randint1(4) + 7);
			}
			else if (!CHECK_MULTISHADOW()) drain_exp(200 + (p_ptr->exp / 100), 200 + (p_ptr->exp / 1000), 75);

			if (prace_is_(RACE_SPECTRE) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("��ʬ���褯�ʤä���");
#else
				msg_print("You feel invigorated!");
#endif

				hp_player(dam / 4);
				learn_spell(monspell);
			}
			else
			{
				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			}

			break;
		}

		/* Water -- stun/confuse */
		case GF_WATER:
		{
#ifdef JP
			if (fuzzy) msg_print("�������ä���Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something wet!");
#endif

			if (!CHECK_MULTISHADOW())
			{
				if (!p_ptr->resist_sound)
				{
					set_stun(p_ptr->stun + randint1(40));
				}
				if (!p_ptr->resist_conf)
				{
					set_confused(p_ptr->confused + randint1(5) + 5);
				}

				if (one_in_(5))
				{
					inven_damage(set_cold_destroy, 3);
				}
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Chaos -- many effects */
		case GF_CHAOS:
		{
#ifdef JP
			if (fuzzy) msg_print("̵�������ư�ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by a wave of anarchy!");
#endif

			if (p_ptr->resist_chaos)
			{
				dam *= 6; dam /= (randint1(4) + 7);
			}

			if (!CHECK_MULTISHADOW())
			{
				if (!p_ptr->resist_conf)
				{
					(void)set_confused(p_ptr->confused + randint0(20) + 10);
				}
				if (!p_ptr->resist_chaos)
				{
					(void)set_image(p_ptr->image + randint1(10));
					if (one_in_(3))
					{
#ifdef JP
						msg_print("���ʤ��ο��Τϥ��������Ϥ�Ǳ���ʤ���줿��");
#else
						msg_print("Your body is twisted by chaos!");
#endif

						(void)gain_random_mutation(0);
					}
				}
				if (!p_ptr->resist_neth && !p_ptr->resist_chaos)
				{
					drain_exp(5000 + (p_ptr->exp / 100), 500 + (p_ptr->exp / 1000), 75);
				}

				if (!p_ptr->resist_chaos || one_in_(9))
				{
					inven_damage(set_elec_destroy, 2);
					inven_damage(set_fire_destroy, 2);
				}
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Shards -- mostly cutting */
		case GF_SHARDS:
		{
#ifdef JP
			if (fuzzy) msg_print("�����Ԥ���Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something sharp!");
#endif

			if (p_ptr->resist_shard)
			{
				dam *= 6; dam /= (randint1(4) + 7);
			}
			else if (!CHECK_MULTISHADOW())
			{
				(void)set_cut(p_ptr->cut + dam);
			}

			if (!p_ptr->resist_shard || one_in_(13))
			{
				inven_damage(set_cold_destroy, 2);
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Sound -- mostly stunning */
		case GF_SOUND:
		{
#ifdef JP
			if (fuzzy) msg_print("�첻�ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by a loud noise!");
#endif

			if (p_ptr->resist_sound)
			{
				dam *= 5; dam /= (randint1(4) + 7);
			}
			else if (!CHECK_MULTISHADOW())
			{
				int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}

			if (!p_ptr->resist_sound || one_in_(13))
			{
				inven_damage(set_cold_destroy, 2);
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Pure confusion */
		case GF_CONFUSION:
		{
#ifdef JP
			if (fuzzy) msg_print("�������𤹤��Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something puzzling!");
#endif

			if (p_ptr->resist_conf)
			{
				dam *= 5; dam /= (randint1(4) + 7);
			}
			else if (!CHECK_MULTISHADOW())
			{
				(void)set_confused(p_ptr->confused + randint1(20) + 10);
			}
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Disenchantment -- see above */
		case GF_DISENCHANT:
		{
#ifdef JP
			if (fuzzy) msg_print("���������ʤ���Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something static!");
#endif

			if (p_ptr->resist_disen)
			{
				dam *= 6; dam /= (randint1(4) + 7);
			}
			else if (!CHECK_MULTISHADOW())
			{
				(void)apply_disenchant(0);
			}
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Nexus -- see above */
		case GF_NEXUS:
		{
#ifdef JP
			if (fuzzy) msg_print("������̯�ʤ�Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something strange!");
#endif

			if (p_ptr->resist_nexus)
			{
				dam *= 6; dam /= (randint1(4) + 7);
			}
			else if (!CHECK_MULTISHADOW())
			{
				apply_nexus(m_ptr);
			}
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Force -- mostly stun */
		case GF_FORCE:
		{
#ifdef JP
			if (fuzzy) msg_print("��ư���ͥ륮���ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by kinetic force!");
#endif

			if (!p_ptr->resist_sound && !CHECK_MULTISHADOW())
			{
				(void)set_stun(p_ptr->stun + randint1(20));
			}
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}


		/* Rocket -- stun, cut */
		case GF_ROCKET:
		{
#ifdef JP
			if (fuzzy) msg_print("��ȯ�����ä���");
#else
			if (fuzzy) msg_print("There is an explosion!");
#endif

			if (!p_ptr->resist_sound && !CHECK_MULTISHADOW())
			{
				(void)set_stun(p_ptr->stun + randint1(20));
			}

			if (p_ptr->resist_shard)
			{
				dam /= 2;
			}
			else if (!CHECK_MULTISHADOW())
			{
				(void)set_cut(p_ptr->cut + (dam / 2));
			}

			if (!p_ptr->resist_shard || one_in_(12))
			{
				inven_damage(set_cold_destroy, 3);
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Inertia -- slowness */
		case GF_INERTIA:
		{
#ifdef JP
			if (fuzzy) msg_print("�����٤���Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something slow!");
#endif

			if (!CHECK_MULTISHADOW()) (void)set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Lite -- blinding */
		case GF_LITE:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something!");
#endif

			if (p_ptr->resist_lite)
			{
				dam *= 4; dam /= (randint1(4) + 7);
			}
			else if (!blind && !p_ptr->resist_blind && !CHECK_MULTISHADOW())
			{
				(void)set_blind(p_ptr->blind + randint1(5) + 2);
			}

			if (prace_is_(RACE_VAMPIRE) || (p_ptr->mimic_form == MIMIC_VAMPIRE))
			{
#ifdef JP
				if (!CHECK_MULTISHADOW()) msg_print("�������Τ��Ǥ����줿��");
#else
				if (!CHECK_MULTISHADOW()) msg_print("The light scorches your flesh!");
#endif

				dam *= 2;
			}
			else if (prace_is_(RACE_S_FAIRY))
			{
				dam = dam * 4 / 3;
			}

			if (p_ptr->wraith_form) dam *= 2;
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);

			if (p_ptr->wraith_form && !CHECK_MULTISHADOW())
			{
				p_ptr->wraith_form = 0;
#ifdef JP
				msg_print("�����Τ�����ʪ��Ū�ʱƤ�¸�ߤǤ����ʤ��ʤä���");
#else
				msg_print("The light forces you out of your incorporeal shadow form.");
#endif

				p_ptr->redraw |= PR_MAP;
				/* Update monsters */
				p_ptr->update |= (PU_MONSTERS);
				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

				/* Redraw status bar */
				p_ptr->redraw |= (PR_STATUS);

			}

			break;
		}

		/* Dark -- blinding */
		case GF_DARK:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something!");
#endif

			if (p_ptr->resist_dark)
			{
				dam *= 4; dam /= (randint1(4) + 7);

				if (prace_is_(RACE_VAMPIRE) || (p_ptr->mimic_form == MIMIC_VAMPIRE) || p_ptr->wraith_form) dam = 0;
			}
			else if (!blind && !p_ptr->resist_blind && !CHECK_MULTISHADOW())
			{
				(void)set_blind(p_ptr->blind + randint1(5) + 2);
			}
			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Time -- bolt fewer effects XXX */
		case GF_TIME:
		{
#ifdef JP
			if (fuzzy) msg_print("����ξ׷�˹��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by a blast from the past!");
#endif

			if (p_ptr->resist_time)
			{
				dam *= 4;
				dam /= (randint1(4) + 7);
#ifdef JP
				msg_print("���֤��̤�᤮�Ƥ����������롣");
#else
				msg_print("You feel as if time is passing you by.");
#endif
			}
			else if (!CHECK_MULTISHADOW())
			{
				switch (randint1(10))
				{
					case 1: case 2: case 3: case 4: case 5:
					{
						if (p_ptr->prace == RACE_ANDROID) break;
#ifdef JP
						msg_print("����������ꤷ���������롣");
#else
						msg_print("You feel life has clocked back.");
#endif

						lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
						break;
					}

					case 6: case 7: case 8: case 9:
					{
						switch (randint1(6))
						{
#ifdef JP
							case 1: k = A_STR; act = "����"; break;
							case 2: k = A_INT; act = "������"; break;
							case 3: k = A_WIS; act = "������"; break;
							case 4: k = A_DEX; act = "���Ѥ�"; break;
							case 5: k = A_CON; act = "�򹯤�"; break;
							case 6: k = A_CHR; act = "������"; break;
#else
							case 1: k = A_STR; act = "strong"; break;
							case 2: k = A_INT; act = "bright"; break;
							case 3: k = A_WIS; act = "wise"; break;
							case 4: k = A_DEX; act = "agile"; break;
							case 5: k = A_CON; act = "hale"; break;
							case 6: k = A_CHR; act = "beautiful"; break;
#endif
						}

#ifdef JP
						msg_format("���ʤ��ϰ����ۤ�%s�ʤ��ʤäƤ��ޤä�...��", act);
#else
						msg_format("You're not as %s as you used to be...", act);
#endif

						p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
						if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
						p_ptr->update |= (PU_BONUS);
						break;
					}

					case 10:
					{
#ifdef JP
						msg_print("���ʤ��ϰ����ۤ��϶����ʤ��ʤäƤ��ޤä�...��");
#else
						msg_print("You're not as powerful as you used to be...");
#endif

						for (k = 0; k < 6; k++)
						{
							p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 7) / 8;
							if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
						}
						p_ptr->update |= (PU_BONUS);
						break;
					}
				}
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Gravity -- stun plus slowness plus teleport */
		case GF_GRAVITY:
		{
#ifdef JP
			if (fuzzy) msg_print("�����Ť���Τǹ��⤵�줿��");
			msg_print("���դν��Ϥ��椬�����");
#else
			if (fuzzy) msg_print("You are hit by something heavy!");
			msg_print("Gravity warps around you.");
#endif

			if (!CHECK_MULTISHADOW())
			{
				teleport_player(5, TELEPORT_PASSIVE);
				if (!p_ptr->levitation)
					(void)set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
				if (!(p_ptr->resist_sound || p_ptr->levitation))
				{
					int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
					(void)set_stun(p_ptr->stun + k);
				}
			}
			if (p_ptr->levitation)
			{
				dam = (dam * 2) / 3;
			}

			if (!p_ptr->levitation || one_in_(13))
			{
				inven_damage(set_cold_destroy, 2);
			}

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Standard damage */
		case GF_DISINTEGRATE:
		{
#ifdef JP
			if (fuzzy) msg_print("���ʥ��ͥ륮���ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by pure energy!");
#endif

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		case GF_OLD_HEAL:
		{
#ifdef JP
			if (fuzzy) msg_print("���餫�ι���ˤ�äƵ�ʬ���褯�ʤä���");
#else
			if (fuzzy) msg_print("You are hit by something invigorating!");
#endif

			(void)hp_player(dam);
			dam = 0;
			break;
		}

		case GF_OLD_SPEED:
		{
#ifdef JP
			if (fuzzy) msg_print("�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something!");
#endif

			(void)set_fast(p_ptr->fast + randint1(5), FALSE);
			dam = 0;
			break;
		}

		case GF_OLD_SLOW:
		{
#ifdef JP
			if (fuzzy) msg_print("�����٤���Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something slow!");
#endif

			(void)set_slow(p_ptr->slow + randint0(4) + 4, FALSE);
			break;
		}

		case GF_OLD_SLEEP:
		{
			if (p_ptr->free_act)  break;
#ifdef JP
			if (fuzzy) msg_print("̲�äƤ��ޤä���");
#else
			if (fuzzy) msg_print("You fall asleep!");
#endif


			if (ironman_nightmare)
			{
#ifdef JP
				msg_print("���������ʤ�Ƭ���⤫��Ǥ�����");
#else
				msg_print("A horrible vision enters your mind.");
#endif

				/* Pick a nightmare */
				get_mon_num_prep(get_nightmare, NULL);

				/* Have some nightmares */
				have_nightmare(get_mon_num(MAX_DEPTH));

				/* Remove the monster restriction */
				get_mon_num_prep(NULL, NULL);
			}

			set_paralyzed(p_ptr->paralyzed + dam);
			dam = 0;
			break;
		}

		/* Pure damage */
		case GF_MANA:
		case GF_SEEKER:
		case GF_SUPER_RAY:
		{
#ifdef JP
			if (fuzzy) msg_print("��ˡ�Υ�����ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by an aura of magic!");
#endif

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		/* Pure damage */
		case GF_PSY_SPEAR:
		{
#ifdef JP
			if (fuzzy) msg_print("���ͥ륮���β��ǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by an energy!");
#endif

			get_damage = take_hit(DAMAGE_FORCE, dam, killer, monspell);
			break;
		}

		/* Pure damage */
		case GF_METEOR:
		{
#ifdef JP
			if (fuzzy) msg_print("�����������餢�ʤ���Ƭ�������Ƥ�����");
#else
			if (fuzzy) msg_print("Something falls from the sky on you!");
#endif

			get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			if (!p_ptr->resist_shard || one_in_(13))
			{
				if (!p_ptr->immune_fire) inven_damage(set_fire_destroy, 2);
				inven_damage(set_cold_destroy, 2);
			}

			break;
		}

		/* Ice -- cold plus stun plus cuts */
		case GF_ICE:
		{
#ifdef JP
			if (fuzzy) msg_print("�����Ԥ��䤿����Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something sharp and cold!");
#endif

			cold_dam(dam, killer, monspell);
			if (!CHECK_MULTISHADOW())
			{
				if (!p_ptr->resist_shard)
				{
					(void)set_cut(p_ptr->cut + damroll(5, 8));
				}
				if (!p_ptr->resist_sound)
				{
					(void)set_stun(p_ptr->stun + randint1(15));
				}

				if ((!(p_ptr->resist_cold || IS_OPPOSE_COLD())) || one_in_(12))
				{
					if (!p_ptr->immune_cold) inven_damage(set_cold_destroy, 3);
				}
			}

			break;
		}

		/* Death Ray */
		case GF_DEATH_RAY:
		{
#ifdef JP
			if (fuzzy) msg_print("���������䤿����Τǹ��⤵�줿��");
#else
			if (fuzzy) msg_print("You are hit by something extremely cold!");
#endif


			if (p_ptr->mimic_form)
			{
				if (!(mimic_info[p_ptr->mimic_form].MIMIC_FLAGS & MIMIC_IS_NONLIVING))
					get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			}
			else
			{

			switch (p_ptr->prace)
			{
				/* Some races are immune */
				case RACE_GOLEM:
				case RACE_SKELETON:
				case RACE_ZOMBIE:
				case RACE_VAMPIRE:
				case RACE_DEMON:
				case RACE_SPECTRE:
				{
					dam = 0;
					break;
				}
				/* Hurt a lot */
				default:
				{
					get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
					break;
				}
			}
			}

			break;
		}

		/* Drain mana */
		case GF_DRAIN_MANA:
		{
			if (CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("����ϸ��Ƥ�̿�椷�����ʤ��ˤ��Ϥ��ʤ��ä���");
#else
				msg_print("The attack hits Shadow, you are unharmed!");
#endif
			}
			else if (p_ptr->csp)
			{
				/* Basic message */
#ifdef JP
				if (who > 0) msg_format("%^s���������ͥ륮����ۤ�����Ƥ��ޤä���", m_name);
				else msg_print("�������ͥ륮����ۤ�����Ƥ��ޤä���");
#else
				if (who > 0) msg_format("%^s draws psychic energy from you!", m_name);
				else msg_print("Your psychic energy is drawn!");
#endif

				/* Full drain */
				if (dam >= p_ptr->csp)
				{
					dam = p_ptr->csp;
					p_ptr->csp = 0;
					p_ptr->csp_frac = 0;
				}

				/* Partial drain */
				else
				{
					p_ptr->csp -= dam;
				}

				learn_spell(monspell);

				/* Redraw mana */
				p_ptr->redraw |= (PR_MANA);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER);
				p_ptr->window |= (PW_SPELL);

				if (who > 0)
				{
					/* Heal the monster */
					if (m_ptr->hp < m_ptr->maxhp)
					{
						/* Heal */
						m_ptr->hp += (6 * dam);
						if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

						/* Redraw (later) if needed */
						if (p_ptr->health_who == who) p_ptr->redraw |= (PR_HEALTH);
						if (p_ptr->riding == who) p_ptr->redraw |= (PR_UHEALTH);

						/* Special message */
						if (m_ptr->ml)
						{
#ifdef JP
							msg_format("%^s�ϵ�ʬ���ɤ���������", m_name);
#else
							msg_format("%^s appears healthier.", m_name);
#endif
						}
					}
				}
			}

			dam = 0;
			break;
		}

		/* Mind blast */
		case GF_MIND_BLAST:
		{
			if ((randint0(100 + rlev / 2) < MAX(5, p_ptr->skill_sav)) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				if (!CHECK_MULTISHADOW())
				{
#ifdef JP
					msg_print("��Ū���ͥ륮�������������⤵�줿��");
#else
					msg_print("Your mind is blasted by psyonic energy.");
#endif

					if (!p_ptr->resist_conf)
					{
						(void)set_confused(p_ptr->confused + randint0(4) + 4);
					}

					if (!p_ptr->resist_chaos && one_in_(3))
					{
						(void)set_image(p_ptr->image + randint0(250) + 150);
					}

					p_ptr->csp -= 50;
					if (p_ptr->csp < 0)
					{
						p_ptr->csp = 0;
						p_ptr->csp_frac = 0;
					}
					p_ptr->redraw |= PR_MANA;
				}

				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			}
			break;
		}

		/* Brain smash */
		case GF_BRAIN_SMASH:
		{
			if ((randint0(100 + rlev / 2) < MAX(5, p_ptr->skill_sav)) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				if (!CHECK_MULTISHADOW())
				{
#ifdef JP
					msg_print("��Ū���ͥ륮�������������⤵�줿��");
#else
					msg_print("Your mind is blasted by psionic energy.");
#endif

					p_ptr->csp -= 100;
					if (p_ptr->csp < 0)
					{
						p_ptr->csp = 0;
						p_ptr->csp_frac = 0;
					}
					p_ptr->redraw |= PR_MANA;
				}

				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
				if (!CHECK_MULTISHADOW())
				{
					if (!p_ptr->resist_blind)
					{
						(void)set_blind(p_ptr->blind + 8 + randint0(8));
					}
					if (!p_ptr->resist_conf)
					{
						(void)set_confused(p_ptr->confused + randint0(4) + 4);
					}
					if (!p_ptr->free_act)
					{
						(void)set_paralyzed(p_ptr->paralyzed + randint0(4) + 4);
					}
					(void)set_slow(p_ptr->slow + randint0(4) + 4, FALSE);

					while (randint0(100 + rlev / 2) > (MAX(5, p_ptr->skill_sav)))
						(void)do_dec_stat(A_INT);
					while (randint0(100 + rlev / 2) > (MAX(5, p_ptr->skill_sav)))
						(void)do_dec_stat(A_WIS);

					if (!p_ptr->resist_chaos)
					{
						(void)set_image(p_ptr->image + randint0(250) + 150);
					}
				}
			}
			break;
		}

		/* cause 1 */
		case GF_CAUSE_1:
		{
			if ((randint0(100 + rlev / 2) < p_ptr->skill_sav) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				if (!CHECK_MULTISHADOW()) curse_equipment(15, 0);
				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			}
			break;
		}

		/* cause 2 */
		case GF_CAUSE_2:
		{
			if ((randint0(100 + rlev / 2) < p_ptr->skill_sav) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				if (!CHECK_MULTISHADOW()) curse_equipment(25, MIN(rlev / 2 - 15, 5));
				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			}
			break;
		}

		/* cause 3 */
		case GF_CAUSE_3:
		{
			if ((randint0(100 + rlev / 2) < p_ptr->skill_sav) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("���������Ϥ�ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				if (!CHECK_MULTISHADOW()) curse_equipment(33, MIN(rlev / 2 - 15, 15));
				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
			}
			break;
		}

		/* cause 4 */
		case GF_CAUSE_4:
		{
			if ((randint0(100 + rlev / 2) < p_ptr->skill_sav) && !(m_ptr->r_idx == MON_KENSHIROU) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_print("�������빦��ķ���֤�����");
#else
				msg_print("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				get_damage = take_hit(DAMAGE_ATTACK, dam, killer, monspell);
				if (!CHECK_MULTISHADOW()) (void)set_cut(p_ptr->cut + damroll(10, 10));
			}
			break;
		}

		/* Hand of Doom */
		case GF_HAND_DOOM:
		{
			if ((randint0(100 + rlev/2) < p_ptr->skill_sav) && !CHECK_MULTISHADOW())
			{
#ifdef JP
				msg_format("���������Ϥ�ķ���֤�����");
#else
				msg_format("You resist the effects!");
#endif
				learn_spell(monspell);
			}
			else
			{
				if (!CHECK_MULTISHADOW())
				{
#ifdef JP
					msg_print("���ʤ���̿�����ޤäƤ����褦�˴�������");
#else
					msg_print("You feel your life fade away!");
#endif
					curse_equipment(40, 20);
				}

				get_damage = take_hit(DAMAGE_ATTACK, dam, m_name, monspell);

				if (p_ptr->chp < 1) p_ptr->chp = 1; /* Paranoia */
			}
			break;
		}

		/* Default */
		default:
		{
			/* No damage */
			dam = 0;

			break;
		}
	}

	if (p_ptr->tim_eyeeye && (get_damage > 0) && !p_ptr->is_dead && (who > 0))
	{
#ifdef JP
		msg_format("���⤬%s���Ȥ���Ĥ�����", m_name);
#else
		char m_name_self[80];

		/* hisself */
		monster_desc(m_name_self, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE | MD_OBJECTIVE);

		msg_format("The attack of %s has wounded %s!", m_name, m_name_self);
#endif
		project(0, 0, m_ptr->fy, m_ptr->fx, get_damage, GF_MISSILE, PROJECT_KILL, -1);
		set_tim_eyeeye(p_ptr->tim_eyeeye-5, TRUE);
	}

	if (p_ptr->riding && dam > 0)
	{
		rakubadam_p = (dam > 200) ? 200 : dam;
	}


	/* Disturb */
	disturb(1, 0);


	if ((p_ptr->special_defense & NINJA_KAWARIMI) && dam && who && (who != p_ptr->riding))
	{
		(void)kawarimi(FALSE);
	}

	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Find the distance from (x, y) to a line.
 */
int dist_to_line(int y, int x, int y1, int x1, int y2, int x2)
{
	/* Vector from (x, y) to (x1, y1) */
	int py = y1 - y;
	int px = x1 - x;

	/* Normal vector */
	int ny = x2 - x1;
	int nx = y1 - y2;

   /* Length of N */
	int pd = distance(y1, x1, y, x);
	int nd = distance(y1, x1, y2, x2);

	if (pd > nd) return distance(y, x, y2, x2);

	/* Component of P on N */
	nd = ((nd) ? ((py * ny + px * nx) / nd) : 0);

   /* Absolute value */
   return((nd >= 0) ? nd : 0 - nd);
}



/*
 * XXX XXX XXX
 * Modified version of los() for calculation of disintegration balls.
 * Disintegration effects are stopped by permanent walls.
 */
bool in_disintegration_range(int y1, int x1, int y2, int x2)
{
	/* Delta */
	int dx, dy;

	/* Absolute */
	int ax, ay;

	/* Signs */
	int sx, sy;

	/* Fractions */
	int qx, qy;

	/* Scanners */
	int tx, ty;

	/* Scale factors */
	int f1, f2;

	/* Slope, or 1/Slope, of LOS */
	int m;


	/* Extract the offset */
	dy = y2 - y1;
	dx = x2 - x1;

	/* Extract the absolute offset */
	ay = ABS(dy);
	ax = ABS(dx);


	/* Handle adjacent (or identical) grids */
	if ((ax < 2) && (ay < 2)) return (TRUE);


	/* Paranoia -- require "safe" origin */
	/* if (!in_bounds(y1, x1)) return (FALSE); */


	/* Directly South/North */
	if (!dx)
	{
		/* South -- check for walls */
		if (dy > 0)
		{
			for (ty = y1 + 1; ty < y2; ty++)
			{
				if (cave_stop_disintegration(ty, x1)) return (FALSE);
			}
		}

		/* North -- check for walls */
		else
		{
			for (ty = y1 - 1; ty > y2; ty--)
			{
				if (cave_stop_disintegration(ty, x1)) return (FALSE);
			}
		}

		/* Assume los */
		return (TRUE);
	}

	/* Directly East/West */
	if (!dy)
	{
		/* East -- check for walls */
		if (dx > 0)
		{
			for (tx = x1 + 1; tx < x2; tx++)
			{
				if (cave_stop_disintegration(y1, tx)) return (FALSE);
			}
		}

		/* West -- check for walls */
		else
		{
			for (tx = x1 - 1; tx > x2; tx--)
			{
				if (cave_stop_disintegration(y1, tx)) return (FALSE);
			}
		}

		/* Assume los */
		return (TRUE);
	}


	/* Extract some signs */
	sx = (dx < 0) ? -1 : 1;
	sy = (dy < 0) ? -1 : 1;


	/* Vertical "knights" */
	if (ax == 1)
	{
		if (ay == 2)
		{
			if (!cave_stop_disintegration(y1 + sy, x1)) return (TRUE);
		}
	}

	/* Horizontal "knights" */
	else if (ay == 1)
	{
		if (ax == 2)
		{
			if (!cave_stop_disintegration(y1, x1 + sx)) return (TRUE);
		}
	}


	/* Calculate scale factor div 2 */
	f2 = (ax * ay);

	/* Calculate scale factor */
	f1 = f2 << 1;


	/* Travel horizontally */
	if (ax >= ay)
	{
		/* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
		qy = ay * ay;
		m = qy << 1;

		tx = x1 + sx;

		/* Consider the special case where slope == 1. */
		if (qy == f2)
		{
			ty = y1 + sy;
			qy -= f1;
		}
		else
		{
			ty = y1;
		}

		/* Note (below) the case (qy == f2), where */
		/* the LOS exactly meets the corner of a tile. */
		while (x2 - tx)
		{
			if (cave_stop_disintegration(ty, tx)) return (FALSE);

			qy += m;

			if (qy < f2)
			{
				tx += sx;
			}
			else if (qy > f2)
			{
				ty += sy;
				if (cave_stop_disintegration(ty, tx)) return (FALSE);
				qy -= f1;
				tx += sx;
			}
			else
			{
				ty += sy;
				qy -= f1;
				tx += sx;
			}
		}
	}

	/* Travel vertically */
	else
	{
		/* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
		qx = ax * ax;
		m = qx << 1;

		ty = y1 + sy;

		if (qx == f2)
		{
			tx = x1 + sx;
			qx -= f1;
		}
		else
		{
			tx = x1;
		}

		/* Note (below) the case (qx == f2), where */
		/* the LOS exactly meets the corner of a tile. */
		while (y2 - ty)
		{
			if (cave_stop_disintegration(ty, tx)) return (FALSE);

			qx += m;

			if (qx < f2)
			{
				ty += sy;
			}
			else if (qx > f2)
			{
				tx += sx;
				if (cave_stop_disintegration(ty, tx)) return (FALSE);
				qx -= f1;
				ty += sy;
			}
			else
			{
				tx += sx;
				qx -= f1;
				ty += sy;
			}
		}
	}

	/* Assume los */
	return (TRUE);
}


/*
 * breath shape
 */
void breath_shape(u16b *path_g, int dist, int *pgrids, byte *gx, byte *gy, byte *gm, int *pgm_rad, int rad, int y1, int x1, int y2, int x2, int typ)
{
	int by = y1;
	int bx = x1;
	int brad = 0;
	int brev = rad * rad / dist;
	int bdis = 0;
	int cdis;
	int path_n = 0;
	int mdis = distance(y1, x1, y2, x2) + rad;

	while (bdis <= mdis)
	{
		int x, y;

		if ((0 < dist) && (path_n < dist))
		{
			int ny = GRID_Y(path_g[path_n]);
			int nx = GRID_X(path_g[path_n]);
			int nd = distance(ny, nx, y1, x1);

			/* Get next base point */
			if (bdis >= nd)
			{
				by = ny;
				bx = nx;
				path_n++;
			}
		}

		/* Travel from center outward */
		for (cdis = 0; cdis <= brad; cdis++)
		{
			/* Scan the maximal blast area of radius "cdis" */
			for (y = by - cdis; y <= by + cdis; y++)
			{
				for (x = bx - cdis; x <= bx + cdis; x++)
				{
					/* Ignore "illegal" locations */
					if (!in_bounds(y, x)) continue;

					/* Enforce a circular "ripple" */
					if (distance(y1, x1, y, x) != bdis) continue;

					/* Enforce an arc */
					if (distance(by, bx, y, x) != cdis) continue;

					switch (typ)
					{
					case GF_LITE:
					case GF_LITE_WEAK:
						/* Lights are stopped by opaque terrains */
						if (!los(by, bx, y, x)) continue;
						break;
					case GF_DISINTEGRATE:
						/* Disintegration are stopped only by perma-walls */
						if (!in_disintegration_range(by, bx, y, x)) continue;
						break;
					default:
						/* Ball explosions are stopped by walls */
						if (!projectable(by, bx, y, x)) continue;
						break;
					}

					/* Save this grid */
					gy[*pgrids] = y;
					gx[*pgrids] = x;
					(*pgrids)++;
				}
			}
		}

		/* Encode some more "radius" info */
		gm[bdis + 1] = *pgrids;

		/* Increase the size */
		brad = rad * (path_n + brev) / (dist + brev);

		/* Find the next ripple */
		bdis++;
	}

	/* Store the effect size */
	*pgm_rad = bdis;
}


/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (zero for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away.  The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced.  This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that all projections now "explode" at their final destination, even
 * if they were being projected at a more distant destination.  This means
 * that "ball" spells will *always* explode.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the "illumination" of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool project(int who, int rad, int y, int x, int dam, int typ, int flg, int monspell)
{
	int i, t, dist;

	int y1, x1;
	int y2, x2;
	int by, bx;

	int dist_hack = 0;

	int y_saver, x_saver; /* For reflecting monsters */

	int msec = delay_factor * delay_factor * delay_factor;

	/* Assume the player sees nothing */
	bool notice = FALSE;

	/* Assume the player has seen nothing */
	bool visual = FALSE;

	/* Assume the player has seen no blast grids */
	bool drawn = FALSE;

	/* Assume to be a normal ball spell */
	bool breath = FALSE;

	/* Is the player blind? */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	bool old_hide = FALSE;

	/* Number of grids in the "path" */
	int path_n = 0;

	/* Actual grids in the "path" */
	u16b path_g[512];

	/* Number of grids in the "blast area" (including the "beam" path) */
	int grids = 0;

	/* Coordinates of the affected grids */
	byte gx[1024], gy[1024];

	/* Encoded "radius" info (see above) */
	byte gm[32];

	/* Actual radius encoded in gm[] */
	int gm_rad = rad;

	bool jump = FALSE;

	/* Attacker's name (prepared before polymorph)*/
	char who_name[80];

	/* Can the player see the source of this effect? */
	bool see_s_msg = TRUE;

	/* Initialize by null string */
	who_name[0] = '\0';

	rakubadam_p = 0;
	rakubadam_m = 0;

	/* Default target of monsterspell is player */
	monster_target_y=py;
	monster_target_x=px;

	/* Hack -- Jump to target */
	if (flg & (PROJECT_JUMP))
	{
		x1 = x;
		y1 = y;

		/* Clear the flag */
		flg &= ~(PROJECT_JUMP);

		jump = TRUE;
	}

	/* Start at player */
	else if (who <= 0)
	{
		x1 = px;
		y1 = py;
	}

	/* Start at monster */
	else if (who > 0)
	{
		x1 = m_list[who].fx;
		y1 = m_list[who].fy;
		monster_desc(who_name, &m_list[who], MD_IGNORE_HALLU | MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
	}

	/* Oops */
	else
	{
		x1 = x;
		y1 = y;
	}

	y_saver = y1;
	x_saver = x1;

	/* Default "destination" */
	y2 = y;
	x2 = x;


	/* Hack -- verify stuff */
	if (flg & (PROJECT_THRU))
	{
		if ((x1 == x2) && (y1 == y2))
		{
			flg &= ~(PROJECT_THRU);
		}
	}

	/* Handle a breath attack */
	if (rad < 0)
	{
		rad = 0 - rad;
		breath = TRUE;
		if (flg & PROJECT_HIDE) old_hide = TRUE;
		flg |= PROJECT_HIDE;
	}


	/* Hack -- Assume there will be no blast (max radius 32) */
	for (dist = 0; dist < 32; dist++) gm[dist] = 0;


	/* Initial grid */
	y = y1;
	x = x1;
	dist = 0;

	/* Collect beam grids */
	if (flg & (PROJECT_BEAM))
	{
		gy[grids] = y;
		gx[grids] = x;
		grids++;
	}

	switch (typ)
	{
	case GF_LITE:
	case GF_LITE_WEAK:
		if (breath || (flg & PROJECT_BEAM)) flg |= (PROJECT_LOS);
		break;
	case GF_DISINTEGRATE:
		flg |= (PROJECT_GRID);
		if (breath || (flg & PROJECT_BEAM)) flg |= (PROJECT_DISI);
		break;
	}

	/* Calculate the projection path */

	path_n = project_path(path_g, (project_length ? project_length : MAX_RANGE), y1, x1, y2, x2, flg);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Giga-Hack SEEKER & SUPER_RAY */

	if( typ == GF_SEEKER )
	{
		int j;
		int last_i=0;

		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		for (i = 0; i < path_n; ++i)
		{
			int oy = y;
			int ox = x;

			int ny = GRID_Y(path_g[i]);
			int nx = GRID_X(path_g[i]);

			/* Advance */
			y = ny;
			x = nx;

			gy[grids] = y;
			gx[grids] = x;
			grids++;


			/* Only do visuals if requested */
			if (!blind && !(flg & (PROJECT_HIDE)))
			{
				/* Only do visuals if the player can "see" the bolt */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					u16b p;

					byte a;
					char c;

					/* Obtain the bolt pict */
					p = bolt_pict(oy, ox, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects */
					print_rel(c, a, y, x);
					move_cursor_relative(y, x);
					/*if (fresh_before)*/ Term_fresh();
					Term_xtra(TERM_XTRA_DELAY, msec);
					lite_spot(y, x);
					/*if (fresh_before)*/ Term_fresh();

					/* Display "beam" grids */
					if (flg & (PROJECT_BEAM))
					{
						/* Obtain the explosion pict */
						p = bolt_pict(y, x, y, x, typ);

						/* Extract attr/char */
						a = PICT_A(p);
						c = PICT_C(p);

						/* Visual effects */
						print_rel(c, a, y, x);
					}

					/* Hack -- Activate delay */
					visual = TRUE;
				}

				/* Hack -- delay anyway for consistency */
				else if (visual)
				{
					/* Delay for consistency */
					Term_xtra(TERM_XTRA_DELAY, msec);
				}
			}
			if(project_o(0,0,y,x,dam,GF_SEEKER))notice=TRUE;
			if( is_mirror_grid(&cave[y][x]))
			{
			  /* The target of monsterspell becomes tha mirror(broken) */
				monster_target_y=(s16b)y;
				monster_target_x=(s16b)x;

				remove_mirror(y,x);
				next_mirror( &oy,&ox,y,x );

				path_n = i+project_path(&(path_g[i+1]), (project_length ? project_length : MAX_RANGE), y, x, oy, ox, flg);
				for( j = last_i; j <=i ; j++ )
				{
					y = GRID_Y(path_g[j]);
					x = GRID_X(path_g[j]);
					if(project_m(0,0,y,x,dam,GF_SEEKER,flg,TRUE))notice=TRUE;
					if(!who && (project_m_n==1) && !jump ){
					  if(cave[project_m_y][project_m_x].m_idx >0 ){
					    monster_type *m_ptr = &m_list[cave[project_m_y][project_m_x].m_idx];

					    if (m_ptr->ml)
					    {
					      /* Hack -- auto-recall */
					      if (!p_ptr->image) monster_race_track(m_ptr->ap_r_idx);

					      /* Hack - auto-track */
					      health_track(cave[project_m_y][project_m_x].m_idx);
					    }
					  }
					}
					(void)project_f(0,0,y,x,dam,GF_SEEKER);
				}
				last_i = i;
			}
		}
		for( i = last_i ; i < path_n ; i++ )
		{
			int x,y;
			y = GRID_Y(path_g[i]);
			x = GRID_X(path_g[i]);
			if(project_m(0,0,y,x,dam,GF_SEEKER,flg,TRUE))
			  notice=TRUE;
			if(!who && (project_m_n==1) && !jump ){
			  if(cave[project_m_y][project_m_x].m_idx >0 ){
			    monster_type *m_ptr = &m_list[cave[project_m_y][project_m_x].m_idx];

			    if (m_ptr->ml)
			    {
			      /* Hack -- auto-recall */
			      if (!p_ptr->image) monster_race_track(m_ptr->ap_r_idx);

			      /* Hack - auto-track */
			      health_track(cave[project_m_y][project_m_x].m_idx);
			    }
			  }
			}
			(void)project_f(0,0,y,x,dam,GF_SEEKER);
		}
		return notice;
	}
	else if(typ == GF_SUPER_RAY){
		int j;
		int second_step = 0;

		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		for (i = 0; i < path_n; ++i)
		{
			int oy = y;
			int ox = x;

			int ny = GRID_Y(path_g[i]);
			int nx = GRID_X(path_g[i]);

			/* Advance */
			y = ny;
			x = nx;

			gy[grids] = y;
			gx[grids] = x;
			grids++;


			/* Only do visuals if requested */
			if (!blind && !(flg & (PROJECT_HIDE)))
			{
				/* Only do visuals if the player can "see" the bolt */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					u16b p;

					byte a;
					char c;

					/* Obtain the bolt pict */
					p = bolt_pict(oy, ox, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects */
					print_rel(c, a, y, x);
					move_cursor_relative(y, x);
					/*if (fresh_before)*/ Term_fresh();
					Term_xtra(TERM_XTRA_DELAY, msec);
					lite_spot(y, x);
					/*if (fresh_before)*/ Term_fresh();

					/* Display "beam" grids */
					if (flg & (PROJECT_BEAM))
					{
						/* Obtain the explosion pict */
						p = bolt_pict(y, x, y, x, typ);

						/* Extract attr/char */
						a = PICT_A(p);
						c = PICT_C(p);

						/* Visual effects */
						print_rel(c, a, y, x);
					}

					/* Hack -- Activate delay */
					visual = TRUE;
				}

				/* Hack -- delay anyway for consistency */
				else if (visual)
				{
					/* Delay for consistency */
					Term_xtra(TERM_XTRA_DELAY, msec);
				}
			}
			if(project_o(0,0,y,x,dam,GF_SUPER_RAY) )notice=TRUE;
			if (!cave_have_flag_bold(y, x, FF_PROJECT))
			{
				if( second_step )continue;
				break;
			}
			if( is_mirror_grid(&cave[y][x]) && !second_step )
			{
			  /* The target of monsterspell becomes tha mirror(broken) */
				monster_target_y=(s16b)y;
				monster_target_x=(s16b)x;

				remove_mirror(y,x);
				for( j = 0; j <=i ; j++ )
				{
					y = GRID_Y(path_g[j]);
					x = GRID_X(path_g[j]);
					(void)project_f(0,0,y,x,dam,GF_SUPER_RAY);
				}
				path_n = i;
				second_step =i+1;
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y-1, x-1, flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y-1, x  , flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y-1, x+1, flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y  , x-1, flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y  , x+1, flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y+1, x-1, flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y+1, x  , flg);
				path_n += project_path(&(path_g[path_n+1]), (project_length ? project_length : MAX_RANGE), y, x, y+1, x+1, flg);
			}
		}
		for( i = 0; i < path_n ; i++ )
		{
			int x,y;
			y = GRID_Y(path_g[i]);
			x = GRID_X(path_g[i]);
			(void)project_m(0,0,y,x,dam,GF_SUPER_RAY,flg,TRUE);
			if(!who && (project_m_n==1) && !jump ){
			  if(cave[project_m_y][project_m_x].m_idx >0 ){
			    monster_type *m_ptr = &m_list[cave[project_m_y][project_m_x].m_idx];

			    if (m_ptr->ml)
			    {
			      /* Hack -- auto-recall */
			      if (!p_ptr->image) monster_race_track(m_ptr->ap_r_idx);

			      /* Hack - auto-track */
			      health_track(cave[project_m_y][project_m_x].m_idx);
			    }
			  }
			}
			(void)project_f(0,0,y,x,dam,GF_SUPER_RAY);
		}
		return notice;
	}

	/* Project along the path */
	for (i = 0; i < path_n; ++i)
	{
		int oy = y;
		int ox = x;

		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		if (flg & PROJECT_DISI)
		{
			/* Hack -- Balls explode before reaching walls */
			if (cave_stop_disintegration(ny, nx) && (rad > 0)) break;
		}
		else if (flg & PROJECT_LOS)
		{
			/* Hack -- Balls explode before reaching walls */
			if (!cave_los_bold(ny, nx) && (rad > 0)) break;
		}
		else
		{
			/* Hack -- Balls explode before reaching walls */
			if (!cave_have_flag_bold(ny, nx, FF_PROJECT) && (rad > 0)) break;
		}

		/* Advance */
		y = ny;
		x = nx;

		/* Collect beam grids */
		if (flg & (PROJECT_BEAM))
		{
			gy[grids] = y;
			gx[grids] = x;
			grids++;
		}

		/* Only do visuals if requested */
		if (!blind && !(flg & (PROJECT_HIDE | PROJECT_FAST)))
		{
			/* Only do visuals if the player can "see" the bolt */
			if (panel_contains(y, x) && player_has_los_bold(y, x))
			{
				u16b p;

				byte a;
				char c;

				/* Obtain the bolt pict */
				p = bolt_pict(oy, ox, y, x, typ);

				/* Extract attr/char */
				a = PICT_A(p);
				c = PICT_C(p);

				/* Visual effects */
				print_rel(c, a, y, x);
				move_cursor_relative(y, x);
				/*if (fresh_before)*/ Term_fresh();
				Term_xtra(TERM_XTRA_DELAY, msec);
				lite_spot(y, x);
				/*if (fresh_before)*/ Term_fresh();

				/* Display "beam" grids */
				if (flg & (PROJECT_BEAM))
				{
					/* Obtain the explosion pict */
					p = bolt_pict(y, x, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects */
					print_rel(c, a, y, x);
				}

				/* Hack -- Activate delay */
				visual = TRUE;
			}

			/* Hack -- delay anyway for consistency */
			else if (visual)
			{
				/* Delay for consistency */
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}
	}

	path_n = i;

	/* Save the "blast epicenter" */
	by = y;
	bx = x;

	if (breath && !path_n)
	{
		breath = FALSE;
		gm_rad = rad;
		if (!old_hide)
		{
			flg &= ~(PROJECT_HIDE);
		}
	}

	/* Start the "explosion" */
	gm[0] = 0;

	/* Hack -- make sure beams get to "explode" */
	gm[1] = grids;

	dist = path_n;
	dist_hack = dist;

	project_length = 0;

	/* If we found a "target", explode there */
	if (dist <= MAX_RANGE)
	{
		/* Mega-Hack -- remove the final "beam" grid */
		if ((flg & (PROJECT_BEAM)) && (grids > 0)) grids--;

		/*
		 * Create a conical breath attack
		 *
		 *         ***
		 *     ********
		 * D********@**
		 *     ********
		 *         ***
		 */

		if (breath)
		{
			flg &= ~(PROJECT_HIDE);

			breath_shape(path_g, dist, &grids, gx, gy, gm, &gm_rad, rad, y1, x1, by, bx, typ);
		}
		else
		{
			/* Determine the blast area, work from the inside out */
			for (dist = 0; dist <= rad; dist++)
			{
				/* Scan the maximal blast area of radius "dist" */
				for (y = by - dist; y <= by + dist; y++)
				{
					for (x = bx - dist; x <= bx + dist; x++)
					{
						/* Ignore "illegal" locations */
						if (!in_bounds2(y, x)) continue;

						/* Enforce a "circular" explosion */
						if (distance(by, bx, y, x) != dist) continue;

						switch (typ)
						{
						case GF_LITE:
						case GF_LITE_WEAK:
							/* Lights are stopped by opaque terrains */
							if (!los(by, bx, y, x)) continue;
							break;
						case GF_DISINTEGRATE:
							/* Disintegration are stopped only by perma-walls */
							if (!in_disintegration_range(by, bx, y, x)) continue;
							break;
						default:
							/* Ball explosions are stopped by walls */
							if (!projectable(by, bx, y, x)) continue;
							break;
						}

						/* Save this grid */
						gy[grids] = y;
						gx[grids] = x;
						grids++;
					}
				}

				/* Encode some more "radius" info */
				gm[dist+1] = grids;
			}
		}
	}

	/* Speed -- ignore "non-explosions" */
	if (!grids) return (FALSE);


	/* Display the "blast area" if requested */
	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		/* Then do the "blast", from inside out */
		for (t = 0; t <= gm_rad; t++)
		{
			/* Dump everything with this radius */
			for (i = gm[t]; i < gm[t+1]; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Only do visuals if the player can "see" the blast */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					u16b p;

					byte a;
					char c;

					drawn = TRUE;

					/* Obtain the explosion pict */
					p = bolt_pict(y, x, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects -- Display */
					print_rel(c, a, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(by, bx);

			/* Flush each "radius" seperately */
			/*if (fresh_before)*/ Term_fresh();

			/* Delay (efficiently) */
			if (visual || drawn)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		/* Flush the erasing */
		if (drawn)
		{
			/* Erase the explosion drawn above */
			for (i = 0; i < grids; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Hack -- Erase if needed */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					lite_spot(y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(by, bx);

			/* Flush the explosion */
			/*if (fresh_before)*/ Term_fresh();
		}
	}


	/* Update stuff if needed */
	if (p_ptr->update) update_stuff();


	if (flg & PROJECT_KILL)
	{
		see_s_msg = (who > 0) ? is_seen(&m_list[who]) :
			(!who ? TRUE : (player_can_see_bold(y1, x1) && projectable(py, px, y1, x1)));
	}


	/* Check features */
	if (flg & (PROJECT_GRID))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for features */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Find the closest point in the blast */
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, by, bx);

				/* Affect the grid */
				if (project_f(who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				/* Affect the grid */
				if (project_f(who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}

	/* Update stuff if needed */
	if (p_ptr->update) update_stuff();

	/* Check objects */
	if (flg & (PROJECT_ITEM))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for objects */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Find the closest point in the blast */
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, by, bx);

				/* Affect the object in the grid */
				if (project_o(who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				/* Affect the object in the grid */
				if (project_o(who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}


	/* Check monsters */
	if (flg & (PROJECT_KILL))
	{
		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for monsters */
		for (i = 0; i < grids; i++)
		{
			int effective_dist;

			/* Hack -- Notice new "dist" values */
			if (gm[dist + 1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* A single bolt may be reflected */
			if (grids <= 1)
			{
				monster_type *m_ptr = &m_list[cave[y][x].m_idx];
				monster_race *ref_ptr = &r_info[m_ptr->r_idx];

				if ((flg & PROJECT_REFLECTABLE) && cave[y][x].m_idx && (ref_ptr->flags2 & RF2_REFLECTING) &&
				    ((cave[y][x].m_idx != p_ptr->riding) || !(flg & PROJECT_PLAYER)) &&
				    (!who || dist_hack > 1) && !one_in_(10))
				{
					byte t_y, t_x;
					int max_attempts = 10;

					/* Choose 'new' target */
					do
					{
						t_y = y_saver - 1 + randint1(3);
						t_x = x_saver - 1 + randint1(3);
						max_attempts--;
					}
					while (max_attempts && in_bounds2u(t_y, t_x) && !projectable(y, x, t_y, t_x));

					if (max_attempts < 1)
					{
						t_y = y_saver;
						t_x = x_saver;
					}

					if (is_seen(m_ptr))
					{
#ifdef JP
						if ((m_ptr->r_idx == MON_KENSHIROU) || (m_ptr->r_idx == MON_RAOU))
							msg_print("�����Ϳ�����������ؿ����ġ���");
						else if (m_ptr->r_idx == MON_DIO) msg_print("�ǥ������֥��ɡ��ϻذ��ܤǹ�����Ƥ��֤�����");
						else msg_print("�����ķ���֤ä���");
#else
						msg_print("The attack bounces!");
#endif
					}
					if (is_original_ap_and_seen(m_ptr)) ref_ptr->r_flags2 |= RF2_REFLECTING;

					/* Reflected bolts randomly target either one */
					if (player_bold(y, x) || one_in_(2)) flg &= ~(PROJECT_PLAYER);
					else flg |= PROJECT_PLAYER;

					/* The bolt is reflected */
					project(cave[y][x].m_idx, 0, t_y, t_x, dam, typ, flg, monspell);

					/* Don't affect the monster any longer */
					continue;
				}
			}


			/* Find the closest point in the blast */
			if (breath)
			{
				effective_dist = dist_to_line(y, x, y1, x1, by, bx);
			}
			else
			{
				effective_dist = dist;
			}


			/* There is the riding player on this monster */
			if (p_ptr->riding && player_bold(y, x))
			{
				/* Aimed on the player */
				if (flg & PROJECT_PLAYER)
				{
					if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE | PROJECT_AIMED))
					{
						/*
						 * A beam or bolt is well aimed
						 * at the PLAYER!
						 * So don't affects the mount.
						 */
						continue;
					}
					else
					{
						/*
						 * The spell is not well aimed, 
						 * So partly affect the mount too.
						 */
						effective_dist++;
					}
				}

				/*
				 * This grid is the original target.
				 * Or aimed on your horse.
				 */
				else if (((y == y2) && (x == x2)) || (flg & PROJECT_AIMED))
				{
					/* Hit the mount with full damage */
				}

				/*
				 * Otherwise this grid is not the
				 * original target, it means that line
				 * of fire is obstructed by this
				 * monster.
				 */
				/*
				 * A beam or bolt will hit either
				 * player or mount.  Choose randomly.
				 */
				else if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE))
				{
					if (one_in_(2))
					{
						/* Hit the mount with full damage */
					}
					else
					{
						/* Hit the player later */
						flg |= PROJECT_PLAYER;

						/* Don't affect the mount */
						continue;
					}
				}

				/*
				 * The spell is not well aimed, so
				 * partly affect both player and
				 * mount.
				 */
				else
				{
					effective_dist++;
				}
			}

			/* Affect the monster in the grid */
			if (project_m(who, effective_dist, y, x, dam, typ, flg, see_s_msg)) notice = TRUE;
		}


		/* Player affected one monster (without "jumping") */
		if (!who && (project_m_n == 1) && !jump)
		{
			/* Location */
			x = project_m_x;
			y = project_m_y;

			/* Track if possible */
			if (cave[y][x].m_idx > 0)
			{
				monster_type *m_ptr = &m_list[cave[y][x].m_idx];

				if (m_ptr->ml)
				{
					/* Hack -- auto-recall */
					if (!p_ptr->image) monster_race_track(m_ptr->ap_r_idx);

					/* Hack - auto-track */
					if (m_ptr->ml) health_track(cave[y][x].m_idx);
				}
			}
		}
	}


	/* Check player */
	if (flg & (PROJECT_KILL))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for player */
		for (i = 0; i < grids; i++)
		{
			int effective_dist;

			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the player? */
			if (!player_bold(y, x)) continue;

			/* Find the closest point in the blast */
			if (breath)
			{
				effective_dist = dist_to_line(y, x, y1, x1, by, bx);
			}
			else
			{
				effective_dist = dist;
			}

			/* Target may be your horse */
			if (p_ptr->riding)
			{
				/* Aimed on the player */
				if (flg & PROJECT_PLAYER)
				{
					/* Hit the player with full damage */
				}

				/*
				 * Hack -- When this grid was not the
				 * original target, a beam or bolt
				 * would hit either player or mount,
				 * and should be choosen randomly.
				 *
				 * But already choosen to hit the
				 * mount at this point.
				 *
				 * Or aimed on your horse.
				 */
				else if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE | PROJECT_AIMED))
				{
					/*
					 * A beam or bolt is well aimed
					 * at the mount!
					 * So don't affects the player.
					 */
					continue;
				}
				else
				{
					/*
					 * The spell is not well aimed, 
					 * So partly affect the player too.
					 */
					effective_dist++;
				}
			}

			/* Affect the player */
			if (project_p(who, who_name, effective_dist, y, x, dam, typ, flg, monspell)) notice = TRUE;
		}
	}

	if (p_ptr->riding)
	{
		char m_name[80];

		monster_desc(m_name, &m_list[p_ptr->riding], 0);

		if (rakubadam_m > 0)
		{
			if (rakuba(rakubadam_m, FALSE))
			{
#ifdef JP
msg_format("%^s�˿�����Ȥ��줿��", m_name);
#else
				msg_format("%^s has thrown you off!", m_name);
#endif
			}
		}
		if (p_ptr->riding && rakubadam_p > 0)
		{
			if(rakuba(rakubadam_p, FALSE))
			{
#ifdef JP
msg_format("%^s��������Ƥ��ޤä���", m_name);
#else
				msg_format("You have fallen from %s.", m_name);
#endif
			}
		}
	}

	/* Return "something was noticed" */
	return (notice);
}

bool binding_field( int dam )
{
	int mirror_x[10],mirror_y[10]; /* ���Ϥ�äȾ��ʤ� */
	int mirror_num=0;              /* ���ο� */
	int x,y;
	int centersign;
	int x1,x2,y1,y2;
	u16b p;
	int msec= delay_factor*delay_factor*delay_factor;

	/* ���ѷ���ĺ�� */
	int point_x[3];
	int point_y[3];

	/* Default target of monsterspell is player */
	monster_target_y=py;
	monster_target_x=px;

	for( x=0 ; x < cur_wid ; x++ )
	{
		for( y=0 ; y < cur_hgt ; y++ )
		{
			if( is_mirror_grid(&cave[y][x]) &&
			    distance(py,px,y,x) <= MAX_RANGE &&
			    distance(py,px,y,x) != 0 &&
			    player_has_los_bold(y,x) &&
			    projectable(py, px, y, x)
			    ){
				mirror_y[mirror_num]=y;
				mirror_x[mirror_num]=x;
				mirror_num++;
			}
		}
	}

	if( mirror_num < 2 )return FALSE;

	point_x[0] = randint0( mirror_num );
	do {
	  point_x[1] = randint0( mirror_num );
	}
	while( point_x[0] == point_x[1] );

	point_y[0]=mirror_y[point_x[0]];
	point_x[0]=mirror_x[point_x[0]];
	point_y[1]=mirror_y[point_x[1]];
	point_x[1]=mirror_x[point_x[1]];
	point_y[2]=py;
	point_x[2]=px;

	x=point_x[0]+point_x[1]+point_x[2];
	y=point_y[0]+point_y[1]+point_y[2];

	centersign = (point_x[0]*3-x)*(point_y[1]*3-y)
		- (point_y[0]*3-y)*(point_x[1]*3-x);
	if( centersign == 0 )return FALSE;
			    
	x1 = point_x[0] < point_x[1] ? point_x[0] : point_x[1];
	x1 = x1 < point_x[2] ? x1 : point_x[2];
	y1 = point_y[0] < point_y[1] ? point_y[0] : point_y[1];
	y1 = y1 < point_y[2] ? y1 : point_y[2];

	x2 = point_x[0] > point_x[1] ? point_x[0] : point_x[1];
	x2 = x2 > point_x[2] ? x2 : point_x[2];
	y2 = point_y[0] > point_y[1] ? point_y[0] : point_y[1];
	y2 = y2 > point_y[2] ? y2 : point_y[2];

	for( y=y1 ; y <=y2 ; y++ ){
		for( x=x1 ; x <=x2 ; x++ ){
			if( centersign*( (point_x[0]-x)*(point_y[1]-y)
					 -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
			    centersign*( (point_x[1]-x)*(point_y[2]-y)
					 -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
			    centersign*( (point_x[2]-x)*(point_y[0]-y)
					 -(point_y[2]-y)*(point_x[0]-x)) >=0 )
			{
				if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
					/* Visual effects */
					if(!(p_ptr->blind)
					   && panel_contains(y,x)){
					  p = bolt_pict(y,x,y,x, GF_MANA );
					  print_rel(PICT_C(p), PICT_A(p),y,x);
					  move_cursor_relative(y, x);
					  /*if (fresh_before)*/ Term_fresh();
					  Term_xtra(TERM_XTRA_DELAY, msec);
					}
				}
			}
		}
	}
	for( y=y1 ; y <=y2 ; y++ ){
		for( x=x1 ; x <=x2 ; x++ ){
			if( centersign*( (point_x[0]-x)*(point_y[1]-y)
					 -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
			    centersign*( (point_x[1]-x)*(point_y[2]-y)
					 -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
			    centersign*( (point_x[2]-x)*(point_y[0]-y)
					 -(point_y[2]-y)*(point_x[0]-x)) >=0 )
			{
				if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
					(void)project_f(0,0,y,x,dam,GF_MANA); 
				}
			}
		}
	}
	for( y=y1 ; y <=y2 ; y++ ){
		for( x=x1 ; x <=x2 ; x++ ){
			if( centersign*( (point_x[0]-x)*(point_y[1]-y)
					 -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
			    centersign*( (point_x[1]-x)*(point_y[2]-y)
					 -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
			    centersign*( (point_x[2]-x)*(point_y[0]-y)
					 -(point_y[2]-y)*(point_x[0]-x)) >=0 )
			{
				if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
					(void)project_o(0,0,y,x,dam,GF_MANA); 
				}
			}
		}
	}
	for( y=y1 ; y <=y2 ; y++ ){
		for( x=x1 ; x <=x2 ; x++ ){
			if( centersign*( (point_x[0]-x)*(point_y[1]-y)
					 -(point_y[0]-y)*(point_x[1]-x)) >=0 &&
			    centersign*( (point_x[1]-x)*(point_y[2]-y)
					 -(point_y[1]-y)*(point_x[2]-x)) >=0 &&
			    centersign*( (point_x[2]-x)*(point_y[0]-y)
					 -(point_y[2]-y)*(point_x[0]-x)) >=0 )
			{
				if (player_has_los_bold(y, x) && projectable(py, px, y, x)) {
					(void)project_m(0,0,y,x,dam,GF_MANA,
					  (PROJECT_GRID|PROJECT_ITEM|PROJECT_KILL|PROJECT_JUMP),TRUE);
				}
			}
		}
	}
	if( one_in_(7) ){
#ifdef JP
		msg_print("�����볦���Ѥ����줺������Ƥ��ޤä���");
#else
		msg_print("The field broke a mirror");
#endif	
		remove_mirror(point_y[0],point_x[0]);
	}

	return TRUE;
}

void seal_of_mirror( int dam )
{
	int x,y;

	for( x = 0 ; x < cur_wid ; x++ )
	{
		for( y = 0 ; y < cur_hgt ; y++ )
		{
			if( is_mirror_grid(&cave[y][x]))
			{
				if(project_m(0,0,y,x,dam,GF_GENOCIDE,
							 (PROJECT_GRID|PROJECT_ITEM|PROJECT_KILL|PROJECT_JUMP),TRUE))
				{
					if( !cave[y][x].m_idx )
					{
						remove_mirror(y,x);
					}
				}
			}
		}
	}
	return;
}
     
