#!/bin/env python3
from PIL import Image
import os
import sys
import json

if len(sys.argv) != 2:
	print(f'Usage: {sys.argv[0]} <character folder>')
	exit(1)

print("Loading data")
with open(sys.argv[1] + "/framedata.json") as fd:
	framedata = json.load(fd)
sprites = {}
areas = []
datas = {}
for move in framedata:
	datas[move['action']] = move["framedata"]
	for block in move["framedata"]:
		for frame in block:
			if not frame['sprite'] in sprites:
				try:
					path = os.path.join(sys.argv[1], frame['sprite'])
					print("Loading " + path)
					sprites[frame['sprite']] = Image.open(path).convert("RGBA")
				except Exception as e:
					sprites[frame['sprite']] = Image.new("RGB", (0, 0))
					print("Cannot load " + path)
					print(e)
			areas.append({
				'frames': [frame],
				'rect': frame.get("texture_bounds", {"height": sprites[frame['sprite']].size[1], "left": 0, "width": sprites[frame['sprite']].size[0], "top": 0}),
				'sprite': frame['sprite'],
				'deleted': False
			})

print("Computing optimal placement")
for i, a2 in enumerate(areas):
	for j in range(i):
		a = areas[j]
		if a['deleted']:
			continue
		if a2['sprite'] != a['sprite']:
			continue
		if a2['rect']['left'] + a2['rect']['width'] <= a['rect']['left']:
			continue
		if a['rect']['left'] + a['rect']['width'] <= a2['rect']['left']:
			continue
		if a2['rect']['top'] + a2['rect']['height'] <= a['rect']['top']:
			continue
		if a['rect']['top'] + a['rect']['height'] <= a2['rect']['top']:
			continue
		a2['deleted'] = True
		a['rect']['width'] = max(a2['rect']['width'], a['rect']['width']) + abs(a2['rect']['left'] - a['rect']['left'])
		a['rect']['height'] = max(a2['rect']['height'], a['rect']['height']) + abs(a2['rect']['top'] - a['rect']['top'])
		a['rect']['top'] = min(a2['rect']['top'], a['rect']['top'])
		a['rect']['left'] = min(a2['rect']['left'], a['rect']['left'])
		a['frames'] = a['frames'] + a2['frames']
		break
areas = filter(lambda a: not a['deleted'], areas)
print(json.dumps([(f['rect'], f['sprite'], len(f['frames'])) for f in areas], indent=4))
