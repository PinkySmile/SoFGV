import os
import sys
import json
import math
from PIL import Image, ImageDraw

JSON_INDENT = 2


class DefensiveFlag:
	invulnerable      = 1 << 0
	invulnerableArmor = 1 << 1
	superarmor        = 1 << 2
	grabInvulnerable  = 1 << 3
	voidBlock         = 1 << 4
	spiritBlock       = 1 << 5
	matterBlock       = 1 << 6
	neutralBlock      = 1 << 7
	airborne          = 1 << 8
	canBlock          = 1 << 9
	highBlock         = 1 << 10
	lowBlock          = 1 << 11
	karaCancel        = 1 << 12
	resetRotation     = 1 << 13
	counterHit        = 1 << 14
	flash             = 1 << 15
	crouch            = 1 << 16
	projectileInvul   = 1 << 17
	projectile        = 1 << 18
	landCancel        = 1 << 19
	dashCancel        = 1 << 20
	resetSpeed        = 1 << 21
	neutralInvul      = 1 << 22
	matterInvul       = 1 << 23
	spiritInvul       = 1 << 24
	voidInvul         = 1 << 25
	neutralArmor      = 1 << 26
	matterArmor       = 1 << 27
	spiritArmor       = 1 << 28
	voidArmor         = 1 << 29


class OffensiveFlag:
	grab                  = 1 << 0
	airUnblockable        = 1 << 1
	unblockable           = 1 << 2
	voidElement           = 1 << 3
	spiritElement         = 1 << 4
	matterElement         = 1 << 5
	lowHit                = 1 << 6
	highHit               = 1 << 7
	autoHitPos            = 1 << 8
	canCounterHit         = 1 << 9
	hitSwitch             = 1 << 10
	cancelable            = 1 << 11
	jab                   = 1 << 12
	resetHits             = 1 << 13
	resetSpeed            = 1 << 14
	restand               = 1 << 15
	superMove             = 1 << 16
	ultimate              = 1 << 17
	jumpCancelable        = 1 << 18
	transformCancelable   = 1 << 19
	unTransformCancelable = 1 << 20
	dashCancelable        = 1 << 21
	backDashCancelable    = 1 << 22
	voidMana              = 1 << 23
	spiritMana            = 1 << 24
	matterMana            = 1 << 25
	turnAround            = 1 << 26
	forceTurnAround       = 1 << 27
	nextBlockOnHit        = 1 << 28


def rmdir(directory):
	try:
		for item in os.listdir(directory):
			if os.path.isdir(directory + "/" + item):
				rmdir(directory + "/" + item)
			else:
				os.unlink(directory + "/" + item)
		os.rmdir(directory)
	except FileNotFoundError:
		pass


def mkdir(folder):
	try:
		os.mkdir(folder)
	except FileExistsError:
		pass


def rename(src, dest):
	try:
		os.rename(src, dest)
		return True
	except FileNotFoundError:
		return False


def generate_palettes_image(path, idle, palettes):
	frame = idle[0][0]
	start = Image.open(path + "/" + frame['sprite']).convert("RGBA")
	tex = frame.get("texture_bounds", {"height": start.size[1], "left": 0, "width": start.size[0], "top": 0})
	base = start.crop((
		tex["left"],
		tex["top"],
		tex["left"] + tex["width"],
		tex["top"] + tex["height"]
	))
	imgs = [base]
	pals = [[(p['r'], p['b'], p['g'], 255) for p in r] for r in palettes]
	for palette in pals[1:]:
		img = base.copy()
		pixels = img.load()
		for x in range(img.size[0]):
			for y in range(img.size[1]):
				if pixels[x, y] in pals[0]:
					pixels[x, y] = palette[pals[0].index(pixels[x, y])]
		imgs.append(img)
	result = Image.new("RGBA", (base.size[0] * len(imgs) + 16 * (len(imgs) - 1), base.size[1]))
	for i, img in enumerate(imgs):
		result.paste(img, (i * (base.size[0] + 16), 0))
	return result, start


