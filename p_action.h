// Action Routine pointers
void A_Light0();
void A_Light1();
void A_Light2();

void A_WeaponReady();
void A_FireWeapon();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();

void A_FirePistol();

void A_FireShotgun();

void A_FireShotgun2();
void A_CheckReload();
void A_SFXWeapon1();
void A_SFXWeapon2();
void A_SFXWeapon3();

void A_FireCGun();
void A_GunFlash();

void A_FireMissile();

void A_FirePlasma();

void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();

void A_Saw();

// Generally needed for remaining things...
void A_Explode();
void A_Pain();
void A_Scream();
void A_PlayerScream();
void A_XScream();
void A_Fall();

// Needed for the bossbrain.
void A_BrainPain();
void A_BrainScream();
void A_BrainDie();
void A_BrainAwake();
void A_BrainSpit();
void A_SpawnSound();
void A_SpawnFly();
void A_BrainExplode();

// MISC
void DDF_OldThingInit(void);

// Visibility Actions
void P_ActAlterTransluc();
void P_ActAlterVisibility();
void P_ActBecomeLessVisible();
void P_ActBecomeMoreVisible();

// Sound Actions
void P_ActMakeAmbientSound();
void P_ActMakeAmbientSoundRandom();
void P_ActMakeCloseAttemptSound();
void P_ActMakeDyingSound();
void P_ActMakeOverKillSound();
void P_ActMakePainSound();
void P_ActMakeRangeAttemptSound();
void P_ActMakeActiveSound();

// Explosion Damage Actions
void P_ActSetDamageExplosion();
void P_ActVaryingDamageExplosion();

// Stand-by / Looking Actions
void P_ActStandardLook();
void P_ActPlayerSupportLook();

// Meander, aimless movement actions.
void P_ActStandardMeander();
void P_ActPlayerSupportMeander();

// Chasing Actions
void P_ActResurrectChase();
void P_ActStandardChase();
void P_ActWalkSoundChase();

// Attacking Actions
void P_ActComboAttack();
void P_ActMeleeAttack();
void P_ActRangeAttack();
void P_ActSpareAttack();
void P_ActRefireCheck();

// Miscellanous
boolean P_ActLookForTargets(mobj_t* object);
boolean P_ActDecideMeleeAttack(mobj_t* object, attacktype_t *attack);
boolean P_ActDecideRangeAttack (mobj_t* object);
void P_ActFaceTarget();
void P_ActMakeIntoCorpse();
void P_ActResetSpreadCount();

// Projectiles
void P_ActFixedHomingProjectile();
void P_ActRandomHomingProjectile();
void P_ActLaunchOrderedSpread();
void P_ActLaunchRandomSpread();
void P_ActCreateSmokeTrail();

// Trackers
void P_ActEffectTracker();
void P_ActTrackerActive();
void P_ActTrackerFollow();
void P_ActTrackerStart();

// Blood and bullet puffs
void P_ActCheckBlood();
void P_ActCheckMoving();

void P_ActRandomJump();
void A_RandomJump();

void P_BotThink();
void P_BotSpawn();
void P_BotRespawn();

