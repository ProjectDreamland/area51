//=========================================================================
//
//  Loco.cpp
//
//=========================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "Loco.hpp"
#include "e_Engine.hpp"
#include "Parsing\TextIn.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Objects\actor\actor.hpp"

//==============================================================================
// EXTERNS
//==============================================================================
#ifdef X_EDITOR
extern xbool g_game_running;
#endif

const f32 k_DistToIncreaseYaw = 50.0f;
//==============================================================================
// DATA
//==============================================================================

geom::bone_masks  loco::s_ZeroBoneMasks;
geom::bone_masks  loco::s_OneBoneMasks;


// Animation table lookup
static loco::anim_lookup s_AnimsLookup[] =
{
    // "MOVE_STYLE_WALK" style anims
    { loco::ANIM_WALK_IDLE                      ,   "WALK_IDLE"                     },
    { loco::ANIM_WALK_IDLE_TURN_LEFT            ,   "WALK_IDLE_TURN_LEFT"           },
    { loco::ANIM_WALK_IDLE_TURN_RIGHT           ,   "WALK_IDLE_TURN_RIGHT"          },
    { loco::ANIM_WALK_IDLE_TURN_180             ,   "WALK_IDLE_TURN_180"            },
    { loco::ANIM_WALK_IDLE_TURN_LEFT_180        ,   "WALK_IDLE_TURN_LEFT_180"       },
    { loco::ANIM_WALK_IDLE_TURN_RIGHT_180       ,   "WALK_IDLE_TURN_RIGHT_180"      },
    { loco::ANIM_WALK_IDLE_TURN_LEFT_180        ,   "WALK_IDLE_TURN_180_LEFT"       },
    { loco::ANIM_WALK_IDLE_TURN_RIGHT_180       ,   "WALK_IDLE_TURN_180_RIGHT"      },
    { loco::ANIM_WALK_IDLE_FIDGET               ,   "WALK_IDLE_FIDGET"              },
    { loco::ANIM_WALK_MOVE_FRONT                ,   "WALK_MOVE_FRONT"               },  
    { loco::ANIM_WALK_MOVE_LEFT                 ,   "WALK_MOVE_LEFT"                },  
    { loco::ANIM_WALK_MOVE_BACK                 ,   "WALK_MOVE_BACK"                },  
    { loco::ANIM_WALK_MOVE_RIGHT                ,   "WALK_MOVE_RIGHT"               },  
                                                                                    
    // "MOVE_STYLE_RUN" style anims                                                 
    { loco::ANIM_RUN_IDLE                       ,   "RUN_IDLE"                      },
    { loco::ANIM_RUN_IDLE_TURN_LEFT             ,   "RUN_IDLE_TURN_LEFT"            },
    { loco::ANIM_RUN_IDLE_TURN_RIGHT            ,   "RUN_IDLE_TURN_RIGHT"           },
    { loco::ANIM_RUN_IDLE_TURN_180              ,   "RUN_IDLE_TURN_180"             },
    { loco::ANIM_RUN_IDLE_TURN_LEFT_180         ,   "RUN_IDLE_TURN_LEFT_180"        },
    { loco::ANIM_RUN_IDLE_TURN_RIGHT_180        ,   "RUN_IDLE_TURN_RIGHT_180"       },
    { loco::ANIM_RUN_IDLE_TURN_LEFT_180         ,   "RUN_IDLE_TURN_180_LEFT"        },
    { loco::ANIM_RUN_IDLE_TURN_RIGHT_180        ,   "RUN_IDLE_TURN_180_RIGHT"       },
    { loco::ANIM_RUN_IDLE_FIDGET                ,   "RUN_IDLE_FIDGET"               },
    { loco::ANIM_RUN_MOVE_FRONT                 ,   "RUN_MOVE_FRONT"                },  
    { loco::ANIM_RUN_MOVE_LEFT                  ,   "RUN_MOVE_LEFT"                 },  
    { loco::ANIM_RUN_MOVE_BACK                  ,   "RUN_MOVE_BACK"                 },  
    { loco::ANIM_RUN_MOVE_RIGHT                 ,   "RUN_MOVE_RIGHT"                },  

    // "MOVE_STYLE_RUNAIM" style anims                                                 
    { loco::ANIM_RUNAIM_IDLE                       ,   "RUNAIM_IDLE"                      },
    { loco::ANIM_RUNAIM_IDLE_TURN_LEFT             ,   "RUNAIM_IDLE_TURN_LEFT"            },
    { loco::ANIM_RUNAIM_IDLE_TURN_RIGHT            ,   "RUNAIM_IDLE_TURN_RIGHT"           },
    { loco::ANIM_RUNAIM_IDLE_TURN_180              ,   "RUNAIM_IDLE_TURN_180"             },
    { loco::ANIM_RUNAIM_IDLE_TURN_LEFT_180         ,   "RUNAIM_IDLE_TURN_LEFT_180"        },
    { loco::ANIM_RUNAIM_IDLE_TURN_RIGHT_180        ,   "RUNAIM_IDLE_TURN_RIGHT_180"       },
    { loco::ANIM_RUNAIM_IDLE_TURN_LEFT_180         ,   "RUNAIM_IDLE_TURN_180_LEFT"        },
    { loco::ANIM_RUNAIM_IDLE_TURN_RIGHT_180        ,   "RUNAIM_IDLE_TURN_180_RIGHT"       },
    { loco::ANIM_RUNAIM_IDLE_FIDGET                ,   "RUNAIM_IDLE_FIDGET"               },
    { loco::ANIM_RUNAIM_MOVE_FRONT                 ,   "RUNAIM_MOVE_FRONT"                },  
    { loco::ANIM_RUNAIM_MOVE_LEFT                  ,   "RUNAIM_MOVE_LEFT"                 },  
    { loco::ANIM_RUNAIM_MOVE_BACK                  ,   "RUNAIM_MOVE_BACK"                 },  
    { loco::ANIM_RUNAIM_MOVE_RIGHT                 ,   "RUNAIM_MOVE_RIGHT"                },  

    // "MOVE_STYLE_PROWL" style anims
    { loco::ANIM_PROWL_IDLE                     ,   "PROWL_IDLE"                    },
    { loco::ANIM_PROWL_IDLE_TURN_LEFT           ,   "PROWL_IDLE_TURN_LEFT"          },
    { loco::ANIM_PROWL_IDLE_TURN_RIGHT          ,   "PROWL_IDLE_TURN_RIGHT"         },
    { loco::ANIM_PROWL_IDLE_TURN_180            ,   "PROWL_IDLE_TURN_180"           },
    { loco::ANIM_PROWL_IDLE_TURN_LEFT_180       ,   "PROWL_IDLE_TURN_LEFT_180"      },
    { loco::ANIM_PROWL_IDLE_TURN_RIGHT_180      ,   "PROWL_IDLE_TURN_RIGHT_180"     },
    { loco::ANIM_PROWL_IDLE_TURN_LEFT_180       ,   "PROWL_IDLE_TURN_180_LEFT"      },
    { loco::ANIM_PROWL_IDLE_TURN_RIGHT_180      ,   "PROWL_IDLE_TURN_180_RIGHT"     },
    { loco::ANIM_PROWL_IDLE_FIDGET              ,   "PROWL_IDLE_FIDGET"             },
    { loco::ANIM_PROWL_MOVE_FRONT               ,   "PROWL_MOVE_FRONT"              },  
    { loco::ANIM_PROWL_MOVE_LEFT                ,   "PROWL_MOVE_LEFT"               },  
    { loco::ANIM_PROWL_MOVE_BACK                ,   "PROWL_MOVE_BACK"               },  
    { loco::ANIM_PROWL_MOVE_RIGHT               ,   "PROWL_MOVE_RIGHT"              },  

    // "MOVE_STYLE_CROUCH" style anims
    { loco::ANIM_CROUCH_IDLE                    ,   "CROUCH_IDLE"                   },
    { loco::ANIM_CROUCH_IDLE_TURN_LEFT          ,   "CROUCH_IDLE_TURN_LEFT"         },
    { loco::ANIM_CROUCH_IDLE_TURN_RIGHT         ,   "CROUCH_IDLE_TURN_RIGHT"        },
    { loco::ANIM_CROUCH_IDLE_TURN_180           ,   "CROUCH_IDLE_TURN_180"          },
    { loco::ANIM_CROUCH_IDLE_TURN_LEFT_180      ,   "CROUCH_IDLE_TURN_LEFT_180"     },
    { loco::ANIM_CROUCH_IDLE_TURN_RIGHT_180     ,   "CROUCH_IDLE_TURN_RIGHT_180"    },
    { loco::ANIM_CROUCH_IDLE_TURN_LEFT_180      ,   "CROUCH_IDLE_TURN_180_LEFT"     },
    { loco::ANIM_CROUCH_IDLE_TURN_RIGHT_180     ,   "CROUCH_IDLE_TURN_180_RIGHT"    },
    { loco::ANIM_CROUCH_IDLE_FIDGET             ,   "CROUCH_IDLE_FIDGET"            },
    { loco::ANIM_CROUCH_MOVE_FRONT              ,   "CROUCH_MOVE_FRONT"             },  
    { loco::ANIM_CROUCH_MOVE_LEFT               ,   "CROUCH_MOVE_LEFT"              },  
    { loco::ANIM_CROUCH_MOVE_BACK               ,   "CROUCH_MOVE_BACK"              },  
    { loco::ANIM_CROUCH_MOVE_RIGHT              ,   "CROUCH_MOVE_RIGHT"             },  
                                                    
    // "MOVE_STYLE_CROUCHAIM" style anims
    { loco::ANIM_CROUCHAIM_IDLE                 ,   "CROUCHAIM_IDLE"                   },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_LEFT       ,   "CROUCHAIM_IDLE_TURN_LEFT"         },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_RIGHT      ,   "CROUCHAIM_IDLE_TURN_RIGHT"        },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_180        ,   "CROUCHAIM_IDLE_TURN_180"          },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_LEFT_180   ,   "CROUCHAIM_IDLE_TURN_LEFT_180"     },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_RIGHT_180  ,   "CROUCHAIM_IDLE_TURN_RIGHT_180"    },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_LEFT_180   ,   "CROUCHAIM_IDLE_TURN_180_LEFT"     },
    { loco::ANIM_CROUCHAIM_IDLE_TURN_RIGHT_180  ,   "CROUCHAIM_IDLE_TURN_180_RIGHT"    },
    { loco::ANIM_CROUCHAIM_IDLE_FIDGET          ,   "CROUCHAIM_IDLE_FIDGET"            },
    { loco::ANIM_CROUCHAIM_MOVE_FRONT           ,   "CROUCHAIM_MOVE_FRONT"             },  
    { loco::ANIM_CROUCHAIM_MOVE_LEFT            ,   "CROUCHAIM_MOVE_LEFT"              },  
    { loco::ANIM_CROUCHAIM_MOVE_BACK            ,   "CROUCHAIM_MOVE_BACK"              },  
    { loco::ANIM_CROUCHAIM_MOVE_RIGHT           ,   "CROUCHAIM_MOVE_RIGHT"             },  
                  
    // "MOVE_STYLE_CHARGE" style anims
    { loco::ANIM_CHARGE_IDLE                    ,   "CHARGE_IDLE"                   },
    { loco::ANIM_CHARGE_IDLE_TURN_LEFT          ,   "CHARGE_IDLE_TURN_LEFT"         },
    { loco::ANIM_CHARGE_IDLE_TURN_RIGHT         ,   "CHARGE_IDLE_TURN_RIGHT"        },
    { loco::ANIM_CHARGE_IDLE_TURN_180           ,   "CHARGE_IDLE_TURN_180"          },
    { loco::ANIM_CHARGE_IDLE_TURN_LEFT_180      ,   "CHARGE_IDLE_TURN_LEFT_180"     },
    { loco::ANIM_CHARGE_IDLE_TURN_RIGHT_180     ,   "CHARGE_IDLE_TURN_RIGHT_180"    },
    { loco::ANIM_CHARGE_IDLE_TURN_LEFT_180      ,   "CHARGE_IDLE_TURN_180_LEFT"     },
    { loco::ANIM_CHARGE_IDLE_TURN_RIGHT_180     ,   "CHARGE_IDLE_TURN_180_RIGHT"    },
    { loco::ANIM_CHARGE_IDLE_FIDGET             ,   "CHARGE_IDLE_FIDGET"            },
    { loco::ANIM_CHARGE_MOVE_FRONT              ,   "CHARGE_MOVE_FRONT"             },  
    { loco::ANIM_CHARGE_MOVE_LEFT               ,   "CHARGE_MOVE_LEFT"              },  
    { loco::ANIM_CHARGE_MOVE_BACK               ,   "CHARGE_MOVE_BACK"              },  
    { loco::ANIM_CHARGE_MOVE_RIGHT              ,   "CHARGE_MOVE_RIGHT"             },  

    // "MOVE_STYLE_CHARGE_FAST" style anims
    { loco::ANIM_CHARGE_FAST_IDLE                    ,   "CHARGE_FAST_IDLE"                   },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_LEFT          ,   "CHARGE_FAST_IDLE_TURN_LEFT"         },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_RIGHT         ,   "CHARGE_FAST_IDLE_TURN_RIGHT"        },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_180           ,   "CHARGE_FAST_IDLE_TURN_180"          },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_LEFT_180      ,   "CHARGE_FAST_IDLE_TURN_LEFT_180"     },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_RIGHT_180     ,   "CHARGE_FAST_IDLE_TURN_RIGHT_180"    },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_LEFT_180      ,   "CHARGE_FAST_IDLE_TURN_180_LEFT"     },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_RIGHT_180     ,   "CHARGE_FAST_IDLE_TURN_180_RIGHT"    },
    { loco::ANIM_CHARGE_FAST_IDLE_FIDGET             ,   "CHARGE_FAST_IDLE_FIDGET"            },
    { loco::ANIM_CHARGE_FAST_MOVE_FRONT              ,   "CHARGE_FAST_MOVE_FRONT"             },  
    { loco::ANIM_CHARGE_FAST_MOVE_LEFT               ,   "CHARGE_FAST_MOVE_LEFT"              },  
    { loco::ANIM_CHARGE_FAST_MOVE_BACK               ,   "CHARGE_FAST_MOVE_BACK"              },  
    { loco::ANIM_CHARGE_FAST_MOVE_RIGHT              ,   "CHARGE_FAST_MOVE_RIGHT"             },  

    // Cover Animations
    { loco::ANIM_COVER_IDLE                     ,   "COVER_IDLE"                    },
    { loco::ANIM_COVER_SHOOT                    ,   "COVER_SHOOT"                   },
    { loco::ANIM_COVER_PEEK                     ,   "COVER_PEEK"                    },
    { loco::ANIM_COVER_GRENADE                  ,   "COVER_GRENADE"                 },
    { loco::ANIM_COVER_COVERINGFIRE             ,   "COVER_COVERINGFIRE"            },

    // Evade Animations
    { loco::ANIM_EVADE_LEFT                     ,   "EVADE_LEFT"                    },
    { loco::ANIM_EVADE_RIGHT                    ,   "EVADE_RIGHT"                   },
    { loco::ANIM_GRENADE_EVADE_LEFT             ,   "EVADE_GRENADE_LEFT"            },
    { loco::ANIM_GRENADE_EVADE_RIGHT            ,   "EVADE_GRENADE_RIGHT"           },

    // Grenade animations
    { loco::ANIM_GRENADE_THROW_LONG             ,   "GRENADE_LONG"                  },
    { loco::ANIM_GRENADE_THROW_SHORT            ,   "GRENADE_SHORT"                 },
    { loco::ANIM_GRENADE_THROW_OVER_OBJECT      ,   "GRENADE_OVER_OBJECT"           },

    // Melee animations
    { loco::ANIM_MELEE_BACK_LEFT                ,   "MELEE_180_LEFT"                },
    { loco::ANIM_MELEE_BACK_RIGHT               ,   "MELEE_180_RIGHT"               },
    { loco::ANIM_MELEE_SHORT                    ,   "MELEE_SHORT"                   },
    { loco::ANIM_MELEE_LONG                     ,   "MELEE_LONG"                    },
    { loco::ANIM_MELEE_LEAP                     ,   "MELEE_LEAP"                    },
    { loco::ANIM_ATTACK_CHARGE_SWING            ,   "ATTACK_CHARGE_SWING"           },
    { loco::ANIM_ATTACK_CHARGE_MISS             ,   "ATTACK_CHARGE_MISS"            },
    { loco::ANIM_ATTACK_RANGED_ATTACK           ,   "ATTACK_RANGED_ATTACK"          },
    { loco::ANIM_ATTACK_BUBBLE                  ,   "ATTACK_BUBBLE"                 },
    
    // Mutant tank (Theta) specific anims
    { loco::ANIM_STAGE0_RAGE                    ,   "STAGE0_RAGE"                   },
    { loco::ANIM_STAGE1_RAGE                    ,   "STAGE1_RAGE"                   },
    { loco::ANIM_STAGE2_RAGE                    ,   "STAGE2_RAGE"                   },
    { loco::ANIM_STAGE3_RAGE                    ,   "STAGE3_RAGE"                   },
    { loco::ANIM_CANISTER_TO                    ,   "CANISTER_TO"                   },
    { loco::ANIM_CANISTER_IDLE                  ,   "CANISTER_IDLE"                 },
    { loco::ANIM_CANISTER_SMASH                 ,   "CANISTER_SMASH"                },
    { loco::ANIM_CANISTER_FROM                  ,   "CANISTER_FROM"                 },
    { loco::ANIM_SHIELD_ON                      ,   "SHIELD_ON"                     },
    { loco::ANIM_SHIELD_SHOOT                   ,   "SHIELD_SHOOT"                  },
    { loco::ANIM_SHIELD_REGEN                   ,   "SHIELD_REGEN"                  },

    { loco::ANIM_GRATE_TO                       ,   "Leap_To_Grate"                 },
    { loco::ANIM_GRATE_SMASH                    ,   "TELEPORT_PIPE_SMASH"           },
    { loco::ANIM_GRATE_FROM                     ,   "Leap_From_Grate"               },
    { loco::ANIM_PERCH_TO                       ,   "Leap_To_Cave"                  },
    { loco::ANIM_PERCH_FROM                     ,   "Leap_From_Cave"                },
    { loco::ANIM_THETA_CROUCH                   ,   "Spin"                          },
    { loco::ANIM_THETA_JUMP                     ,   "Leap_Generic"                  },

    // Misc animations
    { loco::ANIM_SPOT_TARGET                    ,   "REACT_SIGHT"                   },
    { loco::ANIM_HEAR_TARGET                    ,   "REACT_SOUND"                   },
    { loco::ANIM_ADD_REACT_RAGE                 ,   "REACT_RAGE"                    },
    { loco::ANIM_LOST_TARGET                    ,   "LOST_TARGET"                   },
    { loco::ANIM_FACE_IDLE                      ,   "FACE_IDLE"                     },   
    { loco::ANIM_FACE_IDLE                      ,   "EYE_BLINK"                     },   
    { loco::ANIM_FACE_IDLE                      ,   "FACE_BLINK"                    },   
    { loco::ANIM_DRAIN_LIFE                     ,   "DRAIN_LIFE"                    },   
    { loco::ANIM_REQUEST_COVER                  ,   "REQUEST_COVER"                 },   
    { loco::ANIM_REQUEST_ATTACK                 ,   "REQUEST_ATTACK"                },   
    { loco::ANIM_RESPONSE                       ,   "RESPONSE"                      },   
    
    // Death animations
    { loco::ANIM_DEATH_SIMPLE                   ,   "DEATH_SIMPLE"           },

    { loco::ANIM_DEATH_HARD_SHOT_IN_BACK_HIGH   ,   "DEATH_BIG_HEAD_BACK"           },
    { loco::ANIM_DEATH_HARD_SHOT_IN_BACK_MED    ,   "DEATH_BIG_TORSO_BACK"          },
    { loco::ANIM_DEATH_HARD_SHOT_IN_BACK_LOW    ,   "DEATH_BIG_LEGS_BACK"           },

    { loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_HIGH  ,   "DEATH_BIG_HEAD_FRONT"          },
    { loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_MED   ,   "DEATH_BIG_TORSO_FRONT"         },
    { loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_LOW   ,   "DEATH_BIG_LEGS_FRONT"          },

    { loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_HIGH  ,   "DEATH_SMALL_HEAD_BACK"         },
    { loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_MED   ,   "DEATH_SMALL_TORSO_BACK"        },
    { loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_LOW   ,   "DEATH_SMALL_LEGS_BACK"         },
    
    { loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_HIGH ,   "DEATH_SMALL_HEAD_FRONT"        },
    { loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_MED  ,   "DEATH_SMALL_TORSO_FRONT"       },
    { loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_LOW  ,   "DEATH_SMALL_LEGS_FRONT"        },

    { loco::ANIM_DEATH_CROUCH                   ,   "DEATH_CROUCH"                  },
    { loco::ANIM_DEATH_EXPLOSION                ,   "DEATH_EXPLOSION"               },
   
    { loco::ANIM_TOSS_WEAPON                    ,   "TOSS_WEAPON"                   },
    
    // Reload animations
    { loco::ANIM_RELOAD_SMP                     ,   "SMP_RELOAD"                    },
    { loco::ANIM_RELOAD_SNIPER                  ,   "SNI_RELOAD"                    },
    { loco::ANIM_RELOAD_SHOTGUN                 ,   "SHT_RELOAD"                    },
    { loco::ANIM_RELOAD_GAUSS                   ,   "GAS_RELOAD"                    },
    { loco::ANIM_RELOAD_DESERT_EAGLE            ,   "EGL_RELOAD"                    },
    { loco::ANIM_RELOAD_MSN                     ,   "MSN_RELOAD"                    },
    { loco::ANIM_RELOAD_BBG                     ,   "BBG_RELOAD"                    },
    { loco::ANIM_RELOAD_TRA                     ,   "TRA_RELOAD"                    },
    { loco::ANIM_RELOAD_SCN                     ,   "SCN_RELOAD"                    },
    { loco::ANIM_RELOAD                         ,   "RELOAD"                        },

    // Shoot animations
    { loco::ANIM_SHOOT                          ,   "SHOOT"                         },
    { loco::ANIM_SHOOT_SMP                      ,   "SMP_SHOOT"                     },
    { loco::ANIM_SHOOT_SNIPER                   ,   "SNI_SHOOT"                     },