def generate_frames_images(path, datas, metadata, start=None):
	if start is not None:
		sprites = {datas[0][0][0]['sprite']: start}
	else:
		sprites = {}
	result = []
	print("Loading additional sprites")
	for move in datas.values():
		for block in move:
			for frame in block:
				if not frame['sprite'] in sprites:
					try:
						sprites[frame['sprite']] = Image.open(path + "/" + frame['sprite']).convert("RGBA")
					except Exception as e:
						sprites[frame['sprite']] = Image.new("RGB", (0, 0))
						print("Cannot load " + path + "/" + frame['sprite'])
						print(e)
	print("Generating images")
	for action, move in datas.items():
		for blockId, block in enumerate(move):
			for i, frame in enumerate(block):
				img = sprites[frame['sprite']].copy()
				tex = frame.get("texture_bounds", {"height": img.size[1], "left": 0, "width": img.size[0], "top": 0})
				size = frame.get('size', {'x': tex['width'], 'y': tex['height']})
				size = (size['x'], size['y'])
				offset = frame.get('offset', {"x": 0, "y": 0})
				borders = [
					offset['x'] + int(math.floor(-size[0]) / 2),
					-offset['y'],
					offset['x'] + int(math.ceil(size[0] / 2)),
					-offset['y'] + size[1]
				]
				for hit_box in frame.get('hit_boxes', []):
					borders[0] = min(borders[0], hit_box['left'])
					borders[1] = min(borders[1], hit_box['top'] + size[1])
					borders[2] = max(borders[2], hit_box['left'] + hit_box['width'])
					borders[3] = max(borders[3], hit_box['top'] + size[1] + hit_box['height'])
				for hurt_box in frame.get('hurt_boxes', []):
					borders[0] = min(borders[0], hurt_box['left'])
					borders[1] = min(borders[1], hurt_box['top'] + size[1])
					borders[2] = max(borders[2], hurt_box['left'] + hurt_box['width'])
					borders[3] = max(borders[3], hurt_box['top'] + size[1] + hurt_box['height'])
				if 'collision_box' in frame:
					col_box = frame['collision_box']
					borders[0] = min(borders[0], col_box['left'])
					borders[1] = min(borders[1], col_box['top'] + size[1])
					borders[2] = max(borders[2], col_box['left'] + col_box['width'])
					borders[3] = max(borders[3], col_box['top'] + size[1] + col_box['height'])

				base = img.crop((
					tex["left"],
					tex["top"],
					tex["left"] + tex["width"],
					tex["top"] + tex["height"]
				))
				borders[0] -= 10
				borders[1] -= 10
				borders[2] += 10
				borders[3] += 10
				img = Image.new("RGBA", (borders[2] - borders[0], borders[3] - borders[1]))
				img.paste(base.resize(size, resample=Image.Resampling.NEAREST), (
					offset['x'] + int(math.floor(-size[0]) / 2) - borders[0],
					-borders[1] - offset['y']
				))
				for hurt_box in frame.get('hurt_boxes', []):
					tmp = Image.new("RGBA", (borders[2] - borders[0], borders[3] - borders[1]))
					img_draw = ImageDraw.Draw(tmp)
					img_draw.rectangle((
						-borders[0] + hurt_box['left'],
						-borders[1] + hurt_box['top'] + size[1],
						-borders[0] + hurt_box['left'] + hurt_box['width'],
						-borders[1] + hurt_box['top'] + size[1] + hurt_box['height']
					), fill="#00FF0020", outline="#00FF00")
					img = Image.alpha_composite(img, tmp)
				for hit_box in frame.get('hit_boxes', []):
					tmp = Image.new("RGBA", (borders[2] - borders[0], borders[3] - borders[1]))
					img_draw = ImageDraw.Draw(tmp)
					img_draw.rectangle((
						-borders[0] + hit_box['left'],
						-borders[1] + hit_box['top'] + size[1],
						-borders[0] + hit_box['left'] + hit_box['width'],
						-borders[1] + hit_box['top'] + size[1] + hit_box['height']
					), fill="#FF000020", outline="#FF0000")
					img = Image.alpha_composite(img, tmp)
				if 'collision_box' in frame:
					tmp = Image.new("RGBA", (borders[2] - borders[0], borders[3] - borders[1]))
					img_draw = ImageDraw.Draw(tmp)
					col_box = frame['collision_box']
					img_draw.rectangle((
						-borders[0] + col_box['left'],
						-borders[1] + col_box['top'] + size[1],
						-borders[0] + col_box['left'] + col_box['width'],
						-borders[1] + col_box['top'] + size[1] + col_box['height']
					), fill="#FFFF0020", outline="#FFFF00")
					img = Image.alpha_composite(img, tmp)
				img_draw = ImageDraw.Draw(img)
				metadata[f"{action}_{blockId}_{i}"] = (-borders[0], -borders[1] + size[1], frame.get("duration", 1))
				img_draw.rectangle((
					-borders[0] - 1,
					-borders[1] + size[1] - 1,
					-borders[0] + 1,
					-borders[1] + size[1] + 1
				), fill="white", outline="black")
				result.append((img, action, blockId, i))
	return result


