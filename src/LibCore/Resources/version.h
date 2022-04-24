//
// Created by Gegel85 on 12/03/2022.
//

#ifndef SOFGV_VERSION_H
#define SOFGV_VERSION_H

#define REAL_VERSION_STR "alpha 0.0.9"
#ifdef _DEBUG
#define VERSION_STR REAL_VERSION_STR" DEBUG"
#else
#define VERSION_STR REAL_VERSION_STR
#endif
#define REPLAY_MAGIC 0xF153E8DC

unsigned getMagic();

#endif //SOFGV_VERSION_H