// KSS -- TO ADD NEW WEAPON ??
    { loco::ANIM_SHOOT_SHOTGUN                  ,   "SHT_SHOOT"                     },
    
    { loco::ANIM_SHOOT_GAUSS                    ,   "GAS_SHOOT"                     },
    { loco::ANIM_SHOOT_DESERT_EAGLE             ,   "EGL_SHOOT"                     },
    { loco::ANIM_SHOOT_MHG                      ,   "MHG_SHOOT"                     },
    { loco::ANIM_SHOOT_MSN                      ,   "MSN_SHOOT"                     },
    { loco::ANIM_SHOOT_BBG                      ,   "BBG_SHOOT"                     },
    { loco::ANIM_SHOOT_TRA                      ,   "TRA_SHOOT"                     },
    { loco::ANIM_SHOOT_MUTANT                   ,   "MUT_SHOOT"                     },
    { loco::ANIM_SHOOT_SCN                      ,   "SCN_SHOOT"                     },

    { loco::ANIM_SHOOT_SECONDARY_SMP            ,   "SMP_SHOOT_SECONDARY"           },
    { loco::ANIM_SHOOT_SECONDARY_SNIPER         ,   "SNI_SHOOT_SECONDARY"           },

// KSS -- TO ADD NEW WEAPON ??
    { loco::ANIM_SHOOT_SECONDARY_SHOTGUN        ,   "SHT_SHOOT_SECONDARY"           },

    { loco::ANIM_SHOOT_SECONDARY_GAUSS          ,   "GAS_SHOOT_SECONDARY"           },
    { loco::ANIM_SHOOT_SECONDARY_DESERT_EAGLE   ,   "EGL_SHOOT_SECONDARY"           },
    { loco::ANIM_SHOOT_SECONDARY_MHG            ,   "EGL_SHOOT_SECONDARY"           },
    { loco::ANIM_SHOOT_SECONDARY_MSN            ,   "MSN_SHOOT_SECONDARY"           },
    { loco::ANIM_SHOOT_SECONDARY_MUTANT         ,   "MUT_SHOOT_SECONDARY"           },

    // Shoot animations
// KSS -- TO ADD NEW WEAPON
    { loco::ANIM_SHOOT_IDLE_SMP                 ,   "SMP_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_SNIPER              ,   "SNI_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_SHOTGUN             ,   "SHT_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_GAUSS               ,   "GAS_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_DESERT_EAGLE        ,   "EGL_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_MHG                 ,   "MHG_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_MSN                 ,   "MSN_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_BBG                 ,   "BBG_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_TRA                 ,   "TRA_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_MUTANT              ,   "MUT_SHOOT_IDLE"    },
    { loco::ANIM_SHOOT_IDLE_SCN                 ,   "SCN_SHOOT_IDLE"    },

    // Shoot animations
// KSS -- TO ADD NEW WEAPON
    { loco::ANIM_SHOOT_CROUCH_IDLE_SMP                 ,   "SMP_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_SNIPER              ,   "SNI_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_SHOTGUN             ,   "SHT_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_GAUSS               ,   "GAS_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_DESERT_EAGLE        ,   "EGL_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_MHG                 ,   "MHG_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_MSN                 ,   "MSN_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_BBG                 ,   "BBG_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_TRA                 ,   "TRA_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_MUTANT              ,   "MUT_SHOOT_CROUCH_IDLE"  },
    { loco::ANIM_SHOOT_CROUCH_IDLE_SCN                 ,   "SCN_SHOOT_CROUCH_IDLE"  },

    // Damage animations                            
    { loco::ANIM_DAMAGE_STEP_BACK               ,   "PAIN_BIG_FRONT"                },
    { loco::ANIM_DAMAGE_STEP_FORWARD            ,   "PAIN_BIG_BACK"                 },
    { loco::ANIM_DAMAGE_STEP_LEFT               ,   "PAIN_BIG_RIGHT"                },
    { loco::ANIM_DAMAGE_STEP_RIGHT              ,   "PAIN_BIG_LEFT"                 },

    { loco::ANIM_DAMAGE_SHOCK                   ,   "DAMAGE_SHOCK"                  },
    { loco::ANIM_DAMAGE_PARASITE                ,   "PAIN_BIG_PARASITE"             },    
    { loco::ANIM_DAMAGE_PLAYER_MELEE_0          ,   "PAIN_PLAYER_MELEE_0"           },    

    { loco::ANIM_MESON_STUN                     ,   "MESON_STUN"                    },    

    // Additive impact anims
    { loco::ANIM_PAIN_IDLE_FRONT                ,   "PAIN_IDLE_FRONT"               },
    { loco::ANIM_PAIN_IDLE_BACK                 ,   "PAIN_IDLE_BACK"                },
    { loco::ANIM_PROJECTILE_ATTACHED            ,   "PROJECTILE_ATTACHED"           },

    // Additive impact anims
    { loco::ANIM_ADD_IMPACT_HEAD_FRONT          ,   "PAIN_SMALL_HEAD_FRONT"               },
    { loco::ANIM_ADD_IMPACT_HEAD_BACK           ,   "PAIN_SMALL_HEAD_BACK"                },
    { loco::ANIM_ADD_IMPACT_TORSO_FRONT         ,   "PAIN_SMALL_TORSO_FRONT"              },
    { loco::ANIM_ADD_IMPACT_TORSO_BACK          ,   "PAIN_SMALL_TORSO_BACK"               },
    { loco::ANIM_ADD_IMPACT_SHOULDER_LEFT_FRONT ,   "PAIN_SMALL_SHOULDER_L_FRONT"         },
    { loco::ANIM_ADD_IMPACT_SHOULDER_RIGHT_FRONT,   "PAIN_SMALL_SHOULDER_R_FRONT"         },
    { loco::ANIM_ADD_IMPACT_SHOULDER_LEFT_BACK  ,   "PAIN_SMALL_SHOULDER_L_BACK"          },
    { loco::ANIM_ADD_IMPACT_SHOULDER_RIGHT_BACK ,   "PAIN_SMALL_SHOULDER_R_BACK"          },

    // Jumping anims
    { loco::ANIM_JUMP_OVER                      ,   "JUMP_OVER"                     },
    { loco::ANIM_JUMP_UP                        ,   "JUMP_UP"                       },
    { loco::ANIM_JUMP_DOWN                      ,   "JUMP_DOWN"                     },
    { loco::ANIM_JUMP                           ,   "JUMP"                          },
    
    // Misc multi-player anims
    { loco::ANIM_FALL                           ,   "FALL"                          },
    { loco::ANIM_GRENADE                        ,   "GRENADE"                       },
    { loco::ANIM_MELEE                          ,   "MELEE"                         },                     
    { loco::ANIM_CROUCH_ENTER                   ,   "CROUCH_ENTER"                  },
    { loco::ANIM_CROUCH_EXIT                    ,   "CROUCH_EXIT"                   },                     

    // End the list...
    { loco::ANIM_NULL                           ,   NULL                            }
} ;



/*

Hierarchy: 54 Bones
48.91: B_01_Root
48.91:  B_01_Spine01  (Parent=B_01_Root)
48.91:   B_01_Spine02  (Parent=B_01_Spine01)
48.91:    B_01_Arm_L_UpperArm  (Parent=B_01_Spine02)
48.92:     B_01_Arm_L_ForeArm  (Parent=B_01_Arm_L_UpperArm)
48.92:    B_01_Arm_R_UpperArm  (Parent=B_01_Spine02)
48.92:     B_01_Arm_R_ForeArm  (Parent=B_01_Arm_R_UpperArm)
48.92:    B_01_Neck  (Parent=B_01_Spine02)
48.92:     B_01_Head  (Parent=B_01_Neck)
48.93:  B_01_Leg_L_Thigh  (Parent=B_01_Root)
48.93:   B_01_Leg_L_Calf  (Parent=B_01_Leg_L_Thigh)
48.93:  B_01_Leg_R_Thigh  (Parent=B_01_Root)
48.93:   B_01_Leg_R_Calf  (Parent=B_01_Leg_R_Thigh)
48.93:      B_02_Arm_L_Hand  (Parent=B_01_Arm_L_ForeArm)
48.93:      B_02_Arm_R_Hand  (Parent=B_01_Arm_R_ForeArm)
48.94:    B_02_Leg_L_Foot  (Parent=B_01_Leg_L_Calf)
48.94:    B_02_Leg_R_Foot  (Parent=B_01_Leg_R_Calf)
48.94:      B_03_Face_Jaw  (Parent=B_01_Head)
48.94:       B_03_Hand_L_Finger01  (Parent=B_02_Arm_L_Hand)
48.94:        B_03_Hand_L_Finger02  (Parent=B_03_Hand_L_Finger01)
48.95:       B_03_Hand_L_Mit01  (Parent=B_02_Arm_L_Hand)
48.95:        B_03_Hand_L_Mit02  (Parent=B_03_Hand_L_Mit01)
48.96:       B_03_Hand_L_Thumb01  (Parent=B_02_Arm_L_Hand)
48.96:        B_03_Hand_L_Thumb02  (Parent=B_03_Hand_L_Thumb01)
48.96:       B_03_Hand_R_Finger01  (Parent=B_02_Arm_R_Hand)
48.97:        B_03_Hand_R_Finger02  (Parent=B_03_Hand_R_Finger01)
48.97:       B_03_Hand_R_Mit01  (Parent=B_02_Arm_R_Hand)
48.97:      B_04_Face_RB_Cheek  (Parent=B_01_Head)
49.02:      B_04_Face_RB_Lip  (Parent=B_01_Head)
49.02:      B_04_Face_RM_Brow  (Parent=B_01_Head)
49.03:      B_04_Face_RT_Brow  (Parent=B_01_Head)
49.03:      B_04_Face_RT_Cheek  (Parent=B_01_Head)
49.03:      B_04_Face_RT_Lip  (Parent=B_01_Head)
49.04:      B_04_Face_R_Eye  (Parent=B_01_Head)
49.04:      B_04_Face_R_EyeLidTop  (Parent=B_01_Head)
49.04:      B_04_Face_R_EyelidBot  (Parent=B_01_Head)
49.04:      B_04_Face_R_Lip  (Parent=B_01_Head)
49.05: 

*/

//==============================================================================
// MOVE STYLE INFO
//==============================================================================

loco::move_style_info::move_style_info()
{
    // Clear anims
    x_memset( m_iAnims, -1, sizeof( m_iAnims ) );

    // Setup default blend times
    m_IdleBlendTime          = 0.25f;
    m_MoveBlendTime          = 0.25f;
    m_FromPlayAnimBlendTime  = 0.25f;

    // Setup default turning info
    m_MoveTurnRate            = R_360;

    // Aimer and idle turn threshold info
    m_AimerBlendSpeed        = 1.0f;
    m_IdleDeltaYawMin        = -R_10;
    m_IdleDeltaYawMax        = R_10;
    m_IdleTurnDeltaYawMin    = -R_60;
    m_IdleTurnDeltaYawMax    = R_60;
    m_IdleTurn180DeltaYawMin = -R_140;
    m_IdleTurn180DeltaYawMax = R_140;
}

//==============================================================================

void loco::move_style_info::InitDefaults( move_style_info_default& Default )
{
    m_IdleBlendTime             = Default.m_IdleBlendTime;
    m_MoveBlendTime             = Default.m_MoveBlendTime;
    m_FromPlayAnimBlendTime     = Default.m_FromPlayAnimBlendTime;

    m_MoveTurnRate              = Default.m_MoveTurnRate;
    
    // Override anims?
    if( Default.m_hAnimGroup.GetPointer() )
    {
        // Override all animation info
        m_hAnimGroup = Default.m_hAnimGroup;
        ASSERT( sizeof( m_iAnims ) == sizeof( Default.m_iAnims ) );
        x_memcpy( m_iAnims, Default.m_iAnims, sizeof( m_iAnims ) );
    }
}

//==============================================================================

loco::move_style_info_default::move_style_info_default()
{
    // Clear anims
    x_memset( m_iAnims, -1, sizeof( m_iAnims ) );

    // Setup default blend times
    m_IdleBlendTime          = 0.25f;
    m_MoveBlendTime          = 0.25f;
    m_FromPlayAnimBlendTime  = 0.25f;

    // Setup default turning info
    m_MoveTurnRate           = R_360;
}

//==============================================================================
//==============================================================================
//==============================================================================
// STATES
//==============================================================================
//==============================================================================
//==============================================================================

loco_state::loco_state( loco& Loco, loco::state State ) :
    m_Base  ( Loco ),
    m_State ( State ),
    m_pNext ( Loco.m_pHead )
{
    m_Base.m_pHead = this;
}

//==============================================================================


//==============================================================================
// PLAY ANIM STATE
//==============================================================================

loco_play_anim::loco_play_anim( loco& Loco ) : 
    loco_state( Loco, loco::STATE_PLAY_ANIM ) 
{
    m_PrevState   = loco::STATE_NULL ;  // Previous state before this one was entered
    m_Flags       = 0 ;                 // Play anim flags
    m_PlayTime    = 0 ;                 // How long animation should play in secs or cylces
    m_Timer       = 0 ;                 // Time in state
    m_bComplete   = FALSE ;             // TRUE if play animation is complete
}

//==============================================================================

void loco_play_anim::OnEnter( void )
{
    // Record previous state (unless we are already playing an anim)
    if ((m_Base.m_pActive) && (m_Base.m_pActive->m_State != loco::STATE_PLAY_ANIM))
        m_PrevState = m_Base.m_pActive->m_State ;
    else
        m_PrevState = loco::STATE_IDLE ;

    // Set current motion to NULL so that idle animation is set backup
    //
    //  SH: Disabling the SetSolveActorCollisions(FALSE) call
    //      it's causing anything that does a melee attack
    //      to shove the player around
    //
    //m_Base.m_Physics.SetSolveActorCollisions(FALSE);
    m_Base.m_Motion = loco::MOTION_NULL ;
    m_Flags         = 0 ;
    m_PlayTime      = 0 ;
    m_Timer         = 0 ;
    m_bComplete     = FALSE ;

    // Clear exact look complete (IDLE state will set to TRUE when done)
    m_Base.m_bExactLookComplete = FALSE;
}

//==============================================================================

xbool loco_play_anim::OnExit( void )
{
    // Blend aimer back to what it was before
    m_Base.m_AimController.SetWeight(1.0f, 0.4f) ;
    m_bComplete = TRUE;
    return TRUE ;
}

//==============================================================================

void loco_play_anim::OnAdvance( f32 DeltaTime )
{
    // Always stay in this state if dead
    if (m_Base.m_bDead)
        return ;

    // Update timer
    m_Timer += DeltaTime ;

    //
    // -- Is playback complete?
    //

    // Play inifinitely?
    if (m_PlayTime < 0)
        return ;

    // Play until cycles complete?
    if (m_Flags & loco::ANIM_FLAG_PLAY_TYPE_CYCLIC)
    {
        // Finished cyclic?
        if ( (m_Base.m_Player.IsAtEnd()) && (m_Base.m_Player.GetCycle() >= (s32)m_PlayTime) )
            m_bComplete = TRUE ;
    }
    // Play until time complete?
    else if (m_Flags & loco::ANIM_FLAG_PLAY_TYPE_TIMED)
    {
        // Finished?
        if (m_Timer >= m_PlayTime)
            m_bComplete = TRUE ;
    }
    else
    {
        // DEFAULT: Play just once
        if (m_Base.m_Player.IsAtEnd())
            m_bComplete = TRUE ;
    }

    // Wait until complete...
    if (m_bComplete == FALSE)
        return ;

    //
    // -- What should we do now that playback is complete?
    //

    // Resume to next state? (default)
    if (m_Flags & loco::ANIM_FLAG_END_STATE_RESUME) 
    {
        // Go to idle or move?
        if( m_Base.IsAtDestination() )        
            m_Base.SetState( loco::STATE_IDLE ) ;
        else            
            m_Base.SetState( loco::STATE_MOVE ) ;
    }
    // Always stay in this state?
    else if (m_Flags & loco::ANIM_FLAG_END_STATE_HOLD)
    {
        // Nothing to do...
        return ;
    }
    else
    {
        // DEFAULT: Resume to next state

        // Go to idle or move?
        if( m_Base.IsAtDestination() )        
            m_Base.SetState( loco::STATE_IDLE ) ;
        else            
            m_Base.SetState( loco::STATE_MOVE ) ;
    }
}

//==============================================================================

xbool loco_play_anim::PlayAnim( const anim_group::handle& hAnimGroup, s32 iAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    // If no anims are loaded, then quit now...
    if (!hAnimGroup.IsLoaded())
        return FALSE ;

    // Animation not found?
    if (iAnim == -1)
        return FALSE ;
    
    // Start the bloody thing!
    m_Base.m_Player.SetAnim(hAnimGroup, iAnim, BlendTime, m_Base.m_StateAnimRate[loco::STATE_PLAY_ANIM], Flags ) ;
    
    // Store playback info
    m_Flags    = Flags ;
    m_PlayTime = PlayTime ;

    // Update animation state
    m_Base.m_AnimState = loco::STATE_PLAY_ANIM ;
    
    // Clear exact flags
    m_Base.m_bExactMove = FALSE;
    m_Base.m_bExactLook = FALSE;
    m_Base.m_bExactLookComplete = FALSE;
    m_Base.m_bExactMoveBlending = FALSE;
    m_Base.m_bExactMoveBlendingStarted = FALSE;

    // Success
    return TRUE ;
}

//==============================================================================

//==============================================================================
// IDLE STATE
//==============================================================================

//==============================================================================
loco_idle::loco_idle( loco& Loco ) : 
    loco_state      ( Loco, loco::STATE_IDLE ),
    m_Timer         ( 0.0f  ),
    m_FidgetTimer   ( 0.0f  ),
    m_IdleAnim      ( loco::MOVE_STYLE_ANIM_IDLE )
{
}

//==============================================================================

void loco_idle::OnEnter( void )
{
    m_Timer                     = 0;
    m_FidgetTimer               = x_frand(4,6) ;
    m_IdleAnim                  = loco::MOVE_STYLE_ANIM_IDLE;
    m_Base.m_bExactLookComplete = FALSE;
    m_Base.m_Motion             = loco::MOTION_NULL ;

    //  SH: Disabling the SetSolveActorCollisions(FALSE) call
    //      it's causing anything that does a melee attack
    //      to shove the player around    
    //m_Base.m_Physics.SetSolveActorCollisions(FALSE);
}

//==============================================================================

xbool loco_idle::OnExit( void )
{
    return TRUE ;
}

//==============================================================================

void loco_idle::OnAdvance( f32 DeltaTime )
{
    // Update timer
    m_Timer += DeltaTime;

    // Lookup useful info
    xbool  bBlending = m_Base.m_Player.IsBlending() ;
    xbool  bIsAtEnd  = m_Base.m_Player.IsAtEnd() ;
    
    // Compute current body aim
    f32 H;
    m_Base.ComputeBodyAim( 0.0f, H );

    /*    

    // If it's a big turn, bias towards the current aiming direction
    f32 AimH, AimV;
    m_Base.ComputeAim( m_Base.m_AimController.GetTargetHorizAim(), AimH, AimV );
    
    // Bias direction using current look at
    if(     ( x_abs ( H )    > R_90 )
        &&  ( x_abs ( AimH ) > R_90 ) 
        &&  ( x_sign( AimH ) != x_sign( H ) ) )    
    {
        if( H > 0 )
        {
            H -= R_360;
            ASSERT( H < 0 );
        }            
        else            
        {
            H += R_360;
            ASSERT( H > 0 );
        }
        ASSERT( x_sign( AimH ) == x_sign( H ) );
    }
*/        
    // Lookup style info
    loco::move_style_info& Info = m_Base.m_MoveStyleInfo ;

    // Use absolute delta yaw if controlling a ghost
    if( m_Base.m_bGhostMode )
        H = m_Base.m_DeltaYaw;

    // Only change anims when blending is finished
    if (!bBlending)
    {
        // Switch to new anim?
        f32 BlendTime = Info.m_IdleBlendTime ;

        // Is this the first time in IDLE state and we came from a previous state?
        if( ( m_Base.m_Motion == loco::MOTION_NULL ) && ( m_Base.m_pPrev ) )
        {
            // Use blend time from play anim or move state
            if( m_Base.m_pPrev->m_State == loco::STATE_PLAY_ANIM )
                BlendTime = Info.m_FromPlayAnimBlendTime;
            else if( m_Base.m_pPrev->m_State == loco::STATE_MOVE )
                BlendTime = x_max( Info.m_IdleBlendTime, Info.m_MoveBlendTime );
        }
        
        // Lookup anim
        s32   iAnim = -1 ;
        u32   Flags = 0 ;

        // Turn on yaw and horiz accumulation by default
        xbool bAccumYawMotion    = TRUE;
        xbool bRemoveYawMotion   = TRUE;
        xbool bAccumHorizMotion  = TRUE;
        xbool bRemoveHorizMotion = TRUE;
        xbool bLooping           = TRUE;

        // Turn horiz accumulation if exact move at is set -
        // this stops the NPC moving away from the "move at"
        if( m_Base.m_bExactMove )
            bAccumHorizMotion  = FALSE;

        // Lookup turn threshold - this is the delta angle that means we can exit any
        // turn animation and play the idle animation.
        radian IdleTurnDeltaYawMin = Info.m_IdleTurnDeltaYawMin;
        radian IdleTurnDeltaYawMax = Info.m_IdleTurnDeltaYawMax;
        if( ( m_Base.m_bExactLook ) && ( m_Base.m_bExactLookComplete == FALSE ) )
        {
            IdleTurnDeltaYawMin = 0;
            IdleTurnDeltaYawMax = 0;
        }
        
        // TurnLeft180?
        if(     ( H > Info.m_IdleTurn180DeltaYawMax ) 
            &&  ( Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180 ] != -1 ) )
        {
            if (        ((m_Base.m_Motion == loco::MOTION_IDLE_TURN_180) && (bIsAtEnd))
                    ||   (m_Base.m_Motion != loco::MOTION_IDLE_TURN_180) )
            {
                iAnim           = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180 ] ;
                m_Base.m_Motion = loco::MOTION_IDLE_TURN_180 ;
            }
        }
        else
        // TurnRight180?
        if(     ( H < Info.m_IdleTurn180DeltaYawMin ) 
            &&  ( Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180 ] != -1 ) )
        {
            if (        ((m_Base.m_Motion == loco::MOTION_IDLE_TURN_180) && (bIsAtEnd))
                    ||   (m_Base.m_Motion != loco::MOTION_IDLE_TURN_180) )
            {
                iAnim           = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180 ] ;
                m_Base.m_Motion = loco::MOTION_IDLE_TURN_180 ;
            }
        }
        else
        // Turn180?
        if(    ( ( H > Info.m_IdleTurn180DeltaYawMax ) || ( H < Info.m_IdleTurn180DeltaYawMin ) ) 
            && ( Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_180 ] != -1 ) )
        {
            if (        ((m_Base.m_Motion == loco::MOTION_IDLE_TURN_180) && (bIsAtEnd))
                    ||   (m_Base.m_Motion != loco::MOTION_IDLE_TURN_180) )
            {
                iAnim           = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_180 ] ;
                m_Base.m_Motion = loco::MOTION_IDLE_TURN_180 ;
            }
        }
        else
        // Turn right?
        if(     ( H < IdleTurnDeltaYawMin ) 
            &&  ( m_Base.m_Motion != loco::MOTION_IDLE_TURN_180 ) 
            &&  ( Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_RIGHT ] != -1 ) )
        {
            if (        ((m_Base.m_Motion == loco::MOTION_IDLE_TURN_RIGHT) && (bIsAtEnd))
                    ||   (m_Base.m_Motion != loco::MOTION_IDLE_TURN_RIGHT) )
            {
                iAnim           = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_RIGHT ];
                m_Base.m_Motion = loco::MOTION_IDLE_TURN_RIGHT ;
            }
        }
        else
        // Turn left?
        if(     ( H > IdleTurnDeltaYawMax ) 
            &&  ( m_Base.m_Motion != loco::MOTION_IDLE_TURN_180 ) 
            &&  ( Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_LEFT ] != -1 ) )
        {
            if (        ((m_Base.m_Motion == loco::MOTION_IDLE_TURN_LEFT) && (bIsAtEnd))
                    ||   (m_Base.m_Motion != loco::MOTION_IDLE_TURN_LEFT) )
            {
                iAnim           = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_TURN_LEFT ];
                m_Base.m_Motion = loco::MOTION_IDLE_TURN_LEFT ;
            }
        }
        else
        {
            // If exact look, remove the aimer yaw, and blend onto the facing yaw
            if( m_Base.m_bExactLook )
            {
                // Turn off yaw and horiz accumulation - this blending finished the delta yaw
                if( m_Base.m_bExactLookComplete )
                {
                    bAccumYawMotion    = FALSE;
                    bRemoveYawMotion   = TRUE;
                    bAccumHorizMotion  = FALSE;
                    bRemoveHorizMotion = TRUE;
                    
                    // Turn off yaw and horiz accumulation
                    m_Base.m_Player.GetCurrAnim().SetAccumYawMotion   ( bAccumYawMotion );
                    m_Base.m_Player.GetCurrAnim().SetRemoveYawMotion  ( bRemoveYawMotion );
                    m_Base.m_Player.GetCurrAnim().SetAccumHorizMotion ( bAccumHorizMotion );
                    m_Base.m_Player.GetCurrAnim().SetRemoveHorizMotion( bRemoveHorizMotion );
                }
            }

            // Play idle animation if previous animation has finished
            xbool bPlayIdle = bIsAtEnd ;

            // Play idle animation if not currently playing any
            bPlayIdle |=        (m_Base.m_Motion != loco::MOTION_IDLE)
                            &&  (m_Base.m_Motion != loco::MOTION_IDLE_TURN_LEFT)
                            &&  (m_Base.m_Motion != loco::MOTION_IDLE_TURN_RIGHT)
                            &&  (m_Base.m_Motion != loco::MOTION_IDLE_TURN_180) ;

            // If facing look at, exit turn 180 animations
            bPlayIdle |= ( m_Base.m_Motion == loco::MOTION_IDLE_TURN_180 ) && 
                         ( H >= Info.m_IdleDeltaYawMin ) &&
                         ( H <= Info.m_IdleDeltaYawMax );

            // Do it?
            if (bPlayIdle)
            {
                // Turn off yaw and horiz accumulation
                bAccumYawMotion   = FALSE;
                bAccumHorizMotion = FALSE;

                // Choose new idle fidget animation?
                if( bIsAtEnd )
                {
                    // Default to idle anim
                    m_IdleAnim = loco::MOVE_STYLE_ANIM_IDLE;

                    // Time to play a fidget?
                    m_FidgetTimer -= DeltaTime ;
                    if (m_FidgetTimer < 0)
                    {
                        // Update timer
                        m_FidgetTimer = x_frand(4,6) ;

                        // Only use fidget anim if present
                        if( Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE_FIDGET ] != -1 )
                            m_IdleAnim = loco::MOVE_STYLE_ANIM_IDLE_FIDGET;

                        // Turn off yaw and horiz accumulation
                        bAccumYawMotion   = FALSE;
                        bAccumHorizMotion = FALSE;
                        bLooping          = FALSE;
                    }
                }

                // Play idle/fidget anim
                iAnim = Info.m_iAnims[ m_IdleAnim ];
                m_Base.m_Motion = loco::MOTION_IDLE ;
            }
        }

        // Start a new anim?
        if (iAnim != -1)
        {
            // Trying to play the same anim type?
            if (iAnim == m_Base.m_Player.GetAnimTypeIndex())
            {
                // If not idle (ie playing a fidget or turn) and end of anim was reached,
                // set the animation again. This will either put the NPC into an idle anim,
                // or replay a turn anim.
                if( ( iAnim != Info.m_iAnims[ loco::MOVE_STYLE_ANIM_IDLE ] ) && ( bIsAtEnd ) )
                {
                    m_Base.m_Player.SetAnim( Info.m_hAnimGroup, iAnim, BlendTime, m_Base.m_StateAnimRate[loco::STATE_IDLE], Flags) ;
                    
                    // Override accumulate yaw and horiz motion
                    m_Base.m_Player.GetCurrAnim().SetAccumYawMotion   ( bAccumYawMotion );
                    m_Base.m_Player.GetCurrAnim().SetRemoveYawMotion  ( bRemoveYawMotion );
                    m_Base.m_Player.GetCurrAnim().SetAccumHorizMotion ( bAccumHorizMotion );
                    m_Base.m_Player.GetCurrAnim().SetRemoveHorizMotion( bRemoveHorizMotion );
                    m_Base.m_Player.GetCurrAnim().SetLooping          ( bLooping );
                }                    
            }
            else
            {
                // Playing a new anim - do it!
                m_Base.m_Player.SetAnim( Info.m_hAnimGroup, iAnim, BlendTime, m_Base.m_StateAnimRate[loco::STATE_IDLE], Flags) ;

                // Override accumulate yaw and horiz motion
                m_Base.m_Player.GetCurrAnim().SetAccumYawMotion   ( bAccumYawMotion );
                m_Base.m_Player.GetCurrAnim().SetRemoveYawMotion  ( bRemoveYawMotion );
                m_Base.m_Player.GetCurrAnim().SetAccumHorizMotion ( bAccumHorizMotion );
                m_Base.m_Player.GetCurrAnim().SetRemoveHorizMotion( bRemoveHorizMotion );
                m_Base.m_Player.GetCurrAnim().SetLooping          ( bLooping );
            }

            // Update animation state
            m_Base.m_AnimState = loco::STATE_IDLE ;
        }
    }

    // If exact move, wait for blend to finish since it's blending the position
    // to the move at also!
    if (m_Base.m_bExactMove)
    {
        // Wait for blending to exact move at to finish
        if (m_Base.m_bExactMoveBlending)
            return ;
    }

    // Go back to move?
    if( m_Base.HasArrivedAtPosition(m_Base.m_MoveAt, m_Base.m_ArriveDistSqr) == FALSE &&
       !m_Base.m_Player.IsBlending() )
    {    
        m_Base.SetState( loco::STATE_MOVE );
    }
}