def calc_frame_advantage(block, data):
	l = ["A", "H", "L"]
	if 'guard' not in data or not data['guard']:
		return None
	if (data['guard'][0]["UB"] or data['guard'][0]["Grab"]) and (data['guard'][-1]["UB"] or data['guard'][-1]["Grab"]):
		return None
	dat = [[0, 0], [0, 0]]
	frame = None
	has_hit = False
	total = 0
	for anim in block[::-1]:
		duration = anim.get("duration", 1)
		if 'hit_boxes' in anim:
			stun = anim.get("block_stun", 0)
			stop = anim.get("block_opponent_hit_stop", 0) - anim.get("block_player_hit_stop", 0)
			dat[1][0] = stun + stop - total - 1
			dat[1][1] = stun * 5 // 3 + stop - total - 1
			break
		total += duration

	total = 0
	for anim in block:
		flags = anim.get('offense_flag', 0)
		if flags & OffensiveFlag.resetHits:
			has_hit = False
		total += anim.get("duration", 1)
		if has_hit:
			continue
		if 'hit_boxes' in anim:
			frame = anim
			total = anim.get("duration", 1)
			has_hit = True

	stun = frame.get("block_stun", 0)
	stop = frame.get("block_opponent_hit_stop", 0) - frame.get("block_player_hit_stop", 0)
	dat[0][0] = stun + stop - total
	dat[0][1] = stun * 5 // 3 + stop - total
	if not any(i for k, i in data['guard'][0].items() if k in l):
		dat[0][0] = None
	if not any(i for k, i in data['guard'][-1].items() if k in l):
		dat[1][0] = None
	if all(i for k, i in data['guard'][0].items() if k in l):
		dat[0][1] = None
	if all(i for k, i in data['guard'][-1].items() if k in l):
		dat[1][1] = None
	return dat


def transform_range(block, data):
	result = []
	anim = []
	for a in data:
		start = 0
		for t in range(a):
			start += block[t].get("duration", 1)
		for i in range(block[a].get("duration", 1)):
			anim.append(start + i)
	while anim:
		first = anim.pop(0)
		current = [first, first]
		while anim and current[1] + 1 == anim[0]:
			current[1] = anim.pop(0)
		result.append(current)
	return result


