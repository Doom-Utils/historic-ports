// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=4:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright (C) 2000 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:
//	Implements generalized linedef type handlers
//	Floors, Ceilings, Doors, Locked Doors, Lifts, Stairs, Crushers
//
// This whole module was taken from the Boom engine by TeamTNT.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "doomstat.h"
#include "z_zone.h"
#include "p_local.h"
#include "sounds.h"
#include "s_sound.h"
#include "m_random.h"

//
// EV_DoGenFloor()
//
// Handle generalized floor types
//
// Passed the line activating the generalized floor function
// Returns true if a thinker is created
//
//
int EV_DoGenFloor(line_t *line)
{
	int				secnum;
	int				rtn;
	boolean			manual;
	sector_t		*sec;
	floormove_t		*floor;
	unsigned int	value;
	int				Crsh, ChgT, Targ, Dirn, ChgM, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned)line->special - GenFloorBase;
	Crsh = (value & FloorCrush) >> FloorCrushShift;
	ChgT = (value & FloorChange) >> FloorChangeShift;
	Targ = (value & FloorTarget) >> FloorTargetShift;
	Dirn = (value & FloorDirection) >> FloorDirectionShift;
	ChgM = (value & FloorModel) >> FloorModelShift;
	Sped = (value & FloorSpeed) >> FloorSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	rtn = 0;

	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_floor;
	}

	secnum = -1;
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_floor:                
		// Do not start another function if floor already moving
		if (P_SectorActive(floor_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}

		// new floor thinker
		rtn = 1;
		floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, (void *)0);
		P_AddThinker(&floor->thinker);
		sec->floordata = floor;
		floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
		floor->crush = Crsh;
		floor->direction = Dirn ? 1 : -1;
		floor->sector = sec;
		floor->texture = sec->floorpic;
		floor->newspecial = sec->special;
		floor->oldspecial = sec->oldspecial;
		floor->type = genFloor;

		// set the speed of motion
		switch (Sped)
		{
			case SpeedSlow:
				floor->speed = FLOORSPEED;
				break;

			case SpeedNormal:
				floor->speed = FLOORSPEED * 2;
				break;

			case SpeedFast:
				floor->speed = FLOORSPEED * 4;
				break;

			case SpeedTurbo:
				floor->speed = FLOORSPEED * 8;
				break;

			default:
				break;
		}

		// set the destination height
		switch(Targ)
		{
			case FtoHnF:
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				break;

			case FtoLnF:
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				break;

			case FtoNnF:
				floor->floordestheight = Dirn ?
				P_FindNextHighestFloor(sec, sec->floorheight) :
				P_FindNextLowestFloor(sec,sec->floorheight);
				break;

			case FtoLnC:
				floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
				break;

			case FtoC:
				floor->floordestheight = sec->ceilingheight;
				break;

			case FbyST:
				floor->floordestheight = (floor->sector->floorheight>>FRACBITS)
										 + floor->direction *
										 (P_FindShortestTextureAround(secnum)
										 >> FRACBITS);
				if (floor->floordestheight > 32000)// prevent overflow
					floor->floordestheight = 32000;// wraparound in floor height
				if (floor->floordestheight < -32000)
					floor->floordestheight=-32000;
				floor->floordestheight <<= FRACBITS;
				break;

			case Fby24:
				floor->floordestheight = floor->sector->floorheight +
										 floor->direction * 24 * FRACUNIT;
				break;

			case Fby32:
				floor->floordestheight = floor->sector->floorheight +
										 floor->direction * 32 * FRACUNIT;
				break;

			default:
				break;
		}

		// set texture/type change properties
		if (ChgT)	// if a texture change is indicated
		{
			if (ChgM)	// if a numeric model change
			{
				sector_t *sec;

				// find model with ceiling at target height if target
				// is a ceiling type
				sec = (Targ == FtoLnC || Targ == FtoC) ?
					  P_FindModelCeilingSector(floor->floordestheight, secnum) :
					  P_FindModelFloorSector(floor->floordestheight, secnum);
				if (sec)
				{
					floor->texture = sec->floorpic;
					switch(ChgT)
					{
						case FChgZero:	// zero type
							floor->newspecial = 0;
							floor->oldspecial = 0;
							floor->type = genFloorChg0;
							break;

						case FChgTyp:	// copy type
							floor->newspecial = sec->special;
							floor->oldspecial = sec->oldspecial;
							floor->type = genFloorChgT;
							break;

						case FChgTxt:	// leave type be
							floor->type = genFloorChg;
							break;

						default:
							break;
					}
				}
			}
			else	// else if a trigger model change
			{
				floor->texture = line->frontsector->floorpic;
				switch (ChgT)
				{
					case FChgZero:	// zero type
						floor->newspecial = 0;
						floor->oldspecial = 0;
						floor->type = genFloorChg0;
						break;

					case FChgTyp:	// copy type
						floor->newspecial = line->frontsector->special;
						floor->oldspecial = line->frontsector->oldspecial;
						floor->type = genFloorChgT;
						break;

					case FChgTxt:	// leave type be
						floor->type = genFloorChg;
						break;

					default:
						break;
				}
			}
		}

		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenCeiling()