//==============================================================================
// MOVE STATE
//==============================================================================

//==============================================================================

loco_move::loco_move( loco& Loco ) 
: loco_state( Loco, loco::STATE_MOVE ),
    m_Timer     ( 0.0f  ),
    m_bFirstTime( TRUE )
{
}

//==============================================================================

void loco_move::OnEnter( void )
{
    // Reset timer and first time flag
    m_Timer      = 0 ;
    m_bFirstTime = TRUE ;

    // Reset the delta pos array to track whether of not we are moving.
    m_Base.m_Physics.ResetDeltaPos() ;
    m_Base.m_bLocoIsStuck = FALSE ;
    m_Base.m_Physics.SetSolveActorCollisions(TRUE);

    // Clear exact look complete (IDLE state will set to TRUE when done)
    m_Base.m_bExactLookComplete = FALSE;
    
    // Force computation of motion
    m_Base.m_Motion = loco::MOTION_NULL;
}

//==============================================================================

void loco_move::OnAdvance( f32 DeltaTime )
{
    // Use exact distance prediction?
    if (m_Base.m_bExactMove)
    {
        // Wait until we are playing a move animation
        if (m_Base.m_AnimState == loco::STATE_MOVE)
        {
            // Compute arrive distance from current animation that is playing
            f32 ArriveDistSqr = m_Base.ComputeExactArriveDistSqr() ;

            // Arrived there yet?
            if (m_Base.HasArrivedAtPosition(m_Base.m_MoveAt, ArriveDistSqr))
            {
                // Start exact move blend
                m_Base.m_bExactMoveBlending = TRUE ;
                m_Base.m_bExactMoveBlendingStarted = TRUE;

                // Goto idle
                m_Base.SetState( loco::STATE_IDLE );
                return ;
            }
        }
    }
    else
    {
        // Normal arrive check
        if (m_Base.HasArrivedAtPosition(m_Base.m_MoveAt, m_Base.m_ArriveDistSqr))
        {
            // Goto idle
            m_Base.SetState( loco::STATE_IDLE );
            return ;
        }
    }

    // Lookup style info
    loco::move_style_info& Info = m_Base.m_MoveStyleInfo ;
           
    // Compute valid motion and delta yaw needed
    loco::motion Motion   = loco::MOTION_FRONT ;
    radian       DeltaYaw = 0 ;
    m_Base.ComputeValidMotion(Motion, DeltaYaw) ;

    // Idle?
    if (Motion == loco::MOTION_IDLE)
    {
        m_Base.SetMoveAt(m_Base.GetPosition()) ;
        m_Base.SetState(loco::STATE_IDLE);
        return ;
    }

    // Setup blend time - if coming from idle state use idle blend, otherwise move blend
    f32 BlendTime = m_bFirstTime ? Info.m_IdleBlendTime : Info.m_MoveBlendTime;
        
    // If playing the same motion or it's the first time in move, limit the turning
    if ( (m_bFirstTime) || (m_Base.m_Motion == Motion) || (x_abs(DeltaYaw) < R_45) )
    {
        radian AllDeltaYaw = DeltaYaw;

        // Compute max turning speed based upon time
        radian MaxDeltaYaw = Info.m_MoveTurnRate * DeltaTime ;

        // Make delta yaw dependent upon time and max delta (but don't overshoot)
        if (MaxDeltaYaw < 1)
            DeltaYaw *= MaxDeltaYaw ;

        // Cap
        if (DeltaYaw > MaxDeltaYaw)
            DeltaYaw = MaxDeltaYaw ;
        else
            if (DeltaYaw < -MaxDeltaYaw)
                DeltaYaw = -MaxDeltaYaw ;

        // Compute distance D from moveat
        f32 DSqr = ( m_Base.GetMoveAt() - m_Base.GetPosition() ).LengthSquared();
        f32 D    = 0.0f;
        if( DSqr > 0.0001f )
            D = x_sqrt( DSqr );

        // Now using K (magic dist) and D (distance from moveat), blend between the limited "DeltaYaw" and 		// the full "AllDeltaYaw"
        f32 T = 0.0f;
        if( D < k_DistToIncreaseYaw )
        {
            // When D = 0, we want T = 1
            // When D = K, we want T = 0
            T = 1.0f - ( D / k_DistToIncreaseYaw );

            // Blend between "DeltaYaw" ( D = K, T = 0 ) and "AllDeltaYaw" ( D = 0, T = 1 )
            DeltaYaw += T * x_MinAngleDiff( AllDeltaYaw, DeltaYaw );
        }    
    }

    // Lookup anim to go with motion
    s32 iAnim = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_MOVE_FRONT ];
    u32 Flags = 0;
    switch( Motion )
    {
        case loco::MOTION_FRONT: iAnim = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_MOVE_FRONT ]; break ;
        case loco::MOTION_LEFT:  iAnim = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_MOVE_LEFT  ]; break ;
        case loco::MOTION_RIGHT: iAnim = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_MOVE_RIGHT ]; break ;
        case loco::MOTION_BACK:  iAnim = Info.m_iAnims[ loco::MOVE_STYLE_ANIM_MOVE_BACK  ]; break ;
    }

    // Blending to new motion?
    if (m_Base.m_Player.IsBlending())
    {
        // Only allow delta yaw if logic is still choosing the same motion
        if( Motion != m_Base.m_Motion )
            DeltaYaw = 0 ;
    }

    // Set new anim (waits for blend for sexy smooth transition) ?
    if  (   (!m_Base.m_Player.IsBlending()) && 
            (iAnim != -1) && 
            (m_Base.m_Player.GetAnimTypeIndex() != iAnim) )
    {
        // Lookup current anim parametric frame
        f32 Frame = m_Base.m_Player.GetCurrAnim().GetFrameParametric() ;
                               
        // Set new anim
        m_Base.m_Player.SetAnim( Info.m_hAnimGroup, iAnim, BlendTime, m_Base.m_StateAnimRate[loco::STATE_MOVE], Flags) ;
                
        // Make sure horiz motion flags are on
        m_Base.m_Player.GetCurrAnim().SetAccumHorizMotion ( TRUE );
        m_Base.m_Player.GetCurrAnim().SetRemoveHorizMotion( TRUE );

        // Start at frame previous animation was at to stop the legs intersecting during blending
        // otherwise just pick a random frame to stop all nps running looking the same...
        if ( ( !m_bFirstTime ) || ( m_Base.m_bFrameMatchMoveAnim ) )
        {
            m_Base.m_Player.GetCurrAnim().SetFrameParametric(Frame) ;
        }
        else
        {
            m_Base.m_Player.GetCurrAnim().SetFrameParametric(x_frand(0.0f, 1.0f));
        }
        
        // Update animation state
        m_Base.m_AnimState = loco::STATE_MOVE ;

        // Record new motion
        m_Base.m_Motion = Motion ;
        m_bFirstTime = FALSE ;
    }

    // Match frames when blending between move styles?
    if( ( m_Base.m_Player.IsBlending() ) && ( m_Base.m_bFrameMatchMoveAnim ) )
    {
        // Match blend anim frame to current anim frame so legs etc keep in sync
        f32 Frame = m_Base.m_Player.GetCurrAnim().GetFrameParametric();
        m_Base.m_Player.GetBlendAnim().SetFrameParametric( Frame );
    }
    else
    {
        // End matching
        m_Base.m_bFrameMatchMoveAnim = FALSE;
    }
    
    // Finally, set the new yaw
    m_Base.m_Player.ApplyCurrAnimDeltaYaw( DeltaYaw ) ;

    // Are we stuck?
    if ( m_Base.m_Physics.GetRecentDeltaPos().LengthSquared() < 1.f )
    {
        m_Base.m_bLocoIsStuck = TRUE ;
    }
    else
    {
        m_Base.m_bLocoIsStuck = FALSE ;
    }
}

//==============================================================================

xbool loco_move::OnExit( void )
{
    // Not stuck anymore
    m_Base.m_bLocoIsStuck = FALSE ;

    // Stop frame matching until "SetMoveStyle" sets it to true again
    m_Base.m_bFrameMatchMoveAnim = FALSE;

    return TRUE;
}

//==============================================================================


//==============================================================================
//==============================================================================
//==============================================================================
// LOCOMOTION
//==============================================================================
//==============================================================================
//==============================================================================


//=========================================================================
// PRIVATE FUNCTIONS
//=========================================================================

// Basic arrive at function. Does not mess with ArriveDistSqr whatsoever!
// Performs rough 4 meter check on the Y, and XZ distance check 
// using ArriveDistSqr.
xbool loco::HasArrivedAtPosition( const vector3& Pos, f32 ArriveDistSqr )
{
    // Must have valid arrive distance
    ASSERT(x_isvalid(ArriveDistSqr)) ;
    ASSERT(ArriveDistSqr >= 0) ;

    // If this is a ghost loco, then just wait for the delta pos to be zero and the moving flag to clear
    if( m_bGhostMode )
    {
        // Make sure movement has stopped, and ghost is flagged as not moving
        f32 SpeedSqr = m_DeltaPos.LengthSquared();
        return ( ( SpeedSqr == 0.0f ) && ( m_bGhostIsMoving == FALSE ) );
    }

    // Compute delta between position and target
    vector3 Delta = GetPosition() - Pos ;

    // Check Y first
    if (x_abs(Delta.GetY()) > 400)
        return FALSE ;

    // Now just check X and Z
    f32 XZDistSqr = x_sqr(Delta.GetX()) + x_sqr(Delta.GetZ()) ;
    if (XZDistSqr <= ArriveDistSqr)
        return TRUE ;

    return FALSE ;
}

//=========================================================================

// Computes arrive distance needed to blend from moving to idle and end
// up very close to the final move at position.
f32 loco::ComputeExactArriveDistSqr( void )
{
    // Using "s = ( u * t ) + ( 0.5f * a * t * t )" and "v = u + ( a * t )"
    // s=dist, u=initial vel, a=accel, t=time

    // In time t, we want to get to velocity 0, so compute a using "v = u + ( a * t )"
    // 0 = u + ( a * t )
    // a = -u/t

    // Subst into "s = ( u * t ) + ( 0.5f * a * t * t )"
    // s = ( u * t ) + ( 0.5f * -u/t * t * t )
    // s = ( u * t ) - ( 0.5f * u * t )
    // s = 0.5f * u * t

    // If travelling at speed "u" and it takes time "t" to blend to idle (speed zero),
    // then the distance travelled during the blend is "0.5 * u * t".
    f32 u             = m_Player.GetMovementSpeed() ;
    f32 t             = x_max( m_MoveStyleInfo.m_IdleBlendTime, m_MoveStyleInfo.m_MoveBlendTime );
    f32 ArriveDist    = 0.5f * u * t;
    
    // * 3.0f to allow for slow frame rate. This makes the exact move blend happen early
    // which fixes the case where the npc would be behind the move at, then the next frame
    // it would get to the side of the move at and blend from move front to move side
    //ArriveDist *= 1.5f;
    ArriveDist = x_max( ArriveDist, 10.0f );

    // Returns distance squared
    f32 ArriveDistSqr = x_sqr(ArriveDist) ;
    return ArriveDistSqr ;
}


//=========================================================================
// PUBLIC FUNCTIONS
//=========================================================================

//=========================================================================

// Gets name, given index
const char* loco::GetMoveStyleName( s32 MoveStyle )
{
    // Which state?
    switch( MoveStyle )
    {
        default:
            ASSERTS((MoveStyle >= -1) && (MoveStyle < MOVE_STYLE_COUNT), "Invalid move style passed in!") ;
            ASSERTS(0, "Add your new state to this list or properties will not work!") ;
            return "ERROR - UPDATE ME!" ;

        case MOVE_STYLE_NULL:           return "NULL" ;
        case MOVE_STYLE_WALK:           return "WALK" ;
        case MOVE_STYLE_RUN:            return "RUN" ;
        case MOVE_STYLE_RUNAIM:         return "RUNAIM" ;
        case MOVE_STYLE_PROWL:          return "PROWL" ;
        case MOVE_STYLE_CROUCH:         return "CROUCH" ;
        case MOVE_STYLE_CROUCHAIM:      return "CROUCHAIM" ;
        case MOVE_STYLE_CHARGE:         return "CHARGE" ;
        case MOVE_STYLE_CHARGE_FAST:    return "CHARGE_FAST" ;
    }
}

//=========================================================================

// Gets name, given index, this is only for performance improvements in the property system
const char* loco::GetMoveStyleHeader( s32 MoveStyle )
{
    // Which state?
    switch( MoveStyle )
    {
    default:
        ASSERTS( ( MoveStyle >= loco::MOVE_STYLE_NULL ) && ( MoveStyle < loco::MOVE_STYLE_COUNT ), "Invalid move style passed in!" );
        ASSERTS( 0, "Add your new state to this list or properties will not work!" );
        return "ERROR - UPDATE ME!";

    case loco::MOVE_STYLE_NULL:           return "NULL\\";
    case loco::MOVE_STYLE_WALK:           return "WALK\\";
    case loco::MOVE_STYLE_RUN:            return "RUN\\";
    case loco::MOVE_STYLE_RUNAIM:         return "RUNAIM\\";
    case loco::MOVE_STYLE_PROWL:          return "PROWL\\";
    case loco::MOVE_STYLE_CROUCH:         return "CROUCH\\";
    case loco::MOVE_STYLE_CROUCHAIM:      return "CROUCHAIM\\";
    case loco::MOVE_STYLE_CHARGE:         return "CHARGE\\";
    case loco::MOVE_STYLE_CHARGE_FAST:    return "CHARGE_FAST\\";
    }
}

//=========================================================================

const char* loco::GetMoveStyleAnimName( s32 MoveStyleAnim )
{
    switch( MoveStyleAnim )
    {
        default:
            ASSERTS( 0, "Invalid move style anim type!" );
            return "NULL";
            
        case MOVE_STYLE_ANIM_IDLE:                  return "IDLE";
        case MOVE_STYLE_ANIM_IDLE_TURN_LEFT:        return "IDLE_TURN_LEFT";
        case MOVE_STYLE_ANIM_IDLE_TURN_RIGHT:       return "IDLE_TURN_RIGHT";
        case MOVE_STYLE_ANIM_IDLE_TURN_180:         return "IDLE_TURN_180";
        case MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180:    return "IDLE_TURN_LEFT_180";
        case MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180:   return "IDLE_TURN_RIGHT_180";
        case MOVE_STYLE_ANIM_IDLE_FIDGET:           return "IDLE_FIDGET";
        case MOVE_STYLE_ANIM_MOVE_FRONT:            return "MOVE_FRONT";
        case MOVE_STYLE_ANIM_MOVE_LEFT:             return "MOVE_LEFT";
        case MOVE_STYLE_ANIM_MOVE_BACK:             return "MOVE_BACK";
        case MOVE_STYLE_ANIM_MOVE_RIGHT:            return "MOVE_RIGHT";
    }    
}

//=========================================================================