#ifdef DDF_MAIN
// -KM- 1998/11/25 Added weapon functions.
actioncode_t actions[] = {{ "ALTERTRANSLUC"    , {P_ActAlterTransluc}           },
                          { "ALTERVISIBILITY"  , {P_ActAlterVisibility}         },
                          { "LESSVISIBLE"      , {P_ActBecomeLessVisible}       },
                          { "MOREVISIBLE"      , {P_ActBecomeMoreVisible}       },
                          { "CLOSEATTEMPTSND"  , {P_ActMakeCloseAttemptSound}   },
                          { "COMBOATTACK"      , {P_ActComboAttack}             },
                          { "FACETARGET"       , {P_ActFaceTarget}              },
                          { "MAKESOUND"        , {P_ActMakeAmbientSound}        },
                          { "MAKEACTIVESOUND"  , {P_ActMakeActiveSound}         },
                          { "MAKESOUNDRANDOM"  , {P_ActMakeAmbientSoundRandom}  },
                          { "MAKEDEATHSOUND"   , {P_ActMakeDyingSound}          },
                          { "MAKEDEAD"         , {P_ActMakeIntoCorpse}          },
                          { "MAKEOVERKILLSOUND", {P_ActMakeOverKillSound}       },
                          { "MAKEPAINSOUND"    , {P_ActMakePainSound}           },
                          { "CLOSE COMBAT"     , {P_ActMeleeAttack}             },
                          { "RANGE ATTACK"     , {P_ActRangeAttack}             },
                          { "SPARE ATTACK"     , {P_ActSpareAttack}             },
                          { "RANGEATTEMPTSND"  , {P_ActMakeRangeAttemptSound}   },
                          { "REFIRE CHECK"     , {P_ActRefireCheck}             },
                          { "LOOKOUT"          , {P_ActStandardLook}            },
                          { "SUPPORT LOOKOUT"  , {P_ActPlayerSupportLook}       },
                          { "CHASE"            , {P_ActStandardChase}           },
                          { "RESCHASE"         , {P_ActResurrectChase}          },
                          { "WALKSOUND CHASE"  , {P_ActWalkSoundChase}          },
                          { "MEANDER"          , {P_ActStandardMeander}         },
                          { "SUPPORT MEANDER"  , {P_ActPlayerSupportMeander}    },
                          { "EXPLOSIONDAMAGE"  , {P_ActSetDamageExplosion}      },
                          { "VARIEDEXPDAMAGE"  , {P_ActVaryingDamageExplosion}  },
                          { "TRACER"           , {P_ActFixedHomingProjectile}   },
                          { "RANDOM TRACER"    , {P_ActRandomHomingProjectile}  },
                          { "RESET SPREADER"   , {P_ActResetSpreadCount}        },
                          { "SMOKING"          , {P_ActCreateSmokeTrail}        },
                          { "TRACKERACTIVE"    , {P_ActTrackerActive}           },
                          { "TRACKERFOLLOW"    , {P_ActTrackerFollow}           },
                          { "TRACKERSTART"     , {P_ActTrackerStart}            },
                          { "EFFECTTRACKER"    , {P_ActEffectTracker}           },
                          { "CHECKBLOOD"       , {P_ActCheckBlood}              },
                          { "CHECKMOVING"      , {P_ActCheckMoving}             },
                          { "RANDOMJUMP"       , {P_ActRandomJump}              },
                          { "WEAPON RANDOMJUMP", {A_RandomJump}                 },
                          { "WEAPON RAISE"     , {A_Raise}                      },
                          { "WEAPON LOWER"     , {A_Lower}                      },
                          { "WEAPON READY"     , {A_WeaponReady}                },
                          { "WEAPON SHOOT"     , {A_FireWeapon}                 },
                          { "WEAPON REFIRE"    , {A_ReFire}                     },
                          { "WEAPON LIGHT0"    , {A_Light0}                     },
                          { "WEAPON LIGHT1"    , {A_Light1}                     },
                          { "WEAPON LIGHT2"    , {A_Light2}                     },
                          { "WEAPON CHECKRELOAD",{A_CheckReload}               },
                          { "WEAPON FLASH"     , {A_GunFlash}                  },
                          { "WEAPON SOUND1"      , {A_SFXWeapon1}                },
                          { "WEAPON SOUND2"    , {A_SFXWeapon2}              },
                          { "WEAPON SOUND3"     , {A_SFXWeapon3}               },
                          { "BOT THINK"         , {P_BotThink}                 },
                          { "BOT SPAWN"         , {P_BotSpawn}                 },
                          { "BOT RESPAWN"       , {P_BotRespawn}                 },
                          { "NOTHING"          , {NULL}                         },
                          { COMMAND_TERMINATOR , {NULL}                         }};
#endif

