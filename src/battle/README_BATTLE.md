# Battle Module (Not Wired To Window Yet)

Selected complete zombie set:
- football -> Walk + Attack + Die
  - res\\images\\FootballZombieWalk.gif
  - res\\images\\FootballZombieAttack.gif
  - res\\images\\FootballZombieDie.gif

New code files:
- src\\battle\\BattleTypes.hpp
- src\\battle\\StatsDatabase.hpp
- src\\battle\\StatsDatabase.cpp
- src\\battle\\BattleSimulator.hpp
- src\\battle\\BattleSimulator.cpp

Config files:
- res\\config\\plant_stats.csv
- res\\config\\zombie_stats.csv
- res\\config\\zombie_animations.csv

Usage note:
- BattleSimulator::initialize(...) reads the csv files.
- This module is currently standalone and not hooked into scenes/window.