// Returns move style enum that can be used in property queries
const char* loco::GetMoveStyleEnum( void ) 
{
    // Build enum list
    static char s_Enum[1024] = {0} ;
        
    // Already built?
    if (s_Enum[0])
        return s_Enum ;

    // Add all states to enum
    char* pDest = s_Enum ;
    for (s32 i = 0 ; i < MOVE_STYLE_COUNT ; i++)
    {
        // Lookup state name
        const char* pState = GetMoveStyleName(i) ;

        // Add to enum list
        x_strcpy(pDest, pState) ;

        // Next
        pDest += x_strlen(pState)+1 ;
    }

    // Make sure we didn't overrun the array!
    ASSERT(pDest <= &s_Enum[1024]) ;

    // Done
    return s_Enum ;
}

//=========================================================================

loco::move_style loco::GetMoveStyleByName( const char* pName )
{
    // Check all states
    for (s32 i = 0 ; i < MOVE_STYLE_COUNT ; i++)
    {
        // Found?
        if (x_stricmp(pName, GetMoveStyleName(i)) == 0)
            return (loco::move_style)i ;
    }

    // Not found
    return MOVE_STYLE_NULL ;
}

//=============================================================================

loco::loco( void ) :
    m_bDead                     ( FALSE ),              // TRUE if character is dead
    m_bLocoIsStuck              ( FALSE ),              // TRUE if character has been stuck for a while
    m_bAllowFrontMotion         ( TRUE  ),              // Controls if character can play MOVE_FRONT animations
    m_bAllowLeftMotion          ( TRUE  ),              // Controls if character can play MOVE_LEFT animations
    m_bAllowBackMotion          ( TRUE  ),              // Controls if character can play MOVE_BACK animations
    m_bAllowRightMotion         ( TRUE  ),              // Controls if character can play MOVE_RIGHT animations
    m_bPassedMoveAt             ( FALSE ),              // TRUE if NPC has gone thru the move at
    m_bExactMove                ( FALSE ),              // Use for scripted stuff - pixel perfect if TRUE!
    m_bExactLook                ( FALSE ),              // Use for scripted stuff - pixel perfect if TRUE!
    m_bExactLookComplete        ( FALSE ),              // TRUE if exact look at is complete
    m_bUseAimMoveStyles         ( FALSE ),              // Should we use aimmovestyles?
    m_bMoveAtSnap               ( FALSE ),              // TRUE if NPC should snap if passing move at
    m_bExactMoveBlending        ( FALSE ),              // TRUE if blending from MOVE->IDLE with exact move on
    m_bAdvancePhysics           ( TRUE ),               // TRUE if physics should be advanced
    m_bGhostMode                ( FALSE ),              // TRUE if loco is being used for a net ghost character
    m_bGhostIsMoving            ( FALSE ),              // TRUE if ghost should be moving ie. not idle
    m_bFrameMatchMoveAnim       ( FALSE ),              // Used for smooth move style switching
    m_bDynamicLipSyncAnim       ( FALSE ),              // TRUE if our lipsync anims switches masks depending upon loco state.
    m_bStateChangedThisTick     ( FALSE ),              // True if our state changed this tick.
    m_bFaceIdleEnabled          ( TRUE ),               // TRUE if blinking etc is allowed

    m_AnimState                 ( STATE_NULL ),         // Current animation state
    m_PlayAnimFlags             ( 0 ),                  // Current play animation flags
    m_DeltaPosScale             ( 1,1,1 ),              // Use to slide delta pos
    m_DeltaPos                  ( 0,0,0 ),              // Current delta position
    m_DeltaYaw                  ( R_0 ),                // Current delta yaw

    m_GhostYaw                  ( R_0 ),                // Current ghost yaw
    m_GhostPitch                ( R_0 ),                // Current ghost pitch

    m_MoveAt                    ( 0,0,0 ),              // Location to move to
    m_ExactMoveBlendPos         ( 0,0,0 ),              // Position to blend from

    m_ArriveDistSqr             ( 150*150 ),            // Min distance sqaured to get from move at
    
    m_HeadLookAt                ( 0,0,0 ),              // Head location to look at
    m_BodyLookAt                ( 0,0,0 ),              // Body location to look at
    
    m_Motion                    ( MOTION_NULL ),        // Current motion 
    m_ToTransition              ( MOTION_NULL ),        // Current transition motion
                                                                          
    m_pHead                     ( NULL ),               // Head of state list
    m_pActive                   ( NULL ),               // Currently active state
    m_pPrev                     ( NULL ),               // Previously active state

    m_MoveStyle                 ( MOVE_STYLE_WALK ),    // Current move style
    m_BlendMoveStyle            ( MOVE_STYLE_NULL ),    // Blend move style
    m_BlendMoveStyleAmount      (  0.0f  ),             // 0 = m_MoveStyle, 1 = m_BlendMoveStyle

    m_CinemaRelativePos         ( 0, 0, 0 ),            // Cinema relative position
    m_CinemaRelativeYaw         ( R_0 ),                // Cinema relative yaw

    m_CoverRelativePos          ( 0, 0, 0 ),            // Cover relative position
    m_CoverRelativeYaw          ( R_0 ),                // Cover relative yaw

    m_FaceIdleTimer             ( 0.0f ),               // Time before next face idle (blink etc)
    m_FaceIdleMinInterval       ( 2.0f ),               // Min time before next face idle
    m_FaceIdleMaxInterval       ( 6.0f )                // Max time before next face idle
{
    //==-----------------------------------------    
    //  Defaults for MOVE_STYLE_WALK    
    //==-----------------------------------------
    // Set blend times
    m_MoveStyleInfoDefault[ MOVE_STYLE_WALK ].m_IdleBlendTime           = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_WALK ].m_MoveBlendTime           = 0.5f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_WALK ].m_FromPlayAnimBlendTime   = 0.25f ;

    // Set turning info                         
    m_MoveStyleInfoDefault[ MOVE_STYLE_WALK ].m_MoveTurnRate            = DEG_TO_RAD(180) ;

    //==-----------------------------------------    
    //  Defaults for MOVE_STYLE_RUN    
    //==-----------------------------------------
    // Set blend times
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUN ].m_IdleBlendTime            = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUN ].m_MoveBlendTime            = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUN ].m_FromPlayAnimBlendTime    = 0.25f ;

    // Set turning info                         
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUN ].m_MoveTurnRate             = DEG_TO_RAD(360) ;

    //==-----------------------------------------    
    //  Defaults for MOVE_STYLE_RUNAIM    
    //==-----------------------------------------
    // Set blend times
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUNAIM ].m_IdleBlendTime             = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUNAIM ].m_MoveBlendTime             = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUNAIM ].m_FromPlayAnimBlendTime     = 0.25f ;

    // Set turning info                         
    m_MoveStyleInfoDefault[ MOVE_STYLE_RUNAIM ].m_MoveTurnRate              = DEG_TO_RAD(360) ;

    //==-----------------------------------------    
    //  Defaults for MOVE_STYLE_PROWL    
    //==-----------------------------------------
    // Set blend times
    m_MoveStyleInfoDefault[ MOVE_STYLE_PROWL ].m_IdleBlendTime              = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_PROWL ].m_MoveBlendTime              = 0.75f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_PROWL ].m_FromPlayAnimBlendTime      = 0.25f ;

    // Set turning info                         
    m_MoveStyleInfoDefault[ MOVE_STYLE_PROWL ].m_MoveTurnRate               = DEG_TO_RAD(90) ;

    //==-----------------------------------------    
    //  Defaults for MOVE_STYLE_CROUCH    
    //==-----------------------------------------
    // Set blend times
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCH ].m_IdleBlendTime             = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCH ].m_MoveBlendTime             = 0.75f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCH ].m_FromPlayAnimBlendTime     = 0.25f ;

    // Set turning info                         
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCH ].m_MoveTurnRate              = DEG_TO_RAD(90) ;

    //==-----------------------------------------    
    //  Defaults for MOVE_STYLE_CROUCHAIM    
    //==-----------------------------------------
    // Set blend times
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCHAIM ].m_IdleBlendTime          = 0.25f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCHAIM ].m_MoveBlendTime          = 0.75f ;
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCHAIM ].m_FromPlayAnimBlendTime  = 0.25f ;

    // Set turning info                         
    m_MoveStyleInfoDefault[ MOVE_STYLE_CROUCHAIM ].m_MoveTurnRate           = DEG_TO_RAD(90) ;

    // Initialize state anim rates
    s32 i;
    for( i = 0; i < STATE_TOTAL ; i++ )
        m_StateAnimRate[i] = 1.0f ;

    // Clear bone masks        
    m_pGeom = NULL;
    for( i = 0; i < loco::BONE_MASKS_TYPE_COUNT; i++ )
        m_pBoneMasks[i] = NULL;
}

//=========================================================================

void loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid )
{
    // He's alive!
    m_pGeom = pGeom;
    m_bDead = FALSE ;
    m_bLocoIsStuck = FALSE ;

    // Initialize the animation players
    m_hAnimGroup.SetName( pAnimFileName ) ;
    m_Player.SetLoco(this) ;                            // Track0,1 = current and blend controller
    m_Player.SetTrack( 2, &m_MaskController ) ;  // Reload anim controller
    m_Player.SetTrack( 3, &m_LipSyncController ) ;      // Lip sync controller
    m_Player.SetTrack( 4, &m_AdditiveController[0] ) ;  // Additive controller0
    m_Player.SetTrack( 5, &m_AdditiveController[1] ) ;  // Additive controller1
    m_Player.SetTrack( 6, &m_AimController ) ;          // Aim controller
    m_Player.SetTrack( 7, &m_EyeController ) ;          // Additive eye controller
    m_Player.SetAnimGroup( m_hAnimGroup );

    // Need to build resources if this fires off
    ASSERT(IsAnimLoaded()) ;

    // Initialize physics
    m_Physics.Init(ObjectGuid) ;

    // Clear motions stuff
    m_DeltaPos.Zero() ;
    m_DeltaPosScale.Set(1,1,1) ;
    m_Motion        = MOTION_NULL ;
    m_ToTransition  = MOTION_NULL ;
    m_MoveStyle     = MOVE_STYLE_NULL ;
    
    // Setup anims
    m_AnimLookupTable.Clear();
    SetupAnimLookupTable( s_AnimsLookup ) ;

    // Setup bone masks
    InitBoneMasks( pGeom );

    // Grab properties from geometry
    InitProperties( pGeom );
        
    // Setup lip sync controller
    m_LipSyncController.SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_FACE ), 0.0f ) ;

    // Reset look at and move at
    ResetMoveAndLookAt();

    // Call Init to the register states
    loco_state* pNext = m_pHead ;
    while( pNext )
    {
        pNext->OnInit();
        pNext = pNext->m_pNext;
    }

    // Setup start state
    SetState( loco::STATE_IDLE );
}

//=========================================================================

void loco::OnAdvance( f32 DeltaTime )
{
    CONTEXT( "loco::OnAdvance" );

    // Nothing to do?
    if (DeltaTime == 0)
        return ;

    // Keep the current yaw so we can update the aimer later on
    radian OldYaw = GetYaw() ;

    // Compute current aim
    radian OldH, OldV;
    ComputeHeadAim( 0.0f, OldH, OldV );

    // If playing a full body lip sync, do not advance the state - this simulates "PlayAnim" wait
    if (!( ( m_LipSyncController.IsPlaying() ) && ( m_LipSyncController.IsFullBody() == TRUE ) ) )
    {
        // Advance the state
        if (m_pActive)
            m_pActive->OnAdvance( DeltaTime ) ;
    }

    // Update face idle
    UpdateFaceIdle( DeltaTime );
    
    // Update eye tracking
    UpdateEyeTracking( DeltaTime );

    // Update move style switching and blending
    UpdateMoveStyle();

    // Advance the animation and collect delta pos
    vector3 DeltaPos( 0.0f, 0.0f, 0.0f );
    m_Player.Advance( DeltaTime, DeltaPos );

    // If controlling a ghost, then we are done!
    if( m_bGhostMode )
    {
        m_Physics.ZeroVelocity() ;

        // Make sure aimer is accurate
        m_AimController.SetBlendFactor( x_MinAngleDiff( m_GhostYaw, GetYaw() ), m_GhostPitch, 0.0f );
        return;
    }

    // Scale delta pos
    m_DeltaPos = DeltaPos * m_DeltaPosScale;

    // Sanity check
    ASSERT( x_abs( m_DeltaPos.GetX() ) < ( 100.0f * 10.0f ) );
    ASSERT( x_abs( m_DeltaPos.GetY() ) < ( 100.0f * 10.0f ) );
    ASSERT( x_abs( m_DeltaPos.GetZ() ) < ( 100.0f * 10.0f ) );

    // Get current position
    const vector3& OldPos = m_Player.GetPosition() ;
    loco_motion_controller& CurrAnim = m_Player.GetCurrAnim();

    // Make relative to cinema or cover position?
    if (    ( m_Player.IsBlending() == FALSE )
         && ( CurrAnim.IsCinemaRelativeMode() || CurrAnim.IsCoverRelativeMode() )
         && ( CurrAnim.GetAnimIndex() != -1 ) )
    {
        // Lookup relative info
        vector3 RelPos;
        radian  RelYaw;
        
        // Cinema has higher priority than cover
        if( CurrAnim.IsCinemaRelativeMode() )
        {
            // Use cinema info
            RelPos = m_CinemaRelativePos;
            RelYaw = m_CinemaRelativeYaw;
        }
        else
        {
            // Use cover info
            ASSERT( CurrAnim.IsCoverRelativeMode() );
            RelPos = m_CoverRelativePos;
            RelYaw = m_CoverRelativeYaw;
        }
        
        // Lookup anim info
        f32 Frame = CurrAnim.GetFrame() ;
        const anim_info& AnimInfo = CurrAnim.GetAnimInfo() ;

        // Get current key
        anim_key CurrKey ;
        CurrAnim.GetMotionInterpKey( AnimInfo, Frame, CurrKey ) ;
        radian CurrYaw = CurrKey.Rotation.GetRotation().Yaw + R_180 ;

        // Compute world yaw
        radian Yaw = x_ModAngle2( RelYaw + CurrYaw );

        // Force correct yaw into anim controller
        m_Player.SetCurrAnimYaw( Yaw ) ;

        // Compute world position
        CurrKey.Translation.RotateY( RelYaw + R_180 ) ;
        CurrKey.Translation += RelPos ;

        // Compute new delta pos
        m_DeltaPos = CurrKey.Translation - OldPos ;
        
        // Cinema has higher priority than cover
        if( CurrAnim.IsCinemaRelativeMode() )
        {
            // Turn off relative Y if gravity is turned on otherwise the character will not fall to the 
            // ground and will pop up and down like crazy if they start in the air
            if( CurrAnim.GetGravity() == TRUE )
                m_DeltaPos.GetY() = 0.0f;
        }
        else
        {
            // Always turn off relative Y for cover nodes since the cover nodes are positioned in the air
            m_DeltaPos.GetY() = 0.0f;
        }
    }

    // Clear velocity if accumulating motion to stop huge Y vels at end of "accumulate vert" 
    // anims. eg. the mutant tank leaping down
    if (m_Player.GetCurrAnim().GetAccumVertMotion())
        m_Physics.ZeroVelocity() ;

    // Compute new position
    vector3  NewPos = OldPos + m_DeltaPos ;

    // HELP OUT WHEN THE FRAME RATE IS LOW OR THE ARRIVE DISTANCE IS SMALL:
    // If the character has passed through the "MoveAt" then record this fact
    // so that the function "loco::IsAtDestination" returns TRUE regardless
    // of the distance check.

    // Get ratio of "MoveAt" to the closest point on the line from "OldPos" to "NewPos". 
    // If the closest point on the line is in-between the "OldPos" and "NewPos" then 
    // we have passed through the "MoveAt"!
    f32 T = m_MoveAt.GetClosestPToLSegRatio(OldPos, NewPos) ;        
    if ((T > 0) && (T < 1))
    {
        // Flag we've passed thru the move at
        m_bPassedMoveAt = TRUE ;

        // Snap to it?
        if (m_bMoveAtSnap)
            NewPos = m_MoveAt ;
    }
    
    // Performing an exact blend from MOVE -> IDLE?
    if ( ( m_bExactMove )&& ( m_AnimState == STATE_IDLE ) )
    {
        // Get blend (0 = all blend anim, 1 = all current anim)
        f32 Blend = m_Player.GetBlendAmount() ;

        // Blend to exact position?
        if ((m_bExactMove) && (m_bExactMoveBlending))
        {
            // Compute blended XZ position (0 = m_ExactMoveBlendPos, 1 = m_MoveAt)
            NewPos.GetX() = m_ExactMoveBlendPos.GetX() + (Blend * (m_MoveAt.GetX() - m_ExactMoveBlendPos.GetX())) ;
            NewPos.GetZ() = m_ExactMoveBlendPos.GetZ() + (Blend * (m_MoveAt.GetZ() - m_ExactMoveBlendPos.GetZ())) ;

            // If the blend is complete, update the "blend from" values to stop any popping
            if (Blend == 1)
            {            
                m_bExactMove         = FALSE;
                m_ExactMoveBlendPos  = NewPos ;
                m_bExactMoveBlending = FALSE ;
            }
        }
    }

    // Running from an object?
    if( m_Physics.GetGuid() && m_bAdvancePhysics )
    {
        // For fast and dumb characters
/*
        actor* pActor = (actor*)g_ObjMgr.GetObjectByGuid( m_Physics.GetGuid() );
        if( pActor && pActor->IsDumbAndFast() )
            m_Physics.AdvanceWithoutCollision(NewPos, DeltaTime) ;
        else
*/
        {
            // Update physics officially
            m_Physics.Advance(NewPos, DeltaTime) ;
        }

        // Get position from physics incase a collision happened
        NewPos = m_Physics.GetPosition() ;
    }
    else
    {
        // Running in standalone test app
        
        // Keep above ground plane
        if (NewPos.GetY() < 0)
            NewPos.GetY() = 0 ;
    }

    // Set the new position
    m_Player.SetPosition(NewPos) ;
    m_Physics.SetPosition(NewPos) ;

    // Compute the new aim relative to current yaw
    radian NewH, NewV;
    ComputeHeadAim( 0.0f, NewH, NewV );

    // If exact look at and aim direction was passed, then snap
    if(    ( GetState() == loco::STATE_IDLE)
        && ( m_bExactLook )  
        && ( m_bExactLookComplete == FALSE ) 
        && ( x_abs( OldH ) < R_90 )     
        && ( x_abs( NewH ) < R_90 )     
        && ( x_sign( OldH ) != x_sign( NewH ) ) )
    {
        // Exact look direction has been passed through so compute it and
        // set both animations to be that direction.
        radian LookYaw = GetYaw() + NewH;
        m_Player.SetCurrAnimYaw ( LookYaw );
        m_Player.SetBlendAnimYaw( LookYaw );
        
        // Turn off yaw accumulation
        m_Player.GetCurrAnim().SetAccumYawMotion( FALSE );
        m_Player.GetBlendAnim().SetAccumYawMotion( FALSE );
        
        // Flag exact look at is complete
        m_bExactLookComplete = TRUE;
    }

    // Get new yaw after animation and loco have turned the npc
    radian NewYaw = GetYaw() ;

    // Update the aiming yaw by whatever happened during the animation/logic playback
    // (this keeps the aiming in sync when the character is turning)
    m_AimController.ApplyDeltaHoriz(x_MinAngleDiff(OldYaw, NewYaw)) ;

    // Compute aiming relative to current aim, but then make relative to current yaw again
    // (this stops threshold aim popping when the npc is looking over their shoulder at the
    //  the max horiz limit)
    radian AimH, AimV;
    ComputeHeadAim( m_AimController.GetTargetHorizAim(), AimH, AimV );
    AimH += m_AimController.GetTargetHorizAim();
    m_AimController.SetBlendFactor( AimH, AimV, m_MoveStyleInfo.m_AimerBlendSpeed );

    // When moving, record last position for exact move logic
    if ( ( m_AnimState == STATE_MOVE ) || ( m_AnimState == STATE_PLAY_ANIM ) )
        m_ExactMoveBlendPos = m_Player.GetPosition() ;

    if( m_bDynamicLipSyncAnim &&
        m_bStateChangedThisTick &&
        GetLipSyncController(0).IsPlaying() )
    {
        UpdateDynamicLipSyncBoneMasks();
    }
    m_bStateChangedThisTick = FALSE;
}

//==============================================================================

void loco::UpdateFaceIdle( f32 DeltaTime )
{
    // Skip if dead
    if( m_bDead )
        return;
        
    // Skip if not enabled
    if( !m_bFaceIdleEnabled )
        return;

    // Skip if no face idle anims
    if( GetAnimIndex( loco::ANIM_FACE_IDLE ) == -1 )
        return;
    
    // Skip if lip sync controller is playing
    if( m_LipSyncController.IsPlaying() )
        return;

    // Get additive controller we will play on
    loco_additive_controller& Cont = GetAdditiveController( ANIM_FLAG_CONTROLLER1 );
            
    // Skip if in middle of playing an anim
    if( ( Cont.IsPlaying() ) && ( Cont.IsAtEnd() == FALSE ) )
        return;
        
    // Update the timer and exit if not ready yet
    m_FaceIdleTimer -= DeltaTime;
    if( m_FaceIdleTimer > 0 )
        return;
        
    // Start the blink etc
    PlayAdditiveAnim( loco::ANIM_FACE_IDLE, 0.0f, 0.0f, ANIM_FLAG_CONTROLLER1 );
                    
    // Pick new time interval before next face idle
    m_FaceIdleTimer = x_frand( m_FaceIdleMinInterval, m_FaceIdleMaxInterval );
}

//==============================================================================

void loco::UpdateEyeTracking( f32 DeltaTime )
{
    // Are eye anims present?
    if( !m_EyeController.IsActive() )
        return;
    
    // Lookup current weight and compute blend speed
    f32 Weight = m_EyeController.GetWeight();
    f32 Blend  = DeltaTime * 2.0f;
    
    // Fade out if dead, playing a cinema or lip sync
    if(     ( m_bDead )
        ||  ( m_Player.GetCurrAnim().GetAnimFlags() & loco::ANIM_FLAG_CINEMA )
        ||  ( m_LipSyncController.IsPlaying() ) )
    {
        // Blend out
        Weight = x_max( 0.0f, Weight - Blend );       
    }
    else
    {
        // Blend in
        Weight = x_min( 1.0f, Weight + Blend );       
    }
    
    // Set new weight
    m_EyeController.SetWeight( Weight );
}

//==============================================================================

void loco::UpdateDynamicLipSyncBoneMasks()
{
    if( GetState() == STATE_IDLE )
    {
        GetLipSyncController(0).SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_UPPER_BODY ), 0.5f );
    }
    else 
    {
        GetLipSyncController(0).SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_FACE ), 0.5f );
    }
}

