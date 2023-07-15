import os
import sys
import traceback
from wiki_page_generator import generate_wiki_page
from jinja2 import Environment


entries = []
for f in os.listdir("assets/characters"):
	if f == "template":
		continue
	try:
		d = generate_wiki_page("assets/characters/" + f, no_regen=len(sys.argv) >= 1)
		entries.append(d)
	except:
		traceback.print_exc()
		print("\033[31mCannot generate page for " + f + "\033[0m")

env = Environment()
with open("index_template.html") as fd:
	template = env.from_string(fd.read())
html = template.render(chrs=entries)
with open(f"wiki.html", "w") as fd:
	fd.write(html)
