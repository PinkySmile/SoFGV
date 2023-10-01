import os
import sys
import json
import traceback
from wiki_page_generator import generate_wiki_page
from jinja2 import Environment, FileSystemLoader


def render_file(file, out, root, version):
	env = Environment(loader=FileSystemLoader(os.path.dirname(__file__) + "/templates"))
	with open(file) as fd:
		template = env.from_string(fd.read())
	html = template.render(chrs=entries, root=root, version=version)
	with open(out, "w") as fd:
		fd.write(html)


with open(os.path.dirname(__file__) + "/../LibCore/Resources/version.h") as fd:
	version = fd.read()
	version = [i.split('"')[-2] for i in version.split("\n") if i.startswith("#define REAL_VERSION_STR")][0]
entries=[]
for f in os.listdir("assets/characters"):
	if f == "template":
		continue
	try:
		with open("assets/characters/" + f + "/chr.json") as fd:
			entries.append(json.load(fd))
	except:
		traceback.print_exc()
for f in os.listdir("assets/characters"):
	if f == "template":
		continue
	try:
		generate_wiki_page("assets/characters/" + f, entries, version, no_regen=len(sys.argv) >= 2)
	except:
		traceback.print_exc()
		print("\033[31mCannot generate page for " + f + "\033[0m")

try:
	os.mkdir("generated/base")
except:
	pass
for f in os.listdir(os.path.dirname(__file__) + "/premade"):
	if f.endswith(".html"):
		render_file(os.path.dirname(__file__) + "/premade/" + f, "generated/base/" + f, "../../", version)
	else:
		with open("generated/base/" + f, "wb") as fd1:
			with open(os.path.dirname(__file__) + "/premade/" + f, "rb") as fd2:
				fd1.write(fd2.read())
render_file(os.path.dirname(__file__) + "/templates/index_template.html", "wiki.html", "", version)