//==============================================================================

void loco::SetupAnimLookupTable( anim_lookup AnimLookups[] )
{
    s32 i = 0 ;
    while(AnimLookups[i].m_AnimType != ANIM_NULL)
    {
        // Get lookup
        anim_lookup& Lookup = AnimLookups[i] ;
        
        // Make sure anim is valid
        anim_type AnimType = Lookup.m_AnimType ;
        ASSERT(AnimType >= 0) ;
        ASSERT(AnimType < ANIM_TOTAL) ;

        // If it's not already setup, then set it up!
        if (m_AnimLookupTable.m_Index[AnimType] == -1)
            m_AnimLookupTable.m_Index[AnimType] = m_Player.GetAnimIndex(Lookup.m_pName) ;

        // Next
        i++ ;
    }
}

//==============================================================================

s32 loco::GetMoveStyleAnimIndex ( loco::move_style_anim MoveStyleAnim )
{
    ASSERT( MoveStyleAnim >= 0 );
    ASSERT( MoveStyleAnim < MOVE_STYLE_ANIM_COUNT );
    return m_MoveStyleInfo.m_iAnims[ MoveStyleAnim ];
}

//==============================================================================

s32 loco::GetAnimIndex( loco::anim_type AnimType )
{
    // NULL?
    if (AnimType == loco::ANIM_NULL)
        return -1 ;

    // Lookup animation index 
    ASSERT(AnimType >= 0) ;
    ASSERT(AnimType < loco::ANIM_TOTAL) ;
    s32 Index = m_AnimLookupTable.m_Index[AnimType] ;

    // Return index
    return Index ;
}

//=========================================================================

xbool loco::SetState( loco::state State )
{
    // Anims not ready?
    if (!IsAnimLoaded())
        return FALSE ;

    // Already in this state?
    if( ( m_pActive ) && ( m_pActive->m_State == State ) )
        return TRUE;

    // Fail to exit current state?
    if( m_pActive && m_pActive->OnExit() == FALSE )
        return FALSE;

    // Search for state
    loco_state* pNext = m_pHead;
    while( pNext )
    {
        if( State == pNext->m_State )
        {
            // Keep previous state
            m_pPrev = m_pActive ;

            // Setup new state
            pNext->OnEnter() ;

            // Record we are in it
            m_pActive = pNext ;

            m_bStateChangedThisTick = TRUE;
            return TRUE;
        }

        pNext = pNext->m_pNext;
    }

    return FALSE;
}

//==============================================================================

const char* loco::GetStateName( loco::state State ) const 
{
    // Active state?
    switch (State)
    {
    default:
        ASSERTS(0, "ADD NEW STATE HERE!") ;
        return "UNKNOWN_STATE_TYPE";
    case STATE_NULL:        return "STATE_NULL";
    case STATE_IDLE:        return "STATE_IDLE";
    case STATE_MOVE:        return "STATE_MOVE";
    case STATE_PLAY_ANIM:   return "STATE_PLAY_ANIM";
    }
}

//==============================================================================

loco::state loco::GetState( void ) const
{
    // Active state?
    if (m_pActive)
        return m_pActive->m_State ;
    else
        return STATE_NULL ;
}
//==============================================================================

const char*loco::GetStateName( void ) const
{
    // Active state?
    if (m_pActive)
        return GetStateName(m_pActive->m_State) ;
    else
        return "NO STATE";
}

//==============================================================================

loco::state loco::GetPrevState( void ) const
{
    // Active state?
    if (m_pPrev)
        return m_pPrev->m_State ;
    else
        return STATE_NULL ;
}

//==============================================================================

const char* loco::GetPrevStateName( void ) const
{
    // Active state?
    if (m_pPrev)
        return GetStateName(m_pPrev->m_State) ;
    else
        return "NO STATE";
}

//==============================================================================

void loco::SetStateAnimRate( loco::state State, f32 Rate )
{
    ASSERT(Rate >= 0);
    ASSERT(State >= 0) ;
    ASSERT(State < STATE_TOTAL) ;

    m_StateAnimRate[State] = Rate ;
}

//==============================================================================

f32 loco::GetStateAnimRate( loco::state State ) const
{
    ASSERT(State >= 0) ;
    ASSERT(State < STATE_TOTAL) ;

    return m_StateAnimRate[State] ;
}

//==============================================================================

xbool loco::PlayMaskedAnim( const char* pAnimGroup, const char* pAnim, f32 BlendTime, u32 Flags )
{
    // Lookup controller
    loco_mask_controller& Cont = GetMaskController( Flags );

    // Set anim group
    Cont.SetAnimGroup( pAnimGroup );
    
    // Lookup the anim
    s32 iAnim = Cont.GetAnimIndex( pAnim );        
    if( iAnim == -1 )
        return FALSE;
    
    // Lookup blend time?
    const anim_info& AnimInfo = Cont.GetAnimInfo( iAnim );
    if( AnimInfo.GetBlendTime() != -1.0f )
        BlendTime = AnimInfo.GetBlendTime();
    
    // Lookup bone masks
    loco::bone_masks_type MaskType = loco::BONE_MASKS_TYPE_FULL_BODY;
    if( Flags & ANIM_FLAG_MASK_TYPE_FULL_BODY )
        MaskType = loco::BONE_MASKS_TYPE_FULL_BODY;
    else if( Flags & ANIM_FLAG_MASK_TYPE_UPPER_BODY )
        MaskType = loco::BONE_MASKS_TYPE_UPPER_BODY;
    else if( Flags & loco::ANIM_FLAG_MASK_TYPE_FACE )
        MaskType = loco::BONE_MASKS_TYPE_FACE;
        
    // Finally, start the anim
    Cont.SetBoneMasks( GetBoneMasks( MaskType ), BlendTime );
    Cont.SetAnim( Cont.GetAnimGroupHandle(), iAnim, Flags );
    
    return TRUE;
}

//==============================================================================

xbool loco::PlayMaskedAnim( s32 iAnim, bone_masks_type MaskType, f32 BoneMaskBlendTime, u32 Flags )
{
    // No anim?
    if (iAnim == -1)
        return FALSE ;

    // Lookup controller
    loco_mask_controller& Controller = GetMaskController(Flags) ;

    // Set masks and start the anim
    Controller.SetBoneMasks( GetBoneMasks( MaskType ), BoneMaskBlendTime) ;
    Controller.SetAnim(m_hAnimGroup, iAnim, Flags) ;

    return TRUE ;
}

//==============================================================================

xbool loco::PlayMaskedAnim ( loco::anim_type AnimType, bone_masks_type MaskType, f32 BoneMaskBlendTime, u32 Flags )
{
    // If no anims are loaded, then quit now...
    if (!IsAnimLoaded())
        return FALSE ;

    // Lookup animation index by calling virtual function
    ASSERT(AnimType >= 0) ;
    ASSERT(AnimType < loco::ANIM_TOTAL) ;
    s32 iAnim = GetAnimIndex(AnimType) ;

    return PlayMaskedAnim( iAnim, MaskType, BoneMaskBlendTime, Flags ) ;
}

//==============================================================================

xbool loco::PlayAdditiveAnim( s32 iAnim, f32 BlendInTime, f32 BlendOutTime, u32 Flags )
{
    // No anim found?
    if( iAnim == -1 )
        return FALSE;

    // Lookup controller
    loco_additive_controller& Controller = GetAdditiveController( Flags );

    // Start the animation
    Controller.SetAnim( m_hAnimGroup, iAnim, Flags );
    Controller.SetBlendInTime( BlendInTime );
    Controller.SetBlendOutTime( BlendOutTime );
    Controller.SetRate( 1.0f );

    return TRUE ;
}

//==============================================================================

xbool loco::PlayAdditiveAnim( loco::anim_type AnimType, f32 BlendInTime, f32 BlendOutTime, u32 Flags )
{
    // If no anims are loaded, then quit now...
    if (!IsAnimLoaded())
        return FALSE ;

    // Lookup animation index by calling virtual function
    ASSERT(AnimType >= 0) ;
    ASSERT(AnimType < loco::ANIM_TOTAL) ;
    s32 iAnim = GetAnimIndex(AnimType) ;

    // Play anim
    return PlayAdditiveAnim( iAnim, BlendInTime, BlendOutTime, Flags ) ;
}

//==============================================================================

xbool loco::PlayAdditiveAnim( const char* pAnim, f32 BlendInTime, f32 BlendOutTime, u32 Flags )
{
    // Lookup anim group data
    anim_group* pAnimGroup = (anim_group*)m_hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return FALSE ;

    // Lookup animation index
    s32 iAnim = pAnimGroup->GetAnimIndex(pAnim) ;
    if (iAnim == -1)
        return FALSE ;

    return PlayAdditiveAnim(iAnim, BlendInTime, BlendOutTime, Flags) ;
}

//==============================================================================

xbool loco::PlayLipSyncAnim( const anim_group::handle& hAnimGroup, s32 iAnim, u32 VoiceID, u32 Flags )
{
    m_bDynamicLipSyncAnim = FALSE;
    
    // No anim?
    if ( iAnim == -1 )
        return FALSE ;
    
    // Lookup controller
    loco_lip_sync_controller& Controller = GetLipSyncController(Flags) ;

    // Setup bone masks
    if (Flags & ANIM_FLAG_MASK_TYPE_FULL_BODY)
        Controller.SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_FULL_BODY ), 0.5f);         // Full body?
    else
    if (Flags & ANIM_FLAG_MASK_TYPE_UPPER_BODY)                 // Upper body?
        Controller.SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_UPPER_BODY ), 0.5f );
    else
    if (Flags & ANIM_FLAG_MASK_TYPE_FACE)                       // Face?
        Controller.SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_FACE ), 0.5f ) ;
    else
    if (Flags & ANIM_FLAG_MASK_TYPE_DYNAMIC)                       // Face?
    {
        m_bDynamicLipSyncAnim = TRUE;
        UpdateDynamicLipSyncBoneMasks();
    }
    else
    {
        // DEFAULT: Full body

        // Make sure to flag controller to be full body so that the anim player
        // knows to use it for motion accumulation.
        Flags |= ANIM_FLAG_MASK_TYPE_FULL_BODY;

        // Full body
        Controller.SetBoneMasks( GetBoneMasks( loco::BONE_MASKS_TYPE_FULL_BODY ), 0.5f);         // Full body?
    }
    
    // Start anim and audio
    Controller.SetAnim(hAnimGroup, iAnim, VoiceID, Flags) ;

    return TRUE ;
}

//==============================================================================

xbool loco::PlayLipSyncAnim( const anim_group::handle& hAnimGroup, const char* pAnimName, u32 VoiceID, u32 Flags )
{
    // Lookup anim group data
    anim_group* pAnimGroup = (anim_group*)hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return FALSE ;

    // Lookup animation index
    s32 iAnim = pAnimGroup->GetAnimIndex(pAnimName) ;
    if (iAnim == -1)
        return FALSE ;

    // Try play
    return PlayLipSyncAnim(hAnimGroup, iAnim, VoiceID, Flags) ;
}

//==============================================================================

xbool loco::PlayLipSyncAnim( const anim_group::handle& hAnimGroup, const char* pAnimName, const char* pAudioName, u32 Flags )
{
    // Try start the sound if not in the artist viewer (lip sync controller will start it when event is found)
    u32 VoiceID = g_AudioMgr.Play( pAudioName, ( Flags & loco::ANIM_FLAG_ARTIST_VIEWER ) == 0 );     // Name, AutoStart

    // Try play
    return PlayLipSyncAnim(hAnimGroup, pAnimName, VoiceID, Flags) ;
}

//==============================================================================

xbool loco::PlayLipSyncAnim( s32 iAnim, const char* pAudioName, u32 Flags )
{
    // Try start the sound if not in the artist viewer (lip sync controller will start it when event is found)
    u32 VoiceID = g_AudioMgr.Play( pAudioName, ( Flags & loco::ANIM_FLAG_ARTIST_VIEWER ) == 0 );     // Name, AutoStart

    return PlayLipSyncAnim(m_hAnimGroup, iAnim, VoiceID, Flags) ;
}

//==============================================================================

xbool loco::PlayLipSyncAnim( loco::anim_type AnimType, const char* pAudioName, u32 Flags )
{
    // Lookup animation index
    s32 iAnim = GetAnimIndex(AnimType) ;
    if (iAnim == -1)
        return FALSE ;

    // Try start the sound if not in the artist viewer (lip sync controller will start it when event is found)
    u32 VoiceID = g_AudioMgr.Play( pAudioName, ( Flags & loco::ANIM_FLAG_ARTIST_VIEWER ) == 0 );    // Name, AutoStart

    return PlayLipSyncAnim(m_hAnimGroup, iAnim, VoiceID, Flags) ;
}

//==============================================================================

void loco::InitBoneMasks( const geom* pGeom )
{
    // Simple init struct
    struct init_bone_mask
    {
        const char*             m_pName;    // Name
        loco::bone_masks_type   m_Type;     // Type
        geom::bone_masks*       m_pDefault; // Default geom bone masks
    };
    
    // Name lookups
    static init_bone_mask s_InitBoneMaskList[] = 
    {
        //  Name            Type                            Default masks
        {   "FULL_BODY",    BONE_MASKS_TYPE_FULL_BODY,      &s_OneBoneMasks     },
        {   "UPPER_BODY",   BONE_MASKS_TYPE_UPPER_BODY,     &s_OneBoneMasks     },
        {   "FACE",         BONE_MASKS_TYPE_FACE,           &s_OneBoneMasks     },
        {   "AIM_VERT",     BONE_MASKS_TYPE_AIM_VERT,       &s_ZeroBoneMasks    },
        {   "AIM_HORIZ",    BONE_MASKS_TYPE_AIM_HORIZ,      &s_ZeroBoneMasks    },
        {   "NO_AIM_VERT",  BONE_MASKS_TYPE_NO_AIM_VERT,    &s_ZeroBoneMasks    },
        {   "NO_AIM_HORIZ", BONE_MASKS_TYPE_NO_AIM_HORIZ,   &s_ZeroBoneMasks    },
        {   "RELOAD_SHOOT", BONE_MASKS_TYPE_RELOAD_SHOOT,   &s_OneBoneMasks     },

        // Backwards compatability incase vert and horiz are not present        
        {   "AIM",          BONE_MASKS_TYPE_AIM_VERT,       &s_ZeroBoneMasks    },
        {   "AIM",          BONE_MASKS_TYPE_AIM_HORIZ,      &s_ZeroBoneMasks    },
        {   "NO_AIM",       BONE_MASKS_TYPE_NO_AIM_VERT,    &s_ZeroBoneMasks    },
        {   "NO_AIM",       BONE_MASKS_TYPE_NO_AIM_HORIZ,   &s_ZeroBoneMasks    },
    };

    s32 i;
    
    // Setup "zero" bone masks
    s_ZeroBoneMasks.NameOffset = -1;
    for( i = 0; i < MAX_ANIM_BONES; i++ )
        s_ZeroBoneMasks.Weights[i] = 0.0f;
    s_ZeroBoneMasks.nBones = 0;

    // Setup "one" bone masks
    s_OneBoneMasks.NameOffset = -1;
    for( i = 0; i < MAX_ANIM_BONES; i++ )
        s_OneBoneMasks.Weights[i] = 1.0f;
    s_OneBoneMasks.nBones = ( pGeom ) ? pGeom->m_nBones : 0;

    const s32 Count = ( sizeof( s_InitBoneMaskList ) / sizeof( s_InitBoneMaskList[0] ) );
    
    // Pass 1 - try setup using geometry bone masks
    if( pGeom )
    {
        for( i = 0; i < Count; i++ )
        {
            // Lookup info
            const init_bone_mask& Info = s_InitBoneMaskList[i];

            // Search for masks in geometry if not setup
            if( m_pBoneMasks[ Info.m_Type ] == NULL )
                m_pBoneMasks[ Info.m_Type ] = pGeom->FindBoneMasks( Info.m_pName );
        }        
    }
        
    // Pass 2 - setup any masks not found to use the default masks
    for( i = 0; i < Count; i++ )
    {
        // Lookup info
        const init_bone_mask& Info = s_InitBoneMaskList[i];

        // Set to default if mask not setup
        if( m_pBoneMasks[ Info.m_Type ] == NULL )
            m_pBoneMasks[ Info.m_Type ] = Info.m_pDefault;
    }        
}

//==============================================================================

const geom::bone_masks& loco::GetBoneMasks( loco::bone_masks_type Type )
{
    // Must be valid type
    ASSERT( Type >= 0 );
    ASSERT( Type < loco::BONE_MASKS_TYPE_COUNT );
    
    // Make sure you have updated the init table in loco::InitBoneMasks !
    ASSERT( m_pBoneMasks[ Type ] );
    
    // Return reference
    return *m_pBoneMasks[ Type ];
}

//==============================================================================

void loco::InitProperties( const geom* pGeom )
{
    // No geom?
    if( !pGeom )
        return ;
        
    // Init aimer properties?   
    const geom::property_section* pSection = pGeom->FindPropertySection( "AIMER" );
    if( pSection )
    {
        pGeom->GetPropertyFloat( pSection, "BlendSpeed",    &m_MoveStyleInfo.m_AimerBlendSpeed                  );
        pGeom->GetPropertyAngle( pSection, "HorizMin",      &m_AimController.m_HorizMinLimit    );
        pGeom->GetPropertyAngle( pSection, "HorizMax",      &m_AimController.m_HorizMaxLimit    );
        pGeom->GetPropertyAngle( pSection, "VertMin",       &m_AimController.m_VertMinLimit     );
        pGeom->GetPropertyAngle( pSection, "VertMax",       &m_AimController.m_VertMaxLimit     );
        pGeom->GetPropertyFloat( pSection, "EyeBlendSpeed", &m_EyeController.m_TargetBlendSpeed );
    }

    // Init idle properties?
    pSection = pGeom->FindPropertySection( "IDLE" );
    if( pSection )
    {
        pGeom->GetPropertyAngle( pSection, "DeltaYawMin",        &m_MoveStyleInfo.m_IdleDeltaYawMin        );
        pGeom->GetPropertyAngle( pSection, "DeltaYawMax",        &m_MoveStyleInfo.m_IdleDeltaYawMax        );
        pGeom->GetPropertyAngle( pSection, "TurnDeltaYawMin",    &m_MoveStyleInfo.m_IdleTurnDeltaYawMin    );
        pGeom->GetPropertyAngle( pSection, "TurnDeltaYawMax",    &m_MoveStyleInfo.m_IdleTurnDeltaYawMax    );
        pGeom->GetPropertyAngle( pSection, "Turn180DeltaYawMin", &m_MoveStyleInfo.m_IdleTurn180DeltaYawMin );
        pGeom->GetPropertyAngle( pSection, "Turn180DeltaYawMax", &m_MoveStyleInfo.m_IdleTurn180DeltaYawMax );
    }

    // Init move styles properties?
    for( s32 i = 0; i < loco::MOVE_STYLE_COUNT; i++ )
    {
        // Lookup info
        move_style_info_default& MoveStyle = m_MoveStyleInfoDefault[i];            
        const char*              pMoveStyle = GetMoveStyleName( i );

        // Setup move style defaults?
        pSection = pGeom->FindPropertySection( pMoveStyle );
        if( pSection )
        {
            pGeom->GetPropertyFloat( pSection, "IdleBlendTime",          &MoveStyle.m_IdleBlendTime         );
            pGeom->GetPropertyFloat( pSection, "MoveBlendTime",          &MoveStyle.m_MoveBlendTime         );
            pGeom->GetPropertyFloat( pSection, "FromPlayAnimBlendTime",  &MoveStyle.m_FromPlayAnimBlendTime );
            pGeom->GetPropertyAngle( pSection, "MoveTurnRate",           &MoveStyle.m_MoveTurnRate          );
        }
    }
    
    // Init misc properties?
    pSection = pGeom->FindPropertySection( "MISC" );
    if( pSection )
    {
        pGeom->GetPropertyFloat( pSection, "FaceIdleMinInterval", &m_FaceIdleMinInterval );
        pGeom->GetPropertyFloat( pSection, "FaceIdleMaxInterval", &m_FaceIdleMaxInterval );
    }
}

//==============================================================================

// Uber play anim struct
struct play_anim
{
    anim_group::handle  hAnimGroup;     // Anim group handle
    s32                 iAnim;          // Index of anim
    loco::anim_type     AnimType;       // Loco anim type
    const char*         pAnimName;      // Anim name
    const char*         pAudioName;     // Audio to play
    s32                 VoiceID;        // Voice ID of audio
    f32                 BlendInTime;    // Blend in time
    f32                 BlendOutTime;   // Blend out time
    u32                 Flags;          // Flags
    f32                 PlayTime;       // Play time
    f32                 StartFrame;     // Frame to start on
};

//==============================================================================

xbool loco::PlayAnim( const anim_group::handle& hAnimGroup, s32 iAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    // If no anims are loaded, then quit now...
    if (!hAnimGroup.IsLoaded())
        return FALSE ;

    // No anim?
    if (iAnim == -1)
        return FALSE ;

    // If we are already playing this animation, don't start it over?
    if ((Flags & ANIM_FLAG_RESTART_IF_SAME_ANIM) == 0)
    {
        loco_anim_controller& CurrAnim = m_Player.GetCurrAnim() ;

        if (        (CurrAnim.GetAnimGroupHandle().GetPointer() == hAnimGroup.GetPointer())
                &&  (CurrAnim.GetAnimTypeIndex()                == iAnim) )
            return FALSE ;
    }

    // If this is specified to be a full body anim then make sure to stop the mask controller
    // (this will blend it out nicely)
    if( Flags & ANIM_FLAG_MASK_TYPE_FULL_BODY )
    {
        m_MaskController.Stop();
    }
    
    // Record flags
    m_PlayAnimFlags = Flags ;

    // Goto anim state
    SetState(STATE_PLAY_ANIM) ;
    
    // Lookup current state
    loco_play_anim* pPlayState = (loco_play_anim*)m_pActive ;

    // Make sure it's the play state!
    ASSERTS( pPlayState, "No Anim state" );
    ASSERT( pPlayState->m_State == STATE_PLAY_ANIM );

    // Now try play the anim
    return pPlayState->PlayAnim(hAnimGroup, iAnim, BlendTime, Flags, PlayTime) ;
}

//==============================================================================

