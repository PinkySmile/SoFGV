from chr_data_generator import gen_data
from jinja2 import Environment, FileSystemLoader
import sys
import json
import os

ACTION_IDLE = 0
ACTION_CROUCHING = 1
ACTION_CROUCH = 2
ACTION_STANDING_UP = 3
ACTION_WALK_FORWARD = 4
ACTION_WALK_BACKWARD = 5
ACTION_FORWARD_DASH = 6
ACTION_BACKWARD_DASH = 7
ACTION_NEUTRAL_JUMP = 8
ACTION_FORWARD_JUMP = 9
ACTION_BACKWARD_JUMP = 10
ACTION_NEUTRAL_HIGH_JUMP = 11
ACTION_FORWARD_HIGH_JUMP = 12
ACTION_BACKWARD_HIGH_JUMP = 13
ACTION_FALLING = 14
ACTION_LANDING = 15
ACTION_AIR_DASH_1 = 16
ACTION_AIR_DASH_2 = 17
ACTION_AIR_DASH_3 = 18
ACTION_AIR_DASH_4 = 19
ACTION_AIR_DASH_6 = 20
ACTION_AIR_DASH_7 = 21
ACTION_AIR_DASH_8 = 22
ACTION_AIR_DASH_9 = 23
ACTION_AIR_TRANSFORM = 24
ACTION_GROUND_HIGH_NEUTRAL_BLOCK = 25
ACTION_GROUND_HIGH_SPIRIT_PARRY = 26
ACTION_GROUND_HIGH_MATTER_PARRY = 27
ACTION_GROUND_HIGH_VOID_PARRY = 28
ACTION_GROUND_HIGH_HIT = 29
ACTION_GROUND_LOW_NEUTRAL_BLOCK = 30
ACTION_GROUND_LOW_SPIRIT_PARRY = 31
ACTION_GROUND_LOW_MATTER_PARRY = 32
ACTION_GROUND_LOW_VOID_PARRY = 33
ACTION_GROUND_LOW_HIT = 34
ACTION_AIR_NEUTRAL_BLOCK = 35
ACTION_AIR_SPIRIT_PARRY = 36
ACTION_AIR_MATTER_PARRY = 37
ACTION_AIR_VOID_PARRY = 38
ACTION_AIR_HIT = 39
ACTION_BEING_KNOCKED_DOWN = 40
ACTION_KNOCKED_DOWN = 41
ACTION_NEUTRAL_TECH = 42
ACTION_BACKWARD_TECH = 43
ACTION_FORWARD_TECH = 44
ACTION_FALLING_TECH = 45
ACTION_UP_AIR_TECH = 46
ACTION_DOWN_AIR_TECH = 47
ACTION_FORWARD_AIR_TECH = 48
ACTION_BACKWARD_AIR_TECH = 49
ACTION_AIR_TECH_LANDING_LAG = 50
ACTION_UNTRANSFORM = 51
ACTION_GROUND_SLAM = 52
ACTION_WALL_SLAM = 53
ACTION_HARD_LAND = 54
ACTION_NEUTRAL_AIR_JUMP = 55
ACTION_FORWARD_AIR_JUMP = 56
ACTION_BACKWARD_AIR_JUMP = 57
ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK = 58
ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK = 59
ACTION_AIR_NEUTRAL_WRONG_BLOCK = 60
ACTION_GROUND_HIGH_NEUTRAL_PARRY = 61
ACTION_GROUND_LOW_NEUTRAL_PARRY = 62
ACTION_AIR_NEUTRAL_PARRY = 63
ACTION_5N = 100
ACTION_6N = 101
ACTION_8N = 102
ACTION_3N = 103
ACTION_2N = 104
ACTION_214N = 105
ACTION_236N = 106
ACTION_NEUTRAL_OVERDRIVE = 107
ACTION_623N = 108
ACTION_41236N = 109
ACTION_63214N = 110
ACTION_6321469874N = 111
ACTION_j5N = 112
ACTION_j6N = 113
ACTION_j8N = 114
ACTION_j3N = 115
ACTION_j2N = 116
ACTION_j214N = 117
ACTION_j236N = 118
ACTION_NEUTRAL_AIR_OVERDRIVE = 119
ACTION_j623N = 120
ACTION_j41236N = 121
ACTION_j63214N = 122
ACTION_j6321469874N = 123
ACTION_t5N = 124
ACTION_t6N = 125
ACTION_t8N = 126
ACTION_t2N = 127
ACTION_c28N = 128
ACTION_c46N = 129
ACTION_c64N = 130
ACTION_NEUTRAL_ROMAN_CANCEL = 131
ACTION_NEUTRAL_AIR_ROMAN_CANCEL = 132
ACTION_421N = 133
ACTION_j421N = 134
ACTION_5M = 150
ACTION_6M = 151
ACTION_8M = 152
ACTION_3M = 153
ACTION_2M = 154
ACTION_214M = 155
ACTION_236M = 156
ACTION_MATTER_OVERDRIVE = 157
ACTION_623M = 158
ACTION_41236M = 159
ACTION_63214M = 160
ACTION_6321469874M = 161
ACTION_j5M = 162
ACTION_j6M = 163
ACTION_j8M = 164
ACTION_j3M = 165
ACTION_j2M = 166
ACTION_j214M = 167
ACTION_j236M = 168
ACTION_MATTER_AIR_OVERDRIVE = 169
ACTION_j623M = 170
ACTION_j41236M = 171
ACTION_j63214M = 172
ACTION_j6321469874M = 173
ACTION_t5M = 174
ACTION_t6M = 175
ACTION_t8M = 176
ACTION_t2M = 177
ACTION_c28M = 178
ACTION_c46M = 179
ACTION_c64M = 180
ACTION_MATTER_ROMAN_CANCEL = 181
ACTION_MATTER_AIR_ROMAN_CANCEL = 182
ACTION_421M = 183
ACTION_j421M = 184
ACTION_5S = 200
ACTION_6S = 201
ACTION_8S = 202
ACTION_3S = 203
ACTION_2S = 204
ACTION_214S = 205
ACTION_236S = 206
ACTION_SPIRIT_OVERDRIVE = 207
ACTION_623S = 208
ACTION_41236S = 209
ACTION_63214S = 210
ACTION_6321469874S = 211
ACTION_j5S = 212
ACTION_j6S = 213
ACTION_j8S = 214
ACTION_j3S = 215
ACTION_j2S = 216
ACTION_j214S = 217
ACTION_j236S = 218
ACTION_SPIRIT_AIR_OVERDRIVE = 219
ACTION_j623S = 220
ACTION_j41236S = 221
ACTION_j63214S = 222
ACTION_j6321469874S = 223
ACTION_t5S = 224
ACTION_t6S = 225
ACTION_t8S = 226
ACTION_t2S = 227
ACTION_c28S = 228
ACTION_c46S = 229
ACTION_c64S = 230
ACTION_SPIRIT_ROMAN_CANCEL = 231
ACTION_SPIRIT_AIR_ROMAN_CANCEL = 232
ACTION_421S = 233
ACTION_j421S = 234
ACTION_5V = 250
ACTION_6V = 251
ACTION_8V = 252
ACTION_3V = 253
ACTION_2V = 254
ACTION_214V = 255
ACTION_236V = 256
ACTION_VOID_OVERDRIVE = 257
ACTION_623V = 258
ACTION_41236V = 259
ACTION_63214V = 260
ACTION_6321469874V = 261
ACTION_j5V = 262
ACTION_j6V = 263
ACTION_j8V = 264
ACTION_j3V = 265
ACTION_j2V = 266
ACTION_j214V = 267
ACTION_j236V = 268
ACTION_VOID_AIR_OVERDRIVE = 269
ACTION_j623V = 270
ACTION_j41236V = 271
ACTION_j63214V = 272
ACTION_j6321469874V = 273
ACTION_t5V = 274
ACTION_t6V = 275
ACTION_t8V = 276
ACTION_t2V = 277
ACTION_c28V = 278
ACTION_c46V = 279
ACTION_c64V = 280
ACTION_VOID_ROMAN_CANCEL = 281
ACTION_VOID_AIR_ROMAN_CANCEL = 282
ACTION_421V = 283
ACTION_j421V = 284
ACTION_5A = 300
ACTION_6A = 301
ACTION_8A = 302
ACTION_3A = 303
ACTION_2A = 304
ACTION_214A = 305
ACTION_236A = 306
ACTION_421A = 307
ACTION_623A = 308
ACTION_41236A = 309
ACTION_63214A = 310
ACTION_6321469874A = 311
ACTION_j5A = 312
ACTION_j6A = 313
ACTION_j8A = 314
ACTION_j3A = 315
ACTION_j2A = 316
ACTION_j214A = 317
ACTION_j236A = 318
ACTION_j421A = 319
ACTION_j623A = 320
ACTION_j41236A = 321
ACTION_j63214A = 322
ACTION_j6321469874A = 323
ACTION_t5A = 324
ACTION_t6A = 325
ACTION_t8A = 326
ACTION_t2A = 327
ACTION_c28A = 328
ACTION_c46A = 329
ACTION_c64A = 330
ACTION_214D = 350
ACTION_236D = 351
ACTION_421D = 352
ACTION_623D = 353
ACTION_41236D = 354
ACTION_63214D = 355
ACTION_6321469874D = 356
ACTION_j214D = 357
ACTION_j236D = 358
ACTION_j421D = 359
ACTION_j623D = 360
ACTION_j41236D = 361
ACTION_j63214D = 362
ACTION_j6321469874D = 363
ACTION_t5D = 364
ACTION_t6D = 365
ACTION_t8D = 366
ACTION_t2D = 367
ACTION_WIN_MATCH1 = 400
ACTION_WIN_MATCH2 = 401
ACTION_WIN_MATCH3 = 402
ACTION_WIN_MATCH4 = 403
ACTION_GAME_START1 = 404
ACTION_GAME_START2 = 405
ACTION_GAME_START3 = 406
ACTION_GAME_START4 = 407
ACTION_GROUND_HIGH_REVERSAL = 450
ACTION_GROUND_LOW_REVERSAL = 451
ACTION_AIR_REVERSAL = 452

