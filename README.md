# Traffic_Density_Estimator

A traffic density estimator tool: Given a footage of a piece of road, the tool plots the static traffic density and moving traffic density as a function of time.

## Setup
Put the footage video inside the `code` folder. The code also requires a photo of the same piece of road without any traffic (for comparision) in the `code` folder. <br/>
Run `make` in the `code` folder to complile the program.

## Parameters
Edit the parameters in `config.json` : <br/>
1. `x`: Process only every xth frame in the video
1. `resolve`: Decrease the resolution of each fram by the factor resolve
1. `space_opt`: Keep `true` for using multithreading on each frame and `false` for using multithreading across entire video
1. `space_threads` and `time_threads`: Number of threads to use in each method
1. `print_data`: Keep `true` to print the output in a csv format and `false` to print the comparision with `baseline.csv`

## Execution
Run the program as `./program <video_file> <image_file>`. The execution can take over a minute depending on the parameters used.

## Experimentation
The detailed analysis performed by us on various methods can be found in the `analysis` folder. `script.py` runs the program on a benchmark set of parameters.
`experiments` folder shows the graphs we obtained illustrating the difference between various methods.