//
// Handle generalized ceiling types
//
// Passed the linedef activating the ceiling function
// Returns true if a thinker created
//
int EV_DoGenCeiling(line_t *line)
{
	int				secnum;
	int				rtn;
	boolean			manual;
	fixed_t			targheight;
	sector_t		*sec;
	ceiling_t		*ceiling;
	unsigned int	value;
	int				Crsh, ChgT, Targ, Dirn, ChgM, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned int)line->special - GenCeilingBase;
	Crsh = (value & CeilingCrush) >> CeilingCrushShift;
	ChgT = (value & CeilingChange) >> CeilingChangeShift;
	Targ = (value & CeilingTarget) >> CeilingTargetShift;
	Dirn = (value & CeilingDirection) >> CeilingDirectionShift;
	ChgM = (value & CeilingModel) >> CeilingModelShift;
	Sped = (value & CeilingSpeed) >> CeilingSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	rtn = 0;

	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_ceiling;
	}

	secnum = -1;

	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_ceiling:                
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}

		// new ceiling thinker
		rtn = 1;
		ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, (void *)0);
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling;
		ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
		ceiling->crush = Crsh;
		ceiling->direction = Dirn ? 1 : -1;
		ceiling->sector = sec;
		ceiling->texture = sec->ceilingpic;
		ceiling->newspecial = sec->special;
		ceiling->oldspecial = sec->oldspecial;
		ceiling->tag = sec->tag;
		ceiling->type = genCeiling;

		// set speed of motion
		switch (Sped)
		{
			case SpeedSlow:
				ceiling->speed = CEILSPEED;
				break;

			case SpeedNormal:
				ceiling->speed = CEILSPEED * 2;
				break;

			case SpeedFast:
				ceiling->speed = CEILSPEED * 4;
				break;

			case SpeedTurbo:
				ceiling->speed = CEILSPEED * 8;
				break;

			default:
				break;
		}

		// set destination target height
		targheight = sec->ceilingheight;
		switch(Targ)
		{
			case CtoHnC:
				targheight = P_FindHighestCeilingSurrounding(sec);
				break;

			case CtoLnC:
				targheight = P_FindLowestCeilingSurrounding(sec);
				break;

			case CtoNnC:
				targheight = Dirn ?
							 P_FindNextHighestCeiling(sec, sec->ceilingheight) :
							 P_FindNextLowestCeiling(sec, sec->ceilingheight);
				break;

			case CtoHnF:
				targheight = P_FindHighestFloorSurrounding(sec);
				break;

			case CtoF:
				targheight = sec->floorheight;
				break;

			case CbyST:
				targheight = (ceiling->sector->ceilingheight >> FRACBITS) +
							  ceiling->direction *
							  (P_FindShortestUpperAround(secnum) >> FRACBITS);
				if (targheight > 32000)	// prevent overflow
					targheight = 32000;	// wraparound in ceiling height
				if (targheight < -32000)
					targheight = -32000;
				targheight <<= FRACBITS;
				break;

			case Cby24:
				targheight = ceiling->sector->ceilingheight +
							 ceiling->direction * 24*FRACUNIT;
				break;

			case Cby32:
				targheight = ceiling->sector->ceilingheight +
  							 ceiling->direction * 32 * FRACUNIT;
				break;

			default:
				break;
		}

		if (Dirn)
			 ceiling->topheight = targheight;
		else
			 ceiling->bottomheight = targheight;

		// set texture/type change properties
		if (ChgT)		// if a texture change is indicated
		{
			if (ChgM)	// if a numeric model change
			{
				sector_t *sec;

				// find model with floor at target height if target
				// is a floor type
				sec = (Targ == CtoHnF || Targ == CtoF) ?         
					  P_FindModelFloorSector(targheight, secnum) :
					  P_FindModelCeilingSector(targheight, secnum);
				if (sec)
				{
					ceiling->texture = sec->ceilingpic;
					switch (ChgT)
					{
						case CChgZero:	// type is zeroed
							ceiling->newspecial = 0;
							ceiling->oldspecial = 0;
							ceiling->type = genCeilingChg0;
							break;

						case CChgTyp:	// type is copied
							ceiling->newspecial = sec->special;
							ceiling->oldspecial = sec->oldspecial;
							ceiling->type = genCeilingChgT;
							break;

						case CChgTxt:	// type is left alone
							ceiling->type = genCeilingChg;
							break;

						default:
							break;
					}
				}
			}
			else	// else if a trigger model change
			{
				ceiling->texture = line->frontsector->ceilingpic;
				switch (ChgT)
				{
					case CChgZero:	// type is zeroed
						ceiling->newspecial = 0;
						ceiling->oldspecial = 0;
						ceiling->type = genCeilingChg0;
						break;

					case CChgTyp:	// type is copied
						ceiling->newspecial = line->frontsector->special;
						ceiling->oldspecial = line->frontsector->oldspecial;
						ceiling->type = genCeilingChgT;
						break;

					case CChgTxt:	// type is left alone
						ceiling->type = genCeilingChg;
						break;

					default:
						break;
				}
			}
		}

		P_AddActiveCeiling(ceiling);  // add this ceiling to the active list
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenLift()
//
// Handle generalized lift types
//
// Passed the linedef activating the lift
// Returns true if a thinker is created
//
int EV_DoGenLift(line_t *line)
{
	plat_t			*plat;
	int				secnum;
	int				rtn;
	boolean			manual;
	sector_t		*sec;
	unsigned int	value;
	int				Targ, Dely, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned int)line->special - GenLiftBase;
	Targ = (value & LiftTarget) >> LiftTargetShift;
	Dely = (value & LiftDelay) >> LiftDelayShift;
	Sped = (value & LiftSpeed) >> LiftSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	secnum = -1;
	rtn = 0;

	// Activate all <type> plats that are in_stasis
	if (Targ==LnF2HnF)
		P_ActivateInStasis(line->tag);
        
	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_lift;
	}

	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_lift:
		// Do not start another function if floor already moving
		if (P_SectorActive(floor_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
      
		// Setup the plat thinker
		rtn = 1;
		plat = Z_Malloc(sizeof(*plat), PU_LEVSPEC, (void *)0);
		P_AddThinker(&plat->thinker);
              
		plat->sector = sec;
		plat->sector->floordata = plat;
		plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
		plat->crush = false;
		plat->tag = line->tag;

		plat->type = genLift;
		plat->high = sec->floorheight;
		plat->status = down;

		// setup the target destination height
		switch(Targ)
		{
			case F2LnF:
				plat->low = P_FindLowestFloorSurrounding(sec);
				if (plat->low > sec->floorheight)
  					plat->low = sec->floorheight;
				break;

			case F2NnF:
				plat->low = P_FindNextLowestFloor(sec, sec->floorheight);
				break;

			case F2LnC:
				plat->low = P_FindLowestCeilingSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				break;

			case LnF2HnF:
				plat->type = genPerpetual;
				plat->low = P_FindLowestFloorSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->status = P_Random() & 1;
				break;

			default:
				break;
		}

		// setup the speed of motion
		switch(Sped)
		{
			case SpeedSlow:
				plat->speed = PLATSPEED * 2;
				break;

			case SpeedNormal:
				plat->speed = PLATSPEED * 4;
				break;

			case SpeedFast:
				plat->speed = PLATSPEED * 8;
				break;

			case SpeedTurbo:
				plat->speed = PLATSPEED * 16;
				break;

			default:
				break;
		}

		// setup the delay time before the floor returns
		switch(Dely)
		{
			case 0:
				plat->wait = 1 * 35;
				break;

			case 1:
				plat->wait = PLATWAIT * 35;
				break;

			case 2:
				plat->wait = 5 * 35;
				break;

			case 3:
				plat->wait = 10 * 35;
				break;
		}

		S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);
		P_AddActivePlat(plat); // add this plat to the list of active plats

		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenStairs()
//
// Handle generalized stair building
//
// Passed the linedef activating the stairs
// Returns true if a thinker is created
//
int EV_DoGenStairs(line_t *line)
{
	int				secnum;
	int				osecnum;
	int				height;
	int				i;
	int				newsecnum;
	int				texture;
	int				ok;
	int				rtn;
	boolean			manual;
    
	sector_t		*sec;
	sector_t		*tsec;

	floormove_t		*floor;
    
	fixed_t			stairsize;
	fixed_t			speed;

	unsigned int	value;
	int				Igno, Dirn, Step, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned int)line->special - GenStairsBase;
	Igno = (value & StairIgnore) >> StairIgnoreShift;
	Dirn = (value & StairDirection) >> StairDirectionShift;
	Step = (value & StairStep) >> StairStepShift;
	Sped = (value & StairSpeed) >> StairSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	rtn = 0;

	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_stair;
	}

	secnum = -1;

	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_stair:          
		// Do not start another function if floor already moving
		if (P_SectorActive(floor_special, sec) || sec->stairlock)
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
      
		// new floor thinker
		rtn = 1;
		floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, (void *)0);
		P_AddThinker(&floor->thinker);
		sec->floordata = floor;
		floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
		floor->direction = Dirn ? 1 : -1;
		floor->sector = sec;

		// setup speed of stair building
		switch(Sped)
		{
			default:
			case SpeedSlow:
				floor->speed = FLOORSPEED / 4;
				break;

			case SpeedNormal:
				floor->speed = FLOORSPEED / 2;
				break;

			case SpeedFast:
				floor->speed = FLOORSPEED * 2;
				break;

			case SpeedTurbo:
				floor->speed = FLOORSPEED * 4;
				break;
		}

		// setup stepsize for stairs
		switch(Step)
		{
			default:
			case 0:
				stairsize = 4 * FRACUNIT;
				break;

			case 1:
				stairsize = 8 * FRACUNIT;
				break;

			case 2:
				stairsize = 16 * FRACUNIT;
				break;

			case 3:
				stairsize = 24 * FRACUNIT;
				break;
		}

		speed = floor->speed;
		height = sec->floorheight + floor->direction * stairsize;
		floor->floordestheight = height;
		texture = sec->floorpic;
		floor->crush = false;
		floor->type = genBuildStair;

		sec->stairlock = -2;         // set up lock on current sector
		sec->nextsec = -1;
		sec->prevsec = -1;

		osecnum = secnum;            // preserve loop index  

		// Find next sector to raise
		// 1.     Find 2-sided line with same sector side[0]
		// 2.     Other side is the next sector to raise
		do
		{
			ok = 0;
			for (i = 0; i < sec->linecount; i++)
			{
				if (!((sec->lines[i])->backsector))
  					continue;
                                  
				tsec = (sec->lines[i])->frontsector;
				newsecnum = tsec-sectors;
          
				if (secnum != newsecnum)
  					continue;

				tsec = (sec->lines[i])->backsector;
				newsecnum = tsec - sectors;

				if (!Igno && tsec->floorpic != texture)
					continue;

				if (democompat)
					height += floor->direction * stairsize;

				if (P_SectorActive(floor_special, tsec) || tsec->stairlock)
					continue;
        
				if (!democompat)
					height += floor->direction * stairsize;

				// link the stair chain in both directions
				// lock the stair sector until building complete
				sec->nextsec = newsecnum; // link step to next
				tsec->prevsec = secnum;   // link next back
				tsec->nextsec = -1;       // set next forward link as end
				tsec->stairlock = -2;     // lock the step

				sec = tsec;
				secnum = newsecnum;
				floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);

				P_AddThinker (&floor->thinker);

				sec->floordata = floor;
				floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
				floor->direction = Dirn ? 1 : -1;
				floor->sector = sec;
				floor->speed = speed;
				floor->floordestheight = height;
				floor->crush = false;
				floor->type = genBuildStair;

				ok = 1;
				break;
			}
		} while(ok);

		if (manual)
			return rtn;

		secnum = osecnum; // restore old loop index
	}

	// retriggerable generalized stairs build up or down alternately
	if (rtn)
		line->special ^= StairDirection; // alternate dir on succ activations

	return rtn;
}