def get_data_for_move(mid, move, objs_datas):
	descStuff = [
		"cancelable",
		"dashCancelable",
		"karaCancelable",
		"selfCancelable",
		"jumpCancelable",
		"hitSwitchCancel",
		"backDashCancelable",
		"invulnerable",
		"invulnerableArmor",
		"superarmor",
		"grabInvulnerable",
		"voidBlock",
		"spiritBlock",
		"matterBlock",
		"neutralBlock",
		"canBlock",
		"highBlock",
		"lowBlock",
		"projectileInvul",
		"neutralInvul",
		"matterInvul",
		"spiritInvul",
		"voidInvul",
		"neutralArmor",
		"matterArmor",
		"spiritArmor",
		"voidArmor"
	]
	data = {a: [] for a in descStuff}
	current = None
	anim = 0
	active_frames = 0
	current_frame = 0
	real_current_frame = 0
	old = None
	has_follow_up = False
	lc = False
	while anim < len(move[0]):
		frame = move[0][anim]
		oflags = frame.get('offense_flag', 0)
		dflags = frame.get('defense_flag', 0)

		if (oflags & OffensiveFlag.nextBlockOnHit) or (dflags & DefensiveFlag.landCancel):
			has_follow_up = True
			lc = bool(dflags & DefensiveFlag.landCancel)
		if (oflags & OffensiveFlag.resetHits) and current is not None:
			current['active'] = active_frames
			for i, k in current.items():
				if i not in data:
					data[i] = []
				data[i].append(k)
			current = None
			active_frames = 0
		# Description
		#   -> When cancel
		#   -> Flags (AUB, UB, Armor, ...)
		if oflags & OffensiveFlag.cancelable:
			data["cancelable"].append(anim)
			if oflags & OffensiveFlag.resetHits:
				data["selfCancelable"].append(anim)
			if oflags & OffensiveFlag.jumpCancelable:
				data["jumpCancelable"].append(anim)
			if oflags & OffensiveFlag.dashCancelable:
				data["dashCancelable"].append(anim)
			if oflags & OffensiveFlag.hitSwitch:
				data["hitSwitchCancel"].append(anim)
			if oflags & OffensiveFlag.backDashCancelable:
				data["backDashCancelable"].append(anim)
			if dflags & DefensiveFlag.karaCancel:
				data["karaCancelable"].append(anim)
			if dflags & DefensiveFlag.invulnerable:
				data["invulnerable"].append(anim)
			if dflags & DefensiveFlag.invulnerableArmor:
				data["invulnerableArmor"].append(anim)
			if dflags & DefensiveFlag.superarmor:
				data["superarmor"].append(anim)
			if dflags & DefensiveFlag.grabInvulnerable:
				data["grabInvulnerable"].append(anim)
			if dflags & DefensiveFlag.voidBlock:
				data["voidBlock"].append(anim)
			if dflags & DefensiveFlag.spiritBlock:
				data["spiritBlock"].append(anim)
			if dflags & DefensiveFlag.matterBlock:
				data["matterBlock"].append(anim)
			if dflags & DefensiveFlag.neutralBlock:
				data["neutralBlock"].append(anim)
			if dflags & DefensiveFlag.canBlock:
				data["canBlock"].append(anim)
			if dflags & DefensiveFlag.highBlock:
				data["highBlock"].append(anim)
			if dflags & DefensiveFlag.lowBlock:
				data["lowBlock"].append(anim)
			if dflags & DefensiveFlag.projectileInvul:
				data["projectileInvul"].append(anim)
			if dflags & DefensiveFlag.neutralInvul:
				data["neutralInvul"].append(anim)
			if dflags & DefensiveFlag.matterInvul:
				data["matterInvul"].append(anim)
			if dflags & DefensiveFlag.spiritInvul:
				data["spiritInvul"].append(anim)
			if dflags & DefensiveFlag.voidInvul:
				data["voidInvul"].append(anim)
			if dflags & DefensiveFlag.neutralArmor:
				data["neutralArmor"].append(anim)
			if dflags & DefensiveFlag.matterArmor:
				data["matterArmor"].append(anim)
			if dflags & DefensiveFlag.spiritArmor:
				data["spiritArmor"].append(anim)
			if dflags & DefensiveFlag.voidArmor:
				data["voidArmor"].append(anim)
		# Object info
		if frame.get("marker", 0) or frame.get("subobject", 0) and not current:
			current = {
				# Lock
				'lock': bool(oflags & OffensiveFlag.resetSpeed),
				# Restand
				'restand': bool(oflags & OffensiveFlag.restand),
				# Startup
				'startup': real_current_frame,
				# Startup animation
				'startup_anim': [anim, real_current_frame]
			}
		elif 'hit_boxes' in frame:
			active_frames += frame.get("duration", 1)
			if current is not None and 'damage' in current:
				old = current
			current = {
				# Lock
				'lock': bool(oflags & OffensiveFlag.resetSpeed),
				# Restand
				'restand': bool(oflags & OffensiveFlag.restand),
				# Startup
				'startup': real_current_frame,
				# Startup animation
				'startup_anim': [anim, real_current_frame],
				# Damage
				'damage': frame.get("damage", 0),
				# Chip damage
				'chip_damage': frame.get("chip_damage", 0),
				# Prorate
				'prorate': frame.get("prorate", 0),
				# Limit
				'limit': [
					frame.get("void_limit", 0),
					frame.get("spirit_limit", 0),
					frame.get("matter_limit", 0),
					frame.get("neutral_limit", 0)
				],
				# Mana gain
				'gain': [
					frame.get("mana_gain", 0) * bool(oflags & OffensiveFlag.voidMana),
					frame.get("mana_gain", 0) * bool(oflags & OffensiveFlag.spiritMana),
					frame.get("mana_gain", 0) * bool(oflags & OffensiveFlag.matterMana)
				],
				# Move type (Void, Spirit, Matter, Neutral)
				'type': oflags & (OffensiveFlag.voidElement | OffensiveFlag.matterElement | OffensiveFlag.spiritElement),
				# Guard damage
				'guard_dmg': frame.get("guard_damage", 0)
			}
			if current['type'] == (OffensiveFlag.voidElement | OffensiveFlag.matterElement | OffensiveFlag.spiritElement):
				current['type'] = ["N"]
			elif current['type'] == 0:
				current['type'] = ["NT"]
			else:
				types = []
				if current['type'] & OffensiveFlag.voidElement:
					types.append("V")
				if current['type'] & OffensiveFlag.matterElement:
					types.append("M")
				if current['type'] & OffensiveFlag.spiritElement:
					types.append("S")
				current['type'] = types
			# Guard type
			if oflags & OffensiveFlag.unblockable:
				current['guard'] = {
					"A": False,
					"L": False,
					"H": False,
					"UB": True,
					"Grab": False,
					"Auto": False
				}
			elif oflags & OffensiveFlag.grab:
				current['guard'] = {
					"A": False,
					"L": False,
					"H": False,
					"UB": False,
					"Grab": True,
					"Auto": False
				}
			else:
				if current['guard_dmg'] == 0:
					print(f"Warning: Move {mid}, block 0, anim {anim} doesn't have any guard damage")
				current['guard'] = {
					"A": True,
					"L": True,
					"H": True,
					"UB": False,
					"Grab": False,
					"Auto": False
				}
				if oflags & OffensiveFlag.lowHit:
					current['guard']['H'] = False
				if oflags & OffensiveFlag.highHit:
					current['guard']['L'] = False
				if oflags & OffensiveFlag.autoHitPos:
					current['guard']["Auto"] = True
				if oflags & OffensiveFlag.airUnblockable:
					current['guard']['A'] = False
			if old:
				if 'startup_anim' not in data:
					data['startup_anim'] = []
				data['startup_anim'].append(old['startup_anim'])
				current['startup'] = old['startup']
				old['startup_anim'] = current['startup_anim']
				if old != current:
					print(f"Warning: Move {mid}, block 0, anim {anim} has different data for hit frames")
					for i in current.keys():
						if current[i] != old[i]:
							print(f"\t{i}: Old {old[i]} VS new {current[i]}")
				current = old
				old = None
		anim += 1
		current_frame += frame.get("duration", 1)
		if not (dflags & DefensiveFlag.flash):
			real_current_frame += frame.get("duration", 1)
	if current is not None:
		current['active'] = active_frames
		for i, k in current.items():
			if i not in data:
				data[i] = []
			data[i].append(k)

	for a in descStuff:
		data[a] = transform_range(move[0], data[a])
	# Attack type
	flags = move[0][0].get('offense_flag', 0)
	if flags & OffensiveFlag.ultimate:
		data["super"] = 2
	elif flags & OffensiveFlag.superMove:
		data["super"] = 1
	# TODO: Land Cancel recovery
	# TODO: Follow up info
	# Mana cost
	cost = move[0][0].get("mana_cost", 0)
	data["cost"] = [
		cost * bool(flags & OffensiveFlag.voidMana),
		cost * bool(flags & OffensiveFlag.spiritMana),
		cost * bool(flags & OffensiveFlag.matterMana)
	]
	# Frame advantage
	data['advantage'] = calc_frame_advantage(move[0], data)
	# Total duration
	data["total"] = current_frame
	data["total_real"] = real_current_frame
	return data