xbool loco::PlayAnim( const anim_group::handle& hAnimGroup, const char* pAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    // Lookup anim group data
    anim_group* pAnimGroup = (anim_group*)hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return FALSE ;

    // Lookup animation index
    s32 iAnim = pAnimGroup->GetAnimIndex(pAnim) ;
    if (iAnim == -1)
        return FALSE ;

    // Call main function
    return PlayAnim( hAnimGroup, iAnim, BlendTime, Flags, PlayTime ) ;
}

//==============================================================================

xbool loco::PlayAnim( const char* pAnimGroup, const char* pAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    // Play a masked anim?
    if(     ( Flags & loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY )
        ||  ( Flags & loco::ANIM_FLAG_MASK_TYPE_FACE ) )
    {
        // Play masked anim
        return PlayMaskedAnim( pAnimGroup, pAnim, BlendTime, Flags );
    }
    else
    {        
        // Setup group
        anim_group::handle hAnimGroup;
        hAnimGroup.SetName( pAnimGroup );

        // Play the anim
        return PlayAnim( hAnimGroup, pAnim, BlendTime, Flags, PlayTime );
    }
}

//==============================================================================

xbool loco::PlayAnim( const char* pAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    // If no anims are loaded, then quit now...
    if (!IsAnimLoaded())
        return FALSE ;

    // Lookup animation index
    s32 iAnim = m_Player.GetAnimIndex(pAnim) ;

    // Call main function
    return PlayAnim( iAnim, BlendTime, Flags, PlayTime ) ;
}

//==============================================================================

xbool loco::PlayAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    // If no anims are loaded, then quit now...
    if (!IsAnimLoaded())
        return FALSE ;

    // Lookup animation index by calling virtual function
    ASSERT(AnimType >= 0) ;
    ASSERT(AnimType < loco::ANIM_TOTAL) ;
    s32 iAnim = GetAnimIndex(AnimType) ;

    // Call main function
    return PlayAnim( iAnim, BlendTime, Flags, PlayTime ) ;
}

//=========================================================================

radian loco::GetAimerYaw( void )
{
    radian LocalAimerYaw = m_AimController.GetHorizAim() ;
    radian HandleYaw     = GetYaw() ;
    radian WorldYaw      = HandleYaw + LocalAimerYaw;
 
    return WorldYaw ;
}

//=========================================================================
    
xbool loco::PlayDeathAnim( const anim_group::handle& hAnimGroup, const char* pAnim, f32 BlendTime, u32 Flags )
{
    m_bDead = PlayAnim(hAnimGroup, pAnim, BlendTime, Flags, 0.0f) ;

    // Blend out the aiming and any reload animation ready for the death
    m_AimController.SetBoneMasks( s_ZeroBoneMasks, s_ZeroBoneMasks, 0.5f );
    m_MaskController.SetBoneMasks( s_ZeroBoneMasks, 0.5f );

    return m_bDead ;
}

//=========================================================================

xbool loco::PlayDeathAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags )
{
    m_bDead = PlayAnim(AnimType, BlendTime, Flags, 0.0f) ;

    // Blend out the aiming and any reload animation ready for the death
    m_AimController.SetBoneMasks( s_ZeroBoneMasks, s_ZeroBoneMasks, 0.5f );
    m_MaskController.SetBoneMasks( s_ZeroBoneMasks, 0.5f );

    return m_bDead ;
}
//=========================================================================

xbool loco::IsPlayAnimComplete( void ) const
{
    // Must be in play anim state?
    if ((m_pActive) && (m_pActive->m_State == STATE_PLAY_ANIM))
        return m_pActive->IsComplete() ;

    // We are not in play anim state, so just return TRUE to keep game logic going
    return TRUE ;
}

//=========================================================================
// Controller access functions
//=========================================================================

loco_motion_controller& loco::GetMotionController( u32 AnimFlags )
{
    // Current anim?
    if( AnimFlags & loco::ANIM_FLAG_CONTROLLER0 )
        return m_Player.GetCurrAnim() ;
    
    // Blend anim?
    if( AnimFlags & loco::ANIM_FLAG_CONTROLLER1 )
        return m_Player.GetBlendAnim() ;
    
    // Default
    return m_Player.GetCurrAnim() ;
}

//=========================================================================

loco_additive_controller& loco::GetAdditiveController( u32 AnimFlags )
{
    // Lookup controller index
    s32 iController = 0 ;   // Default if no flags
    if (AnimFlags & ANIM_FLAG_CONTROLLER0) 
        iController = 0 ;
    if (AnimFlags & ANIM_FLAG_CONTROLLER1) 
        iController = 1 ;

    // Lookup controller to use
    ASSERT(iController >= 0) ;
    ASSERT(iController < 2) ;
    loco_additive_controller& Controller = m_AdditiveController[iController] ;

    return Controller ;
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

void loco::RenderInfo( xbool bRenderLoco     /*= TRUE*/, 
                       xbool bLabelLoco      /*= FALSE*/,
                       xbool bRenderSkeleton /*= FALSE*/, 
                       xbool bLabelSkeleton  /*= FALSE*/ )
{
    // Render info?
    if (bRenderLoco)
    {
        radian Yaw ;
        vector3 P;

        // Get eye
        vector3 Eye = GetEyePosition() ;
        vector3 vOffset( 0, 5.f, 0 );

        // Facing yaw
        Yaw = m_Player.GetFacingYaw() ;
        P.Set(0,0,200);
        P.RotateY(Yaw);
        draw_Line( m_Player.GetPosition() + vOffset, m_Player.GetPosition()+P + vOffset, XCOLOR_PURPLE );

        // Blending?
        if( m_Player.IsBlending() )
        {
            // Current yaw
            Yaw = m_Player.GetCurrAnimYaw();
            P.Set(0,0,200);
            P.RotateY(Yaw);
            draw_Line( m_Player.GetPosition() + vOffset, m_Player.GetPosition()+P + vOffset, XCOLOR_WHITE );
            
            // Blend yaw
            Yaw = m_Player.GetBlendAnimYaw();
            P.Set(0,0,200);
            P.RotateY(Yaw);
            draw_Line( m_Player.GetPosition() + vOffset, m_Player.GetPosition()+P + vOffset, XCOLOR_RED );
        }
                
        // Horiz aim
        radian H = m_AimController.GetHorizAim() ;
        P.Set(0,0,300) ;
        P.RotateY(GetYaw() + H) ;
        draw_Line( m_Player.GetPosition() + vOffset, m_Player.GetPosition()+P + vOffset, XCOLOR_GREEN );

        // Render where move at
        draw_Line( GetPosition() + vOffset, m_MoveAt + vOffset, XCOLOR_YELLOW );
        draw_Sphere( m_MoveAt + vOffset, 25.f, XCOLOR_YELLOW);

        // Draw relative mode?
        if( m_Player.GetCurrAnim().IsCinemaRelativeMode() || m_Player.GetCurrAnim().IsCoverRelativeMode() )
        {
            // Lookup info
            vector3 RelPos;
            radian  RelYaw;
            if( m_Player.GetCurrAnim().IsCinemaRelativeMode() )
            {
                RelPos = m_CinemaRelativePos;
                RelYaw = m_CinemaRelativeYaw;
            }
            else
            {
                RelPos = m_CoverRelativePos;
                RelYaw = m_CoverRelativeYaw;
            }
            
            // Draw location
            draw_Sphere( RelPos + vOffset, 15.0f, XCOLOR_RED );

            // Draw direction
            P.Set( 0,0, 250 ) ;
            P.RotateY( RelYaw ) ;
            draw_Line( RelPos + vOffset , RelPos + P + vOffset, XCOLOR_RED );
        }
                
        // Render what they are looking at
        draw_Line  ( Eye, m_HeadLookAt, XCOLOR_AQUA) ;
        draw_Sphere( m_HeadLookAt, 25.f, XCOLOR_AQUA );
        
        // Draw look at line on the ground
        vector3 LookAtFloor = m_HeadLookAt;
        LookAtFloor.GetY() = GetPosition().GetY();
        draw_Line  ( GetPosition() + vOffset, LookAtFloor + vOffset, XCOLOR_AQUA ) ;

        // Show labels
        if (bLabelLoco)
        {
            draw_Label( m_MoveAt,       XCOLOR_WHITE, "Loco:MoveAt" );
            draw_Label( m_HeadLookAt,   XCOLOR_WHITE, "Loco:HeadLookAt" );
            
            if( m_Player.GetCurrAnim().IsCinemaRelativeMode() )
                draw_Label( m_CinemaRelativePos,  XCOLOR_WHITE, "Loco:CinRelPos" );
            else if( m_Player.GetCurrAnim().IsCoverRelativeMode() )
                draw_Label( m_CoverRelativePos,  XCOLOR_WHITE, "Loco:CovRelPos" );
        }
    }

    // Render the animation player
    (void)bRenderSkeleton;
    if (0)
    {
        // Show local bbox from anim player
        ASSERT( m_Player.GetAnimGroup() );
        const bbox& BBox = m_Player.GetAnimGroup()->GetBBox() ;
        draw_SetL2W(GetL2W()) ;
        draw_BBox(BBox, XCOLOR_WHITE) ;
        draw_ClearL2W() ;

        // Show skeleton
        m_Player.RenderSkeleton(bLabelSkeleton) ;
    }
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

xbool loco::IsAtPosition( const vector3& Pos, f32 ArriveDistSqr )
{
    // Waiting on getting to move at?
    if (Pos == m_MoveAt)
    {
        // Exact move?
        if (m_bExactMove)
        {
            // In middle of exact blend?
            if( m_bExactMoveBlending || (!m_bExactMoveBlendingStarted && GetState() == STATE_MOVE) )
                return FALSE ;
        }

        // Passed through move at?
        if (m_bPassedMoveAt)
            return TRUE ;
    }

    // Less than zero means use default (comes from trigger system)
    if (ArriveDistSqr < 0)
        ArriveDistSqr = m_ArriveDistSqr ;

    // Default
    return (HasArrivedAtPosition(Pos, ArriveDistSqr)) ;
}

//=========================================================================
// Look at functions
//=========================================================================

void loco::SetPosition( const vector3& Position )
{
    // If ghost mode, store delta so we can pick the best animation
    if( m_bGhostMode )
    {
        // Compute new delta
        m_DeltaPos = Position - GetPosition();

        // Get speed squared
        f32 SpeedSqr = m_DeltaPos.LengthSquared();
        if( SpeedSqr > 0.000001f )
        {
            // If moving, config move at so state machine will try and move
            f32 Offset = 100.0f / x_sqrt( SpeedSqr );
            m_MoveAt = Position + (m_DeltaPos * Offset) ;
        }

        // If delta is small and loco is in idle, then skip position update
        // so turn animations can play without sliding feet
        if( ( GetState() == STATE_IDLE ) && ( SpeedSqr < x_sqr(0.1f) ) )
            return;
    }

    // Update animation and physics
    m_Player.SetPosition(Position) ;
    m_Physics.SetPosition(Position);

#ifdef X_EDITOR    
    // Reset "move at" and "look at" to point straight forward when editing objects
    // or initializing blue-printed objects to their anchor etc.
    // (without this the eyes look in weird positions)
    if( !g_game_running )
    {
        ResetMoveAndLookAt();
    }        
#endif    
}

//=========================================================================

void loco::SetPitch( radian Pitch )
{
    // If ghost mode do not blend since the ghost already does the blending
    if( m_bGhostMode )
    {
        // Store
        m_GhostPitch = Pitch;

        // Make sure aimer is accurate
        m_AimController.SetBlendFactor( x_MinAngleDiff( m_GhostYaw, GetYaw() ), m_GhostPitch, 0.0f );
    }
}

//=========================================================================

matrix4 loco::GetWeaponL2W( s32 iHand )
{
    // Must be in range!
    ASSERT( iHand >= 0 );
    ASSERT( iHand <= 1 );
    
    // Start with bone L2W
    matrix4 L2W = m_Player.GetBoneL2W( m_Player.m_iWeaponBone[iHand] );

    // NOTE - "weapon_attach" bones bind pose MUST NOT have rotation on it,
    //        otherwise the weapon will be rotated wrong
    //        (since we only take off the bind position from the anim player matrix)
    L2W.PreTranslate( m_Player.GetBoneBindPosition( m_Player.m_iWeaponBone[iHand] ) );

    return L2W;
}

//=========================================================================

// These bind values were tweaked by hand to make the flag look good
static f32 FLAG_BIND_ROT_TWIST = 30.0f;
static f32 FLAG_BIND_ROT_PITCH = 0.0f;
static f32 FLAG_BIND_ROT_YAW   = 0.0f;
static f32 FLAG_BIND_ROT_ROLL  = 0.0f;
static f32 FLAG_BIND_POS_X     = 0.0f;
static f32 FLAG_BIND_POS_Y     = 50.0f;
static f32 FLAG_BIND_POS_Z     = 16.385f;

matrix4 loco::GetFlagL2W( void )
{
    // Lookup bone L2W
    ASSERT( m_Player.m_iFlagBone >= 0 );
    ASSERT( m_Player.m_iFlagBone < GetNBones() );
    const matrix4& BoneL2W = m_Player.GetBoneL2W( m_Player.m_iFlagBone );
    
    // Bind twist (I separated this to make tweaking easier)
    matrix4 FlagBindTwist;
    FlagBindTwist.Identity();
    FlagBindTwist.RotateY( DEG_TO_RAD( FLAG_BIND_ROT_TWIST ) );
    
    // Bind transform
    matrix4 FlagBind;
    FlagBind.Identity();
    FlagBind.SetRotation( radian3( DEG_TO_RAD( FLAG_BIND_ROT_PITCH ),
                                   DEG_TO_RAD( FLAG_BIND_ROT_YAW   ),
                                   DEG_TO_RAD( FLAG_BIND_ROT_ROLL  ) ) );
    FlagBind.SetTranslation( vector3( FLAG_BIND_POS_X, FLAG_BIND_POS_Y, FLAG_BIND_POS_Z ) );

    // Compute final L2W
    matrix4 L2W = BoneL2W * FlagBind * FlagBindTwist;
    return L2W;
}

//=========================================================================

void loco::ClearMoveFlags()
{
    // Reset passed flag
    m_bPassedMoveAt      = FALSE ;

    // Interrupt exact move blending if it's on
    m_bExactMoveBlending = FALSE ;

    // set that we haven't started blending
    m_bExactMoveBlendingStarted = FALSE;

}

//=========================================================================

void loco::SetCinemaRelativeMode( xbool bEnable )
{
    // Set relative mode on all controllers
    m_Player.GetCurrAnim().SetCinemaRelativeMode( bEnable );
    m_Player.GetBlendAnim().SetCinemaRelativeMode( bEnable );
    m_LipSyncController.SetCinemaRelativeMode( bEnable );
}

//=========================================================================

void loco::SetCinemaRelativeInfo( const vector3& Pos, radian Yaw )
{
    m_CinemaRelativePos = Pos ;
    m_CinemaRelativeYaw = Yaw ;
}

//=========================================================================

void loco::SetCoverRelativeMode( xbool bEnable )
{
    // Set relative mode on all controllers
    m_Player.GetCurrAnim().SetCoverRelativeMode( bEnable );
    m_Player.GetBlendAnim().SetCoverRelativeMode( bEnable );
    m_LipSyncController.SetCoverRelativeMode( bEnable );
}

//=========================================================================

void loco::SetCoverRelativeInfo( const vector3& Pos, radian Yaw )
{
    m_CoverRelativePos = Pos ;
    m_CoverRelativeYaw = Yaw ;
}

//=========================================================================

void loco::ResetMoveAndLookAt( void )
{
    // Lookup current transform info
    const vector3& Position  = GetPosition();
    const vector3& EyeOffset = GetEyeOffset();
          radian   Yaw       = GetYaw();
    
    // Reset move at to be current location
    SetMoveAt( Position );

    // Compute look at point straight in front of npc at eye height
    vector3 LookAt( 0.0f, 0.0f, 200.0f );
    LookAt.RotateY( Yaw );
    LookAt += Position + EyeOffset;

    // Set head/body/eyes to look straight forward
    SetLookAt( LookAt );
    m_EyeController.SetLookAt( LookAt, FALSE );
    m_AimController.SetBlendFactor( 0.0f, 0.0f, 0.0f );
}

//=========================================================================

void loco::SetMoveAt( const vector3& Target )
{
    // Must be valid!
    ASSERT(Target.IsValid()) ;

    // If this is a new target, then reset the passed through move at
    if (Target != m_MoveAt)
    {    
        ClearMoveFlags();
    }

    // Keep new target position
    m_MoveAt = Target;
}

//=========================================================================

void loco::SetHeadLookAt( const vector3& Target, xbool bSetEyesLookAt /*= TRUE */ )
{
    // Must be valid!
    ASSERT( Target.IsValid() );

    // Update?
    if( m_HeadLookAt != Target )
    {
        // Set new target
        m_HeadLookAt         = Target;

        // Clear exact look complete - IDLE state will set it to TRUE once done.
        m_bExactLookComplete = FALSE;
    }

    // Make the eyes also follow the look at
    if( bSetEyesLookAt )
        m_EyeController.SetLookAt(Target);
}

//=========================================================================

void loco::SetBodyLookAt( const vector3& Target )
{
    // Must be valid!
    ASSERT( Target.IsValid() );

    // Update?
    if( m_BodyLookAt != Target )
    {
        // Set new target
        m_BodyLookAt = Target;
        
        // Clear exact look complete - IDLE state will set it to TRUE once done.
        m_bExactLookComplete = FALSE;
    }        
}

//=========================================================================

void loco::SetLookAt( const vector3& Target, xbool bSetEyesLookAt /*= TRUE */)
{
    // Set head look at
    SetHeadLookAt( Target, bSetEyesLookAt );
    
    // Set body look at
    SetBodyLookAt( Target );
}

//=========================================================================

void loco::ComputeMotion( xbool           bAllowDir[4],    // F L B R
                          radian          LookYaw,
                          radian          MoveYaw,
                          loco::motion&   Motion,
                          radian&         DeltaYaw )
{
    s32 i;
    
    // STOP! - You've messed up the order of these enums! 
    // Put them back to F L B R in loco.hpp, with F=0
    ASSERT(loco::MOTION_FRONT == 0);
    ASSERT(loco::MOTION_LEFT  == 1);
    ASSERT(loco::MOTION_BACK  == 2);
    ASSERT(loco::MOTION_RIGHT == 3);

    // At least one direction must be available!
    ASSERT( (bAllowDir[loco::MOTION_FRONT]) ||
            (bAllowDir[loco::MOTION_LEFT ]) ||
            (bAllowDir[loco::MOTION_BACK ]) ||
            (bAllowDir[loco::MOTION_RIGHT]) );

    // Blending?
    if( m_Player.IsBlending() )
    {
        // Use current motion if blending to stop wrong decisions during the blend...
        Motion = m_Motion;

        // Make sure it's valid
        if( ( Motion < MOTION_FRONT ) || ( Motion > MOTION_RIGHT ) )
            Motion = MOTION_FRONT;
    }
    else
    {
        // Default
        Motion = loco::MOTION_FRONT;

        // 1) Choose the best motion to take that will take the npc directly to the "move at"
        //    and look at the "look at" as much as possible
        //    ie. the motion which is at 0,90,180,270 degrees from the ideal move at direction,
        //        and keeps the look at delta as small as possible.
        radian BestYaw = F32_MAX;
        for( i = 0; i < 4; i++ )
        {
            // Get test motion
            motion TestMotion  = (motion)i;
            
            // If this motion and the opposite motion are disabled then skip both 
            // motions otherwise the npc will ping-pong depending upon the aimer yaw delta
            // This fixes THETA & DR.CRAY (which have left/right disabled) from ping-ponging between
            // front and back anims as the aimer yaw delta ping-pongs between +ve and -ve
            // relative to the body look at!
            if(         ( bAllowDir[ TestMotion             ] == FALSE )
                    &&  ( bAllowDir[ ( TestMotion + 2 ) & 3 ] == FALSE ) )
            {
                continue;
            }
        
            // Compute move yaw to test
            radian TestMoveYaw = MoveYaw - ( TestMotion * R_90 );
            
            // Compute look delta yaw
            radian Yaw = x_abs( x_MinAngleDiff( TestMoveYaw, LookYaw ) );
                
            // Closest to ideal look yaw so far?
            if (Yaw < BestYaw)
            {
                // Record
                BestYaw = Yaw;
                Motion  = TestMotion;
            }
        }
        
        // 2) If the direction is disabled, then choose the next best one
        if( bAllowDir[ Motion ] == FALSE )
        {
            // Lookup other motions relative to the ideal motion
            loco::motion LeftMotion  = (loco::motion)( ( Motion + 1 ) & 3 );
            loco::motion RightMotion = (loco::motion)( ( Motion - 1 ) & 3 );
            loco::motion BackMotion  = (loco::motion)( ( Motion + 2 ) & 3 );

            // Select from left/right directions?
            if( ( bAllowDir[ LeftMotion ] ) && ( bAllowDir[ RightMotion ] ) )
            {
                // Lookup the current aiming direction
                radian AimYaw  = GetYaw() + m_AimController.GetTargetHorizAim();
            
                // Bias based on delta yaw from the ideal look direction
                if( x_MinAngleDiff( LookYaw, AimYaw ) > 0.0f )
                    Motion = LeftMotion;
                else            
                    Motion = RightMotion;
            }
            else if( bAllowDir[ LeftMotion ] )
            {
                Motion = LeftMotion;
            }
            else if( bAllowDir[ RightMotion ] )
            {
                Motion = RightMotion;
            }
            else
            {
                // Only back available so choose it
                ASSERT( bAllowDir[ BackMotion ] );
                Motion = BackMotion;
            }                    
        }
    }
    
    // Check to make sure above logic has chosen a valid motion
    ASSERT( ( Motion >= MOTION_FRONT ) && ( Motion <= MOTION_RIGHT ) );
    ASSERT( bAllowDir[ Motion ] );
    
    // Compute final desired move yaw
    radian DesiredYaw = x_ModAngle2( MoveYaw - ( Motion * R_90 ) );

    // 3) Compute delta yaw that needs to be applied to the current animation
    DeltaYaw = x_MinAngleDiff( DesiredYaw, m_Player.GetCurrAnimYaw() );
}

//=========================================================================

void loco::ComputeValidMotion( loco::motion& Motion, radian& DeltaYaw )
{
    // Setup defaults
    Motion   = loco::MOTION_FRONT;
    DeltaYaw = 0;

    // Lookup style info
    loco::move_style_info& Info = m_MoveStyleInfo;

    // STOP! - You've messed up the order of these enums! 
    // Put them back to F L B R in loco.hpp, with F=0
    ASSERT(loco::MOTION_FRONT == 0);
    ASSERT(loco::MOTION_LEFT  == 1);
    ASSERT(loco::MOTION_BACK  == 2);
    ASSERT(loco::MOTION_RIGHT == 3);

    // Setup allowable motion directions: F L B R
    xbool bAllowDir[4];
    bAllowDir[loco::MOTION_FRONT] = ( Info.m_iAnims[ MOVE_STYLE_ANIM_MOVE_FRONT ] != -1 ) && ( m_bAllowFrontMotion ); // Front
    bAllowDir[loco::MOTION_LEFT ] = ( Info.m_iAnims[ MOVE_STYLE_ANIM_MOVE_LEFT  ] != -1 ) && ( m_bAllowLeftMotion  ); // Left
    bAllowDir[loco::MOTION_BACK ] = ( Info.m_iAnims[ MOVE_STYLE_ANIM_MOVE_BACK  ] != -1 ) && ( m_bAllowBackMotion  ); // Back
    bAllowDir[loco::MOTION_RIGHT] = ( Info.m_iAnims[ MOVE_STYLE_ANIM_MOVE_RIGHT ] != -1 ) && ( m_bAllowRightMotion ); // Right

    // If the "move style" or "allow strafing" flag has been changed, and the old motion is no longer
    // available, then clear it so the motion picking which stops 180 degree animation blends
    // doesn't set all the allowable directions to FALSE
    if (        (m_Motion >= loco::MOTION_FRONT)
            &&  (m_Motion <= loco::MOTION_RIGHT)
            &&  (bAllowDir[m_Motion] == FALSE) )
    {
        // Clear
        m_Motion = loco::MOTION_NULL;
    }

    // If all animations are missing we are screwed!
    if (    (bAllowDir[loco::MOTION_FRONT] == FALSE) &&
            (bAllowDir[loco::MOTION_LEFT ] == FALSE) &&
            (bAllowDir[loco::MOTION_BACK ] == FALSE) &&
            (bAllowDir[loco::MOTION_RIGHT] == FALSE) )
    {
        // Just idle
        Motion = MOTION_IDLE;
        return;
    }

    // If first time in move state, assume old motion was forward to avoid pops
    if( ( m_Motion < MOTION_FRONT ) || ( m_Motion > MOTION_RIGHT ) )
        m_Motion = MOTION_FRONT;

    // Make sure current motion is forward, left, back, or right
    ASSERT( m_Motion >= MOTION_FRONT );
    ASSERT( m_Motion <= MOTION_RIGHT );

    // Compute body look yaw
    vector3      DeltaLook = GetBodyLookAt() - GetPosition();
    radian       LookYaw   = DeltaLook.GetYaw();
   
    // Compute move yaw
    vector3      DeltaMove = GetMoveAt() - GetPosition();
    radian       MoveYaw   = DeltaMove.GetYaw();

    // If the "look at" is behind the npc and the npc is not moving towards the "move at",
    // then bias the motion that goes towards the "move at" to avoid big blends
    radian FacingYaw    = GetYaw();
    radian LookDeltaYaw = x_MinAngleDiff( LookYaw, FacingYaw ); 
    radian MoveDeltaYaw = x_MinAngleDiff( MoveYaw, FacingYaw ); 
    if( ( x_abs( MoveDeltaYaw ) > R_45 ) && ( x_abs( LookDeltaYaw ) > R_90 ) )
    {
        // Force the "look at" onto the "move at"
        DeltaLook = DeltaMove;
        LookYaw   = MoveYaw;
    }
    
    // Blending?
    if( m_Player.IsBlending() )
    {
        // This function will just use the current motion to stop popping etc.
        ComputeMotion( bAllowDir, LookYaw, MoveYaw, Motion, DeltaYaw );
    }
    else
    {
        // Keep computing a motion and delta yaw until we get the best non-popping solution
        xbool bMotionFound = FALSE;
        while(!bMotionFound)
        {
            // Let's try this motion
            bMotionFound = TRUE;
            ComputeMotion( bAllowDir, LookYaw, MoveYaw, Motion, DeltaYaw );
            
            // Would this new motion cause a big blended turn?
            //
            // 1) Fixes the case when the lookat and moveat are infront of the NPC and then
            //  in the same frame, move 180 degrees behind the NPC. Without this code the NPC
            //  would keep playing the same animation, but walk around in a circle - this
            //  code causes him to use the left/right animations too)

            // 2) Fixes the case where the NPC runs through the lookat and would want to play
            //  forward, then backward animations as he ran through it. This code, makes
            //  him use the left/right motion animations too!)
            //
            if( x_abs( DeltaYaw ) > R_90 )
            {
                // Lookup other motions relative to the ideal motion
                loco::motion LeftMotion  = (loco::motion)( ( Motion + 1 ) & 3 );
                loco::motion RightMotion = (loco::motion)( ( Motion - 1 ) & 3 );
                loco::motion BackMotion  = (loco::motion)( ( Motion + 2 ) & 3 );
                
                // Trying to move in the same or opposite direction?
                if( ( m_Motion == Motion ) || ( m_Motion == BackMotion ) )
                {
                    // Could a left/right motion be used?
                    if( ( bAllowDir[ LeftMotion ] ) || ( bAllowDir[ RightMotion ] ) )
                    {
                        // Disable the motion that would cause the big yaw blend and re-compute a new motion
                        bAllowDir[ Motion ] = FALSE;
                        bMotionFound        = FALSE;
                    }
                }
            }
        }
    }
    
    // If choosing the opposite animation, then make sure the 180 turn rotates through the look at
    if( ( x_abs( DeltaYaw ) > R_90 ) && ( Motion == ( ( m_Motion + 2 ) & 3 ) ) )
    {
        radian DeltaDir = x_MinAngleDiff( LookYaw, MoveYaw );
        if( x_sign( DeltaDir ) != x_sign( DeltaYaw ) )
        {
            // Reverse direction
            DeltaYaw = R_360 - DeltaYaw;             
        }
    }
}

//=========================================================================

loco::move_style_anim loco::GetCurrentMoveStyleAnim( void )
{
    s32 i;
    
    // Is current move style invalid?
    if( ( m_MoveStyle <= MOVE_STYLE_NULL ) || ( m_MoveStyle >= MOVE_STYLE_COUNT ) )
        return MOVE_STYLE_ANIM_NULL;

    // Lookup index of current animation that is playing
    s32 iAnim = m_Player.GetCurrAnim().GetAnimTypeIndex();

    // Using main package or override?        
    s16* AnimIndices = &m_AnimLookupTable.m_Index[ m_MoveStyle * MOVE_STYLE_ANIM_COUNT ];
    if( m_Player.GetCurrAnim().GetAnimGroupHandle().GetPointer() == m_MoveStyleInfoDefault[ m_MoveStyle ].m_hAnimGroup.GetPointer() )
        AnimIndices = &m_MoveStyleInfoDefault[ m_MoveStyle ].m_iAnims[0];

    // Check for playing a move style anim
    for( i = 0 ; i < MOVE_STYLE_ANIM_COUNT ; i++ )
    {
        // Playing a move style animation?
        if( iAnim == AnimIndices[i] )
            return (move_style_anim)i;
    }
    
    // Currently not playing a move style anim
    return MOVE_STYLE_ANIM_NULL;
}

//=========================================================================

s32 loco::GetMoveStyleAnimIndex( move_style Style, move_style_anim Anim )
{
    // Invalid move style?
    if( ( Style <= MOVE_STYLE_NULL ) || ( Style >= MOVE_STYLE_COUNT ) )
    {
        ASSERTS(0, "You passed in an invalid style!");
        return -1;
    }
   
    // Using main package or override?        
    ASSERT( Style > MOVE_STYLE_NULL );
    ASSERT( Style < MOVE_STYLE_COUNT );
    s16* AnimIndices = &m_AnimLookupTable.m_Index[ Style * MOVE_STYLE_ANIM_COUNT ];
    if( m_Player.GetCurrAnim().GetAnimGroupHandle().GetPointer() == m_MoveStyleInfoDefault[ Style ].m_hAnimGroup.GetPointer() )
        AnimIndices = &m_MoveStyleInfoDefault[ Style ].m_iAnims[0];

    // Lookup index
    return AnimIndices[ Anim ];
}

//=========================================================================

xbool loco::IsValidMoveStyle( move_style Style )
{
    // Invalid move style?
    if( ( Style <= MOVE_STYLE_NULL ) || ( Style >= MOVE_STYLE_COUNT ) )
    {
        ASSERTS(0, "You passed in an invalid style!");
        return FALSE;
    }
    
    // Lookup start of indices for this style
    ASSERT( Style > MOVE_STYLE_NULL );
    ASSERT( Style < MOVE_STYLE_COUNT );

    // Use global anim package or move style anim package?
    const move_style_info_default& Info = m_MoveStyleInfoDefault[ Style ];
    const s16* AnimIndices;
    if( Info.m_hAnimGroup.GetPointer() )
    {
        // Use move style anim package
        AnimIndices = &Info.m_iAnims[0];
    }
    else
    {
        // Use global anim package
        AnimIndices = &m_AnimLookupTable.m_Index[ Style * MOVE_STYLE_ANIM_COUNT ];
    }
    
    // Only valid if all of these animations are present
    if(     ( AnimIndices[ MOVE_STYLE_ANIM_IDLE            ] != -1 )
        &&  ( AnimIndices[ MOVE_STYLE_ANIM_IDLE_TURN_LEFT  ] != -1 )
        &&  ( AnimIndices[ MOVE_STYLE_ANIM_IDLE_TURN_RIGHT ] != -1 )
        &&  ( AnimIndices[ MOVE_STYLE_ANIM_MOVE_FRONT      ] != -1 ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

loco::move_style loco::GetValidMoveStyle( loco::move_style Style )
{
    switch(Style)
    {
    case MOVE_STYLE_WALK:
        if( IsValidMoveStyle(MOVE_STYLE_WALK) )
        {        
            return MOVE_STYLE_WALK;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_PROWL) )
        {        
            return MOVE_STYLE_PROWL;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_CROUCH) )
        {        
            return MOVE_STYLE_CROUCH;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_RUN) )
        {        
            return MOVE_STYLE_RUN;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE) )
        {        
            return MOVE_STYLE_CHARGE;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE_FAST) )
        {        
            return MOVE_STYLE_CHARGE_FAST;
        }               
        break;
        
        // run aim tries to get itself first, if not found it's the same as run.
    case MOVE_STYLE_RUNAIM:    
        if( IsValidMoveStyle(MOVE_STYLE_RUNAIM) )
        {        
            return MOVE_STYLE_RUNAIM;
        }
        // DON'T BREAK -- Fall through and try the next group...

    case MOVE_STYLE_RUN:    
        if( IsValidMoveStyle(MOVE_STYLE_RUN) )
        {        
            return MOVE_STYLE_RUN;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_PROWL) )
        {        
            return MOVE_STYLE_PROWL;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_WALK) )
        {        
            return MOVE_STYLE_WALK;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_CROUCH) )
        {        
            return MOVE_STYLE_CROUCH;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE) )
        {        
            return MOVE_STYLE_CHARGE;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE_FAST) )
        {        
            return MOVE_STYLE_CHARGE_FAST;
        }               
        break ;
    case MOVE_STYLE_PROWL:
        // Set animation indices
        if( IsValidMoveStyle(MOVE_STYLE_PROWL) )
        {        
            return MOVE_STYLE_PROWL;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_WALK) )
        {        
            return MOVE_STYLE_WALK;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_CROUCH) )
        {        
            return MOVE_STYLE_CROUCH;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_RUN) )
        {        
            return MOVE_STYLE_RUN;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE) )
        {        
            return MOVE_STYLE_CHARGE;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE_FAST) )
        {        
            return MOVE_STYLE_CHARGE_FAST;
        }               
        break;
        
    case MOVE_STYLE_CROUCHAIM:
        if( IsValidMoveStyle(MOVE_STYLE_CROUCHAIM) )
        {        
            return MOVE_STYLE_CROUCHAIM;
        }
       
        // DON'T BREAK -- Fall through and try the next group...
        
    case MOVE_STYLE_CROUCH:
        if( IsValidMoveStyle(MOVE_STYLE_CROUCH) )
        {        
            return MOVE_STYLE_CROUCH;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_PROWL) )
        {        
            return MOVE_STYLE_PROWL;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_WALK) )
        {        
            return MOVE_STYLE_WALK;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_RUN) )
        {        
            return MOVE_STYLE_RUN;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE) )
        {        
            return MOVE_STYLE_CHARGE;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE_FAST) )
        {        
            return MOVE_STYLE_CHARGE_FAST;
        }               
        break ;
    case MOVE_STYLE_CHARGE_FAST:
        if( IsValidMoveStyle(MOVE_STYLE_CHARGE_FAST) )
        {        
            return MOVE_STYLE_CHARGE_FAST;
        }               
        // DON'T BREAK -- Fall through and try the next group...
        
    case MOVE_STYLE_CHARGE:
        if( IsValidMoveStyle(MOVE_STYLE_CHARGE) )
        {        
            return MOVE_STYLE_CHARGE;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_CHARGE_FAST) )
        {        
            return MOVE_STYLE_CHARGE_FAST;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_RUN) )
        {        
            return MOVE_STYLE_RUN;
        }               
        else if( IsValidMoveStyle(MOVE_STYLE_PROWL) )
        {        
            return MOVE_STYLE_PROWL;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_WALK) )
        {        
            return MOVE_STYLE_WALK;
        }
        else if( IsValidMoveStyle(MOVE_STYLE_CROUCH) )
        {        
            return MOVE_STYLE_CROUCH;
        }
        break;
    default:
        ASSERTS(0, "You passed in an invalid style!");
        break;
    }

    return MOVE_STYLE_NULL;
}