//
// EV_DoGenCrusher()
//
// Handle generalized crusher types
//
// Passed the linedef activating the crusher
// Returns true if a thinker created
//
int EV_DoGenCrusher(line_t *line)
{
	int				secnum;
	int				rtn;
	boolean			manual;
	sector_t		*sec;
	ceiling_t		*ceiling;
	unsigned int	value;
	int				Slnt, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned)line->special - GenCrusherBase;
	Slnt = (value & CrusherSilent) >> CrusherSilentShift;
	Sped = (value & CrusherSpeed) >> CrusherSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	// Reactivate in-stasis ceilings...for certain types.
	rtn = P_ActivateInStasisCeiling(line);

	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_crusher;
	}

	secnum = -1;

	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_crusher:                
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}

		// new ceiling thinker
		rtn = 1;
		ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, (void *)0);
		P_AddThinker(&ceiling->thinker);
		sec->ceilingdata = ceiling;
		ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
		ceiling->crush = true;
		ceiling->direction = -1;
		ceiling->sector = sec;
		ceiling->texture = sec->ceilingpic;
		ceiling->newspecial = sec->special;
		ceiling->tag = sec->tag;
		ceiling->type = Slnt ? genSilentCrusher : genCrusher;
		ceiling->topheight = sec->ceilingheight;
		ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);

		// setup ceiling motion speed
		switch (Sped)
		{
			case SpeedSlow:
				ceiling->speed = CEILSPEED;
				break;

			case SpeedNormal:
				ceiling->speed = CEILSPEED * 2;
				break;

			case SpeedFast:
				ceiling->speed = CEILSPEED * 4;
				break;

			case SpeedTurbo:
				ceiling->speed = CEILSPEED * 8;
				break;

			default:
				break;
		}

		ceiling->oldspeed=ceiling->speed;

		P_AddActiveCeiling(ceiling);  // add to list of active ceilings
		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenLockedDoor()
