#!/bin/env python3
from PIL import Image, ImageDraw
import os
import sys
import json

if len(sys.argv) != 3:
	print(f'Usage: {sys.argv[0]} <character folder> <framedata>')
	exit(1)

print("Loading data")
with open(sys.argv[2]) as fd:
	framedata = json.load(fd)
sprites = {}
sprites_effect = {}
sprites_merged = {}
areas = []
datas = {}
for move in framedata:
	datas[move['action']] = move["framedata"]
	for i, block in enumerate(move["framedata"]):
		for j, frame in enumerate(block):
			if not frame['sprite'] in sprites:
				path = os.path.join(sys.argv[1], frame['sprite'])
				print("Loading " + path)
				sprites[frame['sprite']] = Image.open(path).convert("RGBA")
				path = os.path.join(sys.argv[1], 'effects', frame['sprite'])
				print("Loading " + path)
				sprites_effect[frame['sprite']] = Image.open(path).convert("RGBA")
				sprites_merged[frame['sprite']] = sprites[frame['sprite']].copy()
				if sprites_effect[frame['sprite']]:
					sprites_merged[frame['sprite']].paste(sprites_effect[frame['sprite']], (0, 0), sprites_effect[frame['sprite']])
			areas.append({
				'frameId': f"{move['action']}|{i}|{j}",
				'frames': [frame],
				'rect': frame.get("texture_bounds", {"height": sprites[frame['sprite']].size[1], "left": 0, "width": sprites[frame['sprite']].size[0], "top": 0}).copy(),
				'sprite': frame['sprite'],
				'image': sprites[frame['sprite']],
				'pixels': sprites_merged[frame['sprite']].load(),
				'effect_image': sprites_effect[frame['sprite']],
				'deleted': False
			})
# with open(sys.argv[1] + "/framedata_step0.json", "w") as fd:
# 	json.dump(framedata, fd)

print("Shrinking defined area")
for area in areas:
	base = area['rect'].copy()
	if area['rect']['left'] < 0:
		area['rect']['width'] += area['rect']['left']
		area['rect']['left'] = 0
	if area['rect']['top'] < 0:
		area['rect']['height'] += area['rect']['top']
		area['rect']['top'] = 0

	if area['rect']['left'] + area['rect']['width'] > area['image'].size[0]:
		area['rect']['width'] = area['image'].size[0] - area['rect']['left']
	if area['rect']['top'] + area['rect']['height'] > area['image'].size[1]:
		area['rect']['height'] = area['image'].size[1] - area['rect']['top']

	empty = True
	while True:
		for i in range(area['rect']['width']):
			if area['pixels'][area['rect']['left'] + i, area['rect']['top']][3] != 0:
				empty = False
				break
		if not empty:
			break
		area['rect']['top'] += 1
		area['rect']['height'] -= 1

	empty = True
	while True:
		for i in range(area['rect']['height']):
			if area['pixels'][area['rect']['left'], area['rect']['top'] + i][3] != 0:
				empty = False
				break
		if not empty:
			break
		area['rect']['left'] += 1
		area['rect']['width'] -= 1

	empty = True
	while True:
		for i in range(area['rect']['width']):
			if area['pixels'][area['rect']['left'] + i, area['rect']['top'] + area['rect']['height'] - 1][3] != 0:
				empty = False
				break
		if not empty:
			break
		area['rect']['height'] -= 1

	empty = True
	while True:
		for i in range(area['rect']['height']):
			if area['pixels'][area['rect']['left'] + area['rect']['width'] - 1, area['rect']['top'] + i][3] != 0:
				empty = False
				break
		if not empty:
			break
		area['rect']['width'] -= 1

	scale = area['frames'][0].get("scale", {"x": 1, "y": 1})
	area['frames'][0]["texture_bounds"] = area['rect'].copy()
	area['frames'][0]["offset"] = area['frames'][0].get("offset", {"x": 0, "y": 0})
	area['frames'][0]["offset"]["x"] += (area['rect']['left'] - base['left']) * scale['x']
	area['frames'][0]["offset"]["y"] -= (area['rect']['top'] - base['top']) * scale['y']
	area['frames'][0]["offset"]["x"] -= (base['width'] - area['rect']['width']) * scale['x'] / 2
	area['frames'][0]["offset"]["y"] += (base['height'] - area['rect']['height']) * scale['y']
