import os
import sys
import traceback
from wiki_page_generator import generate_wiki_page
from jinja2 import Environment, FileSystemLoader


def render_file(file, out, root):
	env = Environment(loader=FileSystemLoader(os.path.dirname(__file__) + "/templates"))
	with open(file) as fd:
		template = env.from_string(fd.read())
	html = template.render(chrs=entries, root=root)
	with open(out, "w") as fd:
		fd.write(html)


entries = []
for f in os.listdir("assets/characters"):
	if f == "template":
		continue
	try:
		d = generate_wiki_page("assets/characters/" + f, no_regen=len(sys.argv) >= 2)
		entries.append(d)
	except:
		traceback.print_exc()
		print("\033[31mCannot generate page for " + f + "\033[0m")

try:
	os.mkdir("generated/base")
except:
	pass
for f in os.listdir(os.path.dirname(__file__) + "/premade"):
	if f.endswith(".html"):
		render_file(os.path.dirname(__file__) + "/premade/" + f, "generated/base/" + f, "../../")
	else:
		with open("generated/base/" + f, "wb") as fd1:
			with open(os.path.dirname(__file__) + "/premade/" + f, "rb") as fd2:
				fd1.write(fd2.read())
render_file(os.path.dirname(__file__) + "/templates/index_template.html", "wiki.html", "")
