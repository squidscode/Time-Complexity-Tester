import matplotlib.pyplot as plt
import sys
import os
import json


color_map = {"O(1)":"cornflowerblue", "O(log n)": "deepskyblue", "O(sqrt(n))":"darkturquoise",
    "O(n)":"aqua", "O(n log n)":"mediumspringgreen", "O(n^2)":"limegreen", 
    "O(n^3)":"yellow","O(1.5^n)":"orange", "O(2^n)":"orangered", "O(n^n)": "red"}
# Returns the color of the given function
def get_color(function_name : str) -> str:
    if function_name in color_map:
        return color_map[function_name]
    else:
        return "black"
    

def generate_scatterplot(name:str, data) -> None:
    # Create a scatterplot with the given data
    fig, ax = plt.subplots()
    
    ax.set_title(f"Data Scatterplot for " + name.split("/")[-1].split(".")[0])
    maxx = -1
    sz = -1
    for function_name in data:
        if sz == -1: 
            sz = len(data[function_name]["x"])
            print(f"The size is {sz}\n\n")
        x = data[function_name]["x"]
        maxx = max(maxx, x[-1])
        y = data[function_name]["y"]
        ax.scatter(x, y, s=max(100 * 10/sz, 2), color=get_color(function_name), label=function_name)
    
    ax.set_xlim(-1, maxx + 2)
    ax.set_ylim(-0.25, 3)

    ax.legend(title="Time Complexities", loc="upper right")
    # Show the scatterplot
    plt.show()


# The main driver:
if __name__ == "__main__":
    # If the user does not pass the correct number of arguments:
    if len(sys.argv) != 2:
        print("Correct usage: python data_visualizer.py [PATH-TO-JSON-FILE]")
        exit(1)
    
    # Otherwise check if the file exists:
    path : str = sys.argv[1] # the path
    if not os.path.exists(path):
        print("Path does not exist!")
        exit(1)
    if not os.path.isfile(path):
        print("Path does not point to a file!")
        exit(1)
    
    # Attempt to parse through the file using json
    read_file = open(path, "r")
    data = json.load(read_file)
    generate_scatterplot(path, data["data"])