//=========================================================================

void loco::SwitchMoveStyleSmoothly( move_style Style, move_style_anim MoveStyleAnim )
{
    // Skip if move style is not present
    if( !IsValidMoveStyle( Style ) )
        return;
 
    // Lookup current anim info
    f32 OldFrame = m_Player.GetCurrAnim().GetFrameParametric() ;
    
    // Setup the new move style
    SetMoveStyle( Style );

    // Lookup new animation
    s32 iNewAnim = m_MoveStyleInfo.m_iAnims[ MoveStyleAnim ];
    if( iNewAnim != -1 )
    {
        // Blend to new anim, but jump in at exactly the same frame so it's seamless!
        m_Player.SetAnim( m_MoveStyleInfo.m_hAnimGroup, iNewAnim, 0.3f ) ;
        m_Player.GetCurrAnim().SetFrameParametric( OldFrame ) ;
    }
}

//=========================================================================

inline
f32 Interp( f32 T0, f32 T1, f32 T )
{
    // When T=0, return value is T0
    // When T=1, return value is T1
    return T0 + ( T * ( T1 - T0 ) );
}

//=========================================================================

void loco::UpdateMoveStyle( void )
{
    // Lookup current move style animation (if any)
    s32             iAnim         = m_Player.GetCurrAnim().GetAnimTypeIndex();
    move_style_anim MoveStyleAnim = GetCurrentMoveStyleAnim();
    
    // Not playing a move style animation?
    if( MoveStyleAnim == MOVE_STYLE_ANIM_NULL )
    {  
        // Turn off the mixer!
        m_Player.GetCurrAnim().SetMixAnim( -1, 0.0f ); 
        
        // Nothing else to do...
        return;
    }
            
    // Update switching between aiming and non-aiming move styles?
    if( !m_Player.IsBlending() )
    {
        // Switch to an aiming style?
        if( m_bUseAimMoveStyles )
        {
            // Only switch when not blending and running!
            if( m_MoveStyle == MOVE_STYLE_RUN && IsValidMoveStyle( MOVE_STYLE_RUNAIM ) )
            {
                // change us over to the new move style... 
                SwitchMoveStyleSmoothly( MOVE_STYLE_RUNAIM, MoveStyleAnim );
            }
            else if( m_MoveStyle == MOVE_STYLE_CROUCH && IsValidMoveStyle( MOVE_STYLE_CROUCHAIM ) )
            {
                // change us over to the new move style... 
                SwitchMoveStyleSmoothly( MOVE_STYLE_CROUCHAIM, MoveStyleAnim );
            }
        }
        else
        {        
            // Switch to a non-aiming move style
            if( m_MoveStyle == MOVE_STYLE_RUNAIM && IsValidMoveStyle( MOVE_STYLE_RUN ) )
            {
                // change us over to the new move style... 
                SwitchMoveStyleSmoothly( MOVE_STYLE_RUN, MoveStyleAnim );
            }
            else if( m_MoveStyle == MOVE_STYLE_CROUCHAIM && IsValidMoveStyle( MOVE_STYLE_CROUCH ) )
            {
                // change us over to the new move style... 
                SwitchMoveStyleSmoothly( MOVE_STYLE_CROUCH, MoveStyleAnim );
            }
        }
    }
    
    // Set the motion mixer animation?
    s32 iMixAnim = -1;
    if(         ( m_BlendMoveStyle       != MOVE_STYLE_NULL ) 
            &&  ( m_BlendMoveStyle       != m_MoveStyle     )
            &&  ( m_BlendMoveStyleAmount != 0.0f            ) )
    {
        // Lookup blend move style animation index
        iMixAnim = GetMoveStyleAnimIndex( m_BlendMoveStyle, MoveStyleAnim );
    }
        
    // Turn on/off mixer for this motion controller?
    if( ( iMixAnim != -1 ) && ( iMixAnim != iAnim ) )
        m_Player.GetCurrAnim().SetMixAnim( iMixAnim, m_BlendMoveStyleAmount );
    else
        m_Player.GetCurrAnim().SetMixAnim( -1, 0.0f ); 
}

//=========================================================================

void loco::SetMoveStyle( loco::move_style InputStyle )
{    
    // Map to valid style
    loco::move_style Style = GetValidMoveStyle( InputStyle );

    // Make sure style is valid
    if( ( Style <= MOVE_STYLE_NULL ) || ( Style >= MOVE_STYLE_COUNT ) )
    {
        Style = MOVE_STYLE_WALK;
    }

    // If switching to a new move style and moving, then frame match when starting the new anim
    // (this makes the theta walk/run -> charge/chargefast blend look really smooth)
    m_bFrameMatchMoveAnim =    ( Style != m_MoveStyle ) 
                            && ( GetState() == loco::STATE_MOVE ) 
                            && ( m_Motion >= MOTION_FRONT )
                            && ( m_Motion <= MOTION_RIGHT ); 

    // Set default anim group and animation indices
    s16* pIndices = &m_AnimLookupTable.m_Index[ Style * MOVE_STYLE_ANIM_COUNT ];
    m_MoveStyleInfo.m_hAnimGroup = m_hAnimGroup;
    ASSERT( sizeof( m_MoveStyleInfo.m_iAnims[0] ) == sizeof( m_AnimLookupTable.m_Index[0] ) );
    x_memcpy( m_MoveStyleInfo.m_iAnims, pIndices, sizeof( m_MoveStyleInfo.m_iAnims ) );

    // Setup defaults
    m_MoveStyleInfo.InitDefaults( m_MoveStyleInfoDefault[ Style ] );

    // Record new move style
    m_MoveStyle = Style ;

    // If this is the first time this has been called since loco::OnInit 
    // (blend anim will have an index of -1), then initialize the animation
    // player with the move style idle pose.
    if ( m_Player.GetBlendAnim().GetAnimIndex() == -1 )
    {
        s32 iIdle = m_MoveStyleInfo.m_iAnims[ MOVE_STYLE_ANIM_IDLE ];
        if( iIdle != -1 )
        {
            // Start anim with "no blend" flag just in case some idle anims override the blend time!
            m_Player.SetAnim( m_MoveStyleInfo.m_hAnimGroup,     // hAnimGroup
                              iIdle,                            // iAnim
                              0.0f,                             // BlendTime
                              1.0f,                             // Rate
                              loco::ANIM_FLAG_DO_NO_BLENDING ); // Flags
        }            
    }            
}

//=========================================================================

void loco::SetBlendMoveStyle( move_style Style )
{
    // Lookup valid style?
    if( Style != MOVE_STYLE_NULL )
        m_BlendMoveStyle = GetValidMoveStyle( Style );
    else        
        m_BlendMoveStyle = Style;
}

//=========================================================================

void loco::SetBlendMoveStyleAmount( f32 Amount )
{
    // Update move style info?
    if(     ( m_BlendMoveStyle != MOVE_STYLE_NULL )
        &&  ( m_BlendMoveStyle != m_MoveStyle )
        &&  ( m_BlendMoveStyleAmount != Amount ) )
    {
        // Keep new mix amount
        m_BlendMoveStyleAmount = Amount;
        
        // Lookup default settings
        const move_style_info_default& A = m_MoveStyleInfoDefault[ m_MoveStyle ];
        const move_style_info_default& B = m_MoveStyleInfoDefault[ m_BlendMoveStyle ];

        // Blend movement related
        m_MoveStyleInfo.m_IdleBlendTime          = Interp( A.m_IdleBlendTime,          B.m_IdleBlendTime,          m_BlendMoveStyleAmount );
        m_MoveStyleInfo.m_MoveBlendTime          = Interp( A.m_MoveBlendTime,          B.m_MoveBlendTime,          m_BlendMoveStyleAmount );
        m_MoveStyleInfo.m_FromPlayAnimBlendTime  = Interp( A.m_FromPlayAnimBlendTime,  B.m_FromPlayAnimBlendTime,  m_BlendMoveStyleAmount );
        m_MoveStyleInfo.m_MoveTurnRate           = Interp( A.m_MoveTurnRate,           B.m_MoveTurnRate,           m_BlendMoveStyleAmount );
    }
}

//=========================================================================

void loco::SetMoveStyleDefaults( move_style Style, const loco::move_style_info_default& Defaults )
{
    if ((Style <= MOVE_STYLE_NULL) || (Style >=MOVE_STYLE_COUNT))
    {
        ASSERTS(0, "You passed in an invalid style!") ;
    }

    m_MoveStyleInfoDefault[ Style ] = Defaults;    
}

//=========================================================================

void loco::SetAimerBlendSpeed( f32 AimerBlendSpeed )
{
    // Scale by property
    if( m_pGeom )
    {
        f32 PropertyBlendSpeed = 1.0f;
        
        // TO DO: Store this "m_PropertyBlendSpeed" in the init function
        const geom::property_section* pSection = m_pGeom->FindPropertySection( "AIMER" );
        m_pGeom->GetPropertyFloat( pSection, "BlendSpeed", &PropertyBlendSpeed );
        AimerBlendSpeed *= PropertyBlendSpeed;
    }

    m_MoveStyleInfo.m_AimerBlendSpeed = x_max( 0.1f, AimerBlendSpeed );
}