names = {
    ACTION_IDLE:                           "Idle",
    ACTION_CROUCHING:                      "Crouching",
    ACTION_CROUCH:                         "Crouch",
    ACTION_STANDING_UP:                    "Standing up",
    ACTION_WALK_FORWARD:                   "Walk forward",
    ACTION_WALK_BACKWARD:                  "Walk backward",
    ACTION_FORWARD_DASH:                   "Forward dash",
    ACTION_BACKWARD_DASH:                  "Backward dash",
    ACTION_NEUTRAL_JUMP:                   "Neutral jump",
    ACTION_FORWARD_JUMP:                   "Forward jump",
    ACTION_BACKWARD_JUMP:                  "Backward jump",
    ACTION_NEUTRAL_HIGH_JUMP:              "Neutral high jump",
    ACTION_FORWARD_HIGH_JUMP:              "Forward high jump",
    ACTION_BACKWARD_HIGH_JUMP:             "Backward high jump",
    ACTION_FALLING:                        "Falling",
    ACTION_LANDING:                        "Landing",
    ACTION_AIR_DASH_1:                     "Up back air dash",
    ACTION_AIR_DASH_2:                     "Up air dash",
    ACTION_AIR_DASH_3:                     "Up forward air dash",
    ACTION_AIR_DASH_4:                     "Back air dash",
    ACTION_AIR_DASH_6:                     "Forward air dash",
    ACTION_AIR_DASH_7:                     "Down back air dash",
    ACTION_AIR_DASH_8:                     "Down air dash",
    ACTION_AIR_DASH_9:                     "Down forward air dash",
    ACTION_AIR_TRANSFORM:                  "Air transform",
    ACTION_GROUND_HIGH_NEUTRAL_BLOCK:      "Ground high neutral block",
    ACTION_GROUND_HIGH_NEUTRAL_PARRY:      "Ground high neutral parry",
    ACTION_GROUND_HIGH_SPIRIT_PARRY:       "Ground high spirit parry",
    ACTION_GROUND_HIGH_MATTER_PARRY:       "Ground high matter parry",
    ACTION_GROUND_HIGH_VOID_PARRY:         "Ground high void parry",
    ACTION_GROUND_HIGH_HIT:                "Ground high hit",
    ACTION_GROUND_LOW_NEUTRAL_BLOCK:       "Ground low neutral block",
    ACTION_GROUND_LOW_NEUTRAL_PARRY:       "Ground low neutral parry",
    ACTION_GROUND_LOW_SPIRIT_PARRY:        "Ground low spirit parry",
    ACTION_GROUND_LOW_MATTER_PARRY:        "Ground low matter parry",
    ACTION_GROUND_LOW_VOID_PARRY:          "Ground low void parry",
    ACTION_GROUND_LOW_HIT:                 "Ground low hit",
    ACTION_AIR_NEUTRAL_BLOCK:              "Air neutral block",
    ACTION_AIR_NEUTRAL_PARRY:              "Air neutral parry",
    ACTION_AIR_SPIRIT_PARRY:               "Air spirit parry",
    ACTION_AIR_MATTER_PARRY:               "Air matter parry",
    ACTION_AIR_VOID_PARRY:                 "Air void parry",
    ACTION_AIR_HIT:                        "Air hit",
    ACTION_BEING_KNOCKED_DOWN:             "Being knocked down",
    ACTION_KNOCKED_DOWN:                   "Knocked down",
    ACTION_NEUTRAL_TECH:                   "Neutral tech",
    ACTION_BACKWARD_TECH:                  "Backward tech",
    ACTION_FORWARD_TECH:                   "Forward tech",
    ACTION_FALLING_TECH:                   "Falling tech",
    ACTION_UP_AIR_TECH:                    "Up air tech",
    ACTION_DOWN_AIR_TECH:                  "Down air tech",
    ACTION_FORWARD_AIR_TECH:               "Forward air tech",
    ACTION_BACKWARD_AIR_TECH:              "Backward air tech",
    ACTION_AIR_TECH_LANDING_LAG:           "Air tech landing lag",
    ACTION_UNTRANSFORM:                    "Untransform",
    ACTION_GROUND_SLAM:                    "Ground slam",
    ACTION_WALL_SLAM:                      "Wall slam",
    ACTION_HARD_LAND:                      "Hard land",
    ACTION_NEUTRAL_AIR_JUMP:               "Neutral air jump",
    ACTION_FORWARD_AIR_JUMP:               "Forward air jump",
    ACTION_BACKWARD_AIR_JUMP:              "Backward air jump",
    ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK:"Ground high neutral wrong block",
    ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK: "Ground low neutral wrong block",
    ACTION_AIR_NEUTRAL_WRONG_BLOCK:        "Air neutral wrong block",
    ACTION_5N:                             "5n",
    ACTION_6N:                             "6n",
    ACTION_8N:                             "8n",
    ACTION_3N:                             "3n",
    ACTION_2N:                             "2n",
    ACTION_214N:                           "214n",
    ACTION_236N:                           "236n",
    ACTION_623N:                           "623n",
    ACTION_41236N:                         "41236n",
    ACTION_63214N:                         "63214n",
    ACTION_6321469874N:                    "6321469874n",
    ACTION_j5N:                            "j5n",
    ACTION_j6N:                            "j6n",
    ACTION_j8N:                            "j8n",
    ACTION_j3N:                            "j3n",
    ACTION_j2N:                            "j2n",
    ACTION_j214N:                          "j214n",
    ACTION_j236N:                          "j236n",
    ACTION_j623N:                          "j623n",
    ACTION_j41236N:                        "j41236n",
    ACTION_j63214N:                        "j63214n",
    ACTION_j6321469874N:                   "j6321469874n",
    ACTION_t5N:                            "t5n",
    ACTION_t6N:                            "t6n",
    ACTION_t8N:                            "t8n",
    ACTION_t2N:                            "t2n",
    ACTION_c28N:                           "[2]8n",
    ACTION_c46N:                           "[4]6n",
    ACTION_c64N:                           "[6]4n",
    ACTION_5M:                             "5m",
    ACTION_6M:                             "6m",
    ACTION_8M:                             "8m",
    ACTION_3M:                             "3m",
    ACTION_2M:                             "2m",
    ACTION_214M:                           "214m",
    ACTION_236M:                           "236m",
    ACTION_623M:                           "623m",
    ACTION_41236M:                         "41236m",
    ACTION_63214M:                         "63214m",
    ACTION_6321469874M:                    "6321469874m",
    ACTION_j5M:                            "j5m",
    ACTION_j6M:                            "j6m",
    ACTION_j8M:                            "j8m",
    ACTION_j3M:                            "j3m",
    ACTION_j2M:                            "j2m",
    ACTION_j214M:                          "j214m",
    ACTION_j236M:                          "j236m",
    ACTION_j623M:                          "j623m",
    ACTION_j41236M:                        "j41236m",
    ACTION_j63214M:                        "j63214m",
    ACTION_j6321469874M:                   "j6321469874m",
    ACTION_t5M:                            "t5m",
    ACTION_t6M:                            "t6m",
    ACTION_t8M:                            "t8m",
    ACTION_t2M:                            "t2m",
    ACTION_c28M:                           "[2]8m",
    ACTION_c46M:                           "[4]6m",
    ACTION_c64M:                           "[6]4m",
    ACTION_5S:                             "5s",
    ACTION_6S:                             "6s",
    ACTION_8S:                             "8s",
    ACTION_3S:                             "3s",
    ACTION_2S:                             "2s",
    ACTION_214S:                           "214s",
    ACTION_236S:                           "236s",
    ACTION_623S:                           "623s",
    ACTION_41236S:                         "41236s",
    ACTION_63214S:                         "63214s",
    ACTION_6321469874S:                    "6321469874s",
    ACTION_j5S:                            "j5s",
    ACTION_j6S:                            "j6s",
    ACTION_j8S:                            "j8s",
    ACTION_j3S:                            "j3s",
    ACTION_j2S:                            "j2s",
    ACTION_j214S:                          "j214s",
    ACTION_j236S:                          "j236s",
    ACTION_j623S:                          "j623s",
    ACTION_j41236S:                        "j41236s",
    ACTION_j63214S:                        "j63214s",
    ACTION_j6321469874S:                   "j6321469874s",
    ACTION_t5S:                            "t5s",
    ACTION_t6S:                            "t6s",
    ACTION_t8S:                            "t8s",
    ACTION_t2S:                            "t2s",
    ACTION_c28S:                           "[2]8s",
    ACTION_c46S:                           "[4]6s",
    ACTION_c64S:                           "[6]4s",
    ACTION_5V:                             "5v",
    ACTION_6V:                             "6v",
    ACTION_8V:                             "8v",
    ACTION_3V:                             "3v",
    ACTION_2V:                             "2v",
    ACTION_214V:                           "214v",
    ACTION_236V:                           "236v",
    ACTION_623V:                           "623v",
    ACTION_41236V:                         "41236v",
    ACTION_63214V:                         "63214v",
    ACTION_6321469874V:                    "6321469874v",
    ACTION_j5V:                            "j5v",
    ACTION_j6V:                            "j6v",
    ACTION_j8V:                            "j8v",
    ACTION_j3V:                            "j3v",
    ACTION_j2V:                            "j2v",
    ACTION_j214V:                          "j214v",
    ACTION_j236V:                          "j236v",
    ACTION_j623V:                          "j623v",
    ACTION_j41236V:                        "j41236v",
    ACTION_j63214V:                        "j63214v",
    ACTION_j6321469874V:                   "j6321469874v",
    ACTION_t5V:                            "t5v",
    ACTION_t6V:                            "t6v",
    ACTION_t8V:                            "t8v",
    ACTION_t2V:                            "t2v",
    ACTION_c28V:                           "[2]8v",
    ACTION_c46V:                           "[4]6v",
    ACTION_c64V:                           "[6]4v",
    ACTION_5A:                             "5a",
    ACTION_6A:                             "6a",
    ACTION_8A:                             "8a",
    ACTION_3A:                             "3a",
    ACTION_2A:                             "2a",
    ACTION_214A:                           "214a",
    ACTION_236A:                           "236a",
    ACTION_421A:                           "421a",
    ACTION_623A:                           "623a",
    ACTION_41236A:                         "41236a",
    ACTION_63214A:                         "63214a",
    ACTION_6321469874A:                    "6321469874a",
    ACTION_j5A:                            "j5a",
    ACTION_j6A:                            "j6a",
    ACTION_j8A:                            "j8a",
    ACTION_j3A:                            "j3a",
    ACTION_j2A:                            "j2a",
    ACTION_j214A:                          "j214a",
    ACTION_j236A:                          "j236a",
    ACTION_j421A:                          "j421a",
    ACTION_j623A:                          "j623a",
    ACTION_j41236A:                        "j41236a",
    ACTION_j63214A:                        "j63214a",
    ACTION_j6321469874A:                   "j6321469874a",
    ACTION_t5A:                            "t5a",
    ACTION_t6A:                            "t6a",
    ACTION_t8A:                            "t8a",
    ACTION_t2A:                            "t2a",
    ACTION_c28A:                           "[2]8a",
    ACTION_c46A:                           "[4]6a",
    ACTION_c64A:                           "[6]4a",
    ACTION_214D:                           "214d",
    ACTION_236D:                           "236d",
    ACTION_421D:                           "421d",
    ACTION_623D:                           "623d",
    ACTION_41236D:                         "41236d",
    ACTION_63214D:                         "63214d",
    ACTION_6321469874D:                    "6321469874d",
    ACTION_j214D:                          "j214d",
    ACTION_j236D:                          "j236d",
    ACTION_j421D:                          "j421d",
    ACTION_j623D:                          "j623d",
    ACTION_j41236D:                        "j41236d",
    ACTION_j63214D:                        "j63214d",
    ACTION_j6321469874D:                   "j6321469874d",
    ACTION_t5D:                            "t5d",
    ACTION_t6D:                            "t6d",
    ACTION_t8D:                            "t8d",
    ACTION_t2D:                            "t2d",

    ACTION_NEUTRAL_OVERDRIVE:              "Neutral overdrive",
    ACTION_MATTER_OVERDRIVE:               "Matter overdrive",
    ACTION_SPIRIT_OVERDRIVE:               "Spirit overdrive",
    ACTION_VOID_OVERDRIVE:                 "Void overdrive",
    ACTION_NEUTRAL_AIR_OVERDRIVE:          "Neutral air overdrive",
    ACTION_MATTER_AIR_OVERDRIVE:           "Matter air overdrive",
    ACTION_SPIRIT_AIR_OVERDRIVE:           "Spirit air overdrive",
    ACTION_VOID_AIR_OVERDRIVE:             "Void air overdrive",
    ACTION_NEUTRAL_ROMAN_CANCEL:           "Neutral roman cancel",
    ACTION_MATTER_ROMAN_CANCEL:            "Matter roman cancel",
    ACTION_SPIRIT_ROMAN_CANCEL:            "Spirit roman cancel",
    ACTION_VOID_ROMAN_CANCEL:              "Void roman cancel",
    ACTION_NEUTRAL_AIR_ROMAN_CANCEL:       "Neutral air roman cancel",
    ACTION_MATTER_AIR_ROMAN_CANCEL:        "Matter air roman cancel",
    ACTION_SPIRIT_AIR_ROMAN_CANCEL:        "Spirit air roman cancel",
    ACTION_VOID_AIR_ROMAN_CANCEL:          "Void air roman cancel",
    ACTION_GROUND_HIGH_REVERSAL:           "Ground high reversal",
    ACTION_GROUND_LOW_REVERSAL:            "Ground low reversal",
    ACTION_AIR_REVERSAL:                   "Air reversal",

    ACTION_WIN_MATCH1:                     "Win match1",
    ACTION_WIN_MATCH2:                     "Win match2",
    ACTION_WIN_MATCH3:                     "Win match3",
    ACTION_WIN_MATCH4:                     "Win match4",
    ACTION_GAME_START1:                    "Game start1",
    ACTION_GAME_START2:                    "Game start2",
    ACTION_GAME_START3:                    "Game start3",
    ACTION_GAME_START4:                    "Game start4",
}
color = {
    'V': "void",
    'S': "spirit",
    'M': "matter",
    'N': "neutral",
    'NT': "not-typed"
}
name = {
    'V': "Void",
    'S': "Spirit",
    'M': "Matter",
    'N': "Neutral",
    'NT': "Non-Typed"
}
order = [
    "void",
    "spirit",
    "matter",
    "not-typed"
]