def generate_data(datas, objs_datas, objs):
	result = []
	# General info
	#   -> Dash type, speed, startup, recovery
	#   -> Walk speed (front, back)
	#   -> Backdash data
	#   -> Jump squat
	#   -> Air movement options
	#   -> KD framedata
	#   -> Air techs stuff
	#   -> Ground techs stuff
	for action, move in datas.items():
		# We skip animations before neutral moves
		if action < 100:
			continue
		# We also skip all roman cancels ...
		if action < 300 and (action % 50 == 31 or action % 50 == 32):
			continue
		# ... and bursts
		if action < 300 and (action % 50 == 7 or action % 50 == 19):
			continue
		# Let's also skip round start + win animations
		if 400 <= action < 450:
			continue

		# Damage
		# Prorate
		# Startup
		# Active frames
		# Total duration
		# Frame advantage
		# Guard damage
		# Move type (Void, Spirit, Matter, Neutral)
		# Guard type
		# Description
		#   -> When cancel
		#   -> Attack type
		#   -> Flags (AUB, UB, Armor, ...)
		data = get_data_for_move(action, move, objs_datas)
		data['action'] = action
		result.append(data)
	# Add the extra common moves that can differ
	#   -> Neutral parry reversal
	return result


def generate_diff(data, old_data):
	# TODO:
	return []


