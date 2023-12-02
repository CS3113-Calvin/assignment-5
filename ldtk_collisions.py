import json

FILENAME = "newmap.ldtk"

with open(FILENAME, "r") as f:
    data = json.load(f)
    