def generate_page(data, stats, meta):
    env = Environment(loader=FileSystemLoader([os.path.dirname(__file__) + "resources/" + stats["name"], os.path.dirname(__file__) + "/templates"]))
    with open(os.path.dirname(__file__) + "/templates/chr_template.html") as fd:
        template = env.from_string(fd.read())
    for move in data:
        if 'limit' in move:
            move["limit"] = [" ".join('<span class="' + order[elem] + '">' + str(limit[elem]) + '%</span>' for elem in range(4) if limit[elem]) or "0%" for limit in move["limit"]]
        if 'type' in move:
            move["type"] = [
                "+".join('<span style="cursor:help" class="' + color[t] + '" title="' + name[t] + '">' + t + '</span>' for t in types) for types in move["type"]
            ]
        if 'guard' in move:
            r = ["A", "H", "L"]
            result = []
            for g in move["guard"]:
                if g["Grab"]:
                    result.append("Grab")
                elif g["UB"]:
                    result.append("UB")
                else:
                    result.append("".join(f for f in r if g[f]) or "-")
            move["guard"] = result
    return template.render(moves=data, stats=stats, names=names, meta=meta)


def generate_wiki_page(path, no_regen=False):
    if no_regen:
        print("Loading character files")
        with open(path + "/chr.json") as fd:
            chr_stats = json.load(fd)
        with open(f"generated/{chr_stats['name']}/data.json") as fd:
            data = json.load(fd)
        with open(f"generated/{chr_stats['name']}/meta.json") as fd:
            meta = json.load(fd)
    else:
        data, chr_stats, meta = gen_data(path)
    print("Rendering page")
    html = generate_page(data, chr_stats, meta)
    with open(f"generated/{chr_stats['name']}/index.html", "w") as fd:
        fd.write(html)
    return chr_stats


if __name__ == '__main__':
    generate_wiki_page(sys.argv[1], no_regen=len(sys.argv) > 2)