# with open(sys.argv[1] + "/framedata_step1.json", "w") as fd:
# 	json.dump(framedata, fd)

print("Gathering overlapping sprites")
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
areas = [*filter(lambda a: not a['deleted'], areas)]
# with open(sys.argv[1] + "/framedata_step2.json", "w") as fd:
# 	json.dump(framedata, fd)

print("Computing optimal placement")
areas = [*sorted(areas, key=lambda x: (x['rect']['height'] << 32) | x['rect']['width'])]
infos = {
	'widths': [sum(a['rect']['width'] for a in areas)],
	'heights': [max(a['rect']['height'] for a in areas)],
	'frames': [areas],
	'pivots': 1
}
infos['totalWidth'] = infos['widths'][0]
infos['totalHeight'] = infos['heights'][0]
totalWidth = infos['totalWidth']
while True:
	oldInfos = infos
	infos = {**infos}
	x = 0
	infos['pivots'] += 1
	infos['frames'] = [[] for i in range(infos['pivots'])]
	for a in areas:
		infos['frames'][x * infos['pivots'] // totalWidth].append(a)
		x += a['rect']['width']
	infos['widths'] = [sum(x['rect']['width'] for x in a) for a in infos['frames']]
	infos['heights'] = [max(x['rect']['height'] for x in a) for a in infos['frames']]
	infos['totalWidth'] = max(infos['widths'])
	infos['totalHeight'] = sum(infos['heights'])
	if infos['totalWidth'] + infos['totalHeight'] >= oldInfos['totalWidth'] + oldInfos['totalHeight']:
		break
infos = oldInfos

print("Generating spritesheet")
result = Image.new("RGBA", (infos['totalWidth'], infos['totalHeight']), (0, 0, 0, 0))
result_effect = Image.new("RGBA", (infos['totalWidth'], infos['totalHeight']), (0, 0, 0, 0))
x = 0
totalX = 0
y = 0
i = 0
for a in areas:
	if i != totalX * infos['pivots'] // totalWidth:
		x = 0
		y += infos['heights'][i]
		i = totalX * infos['pivots'] // totalWidth

	mask = Image.new("L", a['image'].size, 0)
	draw = ImageDraw.Draw(mask)
	draw.rectangle((a['rect']['left'], a['rect']['top'], a['rect']['left'] + a['rect']['width'], a['rect']['top'] + a['rect']['height']), fill=255)
	result.paste(a['image'], (x - a['rect']['left'], y - a['rect']['top']), mask)

	if a['effect_image']:
		mask = Image.new("L", a['effect_image'].size, 0)
		draw = ImageDraw.Draw(mask)
		draw.rectangle((a['rect']['left'], a['rect']['top'], a['rect']['left'] + a['rect']['width'], a['rect']['top'] + a['rect']['height']), fill=255)
		result_effect.paste(a['effect_image'], (x - a['rect']['left'], y - a['rect']['top']), mask)

	for f in a['frames']:
		f['sprite'] = "sheet_generated.png"
		f['texture_bounds']['left'] = x
		f['texture_bounds']['top'] = y
	x += a['rect']['width']
	totalX += a['rect']['width']

result.save(sys.argv[1] + "/sheet_generated.png")
result_effect.save(sys.argv[1] + "/effects/sheet_generated.png")
with open(".".join(sys.argv[2].split(".")[:-1]) + "_generated.json", "w") as fd:
	json.dump(framedata, fd)