//=========================================================================

void loco::SetAimerWeight( f32 Weight, f32 BlendTime )
{
    GetAimController().SetWeight(Weight, BlendTime);
}

//=========================================================================
// Ghost mode functions
//=========================================================================

void loco::SetGhostMode( xbool bEnable )
{
    // Store
    m_bGhostMode = bEnable;

    // Always get right onto moveat
    m_ArriveDistSqr = 0.0f;
        
    // Allow aiming
    SetUseAimMoveStyles( TRUE );
}

//=========================================================================
// Misc functions
//=========================================================================

void loco::ComputeHeadAim( radian FacingYawBias, radian& H, radian& V )
{
    // Compute world look yaw 
    // (use npc position instead of eye position otherwise when the tank walks
    //  his eyes move all over the place causing the aimer to oscillate)
    vector3 DeltaLook = GetHeadLookAt() - GetPosition();
    radian  LookYaw   = DeltaLook.GetYaw();

    // Compute world current yaw ( taking aimer into account )
    radian  CurrYaw   = GetYaw() + FacingYawBias;

    // Compute delta look yaw and pitch
    H = x_MinAngleDiff( LookYaw, CurrYaw );

    // Compute world pitch
    DeltaLook = GetHeadLookAt() - GetEyePosition();
    V = DeltaLook.GetPitch();
}

//=========================================================================

void loco::ComputeBodyAim( radian FacingYawBias, radian& H )
{
    // Compute world look yaw 
    // (use npc position instead of eye position otherwise when the tank walks
    //  his eyes move all over the place causing the aimer to oscillate)
    vector3 DeltaLook = GetBodyLookAt() - GetPosition();
    radian  LookYaw   = DeltaLook.GetYaw();

    // Compute world current yaw ( taking aimer into account )
    radian  CurrYaw   = GetYaw() + FacingYawBias;

    // Compute delta look yaw and pitch
    H = x_MinAngleDiff( LookYaw, CurrYaw );
}

//=========================================================================

radian loco::ComputeMoveDir( void )
{
    vector3 DesiredDir = m_MoveAt - m_Player.GetPosition();
    return DesiredDir.GetYaw() ;
}

//=========================================================================

radian loco::GetMotionYaw( loco::motion Motion ) const
{
    switch(Motion)
    {
        case loco::MOTION_FRONT:    return DEG_TO_RAD(180) ;
        case loco::MOTION_LEFT:     return DEG_TO_RAD(270) ;
        case loco::MOTION_RIGHT:    return DEG_TO_RAD(90)  ;
        case loco::MOTION_BACK:     return DEG_TO_RAD(0)   ;
    }

    return 0 ;
}

//=========================================================================

const char* loco::GetMotionName( motion Motion ) const
{
#define CASE_MOTION(__motion__) case __motion__: return #__motion__ ;
    
    switch(Motion)
    {
        CASE_MOTION(MOTION_FRONT) ;

        CASE_MOTION(MOTION_LEFT) ;
        CASE_MOTION(MOTION_BACK) ;
        CASE_MOTION(MOTION_RIGHT) ;

        CASE_MOTION(MOTION_IDLE) ;
        CASE_MOTION(MOTION_IDLE_TURN_LEFT) ;
        CASE_MOTION(MOTION_IDLE_TURN_RIGHT) ;
        CASE_MOTION(MOTION_IDLE_TURN_180) ;

        CASE_MOTION(MOTION_TRANSITION) ;

    default:
        CASE_MOTION(MOTION_NULL) ;
    }
}

//=========================================================================

vector3 LineIntersectLine( const vector3& P1, const vector3& P2,
                           const vector3& P3, const vector3& P4 )
{
    f32 D = (P4.GetX() - P3.GetX())*(P1.GetZ() - P3.GetZ()) - (P4.GetZ() - P3.GetZ())*(P1.GetX() - P3.GetX()) ;
    if (D == 0)
        return vector3(0,0,0) ;
    f32 Q = (P4.GetZ() - P3.GetZ())*(P2.GetX() - P1.GetX()) - (P4.GetX() - P3.GetX())*(P2.GetZ() - P1.GetZ()) ;

    f32 T = Q / D ;

    vector3 P( P1.GetX() + T * (P2.GetX() - P1.GetX()),
               0.0f,
               P1.GetZ() + T * (P2.GetZ() - P1.GetZ()) );
    return P ;
}

//=========================================================================

vector3 ClosestPointOnLine( const vector3& P, 
                            const vector3& P0, const vector3& P1 )
{
    vector3 v = P1 - P0;
    vector3 w = P - P0;

    f32 c1 = w.Dot(v) ;
    if ( c1 <= 0 )
        return P0 ;

    f32 c2 = v.Dot(v) ;
    if ( c2 <= c1 )
        return P1 ;

    f32 b = c1 / c2;
    vector3 Pb = P0 + b * v;
    return Pb ;
}

//=========================================================================

vector3 ClosestPointOnInfiniteLine( const vector3& P, 
                                    const vector3& P0, const vector3& P1 )
{
    vector3 v = P1 - P0;
    vector3 w = P - P0;

    f32 c1 = w.Dot(v) ;
    f32 c2 = v.Dot(v) ;

    f32 b = c1 / c2;
    vector3 Pb = P0 + b * v;
    return Pb ;
}

//=========================================================================

// Get point on movement line, that is closest to the look at
vector3 loco::GetMotionLookPoint( void ) const
{
    return ClosestPointOnInfiniteLine( m_HeadLookAt,
                                       m_MoveAt, m_Player.GetPosition() ) ;

    //return LineIntersectLine( m_HeadLookAt, m_Player.GetPosition(),
                              //m_MoveAt, m_Player.GetPosition() ) ;

}

//=========================================================================

void loco::SetAllowMotion( loco::motion Motion, xbool bEnable )
{
    switch( Motion )
    {
    case loco::MOTION_FRONT:    m_bAllowFrontMotion = bEnable; break;
    case loco::MOTION_LEFT:     m_bAllowLeftMotion  = bEnable; break;
    case loco::MOTION_BACK:     m_bAllowBackMotion  = bEnable; break;
    case loco::MOTION_RIGHT:    m_bAllowRightMotion = bEnable; break;
        
    default:
        ASSERTS( 0, "Invalid motion!" );        
    }
}

//=========================================================================

xbool loco::IsMotionAllowed( loco::motion Motion )
{
    switch( Motion )
    {
    case loco::MOTION_FRONT:    return m_bAllowFrontMotion;
    case loco::MOTION_LEFT:     return m_bAllowLeftMotion;
    case loco::MOTION_BACK:     return m_bAllowBackMotion;
    case loco::MOTION_RIGHT:    return m_bAllowRightMotion;

    default:
        ASSERTS( 0, "Invalid motion!" );  
        return FALSE;      
    }
}

//=========================================================================

void loco::ApplyDeltaYaw( radian DeltaYaw )
{
    m_Player.ApplyCurrAnimDeltaYaw(DeltaYaw) ;
    m_Player.ApplyBlendAnimDeltaYaw(DeltaYaw) ;
}

//=========================================================================

void loco::SetYaw( radian Yaw )
{
    // If ghost mode, store delta so we can pick the best animation
    if( m_bGhostMode )
    {
        // Store ghost actual yaw
        m_GhostYaw = Yaw;

        // Compute delta yaw from current yaw
        radian DeltaYaw    = x_MinAngleDiff( m_GhostYaw, GetYaw() );

        // Catch up ghost yaw if delta has got too big
        if( DeltaYaw > m_MoveStyleInfo.m_IdleTurnDeltaYawMax )
        {
            // Catch up
            ApplyDeltaYaw( DeltaYaw - m_MoveStyleInfo.m_IdleTurnDeltaYawMax ) ;
        }
        if( DeltaYaw < m_MoveStyleInfo.m_IdleTurnDeltaYawMin )
        {
            // Catch up
            ApplyDeltaYaw( DeltaYaw - m_MoveStyleInfo.m_IdleTurnDeltaYawMin );
        }

        // Store delta yaw so idle state can play turn anims
        m_DeltaYaw = DeltaYaw;

        // Update lookat to look infront of "move at" so that move state plays the correct animation
        vector3 Offset(0.0f, GetEyeOffset().GetY(), 100.0f);
        Offset.RotateY( m_GhostYaw );
        m_HeadLookAt = m_BodyLookAt = GetPosition() + Offset;

        // Make sure aimer is accurate
        m_AimController.SetBlendFactor( x_MinAngleDiff( m_GhostYaw, GetYaw() ), m_GhostPitch, 0.0f );
    }
    else
    {
        // Just set new yaw for normal mode
        m_Player.SetCurrAnimYaw( Yaw ) ;
    }
    
#ifdef X_EDITOR    
    // Reset "move at" and "look at" to point straight forward when editing objects
    // or initializing blue-printed objects to their anchor etc.
    // (without this the eyes look in weird positions)
    if( !g_game_running )
    {
        ResetMoveAndLookAt();
    }        
#endif    
}

//=========================================================================

void loco::SetYawFacingTarget( radian TargetYaw, radian MaxDeltaYaw )
{
    // Calculate delta yaw and cap
    radian DeltaYaw = x_MinAngleDiff(TargetYaw, m_Player.GetCurrAnimYaw()) ;
    if (DeltaYaw < -MaxDeltaYaw)
        DeltaYaw = -MaxDeltaYaw ;
    else
    if (DeltaYaw > MaxDeltaYaw)
        DeltaYaw = MaxDeltaYaw ;

    // Apply rotation to yaw
    m_Player.ApplyCurrAnimDeltaYaw( DeltaYaw ) ;
}

//=========================================================================
// Convenient local to world matrix functions
//=========================================================================

void loco::SetL2W( const matrix4& L2W )
{
    SetPosition(L2W.GetTranslation()) ;
    SetYaw(L2W.GetRotation().Yaw) ;
}

//=========================================================================

matrix4 loco::GetL2W( void )
{
    matrix4 L2W ;
    L2W.Identity() ;
    L2W.SetTranslation(GetPosition()) ;
    L2W.SetRotation(radian3(0, GetYaw(), 0)) ;
    return L2W ;
}

//=========================================================================
// Property
//=========================================================================

void  loco::OnEnumProp( prop_enum& List )
{
    // Physics
    m_Physics.OnEnumProp(List) ;

    List.PropEnumBool ( "Lookat Turns", 
                        "Do we use turns to face our lookat?",PROP_TYPE_EXPOSE);

    // Move styles                        
    List.PropEnumHeader ( "LocoStyles", "Values related to movement and animation", 0 );
    for( s32 i = MOVE_STYLE_NULL + 1; i < MOVE_STYLE_COUNT; i++ )
    {
        const char* pName = GetMoveStyleName( i );
        List.PropEnumHeader ( xfs( "LocoStyles\\%s", pName ),  xfs( "Values realted to the %s loco style", pName ), 0 );
        s32 iHeader = List.PushPath( xfs( "LocoStyles\\%s\\", pName ) );
        List.PropEnumExternal( "AnimPackage", "Resource\0anim\0", "Optional anim package to use for move style anims instead of main package", PROP_TYPE_MUST_ENUM );
        List.PopPath( iHeader );
    }
}

//=========================================================================


xbool loco::OnProperty( prop_query& I )
{
    // Physics
    if (m_Physics.OnProperty(I))
        return TRUE ;

    // Loco styles?
    if( I.IsBasePath( "LocoStyles" ) )
    {    
        s32 iHeader = I.PushPath("LocoStyles\\");

        for( s32 i = MOVE_STYLE_NULL + 1; i < MOVE_STYLE_COUNT; i++)
        {
            // If it doesn't contain the move style name then skip checking the styles
            if( !I.IsSimilarPath( GetMoveStyleName( i ) ) )
                continue;   

            s32 iStyleHeader = I.PushPath( GetMoveStyleHeader( i ) );
            move_style_info_default& Info = m_MoveStyleInfoDefault[i];

            // Anim package?
            if( I.IsVar( "AnimPackage" ) )
            {
                if( I.IsRead() )
                    I.SetVarExternal( Info.m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
                else
                {
                    if( I.GetVarExternal()[0] )
                    {
                        Info.m_hAnimGroup.SetName( I.GetVarExternal() );
                        
                        // Lookup move style anims
                        const anim_group* pAnimGroup = Info.m_hAnimGroup.GetPointer();
                        if( pAnimGroup )
                        {
                            // Init anims
                            const char* pStyle = GetMoveStyleName( i );
                            for( s32 j = 0; j < MOVE_STYLE_ANIM_COUNT; j++ )
                            {
                                const char* pAnim = GetMoveStyleAnimName( j );
                                Info.m_iAnims[j] = pAnimGroup->GetAnimIndex( xfs( "%s_%s", pStyle, pAnim ) );
                            }                                
                        }                            
                    }
                }
                return TRUE;
            }
         
            I.PopPath( iStyleHeader );       
        }                
        
        I.PopPath( iHeader );       
    }


    // Not found
    return FALSE ;
}

//=========================================================================

#ifdef X_EDITOR

//=========================================================================

s32 CheckProperty( const geom*                  pGeom,
                   const char*                  pSectionName, 
                   const char*                  pPropertyName,
                   const geom::property::type   PropertyType,
                         xstring&               ErrorMsg )
{                         
    // No geometry?
    if( !pGeom )
        return 0;
        
    // Found section?
    const geom::property_section* pSection = pGeom->FindPropertySection( pSectionName );
    if( pSection )
    {
        // Found property?
        const geom::property* pProperty = pGeom->FindProperty( pSection, pPropertyName, PropertyType );
        if( pProperty )
            return 0;
    }

    // Not found - add to errors                
    ErrorMsg += xstring( " [" ) + pSectionName + "]";
    ErrorMsg += xstring( " [" ) + pPropertyName + "]";
    switch( PropertyType )
    {
    case geom::property::TYPE_FLOAT:    ErrorMsg += " [FLOAT]";   break;
    case geom::property::TYPE_ANGLE:    ErrorMsg += " [ANGLE]";   break;
    case geom::property::TYPE_INTEGER:  ErrorMsg += " [INTEGER]"; break;
    case geom::property::TYPE_STRING:   ErrorMsg += " [STRING]";  break;
    }
    ErrorMsg += "\n";
    return 1;
}

//=========================================================================

s32 CheckBoneMasks( const geom*    pGeom,
                    const char*    pBoneMasks,
                          xstring& ErrorMsg  )
{
    // No geometry?
    if( !pGeom )
        return 0;

    // Present?
    if( pGeom->FindBoneMasks( pBoneMasks ) == NULL )
    {
        ErrorMsg += xstring( pBoneMasks ) + "\n";
        return 1;
    }
    
    return 0;
}

//=========================================================================
                   
s32 loco::OnValidateProperties( const skin_inst& SkinInst, xstring& ErrorMsg )
{
    s32 nErrors = 0;

    // Lookup geometry
    const skin_geom* pGeom = SkinInst.GetSkinGeom();

    // Clear errors
    xstring PropertyErrors;
    xstring MaskErrors;
    s32     nMaskErrors = 0;
    s32     nPropertyErrors = 0;

    // Check for property errors
    nPropertyErrors += CheckProperty( pGeom, "AIMER", "BlendSpeed"    ,   geom::property::TYPE_FLOAT, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "AIMER", "HorizMin"      ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "AIMER", "HorizMax"      ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "AIMER", "VertMin"       ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "AIMER", "VertMax"       ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "AIMER", "EyeBlendSpeed" ,   geom::property::TYPE_FLOAT, PropertyErrors );

    nPropertyErrors += CheckProperty( pGeom, "IDLE", "DeltaYawMin"          ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "IDLE", "DeltaYawMax"          ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "IDLE", "TurnDeltaYawMin"      ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "IDLE", "TurnDeltaYawMax"      ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "IDLE", "Turn180DeltaYawMin"   ,   geom::property::TYPE_ANGLE, PropertyErrors );
    nPropertyErrors += CheckProperty( pGeom, "IDLE", "Turn180DeltaYawMax"   ,   geom::property::TYPE_ANGLE, PropertyErrors );

    // Check move style properties
    for( s32 i = 0; i < loco::MOVE_STYLE_COUNT; i++ )
    {
        // Lookup info
        const char*              pMoveStyle = GetMoveStyleName( i );

        // Make sure move style properties are present
        nPropertyErrors += CheckProperty( pGeom, pMoveStyle, "IdleBlendTime"         ,   geom::property::TYPE_FLOAT, PropertyErrors );
        nPropertyErrors += CheckProperty( pGeom, pMoveStyle, "MoveBlendTime"         ,   geom::property::TYPE_FLOAT, PropertyErrors );
        nPropertyErrors += CheckProperty( pGeom, pMoveStyle, "FromPlayAnimBlendTime" ,   geom::property::TYPE_FLOAT, PropertyErrors );
        nPropertyErrors += CheckProperty( pGeom, pMoveStyle, "MoveTurnRate"          ,   geom::property::TYPE_ANGLE, PropertyErrors );
    }

    // Any property errors?
    if( nPropertyErrors )
    {
        nErrors  += nPropertyErrors;
        ErrorMsg += "The following properties are missing:\n";
        ErrorMsg += PropertyErrors;
        ErrorMsg += "\n\n";
    }

    // Check for bone masks in geometry
    nMaskErrors += CheckBoneMasks( pGeom, "FACE",           MaskErrors );
    nMaskErrors += CheckBoneMasks( pGeom, "AIM_VERT",       MaskErrors );
    nMaskErrors += CheckBoneMasks( pGeom, "AIM_HORIZ",      MaskErrors );
    nMaskErrors += CheckBoneMasks( pGeom, "NO_AIM_VERT",    MaskErrors );
    nMaskErrors += CheckBoneMasks( pGeom, "NO_AIM_HORIZ",   MaskErrors );
    nMaskErrors += CheckBoneMasks( pGeom, "RELOAD_SHOOT",   MaskErrors );
    
    // Any missing masks?
    if( nMaskErrors )
    {
        nErrors  += nMaskErrors;
        ErrorMsg += "The following bone masks are missing:\n";
        ErrorMsg += MaskErrors;
        ErrorMsg += "\n\n";
    }

    // Property or mask errors?
    if( nPropertyErrors || nMaskErrors )
    {
        ErrorMsg += "\nCheck the .skingeom resource ["
            + xstring( SkinInst.GetSkinGeomName() ) + "] :\n\n"
            + "Make sure the resource has the \"Settings\" property pointing "
            + "to a .txt settings file that contains these bone masks.\n\n"
            + "The settings files are next to the bind .matx files in the character folders.\n"
            + "If you have no idea what this means, come get me - Steve Broumley...\n";
    }

    // Check to make sure that the root bone in move left + right animations are
    // less than 180 degrees off from each other so that correct blending occurs
    for( s32 i = 0; i < loco::MOVE_STYLE_COUNT; i++ )
    {
        // Lookup move style info
        move_style_info_default& MoveStyleInfo = m_MoveStyleInfoDefault[i];

        // Default to global anim package
        const anim_group* pAnimGroup     = m_hAnimGroup.GetPointer();
        const char*       pAnimGroupName = m_hAnimGroup.GetName();
        const s16*        AnimIndices    = &m_AnimLookupTable.m_Index[ i * MOVE_STYLE_ANIM_COUNT ];

        // Use move style override package?
        if( MoveStyleInfo.m_hAnimGroup.GetPointer() )
        {
            pAnimGroup     = MoveStyleInfo.m_hAnimGroup.GetPointer();
            pAnimGroupName = MoveStyleInfo.m_hAnimGroup.GetName();
            AnimIndices    = &MoveStyleInfo.m_iAnims[0];
        }

        // Anim group loaded?
        if( pAnimGroup )
        {
            // Do move left + right animations exist?
            s32 iMoveLeft  = AnimIndices[ MOVE_STYLE_ANIM_MOVE_LEFT ];
            s32 iMoveRight = AnimIndices[ MOVE_STYLE_ANIM_MOVE_RIGHT ];
            if( ( iMoveLeft != -1 ) && ( iMoveRight != -1 ) )
            {
                // Lookup anims
                const anim_info& AnimLeft  = pAnimGroup->GetAnimInfo( iMoveLeft );
                const anim_info& AnimRight = pAnimGroup->GetAnimInfo( iMoveRight );
                    
                // Lookup 1st frame keys of root bone
                anim_key LeftKey, RightKey;
                AnimLeft.GetRawKey( 0, 0, LeftKey );
                AnimRight.GetRawKey( 0, 0, RightKey );
                
                // Compute the yaw difference between the rotations
                radian LeftYaw  = LeftKey.Rotation.GetRotation().Yaw;
                radian RightYaw = RightKey.Rotation.GetRotation().Yaw;
                radian DeltaYaw = x_MinAngleDiff( LeftYaw, RightYaw );
                
                // Angle very close to 180?
                if( x_abs( DeltaYaw ) >= R_179 )
                {
                    // Report error
                    nErrors++;
                    ErrorMsg += "\nERROR: In animation package [" + xstring( pAnimGroupName ) + "]\n";
                    ErrorMsg += "   The root bone of animations [" + xstring( AnimLeft.GetName() ) + "] and [" + xstring( AnimRight.GetName() ) + "]\n";
                    ErrorMsg += "   are 180 degrees apart - blending between these anims will cause\n";
                    ErrorMsg += "   npcs to spin the wrong direction - please rotate the root bones\n";
                    ErrorMsg += "   by 5 degrees towards the look direction to fix this problem!!\n\n";
                }
            }
        }
    }
                    
    return nErrors;
}
#endif

//=========================================================================

vector3 loco::GetBonePosition( s32 Bone ) 
{
    const s32 nBones = m_Player.GetNActiveBones();

    if ( (Bone >= 0) && Bone < nBones )
    {
        return m_Player.GetBoneL2W( Bone ).GetTranslation();
    }
    else
    {
        return m_Player.GetPosition();
    }
}

//=========================================================================

s32 loco::GetRandomBone( void ) 
{
    static char* BoneNames[] = {
        "Head",
        "Neck",
        "Shoulder",
        "Face" };

    const s32 nBones = m_Player.GetNBones();

    if ( nBones > 0 )
    {
        while ( TRUE )
        {
            s32 Bone = x_irand( 0, nBones );
            s32 i;
            for ( i = 0; i < 4; ++i )
            {
                ASSERT( m_Player.GetAnimGroup() );
                if ( x_strstr( m_Player.GetAnimGroup()->GetBone( Bone ).Name, BoneNames[i] ) )
                {
                    return Bone;
                }
            }
        }
    }
    else
    {
        return -1;
    }
}
