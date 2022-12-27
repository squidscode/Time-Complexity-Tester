import subprocess as sp
import argparse
import sys
import os

TEST_FLAG : str = "// ~TC-TEST~"
TC_PATH : str = "/Users/Siddhant/Desktop/Github/Time-Complexity-Tester/time_complexity.h"
TIME_COMPLEXITY_INCLUDE : str = f"#include \"{TC_PATH}\"\n"
MAIN_FN_START : str = "\n\nint main(){\n"
MAIN_FN_END : str = "}"
DEFAULT_BUDGET : int = 5000

# Generates the C++ code for the test.
def generate_tester(name : str, budget : int, program : str) -> str:
    program.replace("\\n", "\n") # replace all "\n" with actual new-lines

    # If the program already has the main function, prints an error:
    if program.find("int main(") != -1:
        print("The main function already exists in the program! "
            + "The provided program is invalid!", file=sys.stderr)
        exit(1)
    
    # If the program doesn't have the test flag, print an error:
    if program.find(TEST_FLAG) == -1:
        print("The program is incorrectly formatted. Please indicate which "
            + "function you would like to test by writing \"" + TEST_FLAG + " [NAME-OF-TEST]\" "
            + "before a valid function.", file=sys.stderr)
        exit(1)
    
    start_index : int = 0

    # For each of the flags we find:
    tests = []
    index : int = program.find(TEST_FLAG, start_index)
    while index != -1:
        start_index = index + 1 # increment the start of search index

        # The start of the name, the end of the name
        start_of_name : int = index + len(TEST_FLAG)
        while program[start_of_name] == " ": start_of_name += 1 # increment while there is a space
        end_of_name : int = program.find("\n", start_of_name)

        # the function can return anything:
        fn_start : int = program.find(" ", end_of_name + 1)
        fn_end : int = program.find("(", fn_start)

        # the name of the function:
        function_name : str = program[fn_start:fn_end]
        
        # the name of the test:
        if start_of_name == end_of_name:
            test_name : str = function_name
        else:
            test_name : str = program[start_of_name:end_of_name]

        tests.append(f"    tc.compute_complexity(\"{test_name}\", {function_name});\n")

        index = program.find(TEST_FLAG, start_index) # move the loop forward
    
    tests_str : str = "".join(tests)
    main_func : str = "".join([MAIN_FN_START, f"    time_complexity tc({budget});\n",
        f"    tc.auto_interval = false;\n", tests_str, MAIN_FN_END])

    return "".join([TIME_COMPLEXITY_INCLUDE, program, main_func])

# Writes the code to a test file:
def write_to_file(dir : str, name : str, program : str):
    # Make the directory
    try:
        os.mkdir(dir + "/" + name)
    except OSError as error:
        # only pass through if the directory already exists
        if error.errno != 17:
            print(error, file=sys.stderr)
            exit(1)

    # Write the main.cpp file:
    file_name : str = dir + "/" + name + "/main.cpp"
    if not os.path.exists(file_name):
        open(file_name, "x")
    f = open(file_name, "w")
    f.write(program)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="load_program.py",
        description="Transforms the given C/C++ code to a \
        program that tests the time complexity of the given code\
        , and writes the file to a folder with the given name. \n\n \
        At least one of the given functions in the provided code must have a \
        test flag (ie. \"" + TEST_FLAG + " [TEST-NAME]\").")

    parser.add_argument("-d", "--directory", type=str, default=".", help="The parent directory \
        of the parsed program.")
    parser.add_argument("name", metavar="NAME", type=str, help="the name of the C/C++ program.")
    parser.add_argument("time_budget", metavar="TIME-BUDGET", type=int, default=DEFAULT_BUDGET, help="how long the tester runs for")
    parser.add_argument("program", metavar="PROGRAM", nargs="+", type=str, help="the C/C++ program.")

    args = parser.parse_args()

    test_program : str = generate_tester(args.name, args.time_budget, " ".join(args.program))

    write_to_file(args.directory, args.name, test_program)