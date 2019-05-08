#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tower.h"
#include "path.h"

const char *statNames[] = {
    "Damage",
    "Range",
    "Cooldown"
};

// Upgrade data
const upgradeData_t upgradeData[3][3][4] = {
    {
        { // Standard damage
            {"More Gunpowder", "Twice the bang for your buck"},
            {"Higher Caliber", "More lead = Less alien"},
            {"Explosive Bullets", "Explodes twice for\nmaximum destruction"},
            {"Lazer Pistols", "Set to \"kill\" for best results"}
        },    
        { // Standard range
            {"Longer Barrels", "You'll shoot your eye out, kid!"},
            {"Antigravity Bullets", "Made of disassembled\nalien spaceship"},
            {"Laser Scope", "Also effective at\nattracting cats"},
            {"Recursive Ammuniton", "Bullets that also fire bullets\nthat also fire bullets..."}
        },    
        { // Standard rate
            {"Double Barrel", "To shoot twice as often"},
            {"100 Bullet Cartridges", "pfft, what's reloading?"},
            {"Automatic Weapons", "Your fortress will find the\nfactory-default settings\nquite satisfactory"},
            {"More Gunmen", "Why didn't we think\nof this before?"}
        }
    },
    {
        { // Sniper damage
            {"Larger Ammo", "65% more bullet per bullet"},
            {"Headshot Training", "Wait, do the enemy\neven have heads?"},
            {"Trebuchet Mode", "Objectively better\nthan a catapult"},
            {"Laser Rifles", "The highest muzzle velocity\nyou can get"}
        },
        { // Sniper range
            {"Stairs", "Apparently troops can shoot\nfarther from the top floor"},
            {"Taller Towers", "Doesn't include safety railing"},
            {"ICBMs", "Include a passenger seat\nand a rifle slot"}
        },
        { // Sniper rate
            {"Larger Scopes", "For finding targets faster"},
            {"Silencer", "Wait, I thought this game\nhad no sound"},
            {"Camo Paint", "Makes the tower\n100% undetectable"},
            {"Quantum Reactor", "Quantumly teleports bullets\ninto enemies"}
        }
    },
    {
        { // Burst damage
            {"Hot Tar", "For best results,\ndo not microwave"},
            {"Grenades", "Aim away from the\ntower, please"},
            {"Hidden Spike Trap", "Sharper than attack"}
        },
        { // Burst range
            {"Articifial Hill", "It's over, Alien,\nI have the high ground!"},
            {"Spacetime Warping", "Abuses a 3D tesseract to\nshrink space around the tower"}
        },
        { // Burst rate
            {"Managers", "Reduce the amount of time\nspent on coffee breaks"},
            {"Enhanced Scheduling", "Prcecisely timed for\nmaximum destruction"},
            {"Conveyor Belts", "Nobody knows what they do,\njust like this game's code"}
        }
    }
};

// Formula for damage and range is baseStats[type][stat] + statMultipliers[type][stat] * (level + 1)

// Cooldown is not multiplied and thus does not include a multiplier
const uint24_t statMultipliers[3][2] = {
    { // Standard stats
        3,  // Damage
        25 // Range
    },
    { // Sniper stats
        15,  // Damage
        60  // Range
    },
    { // Burst stats
        1,  // Damage
        15 // Range
    }
};

const uint24_t baseStats[3][3] = {
    {
        0,
        TOWER_RADIUS + PATH_WIDTH / 2,
        15
    },
    {
        15,
        TOWER_RADIUS + PATH_WIDTH / 2,
        150
    },
    {
        1,
        TOWER_RADIUS + PATH_WIDTH / 2,
        20
    }
};
