from flask import Flask, request
import os
import sys
import subprocess

app = Flask(__name__)

SAVE_DIR = "/Users/Siddhant/Desktop/Github/Time-Complexity-Tester/cpp-file-cache"
DATA_DIR = "/Users/Siddhant/Desktop/Github/Time-Complexity-Tester/data"
OUTPUT_PATH = "/Users/Siddhant/Desktop/Github/Time-Complexity-Tester/output/output.txt"
BASH_FILE = "/Users/Siddhant/Desktop/Github/Time-Complexity-Tester/test_input"

# If we want to get, it will return all of the logs in the 
# data directory:
@app.route("/data", methods=['GET'])
def data():
    dirs = os.listdir(DATA_DIR)
    dirs.remove(".DS_Store")
    return {"saved-data":dirs}

# Test some code
@app.route("/test", methods=['POST'])
def test():
    if not request.is_json:
        return {"status":"ERROR", "message": "ERROR: Post type must be JSON."}
    else:
        data = request.get_json()
        if not all(item in data for item in ["name", "budget", "code"]):
            return {"status":"ERROR", 
            "message": ('POST request must contain '
                'fields for name, budget, and code')}
        else:
            # Variables:
            name : str = data["name"]
            budget : int = data["budget"]
            code : str = data["code"]
            fpath : str = SAVE_DIR + "/" + name + ".cpp"

            # String formatting:
            code = code.replace("\\n", "\n") # correctly format newline

            # Create the file if it does not exist
            if not os.path.exists(SAVE_DIR):
                os.mkdir(SAVE_DIR)

            if not os.path.exists(fpath):
                open(fpath, "x")
            
            # Write the given input into the file:
            f = open(fpath, "w")
            f.write(code)
            f.close()

            proc = subprocess.run([BASH_FILE, name, str(budget), fpath],
                capture_output=True)
            
            out_str : str = proc.stdout.decode('utf8')
            error_str : str = proc.stderr.decode('utf8')
            print("STD Out: ", out_str)
            print("STD Error: ", error_str)

            if proc.returncode != 0 or len(error_str) > 0:
                return {"status":"ERROR", "message":error_str} 

            f = open(OUTPUT_PATH, "r")
            output : str = f.read()

            return {"status":"SUCCESS", "message": output}



app.run()