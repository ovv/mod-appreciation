SET @Entry := 6000000;

DELETE FROM `npc_text` WHERE `ID` BETWEEN @Entry AND @Entry+4;
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES
(@Entry, 'A character boost is available for you if you want.'),
(@Entry+1, 'Sorry, there is no character boost available for now.'),
(@Entry+2, 'Sorry, you cannot use a character boost.'),
(@Entry+3, 'Which specialization would you like?'),
(@Entry+4, 'Are you absolutely certain you want to do this? I have to destroy all of your current equipment so make sure you save anything you do not want to lose!');

DELETE FROM `creature_template` WHERE `entry`=@Entry;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `unit_class`, `unit_flags`, `type`, `flags_extra`, `ScriptName`) VALUES
(@Entry, 'Ambert Mendenhall', 'Character Boost', 'Speak', 60, 60, 35, 1, 1, 2, 7, 16777218, 'npc_appreciation');

DELETE FROM `creature_template_model` WHERE `CreatureID`=@Entry;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(@Entry, 0, 21697, 1, 1);

DELETE FROM `creature` WHERE `id1`=@Entry;
INSERT INTO `creature` (`id1`, `map`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES
(@Entry, 0, -8801.2, 629.974, 94.2407, 3.06088), -- Stormwind City
(@Entry, 1, 1674.8527, -4423.74, 18.665726, 2.53), -- Orgrimmar

(@Entry, 1, -592.0922, -4530.223, 41.330013, 1.0327661), -- Valley of trials (orc/troll)
(@Entry, 1, -3085.2869, 67.22118, 78.43654, 4.713962), -- Red cloud mesa (tauren)
(@Entry, 0, 2138.1467, 1300.3608, 53.803986, 1.995867), -- Deathknell (undead)
(@Entry, 530, 10022.481, -6427.6157, 5.0905695, 0.7976799), -- Sunstrider isle (blood elf)

(@Entry, 0, -9069.163, -38.800995, 87.884026, 5.793437), -- Northshire (human)
(@Entry, 0, -6251.5654, 163.01076, 423.62302, 1.8507006), -- Coldridge Valley (gnome/dwarf)
(@Entry, 1, 10214.945, 690.93414, 1360.4836, 0.54187703), -- Shadowglen (elf)
(@Entry, 530, -4257.718, -13256.729, 57.671722, 5.077601); -- Ammen Vale (draenei)