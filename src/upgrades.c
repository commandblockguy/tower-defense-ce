#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tower.h"

// Upgrade data
// STANDARD type damage upgrades
const upgradeData_t stdDmg[] = {
    {"More Gunpowder", "Twice the bang for your buck"},
    {"Higher Caliber", "More lead = Less alien"},
    {"Explosive Bullets", "Explodes twice for maximum destruction"},
    {"Lazer Pistols", "Set to \"kill\" for best results"}
};

// STANDARD type range upgrades
const upgradeData_t stdRange[] = {
    {"Longer Barrels", "You'll shoot your eye out, kid!"},
    {"Antigravity Bullets", "Made of disassembled alien spaceship"},
    {"Laser Scope", "Also effective at attracting cats"},
    {"Recursive Ammuniton", "Bullets that also fire bullets that also fire bullets..."}
};

// STANDARD type firing rate upgrades
const upgradeData_t stdRate[] = {
    {"Double Barrel", "To shoot twice as often"},
    {"100 Bullet Cartridges", "pfft, what's reloading?"},
    {"Automatic Weapons", "Your fortress will find the factory-default settings quite satisfactory"},
    {"More Gunmen", "Why didn't we think of this before?"}
};

const upgradeData_t snipeDmg[] = {
    {"Larger Ammo", "65% more bullet per bullet"},
    {"Headshot Training", "Wait, do the enemy even have those?"},
    {"Trebuchet", "Objectively better than a catapult"},
    {"Laser Rifles", "The highest muzzle velocity you can get"}
};

const upgradeData_t snipeRange[] = {
    {"Stairs", "Apparently troops can shoot farther from the top floor"},
    {"Taller Towers", "Does not include safety railing"},
    {"ICBMs", "Include a passenger seat and a rifle slot"}
};

const upgradeData_t snipeRate[] = {
    {"Larger Scopes", "For finding targets faster"},
    {"Silencer", "Wait, I thought this game had no sound"},
    {"Camo Paint", "Makes the tower 100% undetectable"},
    {"Quantum Reactor", "Quantumly teleports bullets into enemies"}
};

const upgradeData_t burstDmg[] = {
    {"Hot Tar", "For best results, do not microwave"},
    {"Grenades", "Aim away from the tower, please"},
    {"Hidden Spike Trap", "Sharper than attack"}
};

const upgradeData_t burstRange[] = {
    {"Articifial Hill", "It's over, Alien, I have the high ground!"},
    {"Spacetime Warping", "Abuses a 3D tesseract to shrink space around the tower"}
};

const upgradeData_t burstRate[] = {
    {"Managers", "Reduce the amount of time spent on coffee breaks"},
    {"Enhanced Scheduling", "Prcecisely timed for maximum destruction"},
    {"Conveyor Belts", "Nobody knows what they do, just like this game's code"}
};
