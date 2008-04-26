/* File: do-spell.c */

/* Purpose: Do everything for each spell */

#include "angband.h"


/*
 * Generate dice info string such as "foo 2d10"
 */
static cptr info_string_dice(cptr str, int dice, int sides, int base)
{
	/* Fix value */
	if (!dice)
		return format("%s%d", str, base);

	/* Dice only */
	else if (!base)
		return format("%s%dd%d", str, dice, sides);

	/* Dice plus base value */
	else
		return format("%s%dd%d%+d", str, dice, sides, base);
}


/*
 * Generate damage-dice info string such as "dam 2d10"
 */
static cptr info_damage(int dice, int sides, int base)
{
#ifdef JP
	return info_string_dice("»��:", dice, sides, base);
#else
	return info_string_dice("dam ", dice, sides, base);
#endif
}


/*
 * Generate duration info string such as "dur 20+1d20"
 */
static cptr info_duration(int base, int sides)
{
#ifdef JP
	return format("����:%d+1d%d", base, sides);
#else
	return format("dur %d+1d%d", base, sides);
#endif
}


/*
 * Generate range info string such as "range 5"
 */
static cptr info_range(int range)
{
#ifdef JP
	return format("�ϰ�:%d", range);
#else
	return format("range %d", range);
#endif
}


/*
 * Generate heal info string such as "heal 2d8"
 */
static cptr info_heal(int dice, int sides, int base)
{
#ifdef JP
	return info_string_dice("����:", dice, sides, base);
#else
	return info_string_dice("heal ", dice, sides, base);
#endif
}


/*
 * Generate delay info string such as "delay 15+1d15"
 */
static cptr info_delay(int base, int sides)
{
#ifdef JP
	return format("�ٱ�:%d+1d%d", base, sides);
#else
	return format("delay %d+1d%d", base, sides);
#endif
}


/*
 * Generate multiple-damage info string such as "dam 25 each"
 */
static cptr info_multi_damage(int dam)
{
#ifdef JP
	return format("»��:��%d", dam);
#else
	return format("dam %d each", dam);
#endif
}


/*
 * Generate multiple-damage-dice info string such as "dam 5d2 each"
 */
static cptr info_multi_damage_dice(int dice, int sides)
{
#ifdef JP
	return format("»��:��%dd%d", dice, sides);
#else
	return format("dam %dd%d each", dice, sides);
#endif
}


/*
 * Generate power info string such as "power 100"
 */
static cptr info_power(int power)
{
#ifdef JP
	return format("����:%d", power);
#else
	return format("power %d", power);
#endif
}


/*
 * Generate power info string such as "power 1d100"
 */
static cptr info_power_dice(int dice, int sides)
{
#ifdef JP
	return format("����:%dd%d", dice, sides);
#else
	return format("power %dd%d", dice, sides);
#endif
}


/*
 * Generate radius info string such as "rad 100"
 */
static cptr info_radius(int rad)
{
#ifdef JP
	return format("Ⱦ��:%d", rad);
#else
	return format("rad %d", rad);
#endif
}


/*
 * Generate weight info string such as "max wgt 15"
 */
static cptr info_weight(int weight)
{
#ifdef JP
	return format("�������:%d.%dkg", lbtokg1(weight/10), lbtokg2(weight/10));
#else
	return format("max wgt %d", weight/10);
#endif
}


/*
 * Prepare standard probability to become beam for fire_bolt_or_beam()
 */
static int beam_chance(void)
{
	if (p_ptr->pclass == CLASS_MAGE)
		return p_ptr->lev;
	if (p_ptr->pclass == CLASS_HIGH_MAGE || p_ptr->pclass == CLASS_SORCERER)
		return p_ptr->lev + 10;

	return p_ptr->lev / 2;
}


/*
 * Handle summoning and failure of trump spells
 */
static bool trump_summoning(int num, bool pet, int y, int x, int lev, int type, u32b mode)
{
	int plev = p_ptr->lev;

	int who;
	int i;
	bool success = FALSE;

	/* Default level */
	if (!lev) lev = plev * 2 / 3 + randint1(plev / 2);

	if (pet)
	{
		/* Become pet */
		mode |= PM_FORCE_PET;

		/* Only sometimes allow unique monster */
		if (mode & PM_ALLOW_UNIQUE)
		{
			/* Forbid often */
			if (randint1(50 + plev) >= plev / 10)
				mode &= ~PM_ALLOW_UNIQUE;
		}

		/* Player is who summons */
		who = -1;
	}
	else
	{
		/* Prevent taming, allow unique monster */
		mode |= PM_NO_PET;

		/* Behave as if they appear by themselfs */
		who = 0;
	}

	for (i = 0; i < num; i++)
	{
		if (summon_specific(who, y, x, lev, type, mode))
			success = TRUE;
	}

	if (!success)
	{
#ifdef JP
		msg_print("ï�⤢�ʤ��Υ����ɤθƤ����������ʤ���");
#else
		msg_print("Nobody answers to your Trump call.");
#endif
	}

	return success;
}


/*
 * This spell should become more useful (more controlled) as the
 * player gains experience levels.  Thus, add 1/5 of the player's
 * level to the die roll.  This eliminates the worst effects later on,
 * while keeping the results quite random.  It also allows some potent
 * effects only at high level.
 */
static void cast_wonder(int dir)
{
	int plev = p_ptr->lev;
	int die = randint1(100) + plev / 5;
	int vir = virtue_number(V_CHANCE);

	if (vir)
	{
		if (p_ptr->virtues[vir - 1] > 0)
		{
			while (randint1(400) < p_ptr->virtues[vir - 1]) die++;
		}
		else
		{
			while (randint1(400) < (0-p_ptr->virtues[vir - 1])) die--;
		}
	}

	if (die < 26)
		chg_virtue(V_CHANCE, 1);

	if (die > 100)
	{
#ifdef JP
		msg_print("���ʤ����Ϥ��ߤʤ���Τ򴶤�����");
#else
		msg_print("You feel a surge of power!");
#endif
	}

	if (die < 8) clone_monster(dir);
	else if (die < 14) speed_monster(dir);
	else if (die < 26) heal_monster(dir, damroll(4, 6));
	else if (die < 31) poly_monster(dir);
	else if (die < 36)
		fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir,
				  damroll(3 + ((plev - 1) / 5), 4));
	else if (die < 41) confuse_monster(dir, plev);
	else if (die < 46) fire_ball(GF_POIS, dir, 20 + (plev / 2), 3);
	else if (die < 51) (void)lite_line(dir);
	else if (die < 56)
		fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir,
				  damroll(3 + ((plev - 5) / 4), 8));
	else if (die < 61)
		fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
				  damroll(5 + ((plev - 5) / 4), 8));
	else if (die < 66)
		fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
				  damroll(6 + ((plev - 5) / 4), 8));
	else if (die < 71)
		fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
				  damroll(8 + ((plev - 5) / 4), 8));
	else if (die < 76) drain_life(dir, 75);
	else if (die < 81) fire_ball(GF_ELEC, dir, 30 + plev / 2, 2);
	else if (die < 86) fire_ball(GF_ACID, dir, 40 + plev, 2);
	else if (die < 91) fire_ball(GF_ICE, dir, 70 + plev, 3);
	else if (die < 96) fire_ball(GF_FIRE, dir, 80 + plev, 3);
	else if (die < 101) drain_life(dir, 100 + plev);
	else if (die < 104)
	{
		earthquake(py, px, 12);
	}
	else if (die < 106)
	{
		(void)destroy_area(py, px, 13 + randint0(5), FALSE);
	}
	else if (die < 108)
	{
		symbol_genocide(plev+50, TRUE);
	}
	else if (die < 110) dispel_monsters(120);
	else /* RARE */
	{
		dispel_monsters(150);
		slow_monsters();
		sleep_monsters();
		hp_player(300);
	}
}


static void cast_invoke_spirits(int dir)
{
	int plev = p_ptr->lev;
	int die = randint1(100) + plev / 5;
	int vir = virtue_number(V_CHANCE);

	if (vir)
	{
		if (p_ptr->virtues[vir - 1] > 0)
		{
			while (randint1(400) < p_ptr->virtues[vir - 1]) die++;
		}
		else
		{
			while (randint1(400) < (0-p_ptr->virtues[vir - 1])) die--;
		}
	}

#ifdef JP
	msg_print("���ʤ��ϻ�Ԥ������Ϥ򾷽�����...");
#else
	msg_print("You call on the power of the dead...");
#endif
	if (die < 26)
		chg_virtue(V_CHANCE, 1);

	if (die > 100)
	{
#ifdef JP
		msg_print("���ʤ��Ϥ��ɤ��ɤ����ϤΤ��ͤ�򴶤�����");
#else
		msg_print("You feel a surge of eldritch force!");
#endif
	}


	if (die < 8)
	{
#ifdef JP
		msg_print("�ʤ�Ƥ��ä������ʤ��μ�������̤��������ͱƤ�Ω���夬�äƤ�����");
#else
		msg_print("Oh no! Mouldering forms rise from the earth around you!");
#endif

		(void)summon_specific(0, py, px, dun_level, SUMMON_UNDEAD, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
		chg_virtue(V_UNLIFE, 1);
	}
	else if (die < 14)
	{
#ifdef JP
		msg_print("̾�����񤤼ٰ���¸�ߤ����ʤ��ο����̤�᤮�ƹԤä�...");
#else
		msg_print("An unnamable evil brushes against your mind...");
#endif

		set_afraid(p_ptr->afraid + randint1(4) + 4);
	}
	else if (die < 26)
	{
#ifdef JP
		msg_print("���ʤ���Ƭ�����̤�ͩ����������������������󤻤Ƥ���...");
#else
		msg_print("Your head is invaded by a horde of gibbering spectral voices...");
#endif

		set_confused(p_ptr->confused + randint1(4) + 4);
	}
	else if (die < 31)
	{
		poly_monster(dir);
	}
	else if (die < 36)
	{
		fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir,
				  damroll(3 + ((plev - 1) / 5), 4));
	}
	else if (die < 41)
	{
		confuse_monster (dir, plev);
	}
	else if (die < 46)
	{
		fire_ball(GF_POIS, dir, 20 + (plev / 2), 3);
	}
	else if (die < 51)
	{
		(void)lite_line(dir);
	}
	else if (die < 56)
	{
		fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir,
				  damroll(3+((plev-5)/4),8));
	}
	else if (die < 61)
	{
		fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
				  damroll(5+((plev-5)/4),8));
	}
	else if (die < 66)
	{
		fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
				  damroll(6+((plev-5)/4),8));
	}
	else if (die < 71)
	{
		fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
				  damroll(8+((plev-5)/4),8));
	}
	else if (die < 76)
	{
		drain_life(dir, 75);
	}
	else if (die < 81)
	{
		fire_ball(GF_ELEC, dir, 30 + plev / 2, 2);
	}
	else if (die < 86)
	{
		fire_ball(GF_ACID, dir, 40 + plev, 2);
	}
	else if (die < 91)
	{
		fire_ball(GF_ICE, dir, 70 + plev, 3);
	}
	else if (die < 96)
	{
		fire_ball(GF_FIRE, dir, 80 + plev, 3);
	}
	else if (die < 101)
	{
		drain_life(dir, 100 + plev);
	}
	else if (die < 104)
	{
		earthquake(py, px, 12);
	}
	else if (die < 106)
	{
		(void)destroy_area(py, px, 13 + randint0(5), FALSE);
	}
	else if (die < 108)
	{
		symbol_genocide(plev+50, TRUE);
	}
	else if (die < 110)
	{
		dispel_monsters(120);
	}
	else
	{ /* RARE */
		dispel_monsters(150);
		slow_monsters();
		sleep_monsters();
		hp_player(300);
	}

	if (die < 31)
	{
#ifdef JP
		msg_print("�������������������Ф����֤⤦�������ޤ��ϲ桹����֤ˤʤ�������夭�Ԥ衣��");
#else
		msg_print("Sepulchral voices chuckle. 'Soon you will join us, mortal.'");
#endif
	}
}


static void wild_magic(int spell)
{
	int counter = 0;
	int type = SUMMON_BIZARRE1 + randint0(6);

	if (type < SUMMON_BIZARRE1) type = SUMMON_BIZARRE1;
	else if (type > SUMMON_BIZARRE6) type = SUMMON_BIZARRE6;

	switch (randint1(spell) + randint1(8) + 1)
	{
	case 1:
	case 2:
	case 3:
		teleport_player(10, TELEPORT_PASSIVE);
		break;
	case 4:
	case 5:
	case 6:
		teleport_player(100, TELEPORT_PASSIVE);
		break;
	case 7:
	case 8:
		teleport_player(200, TELEPORT_PASSIVE);
		break;
	case 9:
	case 10:
	case 11:
		unlite_area(10, 3);
		break;
	case 12:
	case 13:
	case 14:
		lite_area(damroll(2, 3), 2);
		break;
	case 15:
		destroy_doors_touch();
		break;
	case 16: case 17:
		wall_breaker();
	case 18:
		sleep_monsters_touch();
		break;
	case 19:
	case 20:
		trap_creation(py, px);
		break;
	case 21:
	case 22:
		door_creation();
		break;
	case 23:
	case 24:
	case 25:
		aggravate_monsters(0);
		break;
	case 26:
		earthquake(py, px, 5);
		break;
	case 27:
	case 28:
		(void)gain_random_mutation(0);
		break;
	case 29:
	case 30:
		apply_disenchant(1);
		break;
	case 31:
		lose_all_info();
		break;
	case 32:
		fire_ball(GF_CHAOS, 0, spell + 5, 1 + (spell / 10));
		break;
	case 33:
		wall_stone();
		break;
	case 34:
	case 35:
		while (counter++ < 8)
		{
			(void)summon_specific(0, py, px, (dun_level * 3) / 2, type, (PM_ALLOW_GROUP | PM_NO_PET));
		}
		break;
	case 36:
	case 37:
		activate_hi_summon(py, px, FALSE);
		break;
	case 38:
		(void)summon_cyber(-1, py, px);
		break;
	default:
		{
			int count = 0;
			(void)activate_ty_curse(FALSE, &count);
			break;
		}
	}
}


static void cast_shuffle(void)
{
	int plev = p_ptr->lev;
	int dir;
	int die;
	int vir = virtue_number(V_CHANCE);
	int i;

	/* Card sharks and high mages get a level bonus */
	if ((p_ptr->pclass == CLASS_ROGUE) ||
	    (p_ptr->pclass == CLASS_HIGH_MAGE) ||
	    (p_ptr->pclass == CLASS_SORCERER))
		die = (randint1(110)) + plev / 5;
	else
		die = randint1(120);


	if (vir)
	{
		if (p_ptr->virtues[vir - 1] > 0)
		{
			while (randint1(400) < p_ptr->virtues[vir - 1]) die++;
		}
		else
		{
			while (randint1(400) < (0-p_ptr->virtues[vir - 1])) die--;
		}
	}

#ifdef JP
	msg_print("���ʤ��ϥ����ɤ��ڤäư��������...");
#else
	msg_print("You shuffle the deck and draw a card...");
#endif

	if (die < 30)
		chg_virtue(V_CHANCE, 1);

	if (die < 7)
	{
#ifdef JP
		msg_print("�ʤ�Ƥ��ä����Ի�դ���");
#else
		msg_print("Oh no! It's Death!");
#endif

		for (i = 0; i < randint1(3); i++)
			activate_hi_summon(py, px, FALSE);
	}
	else if (die < 14)
	{
#ifdef JP
		msg_print("�ʤ�Ƥ��ä����԰���դ���");
#else
		msg_print("Oh no! It's the Devil!");
#endif

		summon_specific(0, py, px, dun_level, SUMMON_DEMON, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
	}
	else if (die < 18)
	{
		int count = 0;
#ifdef JP
		msg_print("�ʤ�Ƥ��ä������ߤ�줿�ˡդ���");
#else
		msg_print("Oh no! It's the Hanged Man.");
#endif

		activate_ty_curse(FALSE, &count);
	}
	else if (die < 22)
	{
#ifdef JP
		msg_print("����Ĵ�¤η��դ���");
#else
		msg_print("It's the swords of discord.");
#endif

		aggravate_monsters(0);
	}
	else if (die < 26)
	{
#ifdef JP
		msg_print("�Զ�ԡդ���");
#else
		msg_print("It's the Fool.");
#endif

		do_dec_stat(A_INT);
		do_dec_stat(A_WIS);
	}
	else if (die < 30)
	{
#ifdef JP
		msg_print("��̯�ʥ�󥹥����γ�����");
#else
		msg_print("It's the picture of a strange monster.");
#endif

		trump_summoning(1, FALSE, py, px, (dun_level * 3 / 2), (32 + randint1(6)), PM_ALLOW_GROUP | PM_ALLOW_UNIQUE);
	}
	else if (die < 33)
	{
#ifdef JP
		msg_print("�Է�դ���");
#else
		msg_print("It's the Moon.");
#endif

		unlite_area(10, 3);
	}
	else if (die < 38)
	{
#ifdef JP
		msg_print("�Ա�̿���ءդ���");
#else
		msg_print("It's the Wheel of Fortune.");
#endif

		wild_magic(randint0(32));
	}
	else if (die < 40)
	{
#ifdef JP
		msg_print("�ƥ�ݡ��ȡ������ɤ���");
#else
		msg_print("It's a teleport trump card.");
#endif

		teleport_player(10, TELEPORT_PASSIVE);
	}
	else if (die < 42)
	{
#ifdef JP
		msg_print("�������դ���");
#else
		msg_print("It's Justice.");
#endif

		set_blessed(p_ptr->lev, FALSE);
	}
	else if (die < 47)
	{
#ifdef JP
		msg_print("�ƥ�ݡ��ȡ������ɤ���");
#else
		msg_print("It's a teleport trump card.");
#endif

		teleport_player(100, TELEPORT_PASSIVE);
	}
	else if (die < 52)
	{
#ifdef JP
		msg_print("�ƥ�ݡ��ȡ������ɤ���");
#else
		msg_print("It's a teleport trump card.");
#endif

		teleport_player(200, TELEPORT_PASSIVE);
	}
	else if (die < 60)
	{
#ifdef JP
		msg_print("����դ���");
#else
		msg_print("It's the Tower.");
#endif

		wall_breaker();
	}
	else if (die < 72)
	{
#ifdef JP
		msg_print("�������դ���");
#else
		msg_print("It's Temperance.");
#endif

		sleep_monsters_touch();
	}
	else if (die < 80)
	{
#ifdef JP
		msg_print("����դ���");
#else
		msg_print("It's the Tower.");
#endif

		earthquake(py, px, 5);
	}
	else if (die < 82)
	{
#ifdef JP
		msg_print("ͧ��Ū�ʥ�󥹥����γ�����");
#else
		msg_print("It's the picture of a friendly monster.");
#endif

		trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE1, 0L);
	}
	else if (die < 84)
	{
#ifdef JP
		msg_print("ͧ��Ū�ʥ�󥹥����γ�����");
#else
		msg_print("It's the picture of a friendly monster.");
#endif

		trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE2, 0L);
	}
	else if (die < 86)
	{
#ifdef JP
		msg_print("ͧ��Ū�ʥ�󥹥����γ�����");
#else
		msg_print("It's the picture of a friendly monster.");
#endif

		trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE4, 0L);
	}
	else if (die < 88)
	{
#ifdef JP
		msg_print("ͧ��Ū�ʥ�󥹥����γ�����");
#else
		msg_print("It's the picture of a friendly monster.");
#endif

		trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE5, 0L);
	}
	else if (die < 96)
	{
#ifdef JP
		msg_print("�����͡դ���");
#else
		msg_print("It's the Lovers.");
#endif

		if (get_aim_dir(&dir))
			charm_monster(dir, MIN(p_ptr->lev, 20));
	}
	else if (die < 101)
	{
#ifdef JP
		msg_print("�Ա��ԡդ���");
#else
		msg_print("It's the Hermit.");
#endif

		wall_stone();
	}
	else if (die < 111)
	{
#ifdef JP
		msg_print("�Կ�Ƚ�դ���");
#else
		msg_print("It's the Judgement.");
#endif

		do_cmd_rerate(FALSE);
		if (p_ptr->muta1 || p_ptr->muta2 || p_ptr->muta3)
		{
#ifdef JP
			msg_print("���Ƥ������Ѱۤ����ä���");
#else
			msg_print("You are cured of all mutations.");
#endif

			p_ptr->muta1 = p_ptr->muta2 = p_ptr->muta3 = 0;
			p_ptr->update |= PU_BONUS;
			handle_stuff();
		}
	}
	else if (die < 120)
	{
#ifdef JP
		msg_print("�����ۡդ���");
#else
		msg_print("It's the Sun.");
#endif

		chg_virtue(V_KNOWLEDGE, 1);
		chg_virtue(V_ENLIGHTEN, 1);
		wiz_lite(FALSE);
	}
	else
	{
#ifdef JP
		msg_print("�������դ���");
#else
		msg_print("It's the World.");
#endif

		if (p_ptr->exp < PY_MAX_EXP)
		{
			s32b ee = (p_ptr->exp / 25) + 1;
			if (ee > 5000) ee = 5000;
#ifdef JP
			msg_print("���˷и����Ѥ���褦�ʵ������롣");
#else
			msg_print("You feel more experienced.");
#endif

			gain_exp(ee);
		}
	}
}


/*
 * Drop 10+1d10 meteor ball at random places near the player
 */
static void cast_meteor(int dam, int rad)
{
	int i;
	int b = 10 + randint1(10);

	for (i = 0; i < b; i++)
	{
		int y, x;
		int count;

		for (count = 0; count <= 20; count++)
		{
			int dy, dx, d;

			x = px - 8 + randint0(17);
			y = py - 8 + randint0(17);

			dx = (px > x) ? (px - x) : (x - px);
			dy = (py > y) ? (py - y) : (y - py);

			/* Approximate distance */
			d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

			if (d >= 9) continue;

			if (!in_bounds(y, x) || !projectable(py, px, y, x)
			    || !cave_have_flag_bold(y, x, FF_PROJECT)) continue;

			/* Valid position */
			break;
		}

		if (count > 20) continue;

		project(0, rad, y, x, dam, GF_METEOR, PROJECT_KILL | PROJECT_JUMP | PROJECT_ITEM, -1);
	}
}


/*
 * Drop 10+1d10 disintegration ball at random places near the target
 */
static bool cast_wrath_of_the_god(int dam, int rad)
{
	int x, y, tx, ty;
	int nx, ny;
	int dir, i;
	int b = 10 + randint1(10);

	if (!get_aim_dir(&dir)) return FALSE;

	/* Use the given direction */
	tx = px + 99 * ddx[dir];
	ty = py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}

	x = px;
	y = py;

	while (1)
	{
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		ny = y;
		nx = x;
		mmove2(&ny, &nx, py, px, ty, tx);

		/* Stop at maximum range */
		if (MAX_RANGE <= distance(py, px, ny, nx)) break;

		/* Stopped by walls/doors */
		if (!cave_have_flag_bold(ny, nx, FF_PROJECT)) break;

		/* Stopped by monsters */
		if ((dir != 5) && cave[ny][nx].m_idx != 0) break;

		/* Save the new location */
		x = nx;
		y = ny;
	}
	tx = x;
	ty = y;

	for (i = 0; i < b; i++)
	{
		int count = 20, d = 0;

		while (count--)
		{
			int dx, dy;

			x = tx - 5 + randint0(11);
			y = ty - 5 + randint0(11);

			dx = (tx > x) ? (tx - x) : (x - tx);
			dy = (ty > y) ? (ty - y) : (y - ty);

			/* Approximate distance */
			d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));
			/* Within the radius */
			if (d < 5) break;
		}

		if (count < 0) continue;

		/* Cannot penetrate perm walls */
		if (!in_bounds(y,x) ||
		    cave_stop_disintegration(y,x) ||
		    !in_disintegration_range(ty, tx, y, x))
			continue;

		project(0, rad, y, x, dam, GF_DISINTEGRATE, PROJECT_JUMP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, -1);
	}

	return TRUE;
}


/*
 * An "item_tester_hook" for offer
 */