def gen_data(path):
	print("Loading character files")
	with open(path + "/chr.json") as fd:
		chr_stats = json.load(fd)
	with open(path + "/subobjects.json") as fd:
		objs = json.load(fd)
	with open(path + "/subobj_data.json") as fd:
		objs_data = json.load(fd)
	with open(path + "/framedata.json") as fd:
		chr_data = json.load(fd)
	datas = {}
	objs_datas = {}
	for data in chr_data:
		datas[data['action']] = data['framedata']
	for data in objs_data:
		objs_datas[data['action']] = data['framedata']

	print("Selected " + chr_stats['name'])
	has_old_data = rename(f"generated/{chr_stats['name']}/data.json", "data_old.json")
	rmdir(f"generated/{chr_stats['name']}")
	mkdir("generated")
	mkdir(f"generated/{chr_stats['name']}")

	print("Generating images")
	img, base = generate_palettes_image(path, datas[0], chr_stats['palettes'])
	img.save(f"generated/{chr_stats['name']}/palettes.png")

	print("Generating character images")
	meta = {}
	for img, action, blockId, i in generate_frames_images(path, datas, meta, start=base):
		mkdir(f"generated/{chr_stats['name']}/{action}")
		img.save(f"generated/{chr_stats['name']}/{action}/{blockId}_{i}.png")
	with open(f"generated/{chr_stats['name']}/meta.json", "w") as fd:
		json.dump(meta, fd, indent=JSON_INDENT)

	mkdir(f"generated/{chr_stats['name']}/subobjects")
	print("Generating subobject images")
	meta = {}
	for img, action, blockId, i in generate_frames_images(path, objs_datas, meta):
		mkdir(f"generated/{chr_stats['name']}/subobjects/{action}")
		img.save(f"generated/{chr_stats['name']}/subobjects/{action}/{blockId}_{i}.png")
	with open(f"generated/{chr_stats['name']}/subobjects/meta.json", "w") as fd:
		json.dump(meta, fd, indent=JSON_INDENT)

	print("Generating framedata")
	del chr_stats['palettes']
	with open(f"generated/{chr_stats['name']}/stats.json", "w") as fd:
		json.dump(chr_stats, fd, indent=JSON_INDENT)
	current_data = generate_data(datas, objs_datas, objs)
	with open(f"generated/{chr_stats['name']}/data.json", "w") as fd:
		json.dump(current_data, fd, indent=JSON_INDENT)

	if has_old_data:
		with open("data_old.json") as fd:
			diff = generate_diff(current_data, json.load(fd))
		with open(f"generated/{chr_stats['name']}/data_diff.json", "w") as fd:
			json.dump(diff, fd, indent=JSON_INDENT)
		os.unlink("data_old.json")
	return current_data, chr_stats, meta


if __name__ == '__main__':
	gen_data(sys.argv[1])
