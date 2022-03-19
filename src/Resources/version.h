//
// Created by Gegel85 on 12/03/2022.
//

#ifndef BATTLE_VERSION_H
#define BATTLE_VERSION_H

#ifdef _DEBUG
#define VERSION_STR "alpha 0.0.3 DEBUG"
#else
#define VERSION_STR "alpha 0.0.3"
#endif
#define REPLAY_MAGIC 0xF153E8DC

unsigned getMagic();

#endif //BATTLE_VERSION_H
