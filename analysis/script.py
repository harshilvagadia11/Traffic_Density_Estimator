import json
import subprocess
import pandas as pd
import plotly.express as px

def set_param(x, resolve, space_threads, time_threads, space_opt, print_data):
    data = {
        "x": x,
        "resolve": resolve,
        "space_threads": space_threads,
        "time_threads": time_threads,
        "space_opt": space_opt,
        "print_data": print_data
    }
    with open("../code/config.json", "w") as json_file:
        json.dump(data,json_file)

data = []
data_param_error=[]
data_param_time=[]
for i in [1,2,3,4,5]:
    print("Processing i = " + str(i))
    set_param(5, 1, 1, i, False, False)
    process = subprocess.Popen(["./program", "trafficvideo.mp4", "empty.jpg"], stdout = subprocess.PIPE, universal_newlines = True, cwd = "../code/")
    lines = process.stdout.readlines()
    lines = [line.rstrip("\n") for line in lines]
    lines = [float(line) for line in lines]
    data.append(lines)
    data_param_error.append([i,lines[0],lines[1]])
    data_param_time.append([i,lines[2]])
    
    

def plot_param_error():
    df = pd.DataFrame(data_param_error, columns = ["Param", "Queue", "Dynamic"])
    print(df)
    # df = df.sort_values(by = "Runtime")
    fig = px.line(df, x = "Param", y = ["Queue", "Dynamic"] , title = "#3 param_error")
    fig.show()
    
def plot_param_time():
    df = pd.DataFrame(data_param_time, columns = ["Param", "Runtime"])
    print(df)
    # df = df.sort_values(by = "Runtime")
    fig = px.line(df, x = "Param", y = "Runtime" , title = "#3 param_time")
    fig.show()

def plot_error_time():
    df = pd.DataFrame(data, columns = ["Queue", "Dynamic", "Runtime"])
    df = df.sort_values(by = "Runtime")
    print(df)
    fig = px.line(df, x = "Runtime", y = ["Queue", "Dynamic"], title = "#3 error_time")
    fig.show()

plot_error_time()
plot_param_time()
plot_param_error()