//
// Handle generalized locked door types
//
// Passed the linedef activating the generalized locked door
// Returns true if a thinker created
//
int EV_DoGenLockedDoor(line_t *line)
{
	int				secnum;
	int				rtn;
	sector_t		*sec;
	vldoor_t		*door;
	boolean			manual;
	unsigned int	value;
	int				Kind, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned)line->special - GenLockedBase;
	Kind = (value & LockedKind) >> LockedKindShift;
	Sped = (value & LockedSpeed) >> LockedSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	rtn = 0;

	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_locked;
	}

	secnum = -1;
	rtn = 0;
  
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_locked:
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
  
		// new door thinker
		rtn = 1;
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);
		P_AddThinker(&door->thinker);
		sec->ceilingdata = door;

		door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
		door->sector = sec;
		door->topwait = VDOORWAIT;
		door->line = line;
		door->topheight = P_FindLowestCeilingSurrounding(sec);
		door->topheight -= 4 * FRACUNIT;
		door->direction = 1;

		// setup speed of door motion
		switch(Sped)
		{
			default:
			case SpeedSlow:
				door->type = Kind ? genOpen : genRaise;
				door->speed = VDOORSPEED;
				break;

			case SpeedNormal:
				door->type = Kind ? genOpen : genRaise;
				door->speed = VDOORSPEED * 2;
				break;

			case SpeedFast:
				door->type = Kind ? genBlazeOpen : genBlazeRaise;
				door->speed = VDOORSPEED * 4;
				break;

			case SpeedTurbo:
				door->type = Kind ? genBlazeOpen : genBlazeRaise;
				door->speed = VDOORSPEED * 8;
				break;
		}

		S_StartSound((mobj_t *)&door->sector->soundorg,
					 door->speed >= VDOORSPEED * 4 ? sfx_bdopn : sfx_doropn);

		if (manual)
			return rtn;
	}
	return rtn;
}

