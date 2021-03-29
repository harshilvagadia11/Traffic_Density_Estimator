import json
import subprocess
import pandas as pd
import plotly.express as px

def set_param(x,resolve,space_threads,time_threads,space_opt,print_data):
    data={
        "x": x,
        "resolve": resolve,
        "space_threads": space_threads,
        "time_threads": time_threads,
        "space_opt": space_opt,
        "print_data": print_data
    }
    with open("config.json", "w") as json_file:
        json.dump(data,json_file)


data = []
data_param*util=[]
data_param*time=[]
for i in [1,2,3,4,5]:
    set_param(5, i, 5, 1, False, False)
    process = subprocess.Popen(["./program", "trafficvideo.mp4", "empty.jpg"], stdout = subprocess.PIPE, universal_newlines = True)
    lines = process.stdout.readlines()
    lines = [line.rstrip("\n") for line in lines]
    lines = [float(line) for line in lines]
    data.append(lines)
    data_param*util.append([i,lines[0],lines[1]])                    # instead of i keep the param
    data_param*time.append([i,lines[2]])
    
    

def plot_param*util():
    df = pd.DataFrame(data_param*util, columns = ["Param", "Queue", "Dynamic"])
    print(df)
    # df = df.sort_values(by = "Runtime")
    fig = px.line(df, x = "x", y = ["Queue", "Dynamic"] , title = "Graph")
    fig.show()
    
def plot_param*time():
    df = pd.DataFrame(data_param*time, columns = ["Param", "Runtime"])
    print(df)
    # df = df.sort_values(by = "Runtime")
    fig = px.line(df, x = "Param", y = "Runtime" , title = "Graph")
    fig.show()
    




df = pd.DataFrame(data, columns = ["Queue", "Dynamic", "Runtime"])
print(df)
# df = df.sort_values(by = "Runtime")
fig = px.line(df, x = "Runtime", y = ["Queue", "Dynamic"], title = "Graph")
fig.show()
