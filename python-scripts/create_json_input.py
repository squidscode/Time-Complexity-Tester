import argparse
import os

SAVE_DIR = "/Users/Siddhant/Desktop/Github/Time-Complexity-Tester/sample-input"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="create_json_input.py",
        description="creates a json file using the given input.")
    parser.add_argument("name", metavar="NAME", type=str,
        help="the name of the test.")
    parser.add_argument("budget", metavar="BUDGET", type=int,
        help="the amount of time you want to run the test for, in milliseconds")
    parser.add_argument("file_path", metavar="FILE-PATH", type=str,
        help="the path to the file.")
    
    args = parser.parse_args()

    if not os.path.exists(SAVE_DIR + "/" + args.name + ".json"):
        open(SAVE_DIR + "/" + args.name + ".json", "x")
    f = open(SAVE_DIR + "/" + args.name + ".json", "w")

    fcpp = open(args.file_path, "r")
    code : str = fcpp.read()
    fcpp.close()

    code = code.replace("\n", "\\n")
    code = code.replace("\t", "\\t")
    code = code.replace("\"", "\\\"")

    json_str : str = "".join(["{", f'"name":"{args.name}", "budget": {args.budget},',
        f'"code":"{code}"', "}"])

    f.write(json_str)
    f.close()