//
// EV_DoGenDoor()
//
// Handle generalized door types
//
// Passed the linedef activating the generalized door
// Returns true if a thinker created
//
int EV_DoGenDoor(line_t *line)
{
	int				secnum;
	int				rtn;
	sector_t		*sec;
	boolean			manual;
	vldoor_t		*door;
	unsigned int	value;
	int				Dely, Kind, Sped, Trig;

	// parse the bit fields in the line's special type
	value = (unsigned)line->special - GenDoorBase;
	Dely = (value & DoorDelay) >> DoorDelayShift;
	Kind = (value & DoorKind) >> DoorKindShift;
	Sped = (value & DoorSpeed) >> DoorSpeedShift;
	Trig = (value & TriggerType) >> TriggerTypeShift;

	rtn = 0;

	// check if a manual trigger, if so do just the sector on the backside
	manual = false;
	if (Trig == PushOnce || Trig == PushMany)
	{
		if (!(sec = line->backsector))
			return rtn;
		secnum = sec - sectors;
		manual = true;
		goto manual_door;
	}

	secnum = -1;
	rtn = 0;
  
	// if not manual do all sectors tagged the same as the line
	while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];

manual_door:
		// Do not start another function if ceiling already moving
		if (P_SectorActive(ceiling_special, sec))
		{
			if (!manual)
				continue;
			else
				return rtn;
		}
  
		// new door thinker
		rtn = 1;
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);
		P_AddThinker(&door->thinker);
		sec->ceilingdata = door;

		door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
		door->sector = sec;

		// setup delay for door remaining open/closed
		switch(Dely)
		{
			default:
			case 0:
				door->topwait = 35;
				break;

			case 1:
				door->topwait = VDOORWAIT;
				break;

			case 2:
				door->topwait = 2 * VDOORWAIT;
				break;

			case 3:
				door->topwait = 7 * VDOORWAIT;
				break;
		}

		// setup speed of door motion
		switch(Sped)
		{
			default:
			case SpeedSlow:
				door->speed = VDOORSPEED;
				break;

			case SpeedNormal:
				door->speed = VDOORSPEED * 2;
				break;

			case SpeedFast:
				door->speed = VDOORSPEED * 4;
				break;

			case SpeedTurbo:
				door->speed = VDOORSPEED * 8;
				break;
		}

		door->line = line; //  remember line that triggered us

		// set kind of door, whether it opens then close, opens, closes etc.
		// assign target heights accordingly
		switch(Kind)
		{
			case OdCDoor:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t *)&door->sector->soundorg,
								 door->speed >= VDOORSPEED * 4 ?
								 sfx_bdopn : sfx_doropn);
				door->type = Sped >= SpeedFast ? genBlazeRaise : genRaise;
				break;

			case ODoor:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t *)&door->sector->soundorg,
								 door->speed >= VDOORSPEED * 4 ?
								 sfx_bdopn : sfx_doropn);
				door->type = Sped >= SpeedFast ? genBlazeOpen : genOpen;
				break;

			case CdODoor:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg,
							 door->speed >= VDOORSPEED * 4 ?
							 sfx_bdopn : sfx_doropn);
				door->type = Sped >= SpeedFast ? genBlazeCdO : genCdO;
				break;

			case CDoor:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg,
							 door->speed >= VDOORSPEED * 4 ?
							 sfx_bdopn : sfx_doropn);
				door->type = Sped >= SpeedFast ? genBlazeClose : genClose;
				break;

			default:
				break;
		}
		if (manual)
			return rtn;
	}
	return rtn;
}