static bool item_tester_offer(object_type *o_ptr)
{
	/* Flasks of oil are okay */
	if (o_ptr->tval != TV_CORPSE) return (FALSE);

	if (o_ptr->sval != SV_CORPSE) return (FALSE);

	if (my_strchr("pht", r_info[o_ptr->pval].d_char)) return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
 * Daemon spell Summon Greater Demon
 */
static bool cast_summon_greater_demon(void)
{
	int plev = p_ptr->lev;
	int item;
	cptr q, s;
	int summon_lev;
	object_type *o_ptr;

	item_tester_hook = item_tester_offer;
#ifdef JP
	q = "�ɤλ��Τ������ޤ���? ";
	s = "����������Τ���äƤ��ʤ���";
#else
	q = "Sacrifice which corpse? ";
	s = "You have nothing to scrifice.";
#endif
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return FALSE;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	summon_lev = plev * 2 / 3 + r_info[o_ptr->pval].level;

	if (summon_specific(-1, py, px, summon_lev, SUMMON_HI_DEMON, (PM_ALLOW_GROUP | PM_FORCE_PET)))
	{
#ifdef JP
		msg_print("β���ΰ���������������");
#else
		msg_print("The area fills with a stench of sulphur and brimstone.");
#endif


#ifdef JP
		msg_print("�֤��ѤǤ������ޤ�����������͡�");
#else
		msg_print("'What is thy bidding... Master?'");
#endif

		/* Decrease the item (from the pack) */
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_describe(item);
			inven_item_optimize(item);
		}

		/* Decrease the item (from the floor) */
		else
		{
			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
	}
	else
	{
#ifdef JP
		msg_print("����ϸ���ʤ��ä���");
#else
		msg_print("No Greater Demon arrive.");
#endif
	}

	return TRUE;
}


/*
 * Start singing if the player is a Bard 
 */
static void start_singing(int spell, int song)
{
	/* Remember the song index */
	p_ptr->magic_num1[0] = song;

	/* Remember the index of the spell which activated the song */
	p_ptr->magic_num2[0] = spell;


	/* Now the player is singing */
	set_action(ACTION_SING);


	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw status bar */
	p_ptr->redraw |= (PR_STATUS);
}


/*
 * Stop singing if the player is a Bard 
 */
void stop_singing(void)
{
	if (p_ptr->pclass != CLASS_BARD) return;

 	/* Are there interupted song? */
	if (p_ptr->magic_num1[1])
	{
		/* Forget interupted song */
		p_ptr->magic_num1[1] = 0;
		return;
	}

	/* The player is singing? */
	if (!p_ptr->magic_num1[0]) return;

	/* Hack -- if called from set_action(), avoid recursive loop */
	if (p_ptr->action == ACTION_SING) set_action(ACTION_NONE);

	/* Message text of each song or etc. */
	do_spell(REALM_MUSIC, p_ptr->magic_num2[0], SPELL_STOP);

	p_ptr->magic_num1[0] = MUSIC_NONE;
	p_ptr->magic_num2[0] = 0;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw status bar */
	p_ptr->redraw |= (PR_STATUS);
}


static cptr do_life_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "�ڽ��μ���";
		if (desc) return "��������Ϥ򾯤����������롣";
#else
		if (name) return "Cure Light Wounds";
		if (desc) return "Heals cut and HP a little.";
#endif
    
		{
			int dice = 2;
			int sides = 10;

			if (info) return info_heal(dice, sides, 0);

			if (cast)
			{
				hp_player(damroll(dice, sides));
				set_cut(p_ptr->cut - 10);
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "��ʡ";
		if (desc) return "������֡�̿��Ψ��AC�˥ܡ��ʥ������롣";
#else
		if (name) return "Bless";
		if (desc) return "Gives bonus to hit and AC for a few turns.";
#endif
    
		{
			int base = 12;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_blessed(randint1(base) + base, FALSE);
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "�ڽ�";
		if (desc) return "1�ΤΥ�󥹥����˾����᡼����Ϳ���롣�񹳤�����̵����";
#else
		if (name) return "Cause Light Wounds";
		if (desc) return "Wounds a monster a little unless resisted.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball_hide(GF_WOUNDS, dir, damroll(dice, sides), 0);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "���ξ���";
		if (desc) return "�������Ȥ餷�Ƥ����ϰϤ��������Τ�ʵפ����뤯���롣";
#else
		if (name) return "Call Light";
		if (desc) return "Lights up nearby area and the inside of a room permanently.";
#endif
    
		{
			int dice = 2;
			int sides = plev / 2;
			int rad = plev / 10 + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				lite_area(damroll(dice, sides), rad);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "� & �����ⴶ��";
		if (desc) return "�᤯�����Ƥ�櫤���ȳ��ʤ��Τ��롣";
#else
		if (name) return "Detect Doors & Traps";
		if (desc) return "Detects traps, doors, and stairs in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_traps(rad, TRUE);
				detect_doors(rad);
				detect_stairs(rad);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�Ž��μ���";
		if (desc) return "��������Ϥ������ٲ��������롣";
#else
		if (name) return "Cure Medium Wounds";
		if (desc) return "Heals cut and HP more.";
#endif
    
		{
			int dice = 4;
			int sides = 10;

			if (info) return info_heal(dice, sides, 0);

			if (cast)
			{
				hp_player(damroll(dice, sides));
				set_cut((p_ptr->cut / 2) - 20);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "����";
		if (desc) return "������Ǥ��������";
#else
		if (name) return "Cure Poison";
		if (desc) return "Cure poison status.";
#endif
    
		{
			if (cast)
			{
				set_poisoned(0);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "��ʢ��­";
		if (desc) return "��ʢ�ˤ��롣";
#else
		if (name) return "Satisfy Hunger";
		if (desc) return "Satisfies hunger.";
#endif
    
		{
			if (cast)
			{
				set_food(PY_FOOD_MAX - 1);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "���";
		if (desc) return "�����ƥ�ˤ����ä��夤�����������롣";
#else
		if (name) return "Remove Curse";
		if (desc) return "Removes normal curses from equipped items.";
#endif

		{
			if (cast)
			{
				if (remove_curse())
				{
#ifdef JP
					msg_print("ï���˸�����Ƥ���褦�ʵ������롣");
#else
					msg_print("You feel as if someone is watching over you.");
#endif
				}
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "�Ž�";
		if (desc) return "1�ΤΥ�󥹥���������᡼����Ϳ���롣�񹳤�����̵����";
#else
		if (name) return "Cause Medium Wounds";
		if (desc) return "Wounds a monster unless resisted.";
#endif
    
		{
			int sides = 8 + (plev - 5) / 4;
			int dice = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball_hide(GF_WOUNDS, dir, damroll(sides, dice), 0);
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "��̿���μ���";
		if (desc) return "���Ϥ������˲��������������ۯ۰���֤��������롣";
#else
		if (name) return "Cure Critical Wounds";
		if (desc) return "Heals cut, stun and HP greatly.";
#endif
    
		{
			int dice = 8;
			int sides = 10;

			if (info) return info_heal(dice, sides, 0);

			if (cast)
			{
				hp_player(damroll(dice, sides));
				set_stun(0);
				set_cut(0);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "��Ǯ�Ѵ�";
		if (desc) return "������֡��б���䵤���Ф������������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Heat and Cold";
		if (desc) return "Gives resistance to fire and cold. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_cold(randint1(base) + base, FALSE);
				set_oppose_fire(randint1(base) + base, FALSE);
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "���մ���";
		if (desc) return "���դ��Ϸ����Τ��롣";
#else
		if (name) return "Sense Surroundings";
		if (desc) return "Maps nearby area.";
#endif
    
		{
			int rad = DETECT_RAD_MAP;

			if (info) return info_radius(rad);

			if (cast)
			{
				map_area(rad);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "�ѥ˥å�������ǥå�";
		if (desc) return "�볦��Υ���ǥåɤ��ݤ����롣�񹳤�����̵����";
#else
		if (name) return "Turn Undead";
		if (desc) return "Attempts to scare undead monsters in sight.";
#endif
    
		{
			if (cast)
			{
				turn_undead();
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "���ϲ���";
		if (desc) return "�ˤ�ƶ��Ϥʲ�����ʸ�ǡ������ۯ۰���֤��������롣";
#else
		if (name) return "Healing";
		if (desc) return "Much powerful healing magic, and heals cut and stun completely.";
#endif
    
		{
			int heal = 300;

			if (info) return info_heal(0, 0, heal);

			if (cast)
			{
				hp_player(heal);
				set_stun(0);
				set_cut(0);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "�볦�����";
		if (desc) return "��ʬ�Τ��뾲�ξ�ˡ���󥹥������̤�ȴ�����꾤�����줿�ꤹ�뤳�Ȥ��Ǥ��ʤ��ʤ�롼���������";
#else
		if (name) return "Glyph of Warding";
		if (desc) return "Sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break glyph.";
#endif
    
		{
			if (cast)
			{
				warding_glyph();
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "*���*";
		if (desc) return "�����ƥ�ˤ����ä����Ϥʼ����������롣";
#else
		if (name) return "Dispel Curse";
		if (desc) return "Removes normal and heavy curse from equipped items.";
#endif
    
		{
			if (cast)
			{
				if (remove_all_curse())
				{
#ifdef JP
					msg_print("ï���˸�����Ƥ���褦�ʵ������롣");
#else
					msg_print("You feel as if someone is watching over you.");
#endif
				}
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "�ռ�";
		if (desc) return "�����ƥ���̤��롣";
#else
		if (name) return "Perception";
		if (desc) return "Identifies an item.";
#endif
    
		{
			if (cast)
			{
				if (!ident_spell(FALSE)) return NULL;
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "����ǥå��໶";
		if (desc) return "�볦������ƤΥ���ǥåɤ˥��᡼����Ϳ���롣";
#else
		if (name) return "Dispel Undead";
		if (desc) return "Damages all undead monsters in sight.";
#endif
    
		{
			int dice = 1;
			int sides = plev * 5;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				dispel_undead(damroll(dice, sides));
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "��ι�";
		if (desc) return "�볦������ƤΥ�󥹥�����̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Day of the Dove";
		if (desc) return "Attempts to charm all monsters in sight.";
#endif
    
		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cast)
			{
				charm_monsters(power);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "��̿��";
		if (desc) return "1�ΤΥ�󥹥���������᡼����Ϳ���롣�񹳤�����̵����";
#else
		if (name) return "Cause Critical Wounds";
		if (desc) return "Wounds a monster critically unless resisted.";
#endif
    
		{
			int dice = 5 + (plev - 5) / 3;
			int sides = 15;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball_hide(GF_WOUNDS, dir, damroll(dice, sides), 0);
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "���Ԥξ�";
		if (desc) return "�Ͼ�ˤ���Ȥ��ϥ��󥸥��κǿ����ء����󥸥��ˤ���Ȥ����Ͼ�ؤȰ�ư���롣";
#else
		if (name) return "Word of Recall";
		if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				if (!word_of_recall()) return NULL;
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "���¤κ���";
		if (desc) return "���ߤγ���ƹ������롣";
#else
		if (name) return "Alter Reality";
		if (desc) return "Recreates current dungeon level.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				alter_reality();
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "�����볦";
		if (desc) return "��ʬ�Τ��뾲�ȼ���8�ޥ��ξ��ξ�ˡ���󥹥������̤�ȴ�����꾤�����줿�ꤹ�뤳�Ȥ��Ǥ��ʤ��ʤ�롼���������";
#else
		if (name) return "Warding True";
		if (desc) return "Creates glyphs in all adjacent squares and under you.";
#endif
    
		{
			int rad = 1;

			if (info) return info_radius(rad);

			if (cast)
			{
				warding_glyph();
				glyph_creation();
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "���Ӳ�";
		if (desc) return "���γ������������󥹥����������Ǥ��ʤ��ʤ롣";
#else
		if (name) return "Sterilization";
		if (desc) return "Prevents any breeders on current level from breeding.";
#endif
    
		{
			if (cast)
			{
				num_repro += MAX_REPRO;
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "������";
		if (desc) return "�᤯�����ƤΥ�󥹥�����櫡��⡢���ʡ������������ƥ����ƥ���Τ��롣";
#else
		if (name) return "Detection";
		if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";
#endif

		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_all(rad);
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "����ǥåɾ���";
		if (desc) return "��ʬ�μ��Ϥˤ��륢��ǥåɤ򸽺ߤγ�����ä���롣�񹳤�����̵����";
#else
		if (name) return "Annihilate Undead";
		if (desc) return "Eliminates all nearby undead monsters, exhausting you.  Powerful or unique monsters may be able to resist.";
#endif
    
		{
			int power = plev + 50;

			if (info) return info_power(power);

			if (cast)
			{
				mass_genocide_undead(power, TRUE);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "��Τ��";
		if (desc) return "���γ����Τ�ʵפ˾Ȥ餷�����󥸥���⤹�٤ƤΥ����ƥ���Τ��롣";
#else
		if (name) return "Clairvoyance";
		if (desc) return "Maps and lights whole dungeon level. Knows all objects location. And gives telepathy for a while.";
#endif
    
		{
			if (cast)
			{
				wiz_lite(FALSE);
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "������";
		if (desc) return "���٤ƤΥ��ơ������ȷи��ͤ�������롣";
#else
		if (name) return "Restoration";
		if (desc) return "Restores all stats and experience.";
#endif
    
		{
			if (cast)
			{
				do_res_stat(A_STR);
				do_res_stat(A_INT);
				do_res_stat(A_WIS);
				do_res_stat(A_DEX);
				do_res_stat(A_CON);
				do_res_stat(A_CHR);
				restore_level();
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "*���ϲ���*";
		if (desc) return "�Ƕ��μ�������ˡ�ǡ������ۯ۰���֤��������롣";
#else
		if (name) return "Healing True";
		if (desc) return "The greatest healing magic. Heals all HP, cut and stun.";
#endif
    
		{
			int heal = 2000;

			if (info) return info_heal(0, 0, heal);

			if (cast)
			{
				hp_player(heal);
				set_stun(0);
				set_cut(0);
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "���ʤ�ӥ����";
		if (desc) return "�����ƥ�λ���ǽ�Ϥ������Τ롣";
#else
		if (name) return "Holy Vision";
		if (desc) return "*Identifies* an item.";
#endif
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "��ˤ�����";
		if (desc) return "������֡��������������դ���AC����ˡ�ɸ�ǽ�Ϥ�徺�����롣";
#else
		if (name) return "Ultimate Resistance";
		if (desc) return "Gives ultimate resistance, bonus to AC and speed.";
#endif
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				int v = randint1(base) + base;
				set_fast(v, FALSE);
				set_oppose_acid(v, FALSE);
				set_oppose_elec(v, FALSE);
				set_oppose_fire(v, FALSE);
				set_oppose_cold(v, FALSE);
				set_oppose_pois(v, FALSE);
				set_ultimate_res(v, FALSE);
			}
		}
		break;
	}

	return "";
}


static cptr do_sorcery_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "��󥹥�������";
		if (desc) return "�᤯�����Ƥθ������󥹥������Τ��롣";
#else
		if (name) return "Detect Monsters";
		if (desc) return "Detects all monsters in your vicinity unless invisible.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_normal(rad);
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "���硼�ȡ��ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Phase Door";
		if (desc) return "Teleport short distance.";
#endif
    
		{
			int range = 10;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "櫤��ⴶ��";
		if (desc) return "�᤯�����Ƥ����櫤��Τ��롣";
#else
		if (name) return "Detect Doors and Traps";
		if (desc) return "Detects traps, doors, and stairs in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_traps(rad, TRUE);
				detect_doors(rad);
				detect_stairs(rad);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "�饤�ȡ����ꥢ";
		if (desc) return "�������Ȥ餷�Ƥ����ϰϤ��������Τ�ʵפ����뤯���롣";
#else
		if (name) return "Light Area";
		if (desc) return "Lights up nearby area and the inside of a room permanently.";
#endif
    
		{
			int dice = 2;
			int sides = plev / 2;
			int rad = plev / 10 + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				lite_area(damroll(dice, sides), rad);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "�ѥ˥å�����󥹥���";
		if (desc) return "��󥹥���1�Τ��𤵤��롣�񹳤�����̵����";
#else
		if (name) return "Confuse Monster";
		if (desc) return "Attempts to confuse a monster.";
#endif
    
		{
			int power = (plev * 3) / 2;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				confuse_monster(dir, power);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Teleport";
		if (desc) return "Teleport long distance.";
#endif
    
		{
			int range = plev * 5;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "���꡼�ס���󥹥���";
		if (desc) return "��󥹥���1�Τ�̲�餻�롣�񹳤�����̵����";
#else
		if (name) return "Sleep Monster";
		if (desc) return "Attempts to sleep a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				sleep_monster(dir);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "���Ͻ�Ŷ";
		if (desc) return "��/��ˡ���ν�Ŷ��������䤹������Ŷ��Υ�åɤν�Ŷ���֤򸺤餹��";
#else
		if (name) return "Recharging";
		if (desc) return "Recharges staffs, wands or rods.";
#endif
    
		{
			int power = plev * 4;

			if (info) return info_power(power);

			if (cast)
			{
				if (!recharge(power)) return NULL;
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "��ˡ���Ͽ�";
		if (desc) return "���դ��Ϸ����Τ��롣";
#else
		if (name) return "Magic Mapping";
		if (desc) return "Maps nearby area.";
#endif
    
		{
			int rad = DETECT_RAD_MAP;

			if (info) return info_radius(rad);

			if (cast)
			{
				map_area(rad);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "����";
		if (desc) return "�����ƥ���̤��롣";
#else
		if (name) return "Identify";
		if (desc) return "Identifies an item.";
#endif
    
		{
			if (cast)
			{
				if (!ident_spell(FALSE)) return NULL;
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "��������󥹥���";
		if (desc) return "��󥹥���1�Τ�®���롣�񹳤�����̵����";
#else
		if (name) return "Slow Monster";
		if (desc) return "Attempts to slow a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				slow_monster(dir);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "���ե��꡼��";
		if (desc) return "�볦������ƤΥ�󥹥�����̲�餻�롣�񹳤�����̵����";
#else
		if (name) return "Mass Sleep";
		if (desc) return "Attempts to sleep all monsters in sight.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				sleep_monsters();
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���󥹥���";
		if (desc) return "��󥹥�����ƥ�ݡ��Ȥ�����ӡ�������ġ��񹳤�����̵����";
#else
		if (name) return "Teleport Away";
		if (desc) return "Teleports all monsters on the line away unless resisted.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_AWAY_ALL, dir, power);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "���ԡ���";
		if (desc) return "������֡���®���롣";
#else
		if (name) return "Haste Self";
		if (desc) return "Hastes you for a while.";
#endif
    
		{
			int base = plev;
			int sides = 20 + plev;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_fast(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "��������";
		if (desc) return "�᤯�����ƤΥ�󥹥�����櫡��⡢���ʡ������������ƥ����ƥ���Τ��롣";
#else
		if (name) return "Detection True";
		if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_all(rad);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "��������";
		if (desc) return "�����ƥ�λ���ǽ�Ϥ������Τ롣";
#else
		if (name) return "Identify True";
		if (desc) return "*Identifies* an item.";
#endif
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "ʪ�ΤȺ�������";
		if (desc) return "�᤯�����ƤΥ����ƥ�Ⱥ������Τ��롣";
#else
		if (name) return "Detect items and Treasure";
		if (desc) return "Detects all treasures and items in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_objects_normal(rad);
				detect_treasure(rad);
				detect_objects_gold(rad);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "���㡼�ࡦ��󥹥���";
		if (desc) return "��󥹥���1�Τ�̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Charm Monster";
		if (desc) return "Attempts to charm a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				charm_monster(dir, power);
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "��������";
		if (desc) return "������֡��ƥ�ѥ���ǽ�Ϥ����롣";
#else
		if (name) return "Sense Minds";
		if (desc) return "Gives telepathy for a while.";
#endif
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "����ư";
		if (desc) return "���ذ�ư���롣�Ͼ�ˤ���Ȥ������Ȥ��ʤ���";
#else
		if (name) return "Teleport to town";
		if (desc) return "Teleport to a town which you choose in a moment. Can only be used outdoors.";
#endif
    
		{
			if (cast)
			{
				if (!tele_town()) return NULL;
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "����ʬ��";
		if (desc) return "���ߤμ�ʬ�ξ��֤������Τ롣";
#else
		if (name) return "Self Knowledge";
		if (desc) return "Gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.";
#endif
    
		{
			if (cast)
			{
				self_knowledge();
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���٥�";
		if (desc) return "�ֻ��˾夫���γ��˥ƥ�ݡ��Ȥ��롣";
#else
		if (name) return "Teleport Level";
		if (desc) return "Teleport to up or down stairs in a moment.";
#endif
    
		{
			if (cast)
			{
#ifdef JP
				if (!get_check("������¾�γ��˥ƥ�ݡ��Ȥ��ޤ�����")) return NULL;
#else
				if (!get_check("Are you sure? (Teleport Level)")) return NULL;
#endif
				teleport_level(0);
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "���Ԥμ�ʸ";
		if (desc) return "�Ͼ�ˤ���Ȥ��ϥ��󥸥��κǿ����ء����󥸥��ˤ���Ȥ����Ͼ�ؤȰ�ư���롣";
#else
		if (name) return "Word of Recall";
		if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				if (!word_of_recall()) return NULL;
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "��������";
		if (desc) return "û��Υ��λ��ꤷ�����˥ƥ�ݡ��Ȥ��롣";
#else
		if (name) return "Dimension Door";
		if (desc) return "Teleport to given location.";
#endif
    
		{
			int range = plev / 2 + 10;

			if (info) return info_range(range);

			if (cast)
			{
#ifdef JP
				msg_print("�������⤬����������Ū�Ϥ�����ǲ�������");
#else
				msg_print("You open a dimensional gate. Choose a destination.");
#endif

				if (!dimension_door()) return NULL;
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "Ĵ��";
		if (desc) return "��󥹥�����°�����Ĥ����ϡ��������ϡ����ԡ��ɡ����Τ��Τ롣";
#else
		if (name) return "Probing";
		if (desc) return "Proves all monsters' alignment, HP, speed and their true character.";
#endif
    
		{
			if (cast)
			{
				probing();
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "��ȯ�Υ롼��";
		if (desc) return "��ʬ�Τ��뾲�ξ�ˡ���󥹥������̤����ȯ���ƥ��᡼����Ϳ����롼���������";
#else
		if (name) return "Explosive Rune";
		if (desc) return "Sets a glyph under you. The glyph will explode when a monster moves on it.";
#endif
    
		{
			int dice = 7;
			int sides = 7;
			int base = plev;

			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				explosive_rune();
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "ǰư��";
		if (desc) return "�����ƥ��ʬ��­���ذ�ư�����롣";
#else
		if (name) return "Telekinesis";
		if (desc) return "Pulls a distant item close to you.";
#endif
    
		{
			int weight = plev * 15;

			if (info) return info_weight(weight);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fetch(dir, weight, FALSE);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "��Τ��";
		if (desc) return "���γ����Τ�ʵפ˾Ȥ餷�����󥸥���⤹�٤ƤΥ����ƥ���Τ��롣����ˡ�������֥ƥ�ѥ���ǽ�Ϥ����롣";
#else
		if (name) return "Clairvoyance";
		if (desc) return "Maps and lights whole dungeon level. Knows all objects location. And gives telepathy for a while.";
#endif
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				chg_virtue(V_KNOWLEDGE, 1);
				chg_virtue(V_ENLIGHTEN, 1);

				wiz_lite(FALSE);

				if (!p_ptr->telepathy)
				{
					set_tim_esp(randint1(sides) + base, FALSE);
				}
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "̥λ�λ���";
		if (desc) return "�볦������ƤΥ�󥹥�����̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Charm monsters";
		if (desc) return "Attempts to charm all monsters in sight.";
#endif
    
		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cast)
			{
				charm_monsters(power);
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "ϣ���";
		if (desc) return "�����ƥ�1�Ĥ򤪶���Ѥ��롣";
#else
		if (name) return "Alchemy";
		if (desc) return "Turns an item into 1/3 of its value in gold.";
#endif
    
		{
			if (cast)
			{
				if (!alchemy()) return NULL;
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "��ʪ����";
		if (desc) return "�볦������ƤΥ�󥹥�����ƥ�ݡ��Ȥ����롣�񹳤�����̵����";
#else
		if (name) return "Banishment";
		if (desc) return "Teleports all monsters in sight away unless resisted.";
#endif
    
		{
			int power = plev * 4;

			if (info) return info_power(power);

			if (cast)
			{
				banish_monsters(power);
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "̵���ε�";
		if (desc) return "������֡����᡼��������ʤ��ʤ�Хꥢ��ĥ�롣�ڤ줿�ִ֤˾������������񤹤�Τ���ա�";
#else
		if (name) return "Globe of Invulnerability";
		if (desc) return "Generates barrier which completely protect you from almost all damages. Takes a few your turns when the barrier breaks or duration time is exceeded.";
#endif
    
		{
			int base = 4;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_invuln(randint1(base) + base, FALSE);
			}
		}
		break;
	}

	return "";
}


static cptr do_nature_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

#ifdef JP
	static const char s_dam[] = "»��:";
	static const char s_rng[] = "����";
#else
	static const char s_dam[] = "dam ";
	static const char s_rng[] = "rng ";
#endif

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "��󥹥�������";
		if (desc) return "�᤯�����Ƥθ������󥹥������Τ��롣";
#else
		if (name) return "Detect Creatures";
		if (desc) return "Detects all monsters in your vicinity unless invisible.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_normal(rad);
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "���";
		if (desc) return "�ŷ��û���ӡ�������ġ�";
#else
		if (name) return "Lightning";
		if (desc) return "Fires a short beam of lightning.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;
			int range = plev / 6 + 2;

			if (info) return format("%s%dd%d %s%d", s_dam, dice, sides, s_rng, range);

			if (cast)
			{
				project_length = range;

				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_ELEC, dir, damroll(dice, sides));
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "櫤��ⴶ��";
		if (desc) return "�᤯�����Ƥ�櫤�����Τ��롣";
#else
		if (name) return "Detect Doors and Traps";
		if (desc) return "Detects traps, doors, and stairs in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_traps(rad, TRUE);
				detect_doors(rad);
				detect_stairs(rad);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "��������";
		if (desc) return "�������ĺ��Ф���";
#else
		if (name) return "Produce Food";
		if (desc) return "Produces a Ration of Food.";
#endif
    
		{
			if (cast)
			{
				object_type forge, *q_ptr = &forge;

#ifdef JP
				msg_print("����������������");
#else
				msg_print("A food ration is produced.");
#endif

				/* Create the food ration */
				object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));

				/* Drop the object from heaven */
				drop_near(q_ptr, -1, py, px);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "���θ�";
		if (desc) return "�������Ȥ餷�Ƥ����ϰϤ��������Τ�ʵפ����뤯���롣";
#else
		if (name) return "Daylight";
		if (desc) return "Lights up nearby area and the inside of a room permanently.";
#endif
    
		{
			int dice = 2;
			int sides = plev / 2;
			int rad = (plev / 10) + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				lite_area(damroll(dice, sides), rad);

				if ((prace_is_(RACE_VAMPIRE) || (p_ptr->mimic_form == MIMIC_VAMPIRE)) && !p_ptr->resist_lite)
				{
#ifdef JP
					msg_print("���θ������ʤ������Τ�Ǥ�������");
#else
					msg_print("The daylight scorches your flesh!");
#endif

#ifdef JP
					take_hit(DAMAGE_NOESCAPE, damroll(2, 2), "���θ�", -1);
#else
					take_hit(DAMAGE_NOESCAPE, damroll(2, 2), "daylight", -1);
#endif
				}
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "ưʪ����";
		if (desc) return "ưʪ1�Τ�̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Animal Taming";
		if (desc) return "Attempts to charm an animal.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				charm_animal(dir, power);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "�Ķ��ؤ�����";
		if (desc) return "������֡��䵤���ꡢ�ŷ���Ф������������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Environment";
		if (desc) return "Gives resistance to fire, cold and electricity for a while. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_cold(randint1(base) + base, FALSE);
				set_oppose_fire(randint1(base) + base, FALSE);
				set_oppose_elec(randint1(base) + base, FALSE);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "�����Ǽ���";
		if (desc) return "����������������Ǥ��Τ��鴰���˼����������Ϥ򾯤����������롣";
#else
		if (name) return "Cure Wounds & Poison";
		if (desc) return "Heals all cut and poison status. Heals HP a little.";
#endif
    
		{
			int dice = 2;
			int sides = 8;

			if (info) return info_heal(dice, sides, 0);

			if (cast)
			{
				hp_player(damroll(dice, sides));
				set_cut(0);
				set_poisoned(0);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "�����ϲ�";
		if (desc) return "�ɤ��Ϥ����ƾ��ˤ��롣";
#else
		if (name) return "Stone to Mud";
		if (desc) return "Turns one rock square to mud.";
#endif
    
		{
			int dice = 1;
			int sides = 30;
			int base = 20;

			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				wall_to_mud(dir);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "���������ܥ��";
		if (desc) return "�䵤�Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Frost Bolt";
		if (desc) return "Fires a bolt or beam of cold.";
#endif
    
		{
			int dice = 3 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir, damroll(dice, sides));
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "�����γ���";
		if (desc) return "���դ��Ϸ����Τ����᤯��櫡��⡢���ʡ����ƤΥ�󥹥������Τ��롣";
#else
		if (name) return "Nature Awareness";
		if (desc) return "Maps nearby area. Detects all monsters, traps, doors and stairs.";
#endif
    
		{
			int rad1 = DETECT_RAD_MAP;
			int rad2 = DETECT_RAD_DEFAULT;

			if (info) return info_radius(MAX(rad1, rad2));

			if (cast)
			{
				map_area(rad1);
				detect_traps(rad2, TRUE);
				detect_doors(rad2);
				detect_stairs(rad2);
				detect_monsters_normal(rad2);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "�ե��������ܥ��";
		if (desc) return "�б�Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Fire Bolt";
		if (desc) return "Fires a bolt or beam of fire.";
#endif
    
		{
			int dice = 5 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_bolt_or_beam(beam_chance() - 10, GF_FIRE, dir, damroll(dice, sides));
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "���۸���";
		if (desc) return "���������ġ�����������󥹥����˸��̤����롣";
#else
		if (name) return "Ray of Sunlight";
		if (desc) return "Fires a beam of light which damages to light-sensitive monsters.";
#endif
    
		{
			int dice = 6;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
#ifdef JP
				msg_print("���۸��������줿��");
#else
				msg_print("A line of sunlight appears.");
#endif

				lite_line(dir);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "­����";
		if (desc) return "�볦������ƤΥ�󥹥�����®�����롣�񹳤�����̵����";
#else
		if (name) return "Entangle";
		if (desc) return "Attempts to slow all monsters in sight.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				slow_monsters();
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "ưʪ����";
		if (desc) return "ưʪ��1�ξ������롣";
#else
		if (name) return "Summon Animal";
		if (desc) return "Summons an animal.";
#endif
    
		{
			if (cast)
			{
				if (!(summon_specific(-1, py, px, plev, SUMMON_ANIMAL_RANGER, (PM_ALLOW_GROUP | PM_FORCE_PET))))
				{
#ifdef JP
					msg_print("ưʪ�ϸ���ʤ��ä���");
#else
					msg_print("No animals arrive.");
#endif
				}
				break;
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "������";
		if (desc) return "���Ϥ������˲��������������ۯ۰���֡��Ǥ����������롣";
#else
		if (name) return "Herbal Healing";
		if (desc) return "Heals HP greatly. And heals cut, stun and poison completely.";
#endif
    
		{
			int heal = 500;

			if (info) return info_heal(0, 0, heal);

			if (cast)
			{
				hp_player(heal);
				set_stun(0);
				set_cut(0);
				set_poisoned(0);
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "��������";
		if (desc) return "��ʬ�Τ�����֤˳��ʤ��롣";
#else
		if (name) return "Stair Building";
		if (desc) return "Creates a stair which goes down or up.";
#endif
    
		{
			if (cast)
			{
				stair_creation();
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "ȩ�в�";
		if (desc) return "������֡�AC��徺�����롣";
#else
		if (name) return "Stone Skin";
		if (desc) return "Gives bonus to AC for a while.";
#endif
    
		{
			int base = 20;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_shield(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "��������";
		if (desc) return "������֡������ŷ⡢�ꡢ�䵤���Ǥ��Ф������������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resistance True";
		if (desc) return "Gives resistance to fire, cold, electricity, acid and poison for a while. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_acid(randint1(base) + base, FALSE);
				set_oppose_elec(randint1(base) + base, FALSE);
				set_oppose_fire(randint1(base) + base, FALSE);
				set_oppose_cold(randint1(base) + base, FALSE);
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "������¤";
		if (desc) return "���Ϥ��ڤ���Ф���";
#else
		if (name) return "Forest Creation";
		if (desc) return "Creates trees in all adjacent squares.";
#endif
    
		{
			if (cast)
			{
				tree_creation();
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "ưʪͧ��";
		if (desc) return "�볦������Ƥ�ưʪ��̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Animal Friendship";
		if (desc) return "Attempts to charm all animals in sight.";
#endif
    
		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cast)
			{
				charm_animals(power);
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�����";
		if (desc) return "�����ƥ�λ���ǽ�Ϥ������Τ롣";
#else
		if (name) return "Stone Tell";
		if (desc) return "*Identifies* an item.";
#endif
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "�Ф���";
		if (desc) return "��ʬ�μ��Ϥ˲�־����ɤ��롣";
#else
		if (name) return "Wall of Stone";
		if (desc) return "Creates granite walls in all adjacent squares.";
#endif
    
		{
			if (cast)
			{
				wall_stone();
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "�忩�ɻ�";
		if (desc) return "�����ƥ����ǽ��Ĥ��ʤ��褦�ù����롣";
#else
		if (name) return "Protect from Corrosion";
		if (desc) return "Makes an equipment acid-proof.";
#endif
    
		{
			if (cast)
			{
				if (!rustproof()) return NULL;
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "�Ͽ�";
		if (desc) return "���ϤΥ��󥸥����ɤ餷���ɤȾ��������������Ѥ��롣";
#else
		if (name) return "Earthquake";
		if (desc) return "Shakes dungeon structure, and results in random swapping of floors and walls.";
#endif
    
		{
			int rad = 10;

			if (info) return info_radius(rad);

			if (cast)
			{
				earthquake(py, px, rad);
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "���ޥ�����";
		if (desc) return "�������˸����äƹ��⤹�롣";
#else
		if (name) return "Cyclone";
		if (desc) return "Attacks all adjacent monsters.";
#endif
    
		{
			if (cast)
			{
				int y = 0, x = 0;
				cave_type       *c_ptr;
				monster_type    *m_ptr;

				for (dir = 0; dir < 8; dir++)
				{
					y = py + ddy_ddd[dir];
					x = px + ddx_ddd[dir];
					c_ptr = &cave[y][x];

					/* Get the monster */
					m_ptr = &m_list[c_ptr->m_idx];

					/* Hack -- attack monsters */
					if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
						py_attack(y, x, 0);
				}
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "�֥ꥶ����";
		if (desc) return "������䵤�ε�����ġ�";
#else
		if (name) return "Blizzard";
		if (desc) return "Fires a huge ball of cold.";
#endif
    
		{
			int dam = 70 + plev * 3 / 2;
			int rad = plev / 12 + 1;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_COLD, dir, dam, rad);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "�����";
		if (desc) return "������ŷ�ε�����ġ�";
#else
		if (name) return "Lightning Storm";
		if (desc) return "Fires a huge electric ball.";
#endif
    
		{
			int dam = 90 + plev * 3 / 2;
			int rad = plev / 12 + 1;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball(GF_ELEC, dir, dam, rad);
				break;
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "��Ĭ";
		if (desc) return "����ʿ�ε�����ġ�";
#else
		if (name) return "Whirlpool";
		if (desc) return "Fires a huge ball of water.";
#endif
    
		{
			int dam = 100 + plev * 3 / 2;
			int rad = plev / 12 + 1;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball(GF_WATER, dir, dam, rad);
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "�۸�����";
		if (desc) return "��ʬ���濴�Ȥ������ε��ȯ�������롣����ˡ����γ����Τ�ʵפ˾Ȥ餷�����󥸥���⤹�٤ƤΥ����ƥ���Τ��롣";
#else
		if (name) return "Call Sunlight";
		if (desc) return "Generates ball of light centered on you. Maps and lights whole dungeon level. Knows all objects location.";
#endif
    
		{
			int dam = 150;
			int rad = 8;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
				fire_ball(GF_LITE, 0, dam, rad);
				chg_virtue(V_KNOWLEDGE, 1);
				chg_virtue(V_ENLIGHTEN, 1);
				wiz_lite(FALSE);

				if ((prace_is_(RACE_VAMPIRE) || (p_ptr->mimic_form == MIMIC_VAMPIRE)) && !p_ptr->resist_lite)
				{
#ifdef JP
					msg_print("���������ʤ������Τ�Ǥ�������");
#else
					msg_print("The sunlight scorches your flesh!");
#endif

#ifdef JP
					take_hit(DAMAGE_NOESCAPE, 50, "����", -1);
#else
					take_hit(DAMAGE_NOESCAPE, 50, "sunlight", -1);
#endif
				}
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "����ο�";
		if (desc) return "���˱꤫�䵤��°����Ĥ��롣";
#else
		if (name) return "Elemental Branding";
		if (desc) return "Makes current weapon fire or frost branded.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(randint0(2));
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "�����ζ���";
		if (desc) return "�᤯�����ƤΥ�󥹥����˥��᡼����Ϳ�����Ͽ̤򵯤�������ʬ���濴�Ȥ���ʬ��ε��ȯ�������롣";
#else
		if (name) return "Nature's Wrath";
		if (desc) return "Damages all monsters in sight. Makes quake. Generates disintegration ball centered on you.";
#endif
    
		{
			int d_dam = 4 * plev;
			int b_dam = (100 + plev) * 2;
			int b_rad = 1 + plev / 12;
			int q_rad = 20 + plev / 2;

			if (info) return format("%s%d+%d", s_dam, d_dam, b_dam/2);

			if (cast)
			{
				dispel_monsters(d_dam);
				earthquake(py, px, q_rad);
				project(0, b_rad, py, px, b_dam, GF_DISINTEGRATE, PROJECT_KILL | PROJECT_ITEM, -1);
			}
		}
		break;
	}

	return "";
}


static cptr do_chaos_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

#ifdef JP
	static const char s_dam[] = "»��:";
	static const char s_random[] = "������";
#else
	static const char s_dam[] = "dam ";
	static const char s_random[] = "random";
#endif

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "�ޥ��å����ߥ�����";
		if (desc) return "�夤��ˡ��������ġ�";
#else
		if (name) return "Magic Missile";
		if (desc) return "Fires a weak bolt of magic.";
#endif
    
		{
			int dice = 3 + ((plev - 1) / 5);
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir, damroll(dice, sides));
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "�ȥ�å�/�ɥ��˲�";
		if (desc) return "���ܤ���櫤�����˲����롣";
#else
		if (name) return "Trap / Door Destruction";
		if (desc) return "Destroys all traps in adjacent squares.";
#endif
    
		{
			int rad = 1;

			if (info) return info_radius(rad);

			if (cast)
			{
				destroy_doors_touch();
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "����";
		if (desc) return "�������Ȥ餷�Ƥ����ϰϤ��������Τ�ʵפ����뤯���롣";
#else
		if (name) return "Flash of Light";
		if (desc) return "Lights up nearby area and the inside of a room permanently.";
#endif
    
		{
			int dice = 2;
			int sides = plev / 2;
			int rad = (plev / 10) + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				lite_area(damroll(dice, sides), rad);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "����μ�";
		if (desc) return "�����𤵤��빶���Ǥ���褦�ˤ��롣";
#else
		if (name) return "Touch of Confusion";
		if (desc) return "Attempts to confuse the next monster that you hit.";
#endif
    
		{
			if (cast)
			{
				if (!(p_ptr->special_attack & ATTACK_CONFUSE))
				{
#ifdef JP
					msg_print("���ʤ��μ�ϸ���Ϥ᤿��");
#else
					msg_print("Your hands start glowing.");
#endif

					p_ptr->special_attack |= ATTACK_CONFUSE;
					p_ptr->redraw |= (PR_STATUS);
				}
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "��������";
		if (desc) return "��ˡ�ε�����ġ�";
#else
		if (name) return "Mana Burst";
		if (desc) return "Fires a ball of magic.";
#endif
    
		{
			int dice = 3;
			int sides = 5;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_MAGE ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_MISSILE, dir, damroll(dice, sides) + base, rad);

				/*
				 * Shouldn't actually use GF_MANA, as
				 * it will destroy all items on the
				 * floor
				 */
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�ե��������ܥ��";
		if (desc) return "��Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Fire Bolt";
		if (desc) return "Fires a bolt or beam of fire.";
#endif
    
		{
			int dice = 8 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_FIRE, dir, damroll(dice, sides));
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "�Ϥη�";
		if (desc) return "����������ʬ��ε�����ġ�";
#else
		if (name) return "Fist of Force";
		if (desc) return "Fires a tiny ball of disintegration.";
#endif
    
		{
			int dice = 8 + ((plev - 5) / 4);
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_DISINTEGRATE, dir,
					damroll(dice, sides), 0);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "�ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Teleport Self";
		if (desc) return "Teleport long distance.";
#endif
    
		{
			int range = plev * 5;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "������";
		if (desc) return "��󥹥����˥�����ʸ��̤�Ϳ���롣";
#else
		if (name) return "Wonder";
		if (desc) return "Fires something with random effects.";
#endif
    
		{
			if (info) return s_random;

			if (cast)
			{

				if (!get_aim_dir(&dir)) return NULL;

				cast_wonder(dir);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "���������ܥ��";
		if (desc) return "�������Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Chaos Bolt";
		if (desc) return "Fires a bolt or ball of chaos.";
#endif
    
		{
			int dice = 10 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_CHAOS, dir, damroll(dice, sides));
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "���˥å����֡���";
		if (desc) return "��ʬ���濴�Ȥ����첻�ε��ȯ�������롣";
#else
		if (name) return "Sonic Boom";
		if (desc) return "Generates a ball of sound centered on you.";
#endif
    
		{
			int dam = 60 + plev;
			int rad = plev / 10 + 2;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
#ifdef JP
				msg_print("�ɡ����������ɤ줿��");
#else
				msg_print("BOOM! Shake the room!");
#endif

				project(0, rad, py, px, dam, GF_SOUND, PROJECT_KILL | PROJECT_ITEM, -1);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "���Ǥ���";
		if (desc) return "�������ϤΥӡ�������ġ�";
#else
		if (name) return "Doom Bolt";
		if (desc) return "Fires a beam of pure mana.";
#endif
    
		{
			int dice = 11 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_MANA, dir, damroll(dice, sides));
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�ե��������ܡ���";
		if (desc) return "��ε�����ġ�";
#else
		if (name) return "Fire Ball";
		if (desc) return "Fires a ball of fire.";
#endif
    
		{
			int dam = plev + 55;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_FIRE, dir, dam, rad);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���������";
		if (desc) return "��󥹥�����ƥ�ݡ��Ȥ�����ӡ�������ġ��񹳤�����̵����";
#else
		if (name) return "Teleport Other";
		if (desc) return "Teleports all monsters on the line away unless resisted.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_AWAY_ALL, dir, power);
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "�˲��θ���";
		if (desc) return "���դΥ����ƥࡢ��󥹥������Ϸ����˲����롣";
#else
		if (name) return "Word of Destruction";
		if (desc) return "Destroy everything in nearby area.";
#endif
    
		{
			int base = 12;
			int sides = 4;

			if (cast)
			{
				destroy_area(py, px, base + randint1(sides), FALSE);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "���륹ȯư";
		if (desc) return "����ʥ������ε�����ġ�";
#else
		if (name) return "Invoke Logrus";
		if (desc) return "Fires a huge ball of chaos.";
#endif
    
		{
			int dam = plev * 2 + 99;
			int rad = plev / 5;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_CHAOS, dir, dam, rad);
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "¾������";
		if (desc) return "��󥹥���1�Τ��ѿȤ����롣�񹳤�����̵����";
#else
		if (name) return "Polymorph Other";
		if (desc) return "Attempts to polymorph a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				poly_monster(dir);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "Ϣ�����";
		if (desc) return "���������Ф����ŷ�Υӡ�������ġ�";
#else
		if (name) return "Chain Lightning";
		if (desc) return "Fires lightning beams in all directions.";
#endif
    
		{
			int dice = 5 + plev / 10;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				for (dir = 0; dir <= 9; dir++)
					fire_beam(GF_ELEC, dir, damroll(dice, sides));
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "��������";
		if (desc) return "��/��ˡ���ν�Ŷ��������䤹������Ŷ��Υ�åɤν�Ŷ���֤򸺤餹��";
#else
		if (name) return "Arcane Binding";
		if (desc) return "Recharges staffs, wands or rods.";
#endif
    
		{
			int power = 90;

			if (info) return info_power(power);

			if (cast)
			{
				if (!recharge(power)) return NULL;
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "����ʬ��";
		if (desc) return "�����ʬ��ε�����ġ�";
#else
		if (name) return "Disintegrate";
		if (desc) return "Fires a huge ball of disintegration.";
#endif
    
		{
			int dam = plev + 70;
			int rad = 3 + plev / 40;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_DISINTEGRATE, dir, dam, rad);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "��������";
		if (desc) return "���ߤγ���ƹ������롣";
#else
		if (name) return "Alter Reality";
		if (desc) return "Recreates current dungeon level.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				alter_reality();
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�ޥ��å������å�";
		if (desc) return "���åȤ�ȯ�ͤ��롣";
#else
		if (name) return "Magic Rocket";
		if (desc) return "Fires a magic rocket.";
#endif
    
		{
			int dam = 120 + plev * 2;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

#ifdef JP
				msg_print("���å�ȯ�͡�");
#else
				msg_print("You launch a rocket!");
#endif

				fire_rocket(GF_ROCKET, dir, dam, rad);
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "���٤ο�";
		if (desc) return "���˥�������°����Ĥ��롣";
#else
		if (name) return "Chaos Branding";
		if (desc) return "Makes current weapon a Chaotic weapon.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(2);
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "���⾤��";
		if (desc) return "�����1�ξ������롣";
#else
		if (name) return "Summon Demon";
		if (desc) return "Summons a demon.";
#endif
    
		{
			if (cast)
			{
				u32b mode = 0L;
				bool pet = !one_in_(3);

				if (pet) mode |= PM_FORCE_PET;
				else mode |= PM_NO_PET;
				if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

				if (summon_specific((pet ? -1 : 0), py, px, (plev * 3) / 2, SUMMON_DEMON, mode))
				{
#ifdef JP
					msg_print("β���ΰ���������������");
#else
					msg_print("The area fills with a stench of sulphur and brimstone.");
#endif

					if (pet)
					{
#ifdef JP
						msg_print("�֤��ѤǤ������ޤ�����������͡�");
#else
						msg_print("'What is thy bidding... Master?'");
#endif
					}
					else
					{
#ifdef JP
						msg_print("���ܤ����Ԥ衢�����β��ͤˤ��餺�� �����κ���ĺ��������");
#else
						msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
#endif
					}
				}
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "���ϸ���";
		if (desc) return "���ϤΥӡ�������ġ�";
#else
		if (name) return "Beam of Gravity";
		if (desc) return "Fires a beam of gravity.";
#endif
    
		{
			int dice = 9 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_GRAVITY, dir, damroll(dice, sides));
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "ή����";
		if (desc) return "��ʬ�μ��դ���Ф���Ȥ���";
#else
		if (name) return "Meteor Swarm";
		if (desc) return "Makes meteor balls fall down to nearby random locations.";
#endif
    
		{
			int dam = plev * 2;
			int rad = 2;

			if (info) return info_multi_damage(dam);

			if (cast)
			{
				cast_meteor(dam, rad);
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "��ΰ��";
		if (desc) return "��ʬ���濴�Ȥ���Ķ����ʱ�ε��ȯ�������롣";
#else
		if (name) return "Flame Strike";
		if (desc) return "Generate a huge ball of fire centered on you.";
#endif
    
		{
			int dam = 300 + 3 * plev;
			int rad = 8;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
				fire_ball(GF_FIRE, 0, dam, rad);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "���پ���";
		if (desc) return "�������°���ε��ӡ����ȯ�������롣";
#else
		if (name) return "Call Chaos";
		if (desc) return "Generate random kind of balls or beams.";
#endif
    
		{
			if (info) return format("%s150 / 250", s_dam);

			if (cast)
			{
				call_chaos();
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "��������";
		if (desc) return "��ʬ���ѿȤ����褦�Ȥ��롣";
#else
		if (name) return "Polymorph Self";
		if (desc) return "Polymorphs yourself.";
#endif
    
		{
			if (cast)
			{
#ifdef JP
				if (!get_check("�ѿȤ��ޤ���������Ǥ�����")) return NULL;
#else
				if (!get_check("You will polymorph yourself. Are you sure? ")) return NULL;
#endif
				do_poly_self();
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "���Ϥ���";
		if (desc) return "���˶��Ϥǵ���ʽ������Ϥε�����ġ�";
#else
		if (name) return "Mana Storm";
		if (desc) return "Fires an extremely powerful huge ball of pure mana.";
#endif
    
		{
			int dam = 300 + plev * 4;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_MANA, dir, dam, rad);
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "���륹�Υ֥쥹";
		if (desc) return "���˶��Ϥʥ������ε�����ġ�";
#else
		if (name) return "Breathe Logrus";
		if (desc) return "Fires an extremely powerful ball of chaos.";
#endif
    
		{
			int dam = p_ptr->chp;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_CHAOS, dir, dam, rad);
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "��̵����";
		if (desc) return "��ʬ�˼��Ϥ˸����äơ����åȡ��������Ϥε塢�������Ѵ�ʪ�ε�����ġ����������ɤ����ܤ��ƻ��Ѥ���ȹ��ϰϤ��˲����롣";
#else
		if (name) return "Call the Void";
		if (desc) return "Fires rockets, mana balls and nuclear waste balls in all directions each unless you are not adjacent to any walls. Otherwise *destroys* huge area.";
#endif
    
		{
			if (info) return format("%s3 * 175", s_dam);

			if (cast)
			{
				call_the_();
			}
		}
		break;
	}

	return "";
}


static cptr do_death_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

#ifdef JP
	static const char s_dam[] = "»��:";
	static const char s_random[] = "������";
#else
	static const char s_dam[] = "dam ";
	static const char s_random[] = "random";
#endif

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "̵��̿����";
		if (desc) return "�᤯����̿�Τʤ���󥹥������Τ��롣";
#else
		if (name) return "Detect Unlife";
		if (desc) return "Detects all nonliving monsters in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_nonliving(rad);
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "������";
		if (desc) return "���������ʼٰ����Ϥ���ĥܡ�������ġ����ɤʥ�󥹥����ˤ��礭�ʥ��᡼����Ϳ���롣";
#else
		if (name) return "Malediction";
		if (desc) return "Fires a tiny ball of evil power which hurts good monsters greatly.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;
			int rad = 0;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				/*
				 * A radius-0 ball may (1) be aimed at
				 * objects etc., and will affect them;
				 * (2) may be aimed at ANY visible
				 * monster, unlike a 'bolt' which must
				 * travel to the monster.
				 */

				fire_ball(GF_HELL_FIRE, dir, damroll(dice, sides), rad);

				if (one_in_(5))
				{
					/* Special effect first */
					int effect = randint1(1000);

					if (effect == 666)
						fire_ball_hide(GF_DEATH_RAY, dir, plev * 200, 0);
					else if (effect < 500)
						fire_ball_hide(GF_TURN_ALL, dir, plev, 0);
					else if (effect < 800)
						fire_ball_hide(GF_OLD_CONF, dir, plev, 0);
					else
						fire_ball_hide(GF_STUN, dir, plev, 0);
				}
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "�ٰ�����";
		if (desc) return "�᤯�μٰ��ʥ�󥹥������Τ��롣";
#else
		if (name) return "Detect Evil";
		if (desc) return "Detects all evil monsters in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_evil(rad);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "������";
		if (desc) return "�Ǥε�����ġ�";
#else
		if (name) return "Stinking Cloud";
		if (desc) return "Fires a ball of poison.";
#endif
    
		{
			int dam = 10 + plev / 2;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_POIS, dir, dam, rad);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "����̲��";
		if (desc) return "1�ΤΥ�󥹥�����̲�餻�롣�񹳤�����̵����";
#else
		if (name) return "Black Sleep";
		if (desc) return "Attempts to sleep a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				sleep_monster(dir);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "����";
		if (desc) return "������֡��Ǥؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Poison";
		if (desc) return "Gives resistance to poison. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "����";
		if (desc) return "��󥹥���1�Τ��ݤ�����ۯ۰�����롣�񹳤�����̵����";
#else
		if (name) return "Horrify";
		if (desc) return "Attempts to scare and stun a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fear_monster(dir, power);
				stun_monster(dir, power);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "����ǥåɽ�°";
		if (desc) return "����ǥå�1�Τ�̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Enslave Undead";
		if (desc) return "Attempts to charm an undead monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				control_one_undead(dir, power);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "����ȥ�ԡ��ε�";
		if (desc) return "��̿�Τ���Ԥ˸��̤Τ��������ġ�";
#else
		if (name) return "Orb of Entropy";
		if (desc) return "Fires a ball which damages living monsters.";
#endif
    
		{
			int dice = 3;
			int sides = 6;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_MAGE ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_OLD_DRAIN, dir, damroll(dice, dice) + base, rad);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "�Ϲ�����";
		if (desc) return "�Ϲ��Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Nether Bolt";
		if (desc) return "Fires a bolt or beam of nether.";
#endif
    
		{
			int dice = 8 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_NETHER, dir, damroll(dice, sides));
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "��٤��";
		if (desc) return "��ʬ���濴�Ȥ����Ǥε��ȯ�������롣";
#else
		if (name) return "Cloud kill";
		if (desc) return "Generate a ball of poison centered on you.";
#endif
    
		{
			int dam = (30 + plev) * 2;
			int rad = plev / 10 + 2;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
				project(0, rad, py, px, dam, GF_POIS, PROJECT_KILL | PROJECT_ITEM, -1);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "��󥹥�������";
		if (desc) return "��󥹥���1�Τ�ä���롣�и��ͤ䥢���ƥ�ϼ������ʤ����񹳤�����̵����";
#else
		if (name) return "Genocide One";
		if (desc) return "Attempts to vanish a monster.";
#endif
    
		{
			int power = plev + 50;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball_hide(GF_GENOCIDE, dir, power, 0);
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�Ǥο�";
		if (desc) return "�����Ǥ�°����Ĥ��롣";
#else
		if (name) return "Poison Branding";
		if (desc) return "Makes current weapon poison branded.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(3);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "�۷�ɥ쥤��";
		if (desc) return "��󥹥���1�Τ�����̿�Ϥ�ۤ��Ȥ롣�ۤ��Ȥä���̿�Ϥˤ�ä���ʢ�٤��夬�롣";
#else
		if (name) return "Vampiric Drain";
		if (desc) return "Absorbs some HP from a monster and gives them to you. You will also gain nutritional sustenance from this.";
#endif
    
		{
			int dice = 1;
			int sides = plev * 2;
			int base = plev * 2;

			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				int dam = base + damroll(dice, sides);

				if (!get_aim_dir(&dir)) return NULL;

				if (drain_life(dir, dam))
				{
					chg_virtue(V_SACRIFICE, -1);
					chg_virtue(V_VITALITY, -1);

					hp_player(dam);

					/*
					 * Gain nutritional sustenance:
					 * 150/hp drained
					 *
					 * A Food ration gives 5000
					 * food points (by contrast)
					 * Don't ever get more than
					 * "Full" this way But if we
					 * ARE Gorged, it won't cure
					 * us
					 */
					dam = p_ptr->food + MIN(5000, 100 * dam);

					/* Not gorged already */
					if (p_ptr->food < PY_FOOD_MAX)
						set_food(dam >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dam);
				}
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "ȿ���ν�";
		if (desc) return "���Ϥλ��Τ���������֤���";
#else
		if (name) return "Animate dead";
		if (desc) return "Resurrects nearby corpse and skeletons. And makes these your pets.";
#endif
    
		{
			if (cast)
			{
				animate_dead(0, py, px);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "����";
		if (desc) return "���ꤷ��ʸ���Υ�󥹥����򸽺ߤγ�����ä���롣�񹳤�����̵����";
#else
		if (name) return "Genocide";
		if (desc) return "Eliminates an entire class of monster, exhausting you.  Powerful or unique monsters may resist.";
#endif
    
		{
			int power = plev+50;

			if (info) return info_power(power);

			if (cast)
			{
				symbol_genocide(power, TRUE);
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "����β�";
		if (desc) return "����β��������ݤ����롣";
#else
		if (name) return "Berserk";
		if (desc) return "Gives bonus to hit and HP, immunity to fear for a while. But decreases AC.";
#endif
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_shero(randint1(base) + base, FALSE);
				hp_player(30);
				set_afraid(0);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "�����";
		if (desc) return "��������͡��ʸ��̤������롣";
#else
		if (name) return "Invoke Spirits";
		if (desc) return "Causes random effects.";
#endif
    
		{
			if (info) return s_random;

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				cast_invoke_spirits(dir);
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "�Ź�����";
		if (desc) return "�Ź��Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Dark Bolt";
		if (desc) return "Fires a bolt or beam of darkness.";
#endif
    
		{
			int dice = 4 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_DARK, dir, damroll(dice, sides));
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "�������";
		if (desc) return "����β��������ݤ�������®���롣";
#else
		if (name) return "Battle Frenzy";
		if (desc) return "Gives another bonus to hit and HP, immunity to fear for a while. Hastes you. But decreases AC.";
#endif
    
		{
			int b_base = 25;
			int sp_base = plev / 2;
			int sp_sides = 20 + plev / 2;

			if (info) return info_duration(b_base, b_base);

			if (cast)
			{
				set_shero(randint1(25) + 25, FALSE);
				hp_player(30);
				set_afraid(0);
				set_fast(randint1(sp_sides) + sp_base, FALSE);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "�۷�ο�";
		if (desc) return "���˵۷��°����Ĥ��롣";
#else
		if (name) return "Vampiric Branding";
		if (desc) return "Makes current weapon Vampiric.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(4);
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�����۷�";
		if (desc) return "��󥹥���1�Τ�����̿�Ϥ�ۤ��Ȥ롣�ۤ��Ȥä���̿�Ϥˤ�ä����Ϥ��������롣";
#else
		if (name) return "Vampirism True";
		if (desc) return "Fires 3 bolts. Each of the bolts absorbs some HP from a monster and gives them to you.";
#endif
    
		{
			int dam = 100;

			if (info) return format("%s3*%d", s_dam, dam);

			if (cast)
			{
				int i;

				if (!get_aim_dir(&dir)) return NULL;

				chg_virtue(V_SACRIFICE, -1);
				chg_virtue(V_VITALITY, -1);

				for (i = 0; i < 3; i++)
				{
					if (drain_life(dir, dam))
						hp_player(dam);
				}
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "��θ���";
		if (desc) return "�볦�����̿�Τ����󥹥����˥��᡼����Ϳ���롣";
#else
		if (name) return "Nether Wave";
		if (desc) return "Damages all living monsters in sight.";
#endif
    
		{
			int sides = plev * 3;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_living(randint1(sides));
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "�Ź�����";
		if (desc) return "����ʰŹ��ε�����ġ�";
#else
		if (name) return "Darkness Storm";
		if (desc) return "Fires a huge ball of darkness.";
#endif
    
		{
			int dam = 100 + plev * 2;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_DARK, dir, dam, rad);
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "��θ���";
		if (desc) return "��θ��������ġ�";
#else
		if (name) return "Death Ray";
		if (desc) return "Fires a beam of death.";
#endif
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				death_ray(dir, plev);
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "��Ծ���";
		if (desc) return "1�ΤΥ���ǥåɤ򾤴����롣";
#else
		if (name) return "Raise the Dead";
		if (desc) return "Summons an undead monster.";
#endif
    
		{
			if (cast)
			{
				int type;
				bool pet = one_in_(3);
				u32b mode = 0L;

				type = (plev > 47 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD);

				if (!pet || (pet && (plev > 24) && one_in_(3)))
					mode |= PM_ALLOW_GROUP;

				if (pet) mode |= PM_FORCE_PET;
				else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

				if (summon_specific((pet ? -1 : 0), py, px, (plev * 3) / 2, type, mode))
				{
#ifdef JP
					msg_print("�䤿���������ʤ��μ���˿᤭�Ϥ᤿����������Խ��򱿤�Ǥ���...");
#else
					msg_print("Cold winds begin to blow around you, carrying with them the stench of decay...");
#endif


					if (pet)
					{
#ifdef JP
						msg_print("�Ť��λऻ��Զ������ʤ��˻Ť��뤿���ڤ���ᴤä���");
#else
						msg_print("Ancient, long-dead forms arise from the ground to serve you!");
#endif
					}
					else
					{
#ifdef JP
						msg_print("��Ԥ�ᴤä���̲���˸���뤢�ʤ���ȳ���뤿��ˡ�");
#else
						msg_print("'The dead arise... to punish you for disturbing them!'");
#endif
					}

					chg_virtue(V_UNLIFE, 1);
				}
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "��Ԥ�����";
		if (desc) return "�����ƥ��1�ļ��̤��롣��٥뤬�⤤�ȥ����ƥ��ǽ�Ϥ������Τ뤳�Ȥ��Ǥ��롣";
#else
		if (name) return "Esoteria";
		if (desc) return "Identifies an item. Or *identifies* an item at higher level.";
#endif
    
		{
			if (cast)
			{
				if (randint1(50) > plev)
				{
					if (!ident_spell(FALSE)) return NULL;
				}
				else
				{
					if (!identify_fully(FALSE)) return NULL;
				}
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "�۷쵴�Ѳ�";
		if (desc) return "������֡��۷쵴���Ѳ����롣�Ѳ����Ƥ���֤�����μ�²��ǽ�Ϥ򼺤�������˵۷쵴�Ȥ��Ƥ�ǽ�Ϥ����롣";
#else
		if (name) return "Polymorph Vampire";
		if (desc) return "Mimic a vampire for a while. Loses abilities of original race and gets abilities as a vampire.";
#endif
    
		{
			int base = 10 + plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_mimic(base + randint1(base), MIMIC_VAMPIRE, FALSE);
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "��̿������";
		if (desc) return "���ä��и��ͤ�������롣";
#else
		if (name) return "Restore Life";
		if (desc) return "Restore lost experience.";
#endif
    
		{
			if (cast)
			{
				restore_level();
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "��������";
		if (desc) return "��ʬ�μ��Ϥˤ����󥹥����򸽺ߤγ�����ä���롣�񹳤�����̵����";
#else
		if (name) return "Mass Genocide";
		if (desc) return "Eliminates all nearby monsters, exhausting you.  Powerful or unique monsters may be able to resist.";
#endif
    
		{
			int power = plev + 50;

			if (info) return info_power(power);

			if (cast)
			{
				mass_genocide(power, TRUE);
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "�Ϲ��ι��";
		if (desc) return "�ٰ����Ϥ������������ġ����ɤʥ�󥹥����ˤ��礭�ʥ��᡼����Ϳ���롣";
#else
		if (name) return "Hellfire";
		if (desc) return "Fires a powerful ball of evil power. Hurts good monsters greatly.";
#endif
    
		{
			int dam = 666;
			int rad = 3;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HELL_FIRE, dir, dam, rad);
#ifdef JP
				take_hit(DAMAGE_USELIFE, 20 + randint1(30), "�Ϲ��ι�Фμ�ʸ�򾧤�����ϫ", -1);
#else
				take_hit(DAMAGE_USELIFE, 20 + randint1(30), "the strain of casting Hellfire", -1);
#endif
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "ͩ�β�";
		if (desc) return "������֡��ɤ��̤�ȴ���뤳�Ȥ��Ǥ���������᡼�����ڸ������ͩ�Τξ��֤��ѿȤ��롣";
#else
		if (name) return "Wraithform";
		if (desc) return "Becomes wraith form which gives ability to pass walls and makes all damages half.";
#endif
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_wraith_form(randint1(base) + base, FALSE);
			}
		}
		break;
	}

	return "";
}


static cptr do_trump_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;

#ifdef JP
	static const char s_random[] = "������";
#else
	static const char s_random[] = "random";
#endif

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "���硼�ȡ��ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Phase Door";
		if (desc) return "Teleport short distance.";
#endif
    
		{
			int range = 10;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "����Υ�����";
		if (desc) return "����򾤴����롣";
#else
		if (name) return "Trump Spiders";
		if (desc) return "Summons spiders.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ�������Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of an spider...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_SPIDER, PM_ALLOW_GROUP))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿������ܤäƤ��롪");
#else
						msg_print("The summoned spiders get angry!");
#endif
					}
				}
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "����åե�";
		if (desc) return "�����ɤ��ꤤ�򤹤롣";
#else
		if (name) return "Shuffle";
		if (desc) return "Causes random effects.";
#endif
    
		{
			if (info) return s_random;

			if (cast)
			{
				cast_shuffle();
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "�ե����ꥻ�å�";
		if (desc) return "�ǿ������ѹ����롣";
#else
		if (name) return "Reset Recall";
		if (desc) return "Resets the 'deepest' level for recall spell.";
#endif
    
		{
			if (cast)
			{
				if (!reset_recall()) return NULL;
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "�ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Teleport";
		if (desc) return "Teleport long distance.";
#endif
    
		{
			int range = plev * 4;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "���ΤΥ�����";
		if (desc) return "������֡��ƥ�ѥ���ǽ�Ϥ����롣";
#else
		if (name) return "Trump Spying";
		if (desc) return "Gives telepathy for a while.";
#endif
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���󥹥���";
		if (desc) return "��󥹥�����ƥ�ݡ��Ȥ�����ӡ�������ġ��񹳤�����̵����";
#else
		if (name) return "Teleport Away";
		if (desc) return "Teleports all monsters on the line away unless resisted.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_AWAY_ALL, dir, power);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "ưʪ�Υ�����";
		if (desc) return "1�Τ�ưʪ�򾤴����롣";
#else
		if (name) return "Trump Animals";
		if (desc) return "Summons an animal.";
#endif
    
		{
			if (cast || fail)
			{
				int type = (!fail ? SUMMON_ANIMAL_RANGER : SUMMON_ANIMAL);

#ifdef JP
				msg_print("���ʤ���ưʪ�Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of an animal...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, type, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿ưʪ���ܤäƤ��롪");
#else
						msg_print("The summoned animal gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "��ư�Υ�����";
		if (desc) return "�����ƥ��ʬ��­���ذ�ư�����롣";
#else
		if (name) return "Trump Reach";
		if (desc) return "Pulls a distant item close to you.";
#endif
    
		{
			int weight = plev * 15;

			if (info) return info_weight(weight);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fetch(dir, weight, FALSE);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "���ߥ����Υ�����";
		if (desc) return "ʣ������ȯ�����󥹥����򾤴����롣";
#else
		if (name) return "Trump Kamikaze";
		if (desc) return "Summons monsters which explode by itself.";
#endif
    
		{
			if (cast || fail)
			{
				int x, y;
				int type;

				if (cast)
				{
					if (!target_set(TARGET_KILL)) return NULL;
					x = target_col;
					y = target_row;
				}
				else
				{
					/* Summons near player when failed */
					x = px;
					y = py;
				}

				if (p_ptr->pclass == CLASS_BEASTMASTER)
					type = SUMMON_KAMIKAZE_LIVING;
				else
					type = SUMMON_KAMIKAZE;

#ifdef JP
				msg_print("���ʤ��ϥ��ߥ����Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on several trumps at once...");
#endif

				if (trump_summoning(2 + randint0(plev / 7), !fail, y, x, 0, type, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿��󥹥������ܤäƤ��롪");
#else
						msg_print("The summoned creatures get angry!");
#endif
					}
				}
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "�����";
		if (desc) return "1�Τ�ͩ��򾤴����롣";
#else
		if (name) return "Phantasmal Servant";
		if (desc) return "Summons a ghost.";
#endif
    
		{
			/* Phantasmal Servant is not summoned as enemy when failed */
			if (cast)
			{
				int summon_lev = plev * 2 / 3 + randint1(plev / 2);

				if (trump_summoning(1, !fail, py, px, (summon_lev * 3 / 2), SUMMON_PHANTOM, 0L))
				{
#ifdef JP
					msg_print("���ѤǤ������ޤ����������͡�");
#else
					msg_print("'Your wish, master?'");
#endif
				}
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "���ԡ��ɡ���󥹥���";
		if (desc) return "��󥹥���1�Τ��®�����롣";
#else
		if (name) return "Haste Monster";
		if (desc) return "Hastes a monster.";
#endif
    
		{
			if (cast)
			{
				bool result;

				/* Temporary enable target_pet option */
				bool old_target_pet = target_pet;
				target_pet = TRUE;

				result = get_aim_dir(&dir);

				/* Restore target_pet option */
				target_pet = old_target_pet;

				if (!result) return NULL;

				speed_monster(dir);
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���٥�";
		if (desc) return "�ֻ��˾夫���γ��˥ƥ�ݡ��Ȥ��롣";
#else
		if (name) return "Teleport Level";
		if (desc) return "Teleport to up or down stairs in a moment.";
#endif
    
		{
			if (cast)
			{
#ifdef JP
				if (!get_check("������¾�γ��˥ƥ�ݡ��Ȥ��ޤ�����")) return NULL;
#else
				if (!get_check("Are you sure? (Teleport Level)")) return NULL;
#endif
				teleport_level(0);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "��������";
		if (desc) return "û��Υ��λ��ꤷ�����˥ƥ�ݡ��Ȥ��롣";
#else
		if (name) return "Dimension Door";
		if (desc) return "Teleport to given location.";
#endif
    
		{
			int range = plev / 2 + 10;

			if (info) return info_range(range);

			if (cast)
			{
#ifdef JP
				msg_print("�������⤬����������Ū�Ϥ�����ǲ�������");
#else
				msg_print("You open a dimensional gate. Choose a destination.");
#endif

				if (!dimension_door()) return NULL;
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "���Ԥμ�ʸ";
		if (desc) return "�Ͼ�ˤ���Ȥ��ϥ��󥸥��κǿ����ء����󥸥��ˤ���Ȥ����Ͼ�ؤȰ�ư���롣";
#else
		if (name) return "Word of Recall";
		if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				if (!word_of_recall()) return NULL;
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "��ʪ����";
		if (desc) return "�볦������ƤΥ�󥹥�����ƥ�ݡ��Ȥ����롣�񹳤�����̵����";
#else
		if (name) return "Banish";
		if (desc) return "Teleports all monsters in sight away unless resisted.";
#endif
    
		{
			int power = plev * 4;

			if (info) return info_power(power);

			if (cast)
			{
				banish_monsters(power);
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "���ָ򴹤Υ�����";
		if (desc) return "1�ΤΥ�󥹥����Ȱ��֤�򴹤��롣";
#else
		if (name) return "Swap Position";
		if (desc) return "Swap positions of you and a monster.";
#endif
    
		{
			if (cast)
			{
				bool result;

				/* HACK -- No range limit */
				project_length = -1;

				result = get_aim_dir(&dir);

				/* Restore range to default */
				project_length = 0;

				if (!result) return NULL;

				teleport_swap(dir);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "����ǥåɤΥ�����";
		if (desc) return "1�ΤΥ���ǥåɤ򾤴����롣";
#else
		if (name) return "Trump Undead";
		if (desc) return "Summons an undead monster.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��ϥ���ǥåɤΥ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of an undead creature...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_UNDEAD, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿����ǥåɤ��ܤäƤ��롪");
#else
						msg_print("The summoned undead creature gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "�����Υ�����";
		if (desc) return "1�ΤΥҥɥ�򾤴����롣";
#else
		if (name) return "Trump Reptiles";
		if (desc) return "Summons a hydra.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��������Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of a reptile...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_HYDRA, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿�������ܤäƤ��롪");
#else
						msg_print("The summoned reptile gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "��󥹥����Υ�����";
		if (desc) return "ʣ���Υ�󥹥����򾤴����롣";
#else
		if (name) return "Trump Monsters";
		if (desc) return "Summons some monsters.";
#endif
    
		{
			if (cast || fail)
			{
				int type;

#ifdef JP
				msg_print("���ʤ��ϥ�󥹥����Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on several trumps at once...");
#endif

				if (p_ptr->pclass == CLASS_BEASTMASTER)
					type = SUMMON_LIVING;
				else
					type = 0;

				if (trump_summoning((1 + (plev - 15)/ 10), !fail, py, px, 0, type, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿��󥹥������ܤäƤ��롪");
#else
						msg_print("The summoned creatures get angry!");
#endif
					}
				}

			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "�ϥ���ɤΥ�����";
		if (desc) return "1���롼�פΥϥ���ɤ򾤴����롣";
#else
		if (name) return "Trump Hounds";
		if (desc) return "Summons a group of hounds.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��ϥϥ���ɤΥ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of a hound...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_HOUND, PM_ALLOW_GROUP))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿�ϥ���ɤ��ܤäƤ��롪");
#else
						msg_print("The summoned hounds get angry!");
#endif
					}
				}
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�ȥ��פο�";
		if (desc) return "���˥ȥ��פ�°����Ĥ��롣";
#else
		if (name) return "Trump Branding";
		if (desc) return "Makes current weapon a Trump weapon.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(5);
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "�ʹ֥ȥ���";
		if (desc) return "������˥ƥ�ݡ��Ȥ��������Ѱۤ�����ʬ�ΰջפǥƥ�ݡ��Ȥ��������Ѱۤ��ȤˤĤ���";
#else
		if (name) return "Living Trump";
		if (desc) return "Gives mutation which makes you teleport randomly or makes you able to teleport at will.";
#endif
    
		{
			if (cast)
			{
				int mutation;

				if (one_in_(7))
					/* Teleport control */
					mutation = 12;
				else
					/* Random teleportation (uncontrolled) */
					mutation = 77;

				/* Gain the mutation */
				if (gain_random_mutation(mutation))
				{
#ifdef JP
					msg_print("���ʤ��������Ƥ��륫���ɤ��Ѥ�ä���");
#else
					msg_print("You have turned into a Living Trump.");
#endif
				}
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "�����С��ǡ����Υ�����";
		if (desc) return "1�ΤΥ����С��ǡ����򾤴����롣";
#else
		if (name) return "Trump Cyberdemon";
		if (desc) return "Summons a cyber demon.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��ϥ����С��ǡ����Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of a Cyberdemon...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_CYBER, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿�����С��ǡ������ܤäƤ��롪");
#else
						msg_print("The summoned Cyberdemon gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "ͽ���Υ�����";
		if (desc) return "�᤯�����ƤΥ�󥹥�����櫡��⡢���ʡ������������ƥ����ƥ���Τ��롣";
#else
		if (name) return "Trump Divination";
		if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_all(rad);
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "�μ��Υ�����";
		if (desc) return "�����ƥ�λ���ǽ�Ϥ������Τ롣";
#else
		if (name) return "Trump Lore";
		if (desc) return "*Identifies* an item.";
#endif
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "������󥹥���";
		if (desc) return "��󥹥���1�Τ����Ϥ���������롣";
#else
		if (name) return "Heal Monster";
		if (desc) return "Heal a monster.";
#endif
    
		{
			int heal = plev * 10 + 200;

			if (info) return info_heal(0, 0, heal);

			if (cast)
			{
				bool result;

				/* Temporary enable target_pet option */
				bool old_target_pet = target_pet;
				target_pet = TRUE;

				result = get_aim_dir(&dir);

				/* Restore target_pet option */
				target_pet = old_target_pet;

				if (!result) return NULL;

				heal_monster(dir, heal);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "�ɥ饴��Υ�����";
		if (desc) return "1�ΤΥɥ饴��򾤴����롣";
#else
		if (name) return "Trump Dragon";
		if (desc) return "Summons a dragon.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��ϥɥ饴��Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of a dragon...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_DRAGON, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿�ɥ饴����ܤäƤ��롪");
#else
						msg_print("The summoned dragon gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "��ФΥ�����";
		if (desc) return "��ʬ�μ��դ���Ф���Ȥ���";
#else
		if (name) return "Trump Meteor";
		if (desc) return "Makes meteor balls fall down to nearby random locations.";
#endif
    
		{
			int dam = plev * 2;
			int rad = 2;

			if (info) return info_multi_damage(dam);

			if (cast)
			{
				cast_meteor(dam, rad);
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "�ǡ����Υ�����";
		if (desc) return "1�Τΰ���򾤴����롣";
#else
		if (name) return "Trump Demon";
		if (desc) return "Summons a demon.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��ϥǡ����Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of a demon...");
#endif

				if (trump_summoning(1, !fail, py, px, 0, SUMMON_DEMON, 0L))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿�ǡ������ܤäƤ��롪");
#else
						msg_print("The summoned demon gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "�Ϲ��Υ�����";
		if (desc) return "1�Τξ�饢��ǥåɤ򾤴����롣";
#else
		if (name) return "Trump Greater Undead";
		if (desc) return "Summons a greater undead.";
#endif
    
		{
			if (cast || fail)
			{
#ifdef JP
				msg_print("���ʤ��϶��Ϥʥ���ǥåɤΥ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of a greater undead being...");
#endif
				/* May allow unique depend on level and dice roll */
				if (trump_summoning(1, !fail, py, px, 0, SUMMON_HI_UNDEAD, PM_ALLOW_UNIQUE))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿��饢��ǥåɤ��ܤäƤ��롪");
#else
						msg_print("The summoned greater undead creature gets angry!");
#endif
					}
				}
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "����ɥ饴��Υ�����";
		if (desc) return "1�Τθ���ɥ饴��򾤴����롣";
#else
		if (name) return "Trump Ancient Dragon";
		if (desc) return "Summons an ancient dragon.";
#endif
    
		{
			if (cast)
			{
				int type;

				if (p_ptr->pclass == CLASS_BEASTMASTER)
					type = SUMMON_HI_DRAGON_LIVING;
				else
					type = SUMMON_HI_DRAGON;

#ifdef JP
				msg_print("���ʤ��ϸ���ɥ饴��Υ����ɤ˽��椹��...");
#else
				msg_print("You concentrate on the trump of an ancient dragon...");
#endif

				/* May allow unique depend on level and dice roll */
				if (trump_summoning(1, !fail, py, px, 0, type, PM_ALLOW_UNIQUE))
				{
					if (fail)
					{
#ifdef JP
						msg_print("�������줿����ɥ饴����ܤäƤ��롪");
#else
						msg_print("The summoned ancient dragon gets angry!");
#endif
					}
				}
			}
		}
		break;
	}

	return "";
}


static cptr do_arcane_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "�ŷ�";
		if (desc) return "�ŷ�Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Zap";
		if (desc) return "Fires a bolt or beam of lightning.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 3;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir, damroll(dice, sides));
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "��ˡ�λܾ�";
		if (desc) return "��˸��򤫤��롣";
#else
		if (name) return "Wizard Lock";
		if (desc) return "Locks a door.";
#endif
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				wizard_lock(dir);
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "Ʃ���δ���";
		if (desc) return "�᤯��Ʃ���ʥ�󥹥������Τ��롣";
#else
		if (name) return "Detect Invisibility";
		if (desc) return "Detects all invisible monsters in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_invis(rad);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "��󥹥�������";
		if (desc) return "�᤯�����Ƥθ������󥹥������Τ��롣";
#else
		if (name) return "Detect Monsters";
		if (desc) return "Detects all monsters in your vicinity unless invisible.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_normal(rad);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "���硼�ȡ��ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Blink";
		if (desc) return "Teleport short distance.";
#endif
    
		{
			int range = 10;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�饤�ȡ����ꥢ";
		if (desc) return "�������Ȥ餷�Ƥ����ϰϤ��������Τ�ʵפ����뤯���롣";
#else
		if (name) return "Light Area";
		if (desc) return "Lights up nearby area and the inside of a room permanently.";
#endif
    
		{
			int dice = 2;
			int sides = plev / 2;
			int rad = plev / 10 + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				lite_area(damroll(dice, sides), rad);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "櫤��� �˲�";
		if (desc) return "��ľ��������Ƥ�櫤�����˲����롣";
#else
		if (name) return "Trap & Door Destruction";
		if (desc) return "Fires a beam which destroy traps and doors.";
#endif
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				destroy_door(dir);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "�ڽ��μ���";
		if (desc) return "��������Ϥ򾯤����������롣";
#else
		if (name) return "Cure Light Wounds";
		if (desc) return "Heals cut and HP a little.";
#endif
    
		{
			int dice = 2;
			int sides = 8;

			if (info) return info_heal(dice, sides, 0);

			if (cast)
			{
				hp_player(damroll(dice, sides));
				set_cut(p_ptr->cut - 10);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "櫤��� ����";
		if (desc) return "�᤯�����Ƥ�櫤���ȳ��ʤ��Τ��롣";
#else
		if (name) return "Detect Doors & Traps";
		if (desc) return "Detects traps, doors, and stairs in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_traps(rad, TRUE);
				detect_doors(rad);
				detect_stairs(rad);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "ǳ��";
		if (desc) return "������ǳ������뤹�롣";
#else
		if (name) return "Phlogiston";
		if (desc) return "Adds more turns of light to a lantern or torch.";
#endif
    
		{
			if (cast)
			{
				phlogiston();
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "��������";
		if (desc) return "�᤯�κ������Τ��롣";
#else
		if (name) return "Detect Treasure";
		if (desc) return "Detects all treasures in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_treasure(rad);
				detect_objects_gold(rad);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "��ˡ ����";
		if (desc) return "�᤯����ˡ�������ä������ƥ���Τ��롣";
#else
		if (name) return "Detect Enchantment";
		if (desc) return "Detects all magical items in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_objects_magic(rad);
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�����ƥശ��";
		if (desc) return "�᤯�����ƤΥ����ƥ���Τ��롣";
#else
		if (name) return "Detect Objects";
		if (desc) return "Detects all items in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_objects_normal(rad);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "����";
		if (desc) return "�Ǥ����⤫�鴰���˼�������";
#else
		if (name) return "Cure Poison";
		if (desc) return "Cures poison status.";
#endif
    
		{
			if (cast)
			{
				set_poisoned(0);
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "����";
		if (desc) return "������֡��䵤�ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Cold";
		if (desc) return "Gives resistance to cold. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_cold(randint1(base) + base, FALSE);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "�Ѳ�";
		if (desc) return "������֡���ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Fire";
		if (desc) return "Gives resistance to fire. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_fire(randint1(base) + base, FALSE);
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "����";
		if (desc) return "������֡��ŷ�ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Lightning";
		if (desc) return "Gives resistance to electricity. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_elec(randint1(base) + base, FALSE);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "�ѻ�";
		if (desc) return "������֡����ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Acid";
		if (desc) return "Gives resistance to acid. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_acid(randint1(base) + base, FALSE);
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "�Ž��μ���";
		if (desc) return "��������Ϥ������ٲ��������롣";
#else
		if (name) return "Cure Medium Wounds";
		if (desc) return "Heals cut and HP more.";
#endif
    
		{
			int dice = 4;
			int sides = 8;

			if (info) return info_heal(dice, sides, 0);

			if (cast)
			{
				hp_player(damroll(dice, sides));
				set_cut((p_ptr->cut / 2) - 50);
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "�ƥ�ݡ���";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Teleport";
		if (desc) return "Teleport long distance.";
#endif
    
		{
			int range = plev * 5;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "����";
		if (desc) return "�����ƥ���̤��롣";
#else
		if (name) return "Identify";
		if (desc) return "Identifies an item.";
#endif
    
		{
			if (cast)
			{
				if (!ident_spell(FALSE)) return NULL;
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�����ϲ�";
		if (desc) return "�ɤ��Ϥ����ƾ��ˤ��롣";
#else
		if (name) return "Stone to Mud";
		if (desc) return "Turns one rock square to mud.";
#endif
    
		{
			int dice = 1;
			int sides = 30;
			int base = 20;

			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				wall_to_mud(dir);
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "����";
		if (desc) return "���������ġ�����������󥹥����˸��̤����롣";
#else
		if (name) return "Ray of Light";
		if (desc) return "Fires a beam of light which damages to light-sensitive monsters.";
#endif
    
		{
			int dice = 6;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

#ifdef JP
				msg_print("�����������줿��");
#else
				msg_print("A line of light appears.");
#endif

				lite_line(dir);
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "��ʢ��­";
		if (desc) return "��ʢ�ˤ��롣";
#else
		if (name) return "Satisfy Hunger";
		if (desc) return "Satisfies hunger.";
#endif
    
		{
			if (cast)
			{
				set_food(PY_FOOD_MAX - 1);
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "Ʃ����ǧ";
		if (desc) return "������֡�Ʃ���ʤ�Τ�������褦�ˤʤ롣";
#else
		if (name) return "See Invisible";
		if (desc) return "Gives see invisible for a while.";
#endif
    
		{
			int base = 24;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_invis(randint1(base) + base, FALSE);
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "�����󥿥뾤��";
		if (desc) return "1�ΤΥ����󥿥�򾤴����롣";
#else
		if (name) return "Conjure Elemental";
		if (desc) return "Summons an elemental.";
#endif
    
		{
			if (cast)
			{
				if (!summon_specific(-1, py, px, plev, SUMMON_ELEMENTAL, (PM_ALLOW_GROUP | PM_FORCE_PET)))
				{
#ifdef JP
					msg_print("�����󥿥�ϸ���ʤ��ä���");
#else
					msg_print("No Elementals arrive.");
#endif
				}
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���٥�";
		if (desc) return "�ֻ��˾夫���γ��˥ƥ�ݡ��Ȥ��롣";
#else
		if (name) return "Teleport Level";
		if (desc) return "Teleport to up or down stairs in a moment.";
#endif
    
		{
			if (cast)
			{
#ifdef JP
				if (!get_check("������¾�γ��˥ƥ�ݡ��Ȥ��ޤ�����")) return NULL;
#else
				if (!get_check("Are you sure? (Teleport Level)")) return NULL;
#endif
				teleport_level(0);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "�ƥ�ݡ��ȡ���󥹥���";
		if (desc) return "��󥹥�����ƥ�ݡ��Ȥ�����ӡ�������ġ��񹳤�����̵����";
#else
		if (name) return "Teleport Away";
		if (desc) return "Teleports all monsters on the line away unless resisted.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_AWAY_ALL, dir, power);
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "���Ǥε�";
		if (desc) return "�ꡢ�ŷ⡢�䵤�����Τɤ줫�ε�����ġ�";
#else
		if (name) return "Elemental Ball";
		if (desc) return "Fires a ball of some elements.";
#endif
    
		{
			int dam = 75 + plev;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				int type;

				if (!get_aim_dir(&dir)) return NULL;

				switch (randint1(4))
				{
					case 1:  type = GF_FIRE;break;
					case 2:  type = GF_ELEC;break;
					case 3:  type = GF_COLD;break;
					default: type = GF_ACID;break;
				}

				fire_ball(type, dir, dam, rad);
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "������";
		if (desc) return "�᤯�����ƤΥ�󥹥�����櫡��⡢���ʡ������������ƥ����ƥ���Τ��롣";
#else
		if (name) return "Detection";
		if (desc) return "Detects all monsters, traps, doors, stairs, treasures and items in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_all(rad);
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "���Ԥμ�ʸ";
		if (desc) return "�Ͼ�ˤ���Ȥ��ϥ��󥸥��κǿ����ء����󥸥��ˤ���Ȥ����Ͼ�ؤȰ�ư���롣";
#else
		if (name) return "Word of Recall";
		if (desc) return "Recalls player from dungeon to town, or from town to the deepest level of dungeon.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			if (cast)
			{
				if (!word_of_recall()) return NULL;
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "��Τ��";
		if (desc) return "���γ����Τ�ʵפ˾Ȥ餷�����󥸥���⤹�٤ƤΥ����ƥ���Τ��롣����ˡ�������֥ƥ�ѥ���ǽ�Ϥ����롣";
#else
		if (name) return "Clairvoyance";
		if (desc) return "Maps and lights whole dungeon level. Knows all objects location. And gives telepathy for a while.";
#endif
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				chg_virtue(V_KNOWLEDGE, 1);
				chg_virtue(V_ENLIGHTEN, 1);

				wiz_lite(FALSE);

				if (!p_ptr->telepathy)
				{
					set_tim_esp(randint1(sides) + base, FALSE);
				}
			}
		}
		break;
	}

	return "";
}


static cptr do_craft_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "�ֳ�������";
		if (desc) return "������֡��ֳ������Ϥ���������롣";
#else
		if (name) return "Infravision";
		if (desc) return "Gives infravision for a while.";
#endif
    
		{
			int base = 100;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_infra(base + randint1(base), FALSE);
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "�����϶���";
		if (desc) return "������֡������Ϥ���������롣";
#else
		if (name) return "Regeneration";
		if (desc) return "Gives regeneration ability for a while.";
#endif
    
		{
			int base = 80;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_regen(base + randint1(base), FALSE);
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "��ʢ��­";
		if (desc) return "��ʢ�ˤʤ롣";
#else
		if (name) return "Satisfy Hunger";
		if (desc) return "Satisfies hunger.";
#endif
    
		{
			if (cast)
			{
				set_food(PY_FOOD_MAX - 1);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "���䵤";
		if (desc) return "������֡��䵤�ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Cold";
		if (desc) return "Gives resistance to cold. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_cold(randint1(base) + base, FALSE);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "�Ѳб�";
		if (desc) return "������֡���ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Fire";
		if (desc) return "Gives resistance to fire. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_fire(randint1(base) + base, FALSE);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�ε�����";
		if (desc) return "������֡��ҡ�����ʬ�ˤʤ롣";
#else
		if (name) return "Heroism";
		if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";
#endif
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_hero(randint1(base) + base, FALSE);
				hp_player(10);
				set_afraid(0);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "���ŷ�";
		if (desc) return "������֡��ŷ�ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Lightning";
		if (desc) return "Gives resistance to electricity. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_elec(randint1(base) + base, FALSE);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "�ѻ�";
		if (desc) return "������֡����ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Acid";
		if (desc) return "Gives resistance to acid. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_acid(randint1(base) + base, FALSE);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "Ʃ����ǧ";
		if (desc) return "������֡�Ʃ���ʤ�Τ�������褦�ˤʤ롣";
#else
		if (name) return "See Invisibility";
		if (desc) return "Gives see invisible for a while.";
#endif
    
		{
			int base = 24;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_invis(randint1(base) + base, FALSE);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "���";
		if (desc) return "�����ƥ�ˤ����ä��夤�����������롣";
#else
		if (name) return "Remove Curse";
		if (desc) return "Removes normal curses from equipped items.";
#endif
    
		{
			if (cast)
			{
				if (remove_curse())
				{
#ifdef JP
					msg_print("ï���˸�����Ƥ���褦�ʵ������롣");
#else
					msg_print("You feel as if someone is watching over you.");
#endif
				}
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "����";
		if (desc) return "������֡��Ǥؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Poison";
		if (desc) return "Gives resistance to poison. This resistance can be added to which from equipment for more powerful resistance.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "����β�";
		if (desc) return "����β��������ݤ����롣";
#else
		if (name) return "Berserk";
		if (desc) return "Gives bonus to hit and HP, immunity to fear for a while. But decreases AC.";
#endif
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_shero(randint1(base) + base, FALSE);
				hp_player(30);
				set_afraid(0);
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "����ʬ��";
		if (desc) return "���ߤμ�ʬ�ξ��֤������Τ롣";
#else
		if (name) return "Self Knowledge";
		if (desc) return "Gives you useful info regarding your current resistances, the powers of your weapon and maximum limits of your stats.";
#endif
    
		{
			if (cast)
			{
				self_knowledge();
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "�мٰ��볦";
		if (desc) return "�ٰ��ʥ�󥹥����ι�����ɤ��Хꥢ��ĥ�롣";
#else
		if (name) return "Protection from Evil";
		if (desc) return "Gives aura which protect you from evil monster's physical attack.";
#endif
    
		{
			int base = 3 * plev;
			int sides = 25;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_protevil(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "����";
		if (desc) return "�ǡ�ۯ۰���֡�������������������Ф�ľ����";
#else
		if (name) return "Cure";
		if (desc) return "Heals poison, stun, cut and hallucination completely.";
#endif
    
		{
			if (cast)
			{
				set_poisoned(0);
				set_stun(0);
				set_cut(0);
				set_image(0);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "��ˡ��";
		if (desc) return "������֡������䵤���ꡢ�ŷ⡢�����ǤΤ����줫��°����Ĥ��롣��������ʤ��ȻȤ��ʤ���";
#else
		if (name) return "Mana Branding";
		if (desc) return "Makes current weapon some elemental branded. You must wield weapons.";
#endif
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				if (!choose_ele_attack()) return NULL;
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "�ƥ�ѥ���";
		if (desc) return "������֡��ƥ�ѥ���ǽ�Ϥ����롣";
#else
		if (name) return "Telepathy";
		if (desc) return "Gives telepathy for a while.";
#endif
    
		{
			int base = 25;
			int sides = 30;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "ȩ�в�";
		if (desc) return "������֡�AC��徺�����롣";
#else
		if (name) return "Stone Skin";
		if (desc) return "Gives bonus to AC for a while.";
#endif
    
		{
			int base = 30;
			int sides = 20;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_shield(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "������";
		if (desc) return "������֡������ŷ⡢�ꡢ�䵤���Ǥ��Ф������������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resistance";
		if (desc) return "Gives resistance to fire, cold, electricity, acid and poison for a while. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_acid(randint1(base) + base, FALSE);
				set_oppose_elec(randint1(base) + base, FALSE);
				set_oppose_fire(randint1(base) + base, FALSE);
				set_oppose_cold(randint1(base) + base, FALSE);
				set_oppose_pois(randint1(base) + base, FALSE);
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "���ԡ���";
		if (desc) return "������֡���®���롣";
#else
		if (name) return "Haste Self";
		if (desc) return "Hastes you for a while.";
#endif
    
		{
			int base = plev;
			int sides = 20 + plev;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_fast(randint1(sides) + base, FALSE);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "��ȴ��";
		if (desc) return "������֡�Ⱦʪ�������ɤ��̤�ȴ������褦�ˤʤ롣";
#else
		if (name) return "Walk through Wall";
		if (desc) return "Gives ability to pass walls for a while.";
#endif
    
		{
			int base = plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_kabenuke(randint1(base) + base, FALSE);
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "���᤭";
		if (desc) return "���ȿ�ͤ�°����Ĥ��롣";
#else
		if (name) return "Polish Shield";
		if (desc) return "Makes a shield a shield of reflection.";
#endif
    
		{
			if (cast)
			{
				pulish_shield();
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "���������¤";
		if (desc) return "1�ΤΥ���������¤���롣";
#else
		if (name) return "Create Golem";
		if (desc) return "Creates a golem.";
#endif
    
		{
			if (cast)
			{
				if (summon_specific(-1, py, px, plev, SUMMON_GOLEM, PM_FORCE_PET))
				{
#ifdef JP
					msg_print("���������ä���");
#else
					msg_print("You make a golem.");
#endif
				}
				else
				{
#ifdef JP
					msg_print("���ޤ�����������ʤ��ä���");
#else
					msg_print("No Golems arrive.");
#endif
				}
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "��ˡ�γ�";
		if (desc) return "������֡���ˡ�ɸ��Ϥ�AC���夬�ꡢ��������ܤ�������ȿ��ǽ�ϡ������Τ餺����ͷ�����롣";
#else
		if (name) return "Magical armor";
		if (desc) return "Gives resistance to magic, bonus to AC, resistance to confusion, blindness, reflection, free action and levitation for a while.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_magicdef(randint1(base) + base, FALSE);
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "����̵�ϲ�";
		if (desc) return "���ɶ�ˤ�����줿���������Ϥ����˲�����롣";
#else
		if (name) return "Remove Enchantment";
		if (desc) return "Removes all magics completely from any weapon or armor.";
#endif
    
		{
			if (cast)
			{
				if (!mundane_spell(TRUE)) return NULL;
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "����ʴ��";
		if (desc) return "�����ƥ�ˤ����ä����Ϥʼ����������롣";
#else
		if (name) return "Remove All Curse";
		if (desc) return "Removes normal and heavy curse from equipped items.";
#endif
    
		{
			if (cast)
			{
				if (remove_all_curse())
				{
#ifdef JP
					msg_print("ï���˸�����Ƥ���褦�ʵ������롣");
#else
					msg_print("You feel as if someone is watching over you.");
#endif
				}
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "�����ʤ��μ�";
		if (desc) return "�����ƥ�λ���ǽ�Ϥ������Τ롣";
#else
		if (name) return "Knowledge True";
		if (desc) return "*Identifies* an item.";
#endif
    
		{
			if (cast)
			{
				if (!identify_fully(FALSE)) return NULL;
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "��ﶯ��";
		if (desc) return "����̿��Ψ�����ȥ��᡼�������򶯲����롣";
#else
		if (name) return "Enchant Weapon";
		if (desc) return "Attempts to increase +to-hit, +to-dam of a weapon.";
#endif
    
		{
			if (cast)
			{
				if (!enchant_spell(randint0(4) + 1, randint0(4) + 1, 0)) return NULL;
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "�ɶ񶯲�";
		if (desc) return "�����ɸ潤���򶯲����롣";
#else
		if (name) return "Enchant Armor";
		if (desc) return "Attempts to increase +AC of an armor.";
#endif
    
		{
			if (cast)
			{
				if (!enchant_spell(0, 0, randint0(3) + 2)) return NULL;
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "���°����Ϳ";
		if (desc) return "���˥������°����Ĥ��롣";
#else
		if (name) return "Brand Weapon";
		if (desc) return "Makes current weapon a random ego weapon.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(randint0(18));
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "�ʹ֥ȥ���";
		if (desc) return "������˥ƥ�ݡ��Ȥ��������Ѱۤ�����ʬ�ΰջפǥƥ�ݡ��Ȥ��������Ѱۤ��ȤˤĤ���";
#else
		if (name) return "Living Trump";
		if (desc) return "Gives mutation which makes you teleport randomly or makes you able to teleport at will.";
#endif
    
		{
			if (cast)
			{
				int mutation;

				if (one_in_(7))
					/* Teleport control */
					mutation = 12;
				else
					/* Random teleportation (uncontrolled) */
					mutation = 77;

				/* Gain the mutation */
				if (gain_random_mutation(mutation))
				{
#ifdef JP
					msg_print("���ʤ��������Ƥ��륫���ɤ��Ѥ�ä���");
#else
					msg_print("You have turned into a Living Trump.");
#endif
				}
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "°���ؤ��ȱ�";
		if (desc) return "������֡��䵤���ꡢ�ŷ⡢���Τ����줫���Ф����ȱ֤����롣";
#else
		if (name) return "Immunity";
		if (desc) return "Gives an immunity to fire, cold, electricity or acid for a while.";
#endif
    
		{
			int base = 13;

			if (info) return info_duration(base, base);

			if (cast)
			{
				if (!choose_ele_immune(base + randint1(base))) return NULL;
			}
		}
		break;
	}

	return "";
}


static cptr do_daemon_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

#ifdef JP
	static const char s_dam[] = "»��:";
#else
	static const char s_dam[] = "dam ";
#endif

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "�ޥ��å����ߥ�����";
		if (desc) return "�夤��ˡ��������ġ�";
#else
		if (name) return "Magic Missile";
		if (desc) return "Fires a weak bolt of magic.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir, damroll(dice, sides));
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "̵��̿����";
		if (desc) return "�᤯����̿�Τʤ���󥹥������Τ��롣";
#else
		if (name) return "Detect Unlife";
		if (desc) return "Detects all nonliving monsters in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_nonliving(rad);
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "�٤ʤ��ʡ";
		if (desc) return "������֡�̿��Ψ��AC�˥ܡ��ʥ������롣";
#else
		if (name) return "Evil Bless";
		if (desc) return "Gives bonus to hit and AC for a few turns.";
#endif
    
		{
			int base = 12;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_blessed(randint1(base) + base, FALSE);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "�Ѳб�";
		if (desc) return "������֡���ؤ����������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Resist Fire";
		if (desc) return "Gives resistance to fire, cold and electricity for a while. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_oppose_fire(randint1(base) + base, FALSE);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "����";
		if (desc) return "��󥹥���1�Τ��ݤ�����ۯ۰�����롣�񹳤�����̵����";
#else
		if (name) return "Horrify";
		if (desc) return "Attempts to scare and stun a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fear_monster(dir, power);
				stun_monster(dir, power);
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�Ϲ�����";
		if (desc) return "�Ϲ��Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Nether Bolt";
		if (desc) return "Fires a bolt or beam of nether.";
#endif
    
		{
			int dice = 6 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_NETHER, dir, damroll(dice, sides));
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "����λ����";
		if (desc) return "����λ���򾤴����롣";
#else
		if (name) return "Summon Manes";
		if (desc) return "Summons a manes.";
#endif
    
		{
			if (cast)
			{
				if (!summon_specific(-1, py, px, (plev * 3) / 2, SUMMON_MANES, (PM_ALLOW_GROUP | PM_FORCE_PET)))
				{
#ifdef JP
					msg_print("����λ���ϸ���ʤ��ä���");
#else
					msg_print("No Manes arrive.");
#endif
				}
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "�Ϲ��α�";
		if (desc) return "�ٰ����Ϥ���ĥܡ�������ġ����ɤʥ�󥹥����ˤ��礭�ʥ��᡼����Ϳ���롣";
#else
		if (name) return "Hellish Flame";
		if (desc) return "Fires a ball of evil power. Hurts good monsters greatly.";
#endif
    
		{
			int dice = 3;
			int sides = 6;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_MAGE ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HELL_FIRE, dir, damroll(dice, sides) + base, rad);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "�ǡ�������";
		if (desc) return "����1�Τ�̥λ���롣�񹳤�����̵��";
#else
		if (name) return "Dominate Demon";
		if (desc) return "Attempts to charm a demon.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				control_one_demon(dir, power);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "�ӥ����";
		if (desc) return "���դ��Ϸ����Τ��롣";
#else
		if (name) return "Vision";
		if (desc) return "Maps nearby area.";
#endif
    
		{
			int rad = DETECT_RAD_MAP;

			if (info) return info_radius(rad);

			if (cast)
			{
				map_area(rad);
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "���Ϲ�";
		if (desc) return "������֡��Ϲ��ؤ����������롣";
#else
		if (name) return "Resist Nether";
		if (desc) return "Gives resistance to nether for a while.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_res_nether(randint1(base) + base, FALSE);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "�ץ饺�ޡ��ܥ��";
		if (desc) return "�ץ饺�ޤΥܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Plasma bolt";
		if (desc) return "Fires a bolt or beam of plasma.";
#endif
    
		{
			int dice = 11 + (plev - 5) / 4;
			int sides = 8;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance(), GF_PLASMA, dir, damroll(dice, sides));
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�ե��������ܡ���";
		if (desc) return "��ε�����ġ�";
#else
		if (name) return "Fire Ball";
		if (desc) return "Fires a ball of fire.";
#endif
    
		{
			int dam = plev + 55;
			int rad = 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_FIRE, dir, dam, rad);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "��ο�";
		if (desc) return "���˱��°����Ĥ��롣";
#else
		if (name) return "Fire Branding";
		if (desc) return "Makes current weapon fire branded.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(1);
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "�Ϲ���";
		if (desc) return "�礭���Ϲ��ε�����ġ�";
#else
		if (name) return "Nether Ball";
		if (desc) return "Fires a huge ball of nether.";
#endif
    
		{
			int dam = plev * 3 / 2 + 100;
			int rad = plev / 20 + 2;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_NETHER, dir, dam, rad);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "�ǡ���󾤴�";
		if (desc) return "����1�Τ򾤴����롣";
#else
		if (name) return "Summon Demon";
		if (desc) return "Summons a demon.";
#endif
    
		{
			if (cast)
			{
				bool pet = !one_in_(3);
				u32b mode = 0L;

				if (pet) mode |= PM_FORCE_PET;
				else mode |= PM_NO_PET;
				if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

				if (summon_specific((pet ? -1 : 0), py, px, plev*2/3+randint1(plev/2), SUMMON_DEMON, mode))
				{
#ifdef JP
					msg_print("β���ΰ���������������");
#else
					msg_print("The area fills with a stench of sulphur and brimstone.");
#endif


					if (pet)
					{
#ifdef JP
						msg_print("�֤��ѤǤ������ޤ�����������͡�");
#else
						msg_print("'What is thy bidding... Master?'");
#endif
					}
					else
					{
#ifdef JP
						msg_print("���ܤ����Ԥ衢�����β��ͤˤ��餺�� �����κ���ĺ��������");
#else
						msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
#endif
					}
				}
				else
				{
#ifdef JP
					msg_print("����ϸ���ʤ��ä���");
#else
					msg_print("No demons arrive.");
#endif
				}
				break;
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "�������";
		if (desc) return "������֡��ƥ�ѥ���ǽ�Ϥ����롣";
#else
		if (name) return "Devilish Eye";
		if (desc) return "Gives telepathy for a while.";
#endif
    
		{
			int base = 30;
			int sides = 25;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_tim_esp(randint1(base) + sides, FALSE);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "����Υ�����";
		if (desc) return "���ݤ��������������֡�����䵤����������Υ���������롣�����������ˤ�����������Ѥ��롣";
#else
		if (name) return "Devil Cloak";
		if (desc) return "Removes fear. Gives resistance to fire and cold, and aura of fire. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				int dur = randint1(base) + base;
					
				set_oppose_fire(dur, FALSE);
				set_oppose_cold(dur, FALSE);
				set_tim_sh_fire(dur, FALSE);
				set_afraid(0);
				break;
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "�ϴ�ή";
		if (desc) return "��ʬ���濴�Ȥ�����ε����Ф��������ϴ���Ѥ��롣";
#else
		if (name) return "The Flow of Lava";
		if (desc) return "Generates a ball of fire centered on you which transforms floors to magma.";
#endif
    
		{
			int dam = (55 + plev) * 2;
			int rad = 3;

			if (info) return info_damage(0, 0, dam/2);

			if (cast)
			{
				fire_ball(GF_FIRE, 0, dam, rad);
				fire_ball_hide(GF_LAVA_FLOW, 0, 2 + randint1(2), rad);
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "�ץ饺�޵�";
		if (desc) return "�ץ饺�ޤε�����ġ�";
#else
		if (name) return "Plasma Ball";
		if (desc) return "Fires a ball of plasma.";
#endif
    
		{
			int dam = plev * 3 / 2 + 80;
			int rad = 2 + plev / 40;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_PLASMA, dir, dam, rad);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "�����Ѳ�";
		if (desc) return "������֡�������Ѳ����롣�Ѳ����Ƥ���֤�����μ�²��ǽ�Ϥ򼺤�������˰���Ȥ��Ƥ�ǽ�Ϥ����롣";
#else
		if (name) return "Polymorph Demon";
		if (desc) return "Mimic a demon for a while. Loses abilities of original race and gets abilities as a demon.";
#endif
    
		{
			int base = 10 + plev / 2;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_mimic(base + randint1(base), MIMIC_DEMON, FALSE);
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "�Ϲ�����ư";
		if (desc) return "�볦������ƤΥ�󥹥����˥��᡼����Ϳ���롣���ɤʥ�󥹥������ä��礭�ʥ��᡼����Ϳ���롣";
#else
		if (name) return "Nather Wave";
		if (desc) return "Damages all monsters in sight. Hurts good monsters greatly.";
#endif
    
		{
			int sides1 = plev * 2;
			int sides2 = plev * 2;

			if (info) return format("%sd%d+d%d", s_dam, sides1, sides2);

			if (cast)
			{
				dispel_monsters(randint1(sides1));
				dispel_good(randint1(sides2));
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "������Х�����ʭ";
		if (desc) return "���̺���ε�����ġ�";
#else
		if (name) return "Kiss of Succubus";
		if (desc) return "Fires a ball of nexus.";
#endif
    
		{
			int dam = 100 + plev * 2;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball(GF_NEXUS, dir, dam, rad);
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "���Ǥμ�";
		if (desc) return "���Ǥμ�����ġ�����ä���󥹥����Ϥ��ΤȤ���HP��Ⱦʬ����Υ��᡼��������롣";
#else
		if (name) return "Doom Hand";
		if (desc) return "Attempts to make a monster's HP almost half.";
#endif
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
#ifdef JP
				else msg_print("<���Ǥμ�>�����ä���");
#else
				else msg_print("You invoke the Hand of Doom!");
#endif

				fire_ball_hide(GF_HAND_DOOM, dir, plev * 2, 0);
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "�ε�����";
		if (desc) return "������֡��ҡ�����ʬ�ˤʤ롣";
#else
		if (name) return "Raise the Morale";
		if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";
#endif
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_hero(randint1(base) + base, FALSE);
				hp_player(10);
				set_afraid(0);
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "���Ǥ�����";
		if (desc) return "������֡����ֵ�ž�ؤ����������롣";
#else
		if (name) return "Immortal Body";
		if (desc) return "Gives resistance to time for a while.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_res_time(randint1(base)+base, FALSE);
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "�����αߴ�";
		if (desc) return "��ʬ���濴�Ȥ����������ε塢����ε��ȯ���������᤯�Υ�󥹥�����̥λ���롣";
#else
		if (name) return "Insanity Circle";
		if (desc) return "Generate balls of chaos, confusion and charm centered on you.";
#endif
    
		{
			int dam = 50 + plev;
			int power = 20 + plev;
			int rad = 3 + plev / 20;

			if (info) return format("%s%d+%d", s_dam, dam/2, dam/2);

			if (cast)
			{
				fire_ball(GF_CHAOS, 0, dam, rad);
				fire_ball(GF_CONFUSION, 0, dam, rad);
				fire_ball(GF_CHARM, 0, power, rad);
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "�ڥå�����";
		if (desc) return "���ƤΥڥåȤ���Ū�����ˤ����롣";
#else
		if (name) return "Explode Pets";
		if (desc) return "Makes all pets explode.";
#endif
    
		{
			if (cast)
			{
				discharge_minion();
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "���졼�����ǡ���󾤴�";
		if (desc) return "���ǡ����򾤴����롣��������ˤϿʹ�('p','h','t'��ɽ������󥹥���)�λ��Τ������ʤ���Фʤ�ʤ���";
#else
		if (name) return "Summon Greater Demon";
		if (desc) return "Summons greater demon. It need to sacrifice a corpse of human ('p','h' or 't').";
#endif
    
		{
			if (cast)
			{
				if (!cast_summon_greater_demon()) return NULL;
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "�Ϲ���";
		if (desc) return "Ķ������Ϲ��ε�����ġ�";
#else
		if (name) return "Nether Storm";
		if (desc) return "Generate a huge ball of nether.";
#endif
    
		{
			int dam = plev * 15;
			int rad = plev / 5;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_NETHER, dir, dam, rad);
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "��μ���";
		if (desc) return "��ʬ�����᡼��������뤳�Ȥˤ�ä��оݤ˼����򤫤������᡼����Ϳ���͡��ʸ��̤������������";
#else
		if (name) return "Bloody Curse";
		if (desc) return "Puts blood curse which damages and causes various effects on a monster. You also take damage.";
#endif
    
		{
			int dam = 600;
			int rad = 0;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball_hide(GF_BLOOD_CURSE, dir, dam, rad);
#ifdef JP
				take_hit(DAMAGE_USELIFE, 20 + randint1(30), "��μ���", -1);
#else
				take_hit(DAMAGE_USELIFE, 20 + randint1(30), "Blood curse", -1);
#endif
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "�Ⲧ�Ѳ�";
		if (desc) return "����β����Ѳ����롣�Ѳ����Ƥ���֤�����μ�²��ǽ�Ϥ򼺤�������˰���β��Ȥ��Ƥ�ǽ�Ϥ������ɤ��˲����ʤ����⤯��";
#else
		if (name) return "Polymorph Demonlord";
		if (desc) return "Mimic a demon lord for a while. Loses abilities of original race and gets great abilities as a demon lord. Even hard walls can't stop your walking.";
#endif
    
		{
			int base = 15;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_mimic(base + randint1(base), MIMIC_DEMON_LORD, FALSE);
			}
		}
		break;
	}

	return "";
}


static cptr do_crusade_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "Ĩȳ";
		if (desc) return "�ŷ�Υܥ�Ȥ⤷���ϥӡ�������ġ�";
#else
		if (name) return "Punishment";
		if (desc) return "Fires a bolt or beam of lightning.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir, damroll(dice, sides));
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "�ٰ�¸�ߴ���";
		if (desc) return "�᤯�μٰ��ʥ�󥹥������Τ��롣";
#else
		if (name) return "Detect Evil";
		if (desc) return "Detects all evil monsters in your vicinity.";
#endif
    
		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cast)
			{
				detect_monsters_evil(rad);
			}
		}
		break;

	case 2:
#ifdef JP
		if (name) return "���ݽ���";
		if (desc) return "���ݤ��������";
#else
		if (name) return "Remove Fear";
		if (desc) return "Removes fear.";
#endif
    
		{
			if (cast)
			{
				set_afraid(0);
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "�Ұ�";
		if (desc) return "��󥹥���1�Τ��ݤ����롣�񹳤�����̵����";
#else
		if (name) return "Scare Monster";
		if (desc) return "Attempts to scare a monster.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fear_monster(dir, power);
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "����";
		if (desc) return "���ܤ������ƤΥ�󥹥�����̲�餻�롣�񹳤�����̵����";
#else
		if (name) return "Sanctuary";
		if (desc) return "Attempts to sleep monsters in the adjacent squares.";
#endif
    
		{
			int power = plev;

			if (info) return info_power(power);

			if (cast)
			{
				sleep_monsters_touch();
			}
		}
		break;

	case 5:
#ifdef JP
		if (name) return "����";
		if (desc) return "���Υ�Υƥ�ݡ��Ȥ򤹤롣";
#else
		if (name) return "Portal";
		if (desc) return "Teleport medium distance.";
#endif
    
		{
			int range = 25 + plev / 2;

			if (info) return info_range(range);

			if (cast)
			{
				teleport_player(range, 0L);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "������������";
		if (desc) return "�������å��ն�������Υܥ�Ȥ�Ϣ�ͤ��롣";
#else
		if (name) return "Star Dust";
		if (desc) return "Fires many bolts of light near the target.";
#endif
    
		{
			int dice = 3 + (plev - 1) / 9;
			int sides = 2;

			if (info) return info_multi_damage_dice(dice, sides);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_blast(GF_LITE, dir, dice, sides, 10, 3);
			}
		}
		break;

	case 7:
#ifdef JP
		if (name) return "���ξ���";
		if (desc) return "�����ǡ�ۯ۰�����������롣";
#else
		if (name) return "Purify";
		if (desc) return "Heals all cut, stun and poison status.";
#endif
    
		{
			if (cast)
			{
				set_cut(0);
				set_poisoned(0);
				set_stun(0);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "�ٰ����Ф�";
		if (desc) return "�ٰ��ʥ�󥹥���1�Τ�ƥ�ݡ��Ȥ����롣�񹳤�����̵����";
#else
		if (name) return "Scatter Evil";
		if (desc) return "Attempts to teleport an evil monster away.";
#endif
    
		{
			int power = MAX_SIGHT * 5;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_ball(GF_AWAY_EVIL, dir, power, 0);
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "���ʤ����";
		if (desc) return "���ʤ��Ϥ�����������ġ��ٰ��ʥ�󥹥������Ф����礭�ʥ��᡼����Ϳ���뤬�����ɤʥ�󥹥����ˤϸ��̤��ʤ���";
#else
		if (name) return "Holy Orb";
		if (desc) return "Fires a ball with holy power. Hurts evil monsters greatly, but don't effect good monsters.";
#endif
    
		{
			int dice = 3;
			int sides = 6;
			int rad = (plev < 30) ? 2 : 3;
			int base;

			if (p_ptr->pclass == CLASS_PRIEST ||
			    p_ptr->pclass == CLASS_HIGH_MAGE ||
			    p_ptr->pclass == CLASS_SORCERER)
				base = plev + plev / 2;
			else
				base = plev + plev / 4;


			if (info) return info_damage(dice, sides, base);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_HOLY_FIRE, dir, damroll(dice, sides) + base, rad);
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "����ʧ��";
		if (desc) return "�볦������ƤΥ���ǥåɵڤӰ���˥��᡼����Ϳ�����ٰ��ʥ�󥹥������ݤ����롣";
#else
		if (name) return "Exorcism";
		if (desc) return "Damages all undead and demons in sight, and scares all evil monsters in sight.";
#endif
    
		{
			int sides = plev;
			int power = plev;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_undead(randint1(sides));
				dispel_demons(randint1(sides));
				turn_evil(power);
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "���";
		if (desc) return "�����ƥ�ˤ����ä��夤�����������롣";
#else
		if (name) return "Remove Curse";
		if (desc) return "Removes normal curses from equipped items.";
#endif
    
		{
			if (cast)
			{
				if (remove_curse())
				{
#ifdef JP
					msg_print("ï���˸�����Ƥ���褦�ʵ������롣");
#else
					msg_print("You feel as if someone is watching over you.");
#endif
				}
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "Ʃ����ǧ";
		if (desc) return "������֡�Ʃ���ʤ�Τ�������褦�ˤʤ롣";
#else
		if (name) return "Sense Unseen";
		if (desc) return "Gives see invisible for a while.";
#endif
    
		{
			int base = 24;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_invis(randint1(base) + base, FALSE);
			}
		}
		break;

	case 13:
#ifdef JP
		if (name) return "�мٰ��볦";
		if (desc) return "�ٰ��ʥ�󥹥����ι�����ɤ��Хꥢ��ĥ�롣";
#else
		if (name) return "Protection from Evil";
		if (desc) return "Gives aura which protect you from evil monster's physical attack.";
#endif
    
		{
			int base = 25;
			int sides = 3 * plev;

			if (info) return info_duration(base, sides);

			if (cast)
			{
				set_protevil(randint1(sides) + sides, FALSE);
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "�ۤ�����";
		if (desc) return "���Ϥ��ŷ�Υܥ�Ȥ����ġ�";
#else
		if (name) return "Judgment Thunder";
		if (desc) return "Fires a powerful bolt of lightning.";
#endif
    
		{
			int dam = plev * 5;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				fire_bolt(GF_ELEC, dir, dam);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "���ʤ�����";
		if (desc) return "�볦��μٰ���¸�ߤ��礭�ʥ��᡼����Ϳ�������Ϥ���������ǡ����ݡ�ۯ۰���֡���������������롣";
#else
		if (name) return "Holy Word";
		if (desc) return "Damages all evil monsters in sight, heals HP somewhat, and completely heals poison, fear, stun and cut status.";
#endif
    
		{
			int dam_sides = plev * 6;
			int heal = 100;

#ifdef JP
			if (info) return format("»:1d%d/��%d", dam_sides, heal);
#else
			if (info) return format("dam:d%d/h%d", dam_sides, heal);
#endif

			if (cast)
			{
				dispel_evil(randint1(dam_sides));
				hp_player(heal);
				set_afraid(0);
				set_poisoned(0);
				set_stun(0);
				set_cut(0);
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "�����줿ƻ";
		if (desc) return "��ľ��������Ƥ�櫤�����˲����롣";
#else
		if (name) return "Unbarring Ways";
		if (desc) return "Fires a beam which destroy traps and doors.";
#endif
    
		{
			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				destroy_door(dir);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "����";
		if (desc) return "�ٰ��ʥ�󥹥�����ư����ߤ�롣";
#else
		if (name) return "Arrest";
		if (desc) return "Attempts to paralyze an evil monster.";
#endif
    
		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;
				stasis_evil(dir);
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "���ʤ륪����";
		if (desc) return "������֡��ٰ��ʥ�󥹥�������Ĥ������ʤ륪��������롣";
#else
		if (name) return "Holy Aura";
		if (desc) return "Gives aura of holy power which injures evil monsters which attacked you for a while.";
#endif
    
		{
			int base = 20;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_sh_holy(randint1(base) + base, FALSE);
			}
		}
		break;

	case 19:
#ifdef JP
		if (name) return "����ǥå�&�����໶";
		if (desc) return "�볦������ƤΥ���ǥåɵڤӰ���˥��᡼����Ϳ���롣";
#else
		if (name) return "Dispel Undead & Demons";
		if (desc) return "Damages all undead and demons in sight.";
#endif
    
		{
			int sides = plev * 4;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_undead(randint1(sides));
				dispel_demons(randint1(sides));
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "�ٰ��໶";
		if (desc) return "�볦������Ƥμٰ��ʥ�󥹥����˥��᡼����Ϳ���롣";
#else
		if (name) return "Dispel Evil";
		if (desc) return "Damages all evil monsters in sight.";
#endif
    
		{
			int sides = plev * 4;

			if (info) return info_damage(1, sides, 0);

			if (cast)
			{
				dispel_evil(randint1(sides));
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "���ʤ��";
		if (desc) return "�̾�������Ǽ٤�°����Ĥ��롣";
#else
		if (name) return "Holy Blade";
		if (desc) return "Makes current weapon especially deadly against evil monsters.";
#endif
    
		{
			if (cast)
			{
				brand_weapon(13);
			}
		}
		break;

	case 22:
#ifdef JP
		if (name) return "�������С�����";
		if (desc) return "����������ε�����ġ�";
#else
		if (name) return "Star Burst";
		if (desc) return "Fires a huge ball of powerful light.";
#endif
    
		{
			int dam = 100 + plev * 2;
			int rad = 4;

			if (info) return info_damage(0, 0, dam);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_LITE, dir, dam, rad);
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "ŷ�Ⱦ���";
		if (desc) return "ŷ�Ȥ�1�ξ������롣";
#else
		if (name) return "Summon Angel";
		if (desc) return "Summons an angel.";
#endif
    
		{
			if (cast)
			{
				bool pet = !one_in_(3);
				u32b mode = 0L;

				if (pet) mode |= PM_FORCE_PET;
				else mode |= PM_NO_PET;
				if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

				if (summon_specific((pet ? -1 : 0), py, px, (plev * 3) / 2, SUMMON_ANGEL, mode))
				{
					if (pet)
					{
#ifdef JP
						msg_print("�֤��ѤǤ������ޤ�����������͡�");
#else
						msg_print("'What is thy bidding... Master?'");
#endif
					}
					else
					{
#ifdef JP
						msg_print("�ֲ����β��ͤˤ��餺�� ���ԼԤ衢��������衪��");
#else
						msg_print("Mortal! Repent of thy impiousness.");
#endif
					}
				}
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "�ε�����";
		if (desc) return "������֡��ҡ�����ʬ�ˤʤ롣";
#else
		if (name) return "Heroism";
		if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";
#endif
    
		{
			int base = 25;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_hero(randint1(base) + base, FALSE);
				hp_player(10);
				set_afraid(0);
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "�����໶";
		if (desc) return "�����ƥ�ˤ����ä����Ϥʼ����������롣";
#else
		if (name) return "Dispel Curse";
		if (desc) return "Removes normal and heavy curse from equipped items.";
#endif
    
		{
			if (cast)
			{
				if (remove_all_curse())
				{
#ifdef JP
					msg_print("ï���˸�����Ƥ���褦�ʵ������롣");
#else
					msg_print("You feel as if someone is watching over you.");
#endif
				}
			}
		}
		break;

	case 26:
#ifdef JP
		if (name) return "�ٰ�����";
		if (desc) return "�볦������Ƥμٰ��ʥ�󥹥�����ƥ�ݡ��Ȥ����롣�񹳤�����̵����";
#else
		if (name) return "Banish Evil";
		if (desc) return "Teleports all evil monsters in sight away unless resisted.";
#endif
    
		{
			int power = 100;

			if (info) return info_power(power);

			if (cast)
			{
				if (banish_evil(power))
				{
#ifdef JP
					msg_print("�������Ϥ��ٰ����Ǥ�ʧ�ä���");
#else
					msg_print("The holy power banishes evil!");
#endif

				}
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "�ϥ�ޥ��ɥ�";
		if (desc) return "���դΥ����ƥࡢ��󥹥������Ϸ����˲����롣";
#else
		if (name) return "Armageddon";
		if (desc) return "Destroy everything in nearby area.";
#endif
    
		{
			int base = 12;
			int sides = 4;

			if (cast)
			{
				destroy_area(py, px, base + randint1(sides), FALSE);
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "�ܤˤ��ܤ�";
		if (desc) return "������֡���ʬ�����᡼����������Ȥ��˹����Ԥä���󥹥������Ф���Ʊ���Υ��᡼����Ϳ���롣";
#else
		if (name) return "An Eye for an Eye";
		if (desc) return "Gives special aura for a while. When you are attacked by a monster, the monster are injured with same amount of damage as you take.";
#endif
    
		{
			int base = 10;

			if (info) return info_duration(base, base);

			if (cast)
			{
				set_tim_eyeeye(randint1(base) + base, FALSE);
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "�����ܤ�";
		if (desc) return "�������åȤμ��Ϥ�ʬ��ε��¿����Ȥ���";
#else
		if (name) return "Wrath of the God";
		if (desc) return "Drops many balls of disintegration near the target.";
#endif
    
		{
			int dam = plev * 3 + 25;
			int rad = 2;

			if (info) return info_multi_damage(dam);

			if (cast)
			{
				if (!cast_wrath_of_the_god(dam, rad)) return NULL;
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "����";
		if (desc) return "���ܤ����󥹥��������ʤ���᡼����Ϳ�����볦��Υ�󥹥����˥��᡼������®��ۯ۰�����𡢶��ݡ�̲���Ϳ���롣��������Ϥ�������롣";
#else
		if (name) return "Divine Intervention";
		if (desc) return "Damages all adjacent monsters with holy power. Damages and attempt to slow, stun, confuse, scare and freeze all monsters in sight. And heals HP.";
#endif
    
		{
			int b_dam = plev * 11;
			int d_dam = plev * 4;
			int heal = 100;
			int power = plev * 4;

#ifdef JP
			if (info) return format("��%d/»%d+%d", heal, d_dam, b_dam/2);
#else
			if (info) return format("h%d/dm%d+%d", heal, d_dam, b_dam/2);
#endif

			if (cast)
			{
				project(0, 1, py, px, b_dam, GF_HOLY_FIRE, PROJECT_KILL, -1);
				dispel_monsters(d_dam);
				slow_monsters();
				stun_monsters(power);
				confuse_monsters(power);
				turn_monsters(power);
				stasis_monsters(power);
				hp_player(heal);
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "����";
		if (desc) return "�볦������ɤʥ�󥹥�����ڥåȤˤ��褦�Ȥ����ʤ�ʤ��ä����ڤ����ɤǤʤ���󥹥������ݤ����롣�����¿���β�®���줿���Τ򾤴������ҡ�������ʡ����®���мٰ��볦�����롣";
#else
		if (name) return "Crusade";
		if (desc) return "Attempts to charm all good monsters in sight, and scare all non-charmed monsters, and summons great number of knights, and gives heroism, bless, speed and protection from evil.";
#endif
    
		{
			if (cast)
			{
				int base = 25;
				int sp_sides = 20 + plev;
				int sp_base = plev;

				int i;
				crusade();
				for (i = 0; i < 12; i++)
				{
					int attempt = 10;
					int my, mx;

					while (attempt--)
					{
						scatter(&my, &mx, py, px, 4, 0);

						/* Require empty grids */
						if (cave_empty_bold2(my, mx)) break;
					}
					if (attempt < 0) continue;
					summon_specific(-1, my, mx, plev, SUMMON_KNIGHTS, (PM_ALLOW_GROUP | PM_FORCE_PET | PM_HASTE));
				}
				set_hero(randint1(base) + base, FALSE);
				set_blessed(randint1(base) + base, FALSE);
				set_fast(randint1(sp_sides) + sp_base, FALSE);
				set_protevil(randint1(base) + base, FALSE);
				set_afraid(0);
			}
		}
		break;
	}

	return "";
}


static cptr do_music_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;
	bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
	bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

#ifdef JP
	static const char s_dam[] = "»��:";
#else
	static const char s_dam[] = "dam ";
#endif

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "���ߤβ�";
		if (desc) return "�볦������ƤΥ�󥹥�����®�����롣�񹳤�����̵����";
#else
		if (name) return "Song of Holding";
		if (desc) return "Attempts to slow all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("��ä���Ȥ������ǥ���������߻Ϥ᤿������");
#else
			msg_print("You start humming a slow, steady melody...");
#endif
			start_singing(spell, MUSIC_SLOW);
		}

		{
			int power = plev;

			if (info) return info_power(power);

			if (cont)
			{
				slow_monsters();
			}
		}
		break;

	case 1:
#ifdef JP
		if (name) return "��ʡ�β�";
		if (desc) return "̿��Ψ��AC�Υܡ��ʥ������롣";
#else
		if (name) return "Song of Blessing";
		if (desc) return "Gives bonus to hit and AC for a few turns.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�����ʥ��ǥ����դǻϤ᤿������");
#else
			msg_print("The holy power of the Music of the Ainur enters you...");
#endif
			start_singing(spell, MUSIC_BLESS);
		}

		if (stop)
		{
			if (!p_ptr->blessed)
			{
#ifdef JP
				msg_print("���ʵ�ʬ���ä���������");
#else
				msg_print("The prayer has expired.");
#endif
			}
		}

		break;

	case 2:
#ifdef JP
		if (name) return "�����β���";
		if (desc) return "�첻�Υܥ�Ȥ����ġ�";
#else
		if (name) return "Wrecking Note";
		if (desc) return "Fires a bolt of sound.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		{
			int dice = 4 + (plev - 1) / 5;
			int sides = 4;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_bolt(GF_SOUND, dir, damroll(dice, sides));
			}
		}
		break;

	case 3:
#ifdef JP
		if (name) return "ۯ۰����Χ";
		if (desc) return "�볦������ƤΥ�󥹥�����ۯ۰�����롣�񹳤�����̵����";
#else
		if (name) return "Stun Pattern";
		if (desc) return "Attempts to stun all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("���Ǥ�������ǥ����դǻϤ᤿������");
#else
			msg_print("You weave a pattern of sounds to bewilder and daze...");
#endif
			start_singing(spell, MUSIC_STUN);
		}

		{
			int dice = plev / 10;
			int sides = 2;

			if (info) return info_power_dice(dice, sides);

			if (cont)
			{
				stun_monsters(damroll(dice, sides));
			}
		}

		break;

	case 4:
#ifdef JP
		if (name) return "��̿��ή��";
		if (desc) return "���Ϥ򾯤����������롣";
#else
		if (name) return "Flow of Life";
		if (desc) return "Heals HP a little.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�Τ��̤����Τ˳赤����äƤ���������");
#else
			msg_print("Life flows through you as you sing a song of healing...");
#endif
			start_singing(spell, MUSIC_L_LIFE);
		}

		{
			int dice = 2;
			int sides = 6;

			if (info) return info_heal(dice, sides, 0);

			if (cont)
			{
				hp_player(damroll(dice, sides));
			}
		}

		break;

	case 5:
#ifdef JP
		if (name) return "���ۤβ�";
		if (desc) return "�������Ȥ餷�Ƥ����ϰϤ��������Τ�ʵפ����뤯���롣";
#else
		if (name) return "Song of the Sun";
		if (desc) return "Lights up nearby area and the inside of a room permanently.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		{
			int dice = 2;
			int sides = plev / 2;
			int rad = plev / 10 + 1;

			if (info) return info_damage(dice, sides, 0);

			if (cast)
			{
#ifdef JP
				msg_print("���굱���Τ��դ��Ȥ餷����");
#else
				msg_print("Your uplifting song brings brightness to dark places...");
#endif

				lite_area(damroll(dice, sides), rad);
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "���ݤβ�";
		if (desc) return "�볦������ƤΥ�󥹥������ݤ����롣�񹳤�����̵����";
#else
		if (name) return "Song of Fear";
		if (desc) return "Attempts to scare all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("���ɤ��ɤ������ǥ����դǻϤ᤿������");
#else
			msg_print("You start weaving a fearful pattern...");
#endif
			start_singing(spell, MUSIC_FEAR);			
		}

		{
			int power = plev;

			if (info) return info_power(power);

			if (cont)
			{
				project_hack(GF_TURN_ALL, power);
			}
		}

		break;

	case 7:
#ifdef JP
		if (name) return "�襤�β�";
		if (desc) return "�ҡ�����ʬ�ˤʤ롣";
#else
		if (name) return "Heroic Ballad";
		if (desc) return "Removes fear, and gives bonus to hit and 10 more HP for a while.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�㤷���襤�βΤ�Τä�������");
#else
			msg_print("You start singing a song of intense fighting...");
#endif

			(void)hp_player(10);
			(void)set_afraid(0);

			/* Recalculate hitpoints */
			p_ptr->update |= (PU_HP);

			start_singing(spell, MUSIC_HERO);
		}

		if (stop)
		{
			if (!p_ptr->hero)
			{
#ifdef JP
				msg_print("�ҡ����ε�ʬ���ä���������");
#else
				msg_print("The heroism wears off.");
#endif
				/* Recalculate hitpoints */
				p_ptr->update |= (PU_HP);
			}
		}

		break;

	case 8:
#ifdef JP
		if (name) return "��Ū�γ�";
		if (desc) return "�᤯���/��/���ʤ��Τ��롣��٥�15�����ƤΥ�󥹥�����20�Ǻ����ȥ����ƥ���ΤǤ���褦�ˤʤ롣��٥�25�Ǽ��դ��Ϸ����Τ���40�Ǥ��γ����Τ�ʵפ˾Ȥ餷�����󥸥����Τ��٤ƤΥ����ƥ���Τ��롣���θ��̤ϲΤ�³���뤳�Ȥǽ�˵����롣";
#else
		if (name) return "Clairaudience";
		if (desc) return "Detects traps, doors and stairs in your vicinity. And detects all monsters at level 15, treasures and items at level 20. Maps nearby area at level 25. Lights and know the whole level at level 40. These effects occurs by turns while this song continues.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�Ť��ʲ��ڤ����Ф򸦤����ޤ�����������");
#else
			msg_print("Your quiet music sharpens your sense of hearing...");
#endif

			/* Hack -- Initialize the turn count */
			p_ptr->magic_num1[2] = 0;

			start_singing(spell, MUSIC_DETECT);
		}

		{
			int rad = DETECT_RAD_DEFAULT;

			if (info) return info_radius(rad);

			if (cont)
			{
				int count = p_ptr->magic_num1[2];

				if (count >= 19) wiz_lite(FALSE);
				if (count >= 11)
				{
					map_area(rad);
					if (plev > 39 && count < 19)
						p_ptr->magic_num1[2] = count + 1;
				}
				if (count >= 6)
				{
					/* There are too many hidden treasure.  So... */
					/* detect_treasure(rad); */
					detect_objects_gold(rad);
					detect_objects_normal(rad);

					if (plev > 24 && count < 11)
						p_ptr->magic_num1[2] = count + 1;
				}
				if (count >= 3)
				{
					detect_monsters_invis(rad);
					detect_monsters_normal(rad);

					if (plev > 19 && count < 6)
						p_ptr->magic_num1[2] = count + 1;
				}
				detect_traps(rad, TRUE);
				detect_doors(rad);
				detect_stairs(rad);

				if (plev > 14 && count < 3)
					p_ptr->magic_num1[2] = count + 1;
			}
		}

		break;

	case 9:
#ifdef JP
		if (name) return "���β�";
		if (desc) return "�볦������ƤΥ�󥹥������Ф������������Ԥ���";
#else
		if (name) return "Soul Shriek";
		if (desc) return "Damages all monsters in sight with PSI damages.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("������Ǳ���ʤ���Τ�Τä�������");
#else
			msg_print("You start singing a song of soul in pain...");
#endif
			start_singing(spell, MUSIC_PSI);
		}

		{
			int dice = 1;
			int sides = plev * 3 / 2;

			if (info) return info_damage(dice, sides, 0);

			if (cont)
			{
				project_hack(GF_PSI, damroll(dice, sides));
			}
		}

		break;

	case 10:
#ifdef JP
		if (name) return "�μ��β�";
		if (desc) return "��ʬ�Τ���ޥ����٤�Υޥ�������Ƥ��륢���ƥ����ꤹ�롣";
#else
		if (name) return "Song of Lore";
		if (desc) return "Identifies all items which are in the adjacent squares.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�����������μ���ή�����Ǥ���������");
#else
			msg_print("You recall the rich lore of the world...");
#endif
			start_singing(spell, MUSIC_ID);
		}

		{
			int rad = 1;

			if (info) return info_radius(rad);

			/*
			 * �Τγ��ϻ��ˤ����ȯư��
			 * MP��­�Ǵ��꤬ȯư��������˲Τ����Ǥ��Ƥ��ޤ��Τ��ɻߡ�
			 */
			if (cont || cast)
			{
				project(0, rad, py, px, 0, GF_IDENTIFY, PROJECT_ITEM, -1);
			}
		}

		break;

	case 11:
#ifdef JP
		if (name) return "���ۤβ�";
		if (desc) return "��̩��ưǽ�Ϥ�徺�����롣";
#else
		if (name) return "Hiding Tune";
		if (desc) return "Gives improved stealth.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("���ʤ��λѤ��ʿ��ˤȤ�����Ǥ��ä�������");
#else
			msg_print("Your song carries you beyond the sight of mortal eyes...");
#endif
			start_singing(spell, MUSIC_STEALTH);
		}

		if (stop)
		{
			if (!p_ptr->tim_stealth)
			{
#ifdef JP
				msg_print("�Ѥ��Ϥä���ȸ�����褦�ˤʤä���");
#else
				msg_print("You are no longer hided.");
#endif
			}
		}

		break;

	case 12:
#ifdef JP
		if (name) return "���Ƥ���Χ";
		if (desc) return "�볦������ƤΥ�󥹥������𤵤��롣�񹳤�����̵����";
#else
		if (name) return "Illusion Pattern";
		if (desc) return "Attempts to confuse all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�դ���̤˸��Ƥ����줿������");
#else
			msg_print("You weave a pattern of sounds to beguile and confuse...");
#endif
			start_singing(spell, MUSIC_CONF);
		}

		{
			int power = plev * 2;

			if (info) return info_power(power);

			if (cont)
			{
				confuse_monsters(power);
			}
		}

		break;

	case 13:
#ifdef JP
		if (name) return "���Ǥζ���";
		if (desc) return "�볦������ƤΥ�󥹥������Ф��ƹ첻�����Ԥ���";
#else
		if (name) return "Doomcall";
		if (desc) return "Damages all monsters in sight with booming sound.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�첻��������������");
#else
			msg_print("The fury of the Downfall of Numenor lashes out...");
#endif
			start_singing(spell, MUSIC_SOUND);
		}

		{
			int dice = 10 + plev / 5;
			int sides = 7;

			if (info) return info_damage(dice, sides, 0);

			if (cont)
			{
				project_hack(GF_SOUND, damroll(dice, sides));
			}
		}

		break;

	case 14:
#ifdef JP
		if (name) return "�ե��ꥨ��β�";
		if (desc) return "���Ϥλ��Τ���������֤���";
#else
		if (name) return "Firiel's Song";
		if (desc) return "Resurrects nearby corpse and skeletons. And makes these your pets.";
#endif
    
		{
			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
#ifdef JP
				msg_print("��̿������Υơ��ޤ��դǻϤ᤿������");
#else
				msg_print("The themes of life and revival are woven into your song...");
#endif

				animate_dead(0, py, px);
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "ι�����";
		if (desc) return "�볦������ƤΥ�󥹥�����̥λ���롣�񹳤�����̵����";
#else
		if (name) return "Fellowship Chant";
		if (desc) return "Attempts to charm all monsters in sight.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�¤餫�ʥ��ǥ����դǻϤ᤿������");
#else
			msg_print("You weave a slow, soothing melody of imploration...");
#endif
			start_singing(spell, MUSIC_CHARM);
		}

		{
			int dice = 10 + plev / 15;
			int sides = 6;

			if (info) return info_power_dice(dice, sides);

			if (cont)
			{
				charm_monsters(damroll(dice, sides));
			}
		}

		break;

	case 16:
#ifdef JP
		if (name) return "ʬ����";
		if (desc) return "�ɤ򷡤�ʤࡣ��ʬ��­���Υ����ƥ�Ͼ�ȯ���롣";
#else
		if (name) return "Sound of disintegration";
		if (desc) return "Makes you be able to burrow into walls. Objects under your feet evaporate.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("ʴ�դ�����ǥ����դǻϤ᤿������");
#else
			msg_print("You weave a violent pattern of sounds to break wall.");
#endif
			start_singing(spell, MUSIC_WALL);
		}

		{
			/*
			 * �Τγ��ϻ��ˤ����ȯư��
			 * MP��­�Ǹ��̤�ȯư��������˲Τ����Ǥ��Ƥ��ޤ��Τ��ɻߡ�
			 */
			if (cont || cast)
			{
				project(0, 0, py, px,
					0, GF_DISINTEGRATE, PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE, -1);
			}
		}
		break;

	case 17:
#ifdef JP
		if (name) return "��������";
		if (desc) return "�����ŷ⡢�ꡢ�䵤���Ǥ��Ф������������롣�����ˤ�����������Ѥ��롣";
#else
		if (name) return "Finrod's Resistance";
		if (desc) return "Gives resistance to fire, cold, electricity, acid and poison. These resistances can be added to which from equipment for more powerful resistances.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("���Ǥ��Ϥ��Ф���Ǧ�ѤβΤ�Τä���");
#else
			msg_print("You sing a song of perseverance against powers...");
#endif
			start_singing(spell, MUSIC_RESIST);
		}

		if (stop)
		{
			if (!p_ptr->oppose_acid)
			{
#ifdef JP
				msg_print("���ؤ����������줿�������롣");
#else
				msg_print("You feel less resistant to acid.");
#endif
			}

			if (!p_ptr->oppose_elec)
			{
#ifdef JP
				msg_print("�ŷ�ؤ����������줿�������롣");
#else
				msg_print("You feel less resistant to elec.");
#endif
			}

			if (!p_ptr->oppose_fire)
			{
#ifdef JP
				msg_print("�Фؤ����������줿�������롣");
#else
				msg_print("You feel less resistant to fire.");
#endif
			}

			if (!p_ptr->oppose_cold)
			{
#ifdef JP
				msg_print("�䵤�ؤ����������줿�������롣");
#else
				msg_print("You feel less resistant to cold.");
#endif
			}

			if (!p_ptr->oppose_pois)
			{
#ifdef JP
				msg_print("�Ǥؤ����������줿�������롣");
#else
				msg_print("You feel less resistant to pois.");
#endif
			}
		}

		break;

	case 18:
#ifdef JP
		if (name) return "�ۥӥåȤΥ��ǥ�";
		if (desc) return "��®���롣";
#else
		if (name) return "Hobbit Melodies";
		if (desc) return "Hastes you.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�ڲ��ʲΤ�������߻Ϥ᤿������");
#else
			msg_print("You start singing joyful pop song...");
#endif
			start_singing(spell, MUSIC_SPEED);
		}

		if (stop)
		{
			if (!p_ptr->fast)
			{
#ifdef JP
				msg_print("ư�������ᤵ���ʤ��ʤä��褦����");
#else
				msg_print("You feel yourself slow down.");
#endif
			}
		}

		break;

	case 19:
#ifdef JP
		if (name) return "�Ĥ������";
		if (desc) return "�᤯�Υ�󥹥�����ƥ�ݡ��Ȥ����롣�񹳤�����̵����";
#else
		if (name) return "World Contortion";
		if (desc) return "Teleports all nearby monsters away unless resisted.";
#endif
    
		{
			int rad = plev / 15 + 1;
			int power = plev * 3 + 1;

			if (info) return info_radius(rad);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
#ifdef JP
				msg_print("�Τ����֤��Ĥ᤿������");
#else
				msg_print("Reality whirls wildly as you sing a dizzying melody...");
#endif

				project(0, rad, py, px, power, GF_AWAY_ALL, PROJECT_KILL, -1);
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "�໶�β�";
		if (desc) return "�볦������ƤΥ�󥹥����˥��᡼����Ϳ���롣�ٰ��ʥ�󥹥������ä��礭�ʥ��᡼����Ϳ���롣";
#else
		if (name) return "Dispelling chant";
		if (desc) return "Damages all monsters in sight. Hurts evil monsters greatly.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�Ѥ����ʤ��Զ��²���Ũ���դ�Ω�Ƥ�������");
#else
			msg_print("You cry out in an ear-wracking voice...");
#endif
			start_singing(spell, MUSIC_DISPEL);
		}

		{
			int m_sides = plev * 3;
			int e_sides = plev * 3;

			if (info) return format("%s1d%d+1d%d", s_dam, m_sides, e_sides);

			if (cont)
			{
				dispel_monsters(randint1(m_sides));
				dispel_evil(randint1(e_sides));
			}
		}
		break;

	case 21:
#ifdef JP
		if (name) return "����ޥ�δŸ�";
		if (desc) return "�볦������ƤΥ�󥹥�����®������̲�餻�褦�Ȥ��롣�񹳤�����̵����";
#else
		if (name) return "The Voice of Saruman";
		if (desc) return "Attempts to slow and sleep all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("ͥ������̥��Ū�ʲΤ�������߻Ϥ᤿������");
#else
			msg_print("You start humming a gentle and attractive song...");
#endif
			start_singing(spell, MUSIC_SARUMAN);
		}

		{
			int power = plev;

			if (info) return info_power(power);

			if (cont)
			{
				slow_monsters();
				sleep_monsters();
			}
		}

		break;

	case 22:
#ifdef JP
		if (name) return "��β���";
		if (desc) return "�첻�Υӡ�������ġ�";
#else
		if (name) return "Song of the Tempest";
		if (desc) return "Fires a beam of sound.";
#endif
    
		{
			int dice = 15 + (plev - 1) / 2;
			int sides = 10;

			if (info) return info_damage(dice, sides, 0);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_beam(GF_SOUND, dir, damroll(dice, sides));
			}
		}
		break;

	case 23:
#ifdef JP
		if (name) return "�⤦��Ĥ�����";
		if (desc) return "���ߤγ���ƹ������롣";
#else
		if (name) return "Ambarkanta";
		if (desc) return "Recreates current dungeon level.";
#endif
    
		{
			int base = 15;
			int sides = 20;

			if (info) return info_delay(base, sides);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
#ifdef JP
				msg_print("���Ϥ��Ѳ����Ϥ᤿������");
#else
				msg_print("You sing of the primeval shaping of Middle-earth...");
#endif

				alter_reality();
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "�˲�����Χ";
		if (desc) return "���ϤΥ��󥸥����ɤ餷���ɤȾ��������������Ѥ��롣";
#else
		if (name) return "Wrecking Pattern";
		if (desc) return "Shakes dungeon structure, and results in random swapping of floors and walls.";
#endif

		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�˲�Ū�ʲΤ������錄�ä�������");
#else
			msg_print("You weave a pattern of sounds to contort and shatter...");
#endif
			start_singing(spell, MUSIC_QUAKE);
		}

		{
			int rad = 10;

			if (info) return info_radius(rad);

			if (cont)
			{
				earthquake(py, px, 10);
			}
		}

		break;


	case 25:
#ifdef JP
		if (name) return "���ڤβ�";
		if (desc) return "�볦������ƤΥ�󥹥��������㤵���褦�Ȥ��롣�񹳤�����̵����";
#else
		if (name) return "Stationary Shriek";
		if (desc) return "Attempts to freeze all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("��ä���Ȥ������ǥ����դǻϤ᤿������");
#else
			msg_print("You weave a very slow pattern which is almost likely to stop...");
#endif
			start_singing(spell, MUSIC_STASIS);
		}

		{
			int power = plev * 4;

			if (info) return info_power(power);

			if (cont)
			{
				stasis_monsters(power);
			}
		}

		break;

	case 26:
#ifdef JP
		if (name) return "���β�";
		if (desc) return "��ʬ�Τ��뾲�ξ�ˡ���󥹥������̤�ȴ�����꾤�����줿�ꤹ�뤳�Ȥ��Ǥ��ʤ��ʤ�롼���������";
#else
		if (name) return "Endurance";
		if (desc) return "Sets a glyph on the floor beneath you. Monsters cannot attack you if you are on a glyph, but can try to break glyph.";
#endif
    
		{
			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
#ifdef JP
				msg_print("�Τ������ʾ����Ф���������");
#else
				msg_print("The holy power of the Music is creating sacred field...");
#endif

				warding_glyph();
			}
		}
		break;

	case 27:
#ifdef JP
		if (name) return "��ͺ�λ�";
		if (desc) return "��®�����ҡ�����ʬ�ˤʤꡢ�볦������ƤΥ�󥹥����˥��᡼����Ϳ���롣";
#else
		if (name) return "The Hero's Poem";
		if (desc) return "Hastes you. Gives heroism. Damages all monsters in sight.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("��ͺ�βΤ���������������");
#else
			msg_print("You chant a powerful, heroic call to arms...");
#endif
			(void)hp_player(10);
			(void)set_afraid(0);

			/* Recalculate hitpoints */
			p_ptr->update |= (PU_HP);

			start_singing(spell, MUSIC_SHERO);
		}

		if (stop)
		{
			if (!p_ptr->hero)
			{
#ifdef JP
				msg_print("�ҡ����ε�ʬ���ä���������");
#else
				msg_print("The heroism wears off.");
#endif
				/* Recalculate hitpoints */
				p_ptr->update |= (PU_HP);
			}

			if (!p_ptr->fast)
			{
#ifdef JP
				msg_print("ư�������ᤵ���ʤ��ʤä��褦����");
#else
				msg_print("You feel yourself slow down.");
#endif
			}
		}

		{
			int dice = 1;
			int sides = plev * 3;

			if (info) return info_damage(dice, sides, 0);

			if (cont)
			{
				dispel_monsters(damroll(dice, sides));
			}
		}
		break;

	case 28:
#ifdef JP
		if (name) return "�������ʤν���";
		if (desc) return "���Ϥʲ����βΤǡ������ۯ۰���֤��������롣";
#else
		if (name) return "Relief of Yavanna";
		if (desc) return "Powerful healing song. Also heals cut and stun completely.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
			msg_print("�Τ��̤����Τ˳赤����äƤ���������");
#else
			msg_print("Life flows through you as you sing the song...");
#endif
			start_singing(spell, MUSIC_H_LIFE);
		}

		{
			int dice = 15;
			int sides = 10;

			if (info) return info_heal(dice, sides, 0);

			if (cont)
			{
				hp_player(damroll(dice, sides));
				set_stun(0);
				set_cut(0);
			}
		}

		break;

	case 29:
#ifdef JP
		if (name) return "�����β�";
		if (desc) return "���٤ƤΥ��ơ������ȷи��ͤ�������롣";
#else
		if (name) return "Goddess' rebirth";
		if (desc) return "Restores all stats and experience.";
#endif
    
		{
			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
#ifdef JP
				msg_print("�Ź�����˸�������դ�ޤ������Τ����γ��Ϥ����ᤷ����");
#else
				msg_print("You strewed light and beauty in the dark as you sing. You feel refreshed.");
#endif
				(void)do_res_stat(A_STR);
				(void)do_res_stat(A_INT);
				(void)do_res_stat(A_WIS);
				(void)do_res_stat(A_DEX);
				(void)do_res_stat(A_CON);
				(void)do_res_stat(A_CHR);
				(void)restore_level();
			}
		}
		break;

	case 30:
#ifdef JP
		if (name) return "�����������";
		if (desc) return "���˶��ϤǤ����������첻�ε�����ġ�";
#else
		if (name) return "Wizardry of Sauron";
		if (desc) return "Fires an extremely powerful tiny ball of sound.";
#endif
    
		{
			int dice = 50 + plev;
			int sides = 10;
			int rad = 0;

			if (info) return info_damage(dice, sides, 0);

			/* Stop singing before start another */
			if (cast || fail) stop_singing();

			if (cast)
			{
				if (!get_aim_dir(&dir)) return NULL;

				fire_ball(GF_SOUND, dir, damroll(dice, sides), rad);
			}
		}
		break;

	case 31:
#ifdef JP
		if (name) return "�ե��󥴥�ե����ĩ��";
		if (desc) return "���᡼��������ʤ��ʤ�Хꥢ��ĥ�롣";
#else
		if (name) return "Fingolfin's Challenge";
		if (desc) return "Generates barrier which completely protect you from almost all damages. Takes a few your turns when the barrier breaks.";
#endif
    
		/* Stop singing before start another */
		if (cast || fail) stop_singing();

		if (cast)
		{
#ifdef JP
				msg_print("�ե��󥴥�ե����̽���ؤ�ĩ���Τä�������");
#else
				msg_print("You recall the valor of Fingolfin's challenge to the Dark Lord...");
#endif

				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);
		
				/* Update monsters */
				p_ptr->update |= (PU_MONSTERS);
		
				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

				start_singing(spell, MUSIC_INVULN);
		}

		if (stop)
		{
			if (!p_ptr->invuln)
			{
#ifdef JP
				msg_print("̵Ũ�ǤϤʤ��ʤä���");
#else
				msg_print("The invulnerability wears off.");
#endif
				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);

				/* Update monsters */
				p_ptr->update |= (PU_MONSTERS);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
			}
		}

		break;
	}

	return "";
}


static cptr do_hissatsu_spell(int spell, int mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
#ifdef JP
		if (name) return "���ӹ�";
		if (desc) return "2�ޥ�Υ�줿�Ȥ���ˤ����󥹥����򹶷⤹�롣";
#else
		if (name) return "Tobi-Izuna";
		if (desc) return "Attacks a two squares distant monster.";
#endif
    
		if (cast)
		{
			project_length = 2;
			if (!get_aim_dir(&dir)) return NULL;

			project_hook(GF_ATTACK, dir, HISSATSU_2, PROJECT_STOP | PROJECT_KILL);
		}
		break;

	case 1:
#ifdef JP
		if (name) return "�޷�¤�";
		if (desc) return "3�������Ф��ƹ��⤹�롣";
#else
		if (name) return "3-Way Attack";
		if (desc) return "Attacks in 3 directions in one time.";
#endif
    
		if (cast)
		{
			int cdir;
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			for (cdir = 0;cdir < 8; cdir++)
			{
				if (cdd[cdir] == dir) break;
			}

			if (cdir == 8) return NULL;

			y = py + ddy_cdd[cdir];
			x = px + ddx_cdd[cdir];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
#ifdef JP
				msg_print("����϶����ڤä���");
#else
				msg_print("You attack the empty air.");
#endif
			y = py + ddy_cdd[(cdir + 7) % 8];
			x = px + ddx_cdd[(cdir + 7) % 8];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
#ifdef JP
				msg_print("����϶����ڤä���");
#else
				msg_print("You attack the empty air.");
#endif
			y = py + ddy_cdd[(cdir + 1) % 8];
			x = px + ddx_cdd[(cdir + 1) % 8];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
#ifdef JP
				msg_print("����϶����ڤä���");
#else
				msg_print("You attack the empty air.");
#endif
		}
		break;

	case 2:
#ifdef JP
		if (name) return "�֡�����";
		if (desc) return "����긵����äƤ���褦���ꤲ�롣��äƤ��ʤ����Ȥ⤢�롣";
#else
		if (name) return "Boomerang";
		if (desc) return "Throws current weapon. And it'll return to your hand unless failed.";
#endif
    
		if (cast)
		{
			if (!do_cmd_throw_aux(1, TRUE, 0)) return NULL;
		}
		break;

	case 3:
#ifdef JP
		if (name) return "����";
		if (desc) return "�б������Τʤ���󥹥���������᡼����Ϳ���롣";
#else
		if (name) return "Burning Strike";
		if (desc) return "Attacks a monster with more damage unless it has resistance to fire.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_FIRE);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 4:
#ifdef JP
		if (name) return "��������";
		if (desc) return "�᤯�λ׹ͤ��뤳�Ȥ��Ǥ����󥹥������Τ��롣";
#else
		if (name) return "Detect Ferocity";
		if (desc) return "Detects all monsters except mindless in your vicinity.";
#endif
    
		if (cast)
		{
			detect_monsters_mind(DETECT_RAD_DEFAULT);
		}
		break;

	case 5:
#ifdef JP
		if (name) return "�ߤ��Ǥ�";
		if (desc) return "���˥��᡼����Ϳ���ʤ�����ۯ۰�Ȥ����롣";
#else
		if (name) return "Strike to Stun";
		if (desc) return "Attempts to stun a monster in the adjacent.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_MINEUCHI);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 6:
#ifdef JP
		if (name) return "�����󥿡�";
		if (desc) return "���˹��⤵�줿�Ȥ���ȿ�⤹�롣ȿ�⤹�뤿�Ӥ�MP�����";
#else
		if (name) return "Counter";
		if (desc) return "Prepares to counterattack. When attack by a monster, strikes back using SP each time.";
#endif
    
		if (cast)
		{
			if (p_ptr->riding)
			{
#ifdef JP
				msg_print("������ˤ�̵������");
#else
				msg_print("You cannot do it when riding.");
#endif
				return NULL;
			}
#ifdef JP
			msg_print("���ι�����Ф��ƿȹ�������");
#else
			msg_print("You prepare to counter blow.");
#endif
			p_ptr->counter = TRUE;
		}
		break;

	case 7:
#ifdef JP
		if (name) return "ʧ��ȴ��";
		if (desc) return "���⤷���塢ȿ��¦��ȴ���롣";
#else
		if (name) return "Harainuke";
		if (desc) return "Attacks monster with your weapons normally, then move through counter side of the monster.";
#endif
    
		if (cast)
		{
			int y, x;

			if (p_ptr->riding)
			{
#ifdef JP
				msg_print("������ˤ�̵������");
#else
				msg_print("You cannot do it when riding.");
#endif
				return NULL;
			}
	
			if (!get_rep_dir2(&dir)) return NULL;
	
			if (dir == 5) return NULL;
			y = py + ddy[dir];
			x = px + ddx[dir];
	
			if (!cave[y][x].m_idx)
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
	
			py_attack(y, x, 0);
	
			if (!player_can_enter(cave[y][x].feat, 0) || is_trap(cave[y][x].feat))
				break;
	
			y += ddy[dir];
			x += ddx[dir];
	
			if (player_can_enter(cave[y][x].feat, 0) && !is_trap(cave[y][x].feat) && !cave[y][x].m_idx)
			{
				msg_print(NULL);
	
				/* Move the player */
				(void)move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
			}
		}
		break;

	case 8:
#ifdef JP
		if (name) return "�����ڥ�ĥ���";
		if (desc) return "�������Τʤ���󥹥���������᡼����Ϳ���롣";
#else
		if (name) return "Serpent's Tongue";
		if (desc) return "Attacks a monster with more damage unless it has resistance to poison.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_POISON);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 9:
#ifdef JP
		if (name) return "�������������";
		if (desc) return "��̿�Τʤ��ٰ��ʥ�󥹥���������᡼����Ϳ���뤬��¾�Υ�󥹥����ˤ��������̤��ʤ���";
#else
		if (name) return "Zammaken";
		if (desc) return "Attacks an evil unliving monster with great damage. No effect to other  monsters.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_ZANMA);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 10:
#ifdef JP
		if (name) return "������";
		if (desc) return "���⤷����������ؿ᤭���Ф���";
#else
		if (name) return "Wind Blast";
		if (desc) return "Attacks an adjacent monster, and blow it away.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
			if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
			{
				return "";
			}
			if (cave[y][x].m_idx)
			{
				int i;
				int ty = y, tx = x;
				int oy = y, ox = x;
				int m_idx = cave[y][x].m_idx;
				monster_type *m_ptr = &m_list[m_idx];
				char m_name[80];
	
				monster_desc(m_name, m_ptr, 0);
	
				for (i = 0; i < 5; i++)
				{
					y += ddy[dir];
					x += ddx[dir];
					if (cave_empty_bold(y, x))
					{
						ty = y;
						tx = x;
					}
					else break;
				}
				if ((ty != oy) || (tx != ox))
				{
#ifdef JP
					msg_format("%s��᤭���Ф�����", m_name);
#else
					msg_format("You blow %s away!", m_name);
#endif
					cave[oy][ox].m_idx = 0;
					cave[ty][tx].m_idx = m_idx;
					m_ptr->fy = ty;
					m_ptr->fx = tx;
	
					update_mon(m_idx, TRUE);
					lite_spot(oy, ox);
					lite_spot(ty, tx);
	
					if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
						p_ptr->update |= (PU_MON_LITE);
				}
			}
		}
		break;

	case 11:
#ifdef JP
		if (name) return "�ᾢ��������";
		if (desc) return "���ɶ��1�ļ��̤��롣��٥�45�ʾ�����ɶ��ǽ�Ϥ������Τ뤳�Ȥ��Ǥ��롣";
#else
		if (name) return "Judge";
		if (desc) return "Identifies a weapon or armor. Or *identifies* these at level 45.";
#endif
    
		if (cast)
		{
			if (plev > 44)
			{
				if (!identify_fully(TRUE)) return NULL;
			}
			else
			{
				if (!ident_spell(TRUE)) return NULL;
			}
		}
		break;

	case 12:
#ifdef JP
		if (name) return "�˴��";
		if (desc) return "�����������зϤΥ�󥹥���������᡼����Ϳ���롣";
#else
		if (name) return "Rock Smash";
		if (desc) return "Breaks rock. Or greatly damage a monster made by rocks.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_HAGAN);
	
			if (!cave_have_flag_bold(y, x, FF_HURT_ROCK)) break;
	
			/* Destroy the feature */
			cave_alter_feat(y, x, FF_HURT_ROCK);
	
			/* Update some things */
			p_ptr->update |= (PU_FLOW);
		}
		break;

	case 13:
#ifdef JP
		if (name) return "�������";
		if (desc) return "���������������䵤�����Τʤ���󥹥���������᡼����Ϳ���롣";
#else
		if (name) return "Midare-Setsugekka";
		if (desc) return "Attacks a monster with increased number of attacks and more damage unless it has resistance to cold.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_COLD);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 14:
#ifdef JP
		if (name) return "�޽��ͤ�";
		if (desc) return "��󥹥���������ݤ�����򷫤�Ф������Ԥ����1���������᡼����Ϳ�����ʤ���";
#else
		if (name) return "Spot Aiming";
		if (desc) return "Attempts to kill a monster instantly. If failed cause only 1HP of damage.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_KYUSHO);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 15:
#ifdef JP
		if (name) return "����¤�";
		if (desc) return "�񿴤ΰ��ǹ��⤹�롣���⤬���蘆��䤹����";
#else
		if (name) return "Majingiri";
		if (desc) return "Attempts to attack with critical hit. But this attack is easy to evade for a monster.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_MAJIN);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 16:
#ifdef JP
		if (name) return "�Τƿ�";
		if (desc) return "���Ϥʹ���򷫤�Ф������Υ�����ޤǤδ֡����餦���᡼���������롣";
#else
		if (name) return "Desperate Attack";
		if (desc) return "Attacks with all of your power. But all damages you take will be doubled for one turn.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_SUTEMI);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
			p_ptr->sutemi = TRUE;
		}
		break;

	case 17:
#ifdef JP
		if (name) return "������޻�";
		if (desc) return "�ŷ������Τʤ���󥹥����������礭�����᡼����Ϳ���롣";
#else
		if (name) return "Lightning Eagle";
		if (desc) return "Attacks a monster with more damage unless it has resistance to electricity.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_ELEC);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 18:
#ifdef JP
		if (name) return "����";
		if (desc) return "���᤯���˶��깶�⤹�롣";
#else
		if (name) return "Rush Attack";
		if (desc) return "Steps close to a monster and attacks at a time.";
#endif
    
		if (cast)
		{
			if (!rush_attack(NULL)) return NULL;
		}
		break;

	case 19:
#ifdef JP
		if (name) return "��ή��";
		if (desc) return "��ʬ���Ȥ������Ĥġ����ν��������ۤ��礭�����Ϥ���������Ũ�򹶷�Ǥ��롣�����Ƥ��ʤ���󥹥����ˤϸ��̤��ʤ���";
#else
		if (name) return "Bloody Maelstrom";
		if (desc) return "Attacks all adjacent monsters with power corresponding to your cut status. Then increases your cut status. No effect to unliving monsters.";
#endif
    
		if (cast)
		{
			int y = 0, x = 0;

			cave_type       *c_ptr;
			monster_type    *m_ptr;
	
			if (p_ptr->cut < 300)
				set_cut(p_ptr->cut + 300);
			else
				set_cut(p_ptr->cut * 2);
	
			for (dir = 0; dir < 8; dir++)
			{
				y = py + ddy_ddd[dir];
				x = px + ddx_ddd[dir];
				c_ptr = &cave[y][x];
	
				/* Get the monster */
				m_ptr = &m_list[c_ptr->m_idx];
	
				/* Hack -- attack monsters */
				if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
				{
					if (!monster_living(&r_info[m_ptr->r_idx]))
					{
						char m_name[80];
	
						monster_desc(m_name, m_ptr, 0);
#ifdef JP
						msg_format("%s�ˤϸ��̤��ʤ���", m_name);
#else
						msg_format("%s is unharmed!", m_name);
#endif
					}
					else py_attack(y, x, HISSATSU_SEKIRYUKA);
				}
			}
		}
		break;

	case 20:
#ifdef JP
		if (name) return "��̷�";
		if (desc) return "�Ͽ̤򵯤�����";
#else
		if (name) return "Earthquake Blow";
		if (desc) return "Shakes dungeon structure, and results in random swapping of floors and walls.";
#endif
    
		if (cast)
		{
			int y,x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_QUAKE);
			else
				earthquake(py, px, 10);
		}
		break;

	case 21:
#ifdef JP
		if (name) return "������";
		if (desc) return "�׷��ȤΥӡ�������ġ�";
#else
		if (name) return "Crack";
		if (desc) return "Fires a beam of shock wave.";
#endif
    
		if (cast)
		{
			int total_damage = 0, basedam, i;
			u32b flgs[TR_FLAG_SIZE];
			object_type *o_ptr;
			if (!get_aim_dir(&dir)) return NULL;
#ifdef JP
			msg_print("�����礭�����겼������");
#else
			msg_print("You swing your weapon downward.");
#endif
			for (i = 0; i < 2; i++)
			{
				int damage;
	
				if (!buki_motteruka(INVEN_RARM+i)) break;
				o_ptr = &inventory[INVEN_RARM+i];
				basedam = (o_ptr->dd * (o_ptr->ds + 1)) * 50;
				damage = o_ptr->to_d * 100;
				object_flags(o_ptr, flgs);
				if ((o_ptr->name1 == ART_VORPAL_BLADE) || (o_ptr->name1 == ART_CHAINSWORD))
				{
					/* vorpal blade */
					basedam *= 5;
					basedam /= 3;
				}
				else if (have_flag(flgs, TR_VORPAL))
				{
					/* vorpal flag only */
					basedam *= 11;
					basedam /= 9;
				}
				damage += basedam;
				damage *= p_ptr->num_blow[i];
				total_damage += damage / 200;
				if (i) total_damage = total_damage*7/10;
			}
			fire_beam(GF_FORCE, dir, total_damage);
		}
		break;

	case 22:
#ifdef JP
		if (name) return "������ͺ����";
		if (desc) return "�볦�������󥹥������Ф��ƹ첻�ι����Ԥ�������ˡ��᤯�ˤ����󥹥������ܤ餻�롣";
#else
		if (name) return "War Cry";
		if (desc) return "Damages all monsters in sight with sound. Aggravate nearby monsters.";
#endif
    
		if (cast)
		{
#ifdef JP
			msg_print("ͺ���Ӥ򤢤�����");
#else
			msg_print("You roar out!");
#endif
			project_hack(GF_SOUND, randint1(plev * 3));
			aggravate_monsters(0);
		}
		break;

	case 23:
#ifdef JP
		if (name) return "̵�л���";
		if (desc) return "���Ϥ�3�ʹ���򷫤�Ф���";
#else
		if (name) return "Musou-Sandan";
		if (desc) return "Attacks with powerful 3 strikes.";
#endif
    
		if (cast)
		{
			int i;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			for (i = 0; i < 3; i++)
			{
				int y, x;
				int ny, nx;
				int m_idx;
				cave_type *c_ptr;
				monster_type *m_ptr;
	
				y = py + ddy[dir];
				x = px + ddx[dir];
				c_ptr = &cave[y][x];
	
				if (c_ptr->m_idx)
					py_attack(y, x, HISSATSU_3DAN);
				else
				{
#ifdef JP
					msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
					msg_print("There is no monster.");
#endif
					return NULL;
				}
	
				if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
				{
					return "";
				}
	
				/* Monster is dead? */
				if (!c_ptr->m_idx) break;
	
				ny = y + ddy[dir];
				nx = x + ddx[dir];
				m_idx = c_ptr->m_idx;
				m_ptr = &m_list[m_idx];
	
				/* Monster cannot move back? */
				if (!monster_can_enter(ny, nx, &r_info[m_ptr->r_idx], 0))
				{
					/* -more- */
					if (i < 2) msg_print(NULL);
					continue;
				}
	
				c_ptr->m_idx = 0;
				cave[ny][nx].m_idx = m_idx;
				m_ptr->fy = ny;
				m_ptr->fx = nx;
	
				update_mon(m_idx, TRUE);
	
				/* Redraw the old spot */
				lite_spot(y, x);
	
				/* Redraw the new spot */
				lite_spot(ny, nx);
	
				/* Player can move forward? */
				if (player_can_enter(c_ptr->feat, 0))
				{
					/* Move the player */
					if (!move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP)) break;
				}
				else
				{
					break;
				}

				/* -more- */
				if (i < 2) msg_print(NULL);
			}
		}
		break;

	case 24:
#ifdef JP
		if (name) return "�۷쵴�β�";
		if (desc) return "���⤷���������Ϥ�ۤ��Ȥꡢ��ʬ�����Ϥ���������롣��̿������ʤ���󥹥����ˤ��̤��ʤ���";
#else
		if (name) return "Vampire's Fang";
		if (desc) return "Attacks with vampiric strikes which absorbs HP from a monster and gives them to you. No effect to unliving monsters.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_DRAIN);
			else
			{
#ifdef JP
					msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
					msg_print("There is no monster.");
#endif
				return NULL;
			}
		}
		break;

	case 25:
#ifdef JP
		if (name) return "����";
		if (desc) return "�볦��ε����Ƥ�������󥹥�����ۯ۰������̲���Ϳ���褦�Ȥ��롣";
#else
		if (name) return "Moon Dazzling";
		if (desc) return "Attempts to stun, confuse and sleep all waking monsters.";
#endif
    
		if (cast)
		{
#ifdef JP
			msg_print("�����Ե�§���ɤ餷��������");
#else
			msg_print("You irregularly wave your weapon...");
#endif
			project_hack(GF_ENGETSU, plev * 4);
			project_hack(GF_ENGETSU, plev * 4);
			project_hack(GF_ENGETSU, plev * 4);
		}
		break;

	case 26:
#ifdef JP
		if (name) return "ɴ�ͻ¤�";
		if (desc) return "Ϣ³�������Ȥǥ�󥹥����򹶷⤹�롣���⤹�뤿�Ӥ�MP�����MP���ʤ��ʤ뤫����󥹥������ݤ��ʤ��ä���ɴ�ͻ¤�Ͻ�λ���롣";
#else
		if (name) return "Hundred Slaughter";
		if (desc) return "Performs a series of rush attacks. The series continues while killing each monster in a time and SP remains.";
#endif
    
		if (cast)
		{
			const int mana_cost_per_monster = 8;
			bool new = TRUE;
			bool mdeath;

			do
			{
				if (!rush_attack(&mdeath)) break;
				if (new)
				{
					/* Reserve needed mana point */
					p_ptr->csp -= technic_info[REALM_HISSATSU - MIN_TECHNIC][26].smana;
					new = FALSE;
				}
				else
					p_ptr->csp -= mana_cost_per_monster;

				if (!mdeath) break;
				command_dir = 0;

				p_ptr->redraw |= PR_MANA;
				handle_stuff();
			}
			while (p_ptr->csp > mana_cost_per_monster);

			if (new) return NULL;
	
			/* Restore reserved mana */
			p_ptr->csp += technic_info[REALM_HISSATSU - MIN_TECHNIC][26].smana;
		}
		break;

	case 27:
#ifdef JP
		if (name) return "ŷ��ζ��";
		if (desc) return "�볦��ξ�����ꤷ�ơ����ξ��ȼ�ʬ�δ֤ˤ�������󥹥����򹶷⤷�����ξ��˰�ư���롣";
#else
		if (name) return "Dragonic Flash";
		if (desc) return "Runs toward given location while attacking all monsters on the path.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!tgt_pt(&x, &y)) return NULL;

			if (!cave_player_teleportable_bold(y, x, 0L) ||
			    (distance(y, x, py, px) > MAX_SIGHT / 2) ||
			    !projectable(py, px, y, x))
			{
#ifdef JP
				msg_print("���ԡ�");
#else
				msg_print("You cannot move to that place!");
#endif
				break;
			}
			if (p_ptr->anti_tele)
			{
#ifdef JP
				msg_print("�Ի׵Ĥ��Ϥ��ƥ�ݡ��Ȥ��ɤ�����");
#else
				msg_print("A mysterious force prevents you from teleporting!");
#endif
	
				break;
			}
			project(0, 0, y, x, HISSATSU_ISSEN, GF_ATTACK, PROJECT_BEAM | PROJECT_KILL, -1);
			teleport_player_to(y, x, 0L);
		}
		break;

	case 28:
#ifdef JP
		if (name) return "��Ťη���";
		if (desc) return "1�������2�ٹ����Ԥ���";
#else
		if (name) return "Twin Slash";
		if (desc) return "double attacks at a time.";
#endif
    
		if (cast)
		{
			int x, y;
	
			if (!get_rep_dir(&dir, FALSE)) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
			{
				py_attack(y, x, 0);
				if (cave[y][x].m_idx)
				{
					handle_stuff();
					py_attack(y, x, 0);
				}
			}
			else
			{
#ifdef JP
	msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("You don't see any monster in this direction");
#endif
				return NULL;
			}
		}
		break;

	case 29:
#ifdef JP
		if (name) return "����������";
		if (desc) return "���Ϥʹ����Ԥ����᤯�ξ��ˤ���̤��ڤ֡�";
#else
		if (name) return "Kofuku-Zettousei";
		if (desc) return "Performs a powerful attack which even effect nearby monsters.";
#endif
    
		if (cast)
		{
			int total_damage = 0, basedam, i;
			int y, x;
			u32b flgs[TR_FLAG_SIZE];
			object_type *o_ptr;
	
			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
			{
#ifdef JP
				msg_print("�ʤ������⤹�뤳�Ȥ��Ǥ��ʤ���");
#else
				msg_print("Something prevent you from attacking.");
#endif
				return "";
			}
#ifdef JP
			msg_print("�����礭�����겼������");
#else
			msg_print("You swing your weapon downward.");
#endif
			for (i = 0; i < 2; i++)
			{
				int damage;
				if (!buki_motteruka(INVEN_RARM+i)) break;
				o_ptr = &inventory[INVEN_RARM+i];
				basedam = (o_ptr->dd * (o_ptr->ds + 1)) * 50;
				damage = o_ptr->to_d * 100;
				object_flags(o_ptr, flgs);
				if ((o_ptr->name1 == ART_VORPAL_BLADE) || (o_ptr->name1 == ART_CHAINSWORD))
				{
					/* vorpal blade */
					basedam *= 5;
					basedam /= 3;
				}
				else if (have_flag(flgs, TR_VORPAL))
				{
					/* vorpal flag only */
					basedam *= 11;
					basedam /= 9;
				}
				damage += basedam;
				damage += p_ptr->to_d[i] * 100;
				damage *= p_ptr->num_blow[i];
				total_damage += (damage / 100);
			}
			project(0, (cave_have_flag_bold(y, x, FF_PROJECT) ? 5 : 0), y, x, total_damage * 3 / 2, GF_METEOR, PROJECT_KILL | PROJECT_JUMP | PROJECT_ITEM, -1);
		}
		break;

	case 30:
#ifdef JP
		if (name) return "�ı���Ǧ��";
		if (desc) return "��ʬ����᡼���򤯤餦�������������礭�ʥ��᡼����Ϳ���롣����ǥåɤˤ��ä˸��̤����롣";
#else
		if (name) return "Keiun-Kininken";
		if (desc) return "Attacks a monster with extremely powerful damage. But you also takes some damages. Hurts a undead monster greatly.";
#endif
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = py + ddy[dir];
			x = px + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_UNDEAD);
			else
			{
#ifdef JP
				msg_print("���������ˤϥ�󥹥����Ϥ��ޤ���");
#else
				msg_print("There is no monster.");
#endif
				return NULL;
			}
#ifdef JP
			take_hit(DAMAGE_NOESCAPE, 100 + randint1(100), "�ı���Ǧ����Ȥä��׷�", -1);
#else
			take_hit(DAMAGE_NOESCAPE, 100 + randint1(100), "exhaustion on using Keiun-Kininken", -1);
#endif
		}
		break;

	case 31:
#ifdef JP
		if (name) return "��ʢ";
		if (desc) return "�����ƻ�Ȥϡ���̤��Ȥȸ��Ĥ����ꡣ��";
#else
		if (name) return "Harakiri";
		if (desc) return "'Busido is found in death'";
#endif

		if (cast)
		{
			int i;
#ifdef JP
	if (!get_check("�����˼������ޤ�����")) return NULL;
#else
			if (!get_check("Do you really want to commit suicide? ")) return NULL;
#endif
				/* Special Verification for suicide */
#ifdef JP
	prt("��ǧ�Τ��� '@' �򲡤��Ʋ�������", 0, 0);
#else
			prt("Please verify SUICIDE by typing the '@' sign: ", 0, 0);
#endif
	
			flush();
			i = inkey();
			prt("", 0, 0);
			if (i != '@') return NULL;
			if (p_ptr->total_winner)
			{
				take_hit(DAMAGE_FORCE, 9999, "Seppuku", -1);
				p_ptr->total_winner = TRUE;
			}
			else
			{
#ifdef JP
				msg_print("���ƻ�Ȥϡ���̤��Ȥȸ��Ĥ����ꡣ");
#else
				msg_print("Meaning of Bushi-do is found in the death.");
#endif
				take_hit(DAMAGE_FORCE, 9999, "Seppuku", -1);
			}
		}
		break;
	}

	return "";
}


/*
 * Do everything for each spell
 */
cptr do_spell(int realm, int spell, int mode)
{
	switch (realm)
	{
	case REALM_LIFE:     return do_life_spell(spell, mode);
	case REALM_SORCERY:  return do_sorcery_spell(spell, mode);
	case REALM_NATURE:   return do_nature_spell(spell, mode);
	case REALM_CHAOS:    return do_chaos_spell(spell, mode);
	case REALM_DEATH:    return do_death_spell(spell, mode);
	case REALM_TRUMP:    return do_trump_spell(spell, mode);
	case REALM_ARCANE:   return do_arcane_spell(spell, mode);
	case REALM_CRAFT:    return do_craft_spell(spell, mode);
	case REALM_DAEMON:   return do_daemon_spell(spell, mode);
	case REALM_CRUSADE:  return do_crusade_spell(spell, mode);
	case REALM_MUSIC:    return do_music_spell(spell, mode);
	case REALM_HISSATSU: return do_hissatsu_spell(spell, mode);
	}

	return NULL;
}
