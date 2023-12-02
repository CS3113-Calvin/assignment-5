
import json
from pprint import pprint

WIDTH  = 38 # 304 x 304 / 8 pixels
HEIGHT = 38 # 304 x 304 / 8 pixels

# world data
world_data = [[], [], []]
sky_data   = [[], [], []]

FILENAME = "newmap.ldtk"
with open(FILENAME, "r") as f:
    data = json.load(f)
    for level in data["levels"]:
        level_data = [[(0, 0)] * WIDTH for _ in range(HEIGHT)]
        sky = [[(0, 0)] * WIDTH for _ in range(HEIGHT)]
        # for entity in level["entities"]:
        #     if entity["__identifier"] == "Player":
        #         print(f"Player position: {entity['px']}, {entity['py']}")
        #         break
        # else:
        #     print("Player not found.")
        #     break
        for layer in level["layerInstances"]:
            if layer["__identifier"] == "IntGrid_layer":
                for tile in layer["autoLayerTiles"]:
                    x = tile["px"][0] // 8
                    y = tile["px"][1] // 8
                    t = tile["t"]
                    f = tile["f"]
                    level_data[y][x] = (t, f)
            elif layer["__identifier"] == "Sky":
                for tile in layer["autoLayerTiles"]:
                    x = tile["px"][0] // 8
                    y = tile["px"][1] // 8
                    t = tile["t"]
                    f = tile["f"]
                    sky[y][x] = (t, f)
        if level["identifier"] == "World_Level_0":
            index = 0
        elif level["identifier"] == "World_Level_1":
            index = 1
        elif level["identifier"] == "World_Level_2":
            index = 2

        world_data[index] = level_data
        sky_data[index] = sky

def print_data(data):
    for i, data_layer in enumerate(data):
        print(f"World {i}:")
        for row in data_layer:
            # print(row[0])
            # new_row = []
            # for x in row:
            #     if x == 0:
            #         print(x)
            #         print(row)
            #         raise ValueError("0 found in data")
            #         new_row.append(0)
            #     else:
            #         new_row.append(f"{{{x[0]}, {x[1]}}}")

            new_row = [f"{{{x[0]}, {x[1]}}}" for x in row]
            # print(*row, sep=", ", end="")
            print(*new_row, sep=", ", end="")
            print(",")
        print()

print("World data:")
print_data(world_data)
# pprint(sky_data)
