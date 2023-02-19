import os
import sys
import pdb
import json
import math
from PIL import Image, ImageDraw


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
    except FileNotFoundError:
        pass


def generate_palettes_image(path, idle, palettes):
    frame = idle[0][0]
    tex = frame["texture_bounds"]
    start = Image.open(path + "/" + frame['sprite']).convert("RGBA")
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


def generate_frames_images(path, datas, start=None):
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
                    sprites[frame['sprite']] = Image.open(path + "/" + frame['sprite']).convert("RGBA")
    print("Generating images")
    for action, move in datas.items():
        for blockId, block in enumerate(move):
            for i, frame in enumerate(block):
                img = sprites[frame['sprite']].copy()
                tex = frame["texture_bounds"]
                size = (frame['size']['x'], frame['size']['y'])
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
                print(action, blockId, i, offset, borders)
                img.paste(base.resize(size, resample=Image.Resampling.NEAREST), (
                    offset['x'] + int(math.floor(-size[0]) / 2) - borders[0],
                    -borders[1] + offset['y']
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
                result.append((img, action, blockId, i))
    return result


def main(path):
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
    rename(f"generated/{chr_stats['name']}/data.json", "data_old.json")
    rmdir(f"generated/{chr_stats['name']}")
    mkdir("generated")
    mkdir(f"generated/{chr_stats['name']}")

    print("Generating images")
    img, base = generate_palettes_image(path, datas[0], chr_stats['palettes'])
    img.save(f"generated/{chr_stats['name']}/palettes.png")
    print("Generating character images")
    for img, action, blockId, i in generate_frames_images(path, datas, base):
        mkdir(f"generated/{chr_stats['name']}/{action}")
        img.save(f"generated/{chr_stats['name']}/{action}/{blockId}_{i}.png")
    mkdir(f"generated/{chr_stats['name']}/subobjects")
    print("Generating subobject images")
    for img, action, blockId, i in generate_frames_images(path, objs_datas):
        mkdir(f"generated/{chr_stats['name']}/subobjects/{action}")
        img.save(f"generated/{chr_stats['name']}/subobjects/{action}/{blockId}_{i}.png")

    print("Generating data")

    try:
        os.unlink("data_old.json")
    except FileNotFoundError:
        pass


if __name__ == '__main__':
    main(sys.argv[1])
