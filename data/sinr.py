# Graph SINR from ns-3 file using line graph
#
# Command line run: python sinr.py [simulation-name] [tcp_version]
# Example: python sinr.py 1b new-reno
# Example 2: python sinr.py 1d all 
# (all to put all tcp versions in the same graph)
#
# Author: Quan Chau, quananhchau@gmail.com
# Dickinson College 2020

from bokeh.plotting import figure, output_file, show
from bokeh.io import curdoc, show
from bokeh.models import ColumnDataSource, Grid, LinearAxis, Plot, Step, Line
from bokeh.models.annotations import Title
import sys

if len(sys.argv) < 3:
	print("Not enough arguments. Please read file instruction for the correct format.")

simulation = sys.argv[1].lower()
tcp_version = sys.argv[2].lower()

# The text file to read from must have the name "cw.txt". If using a different name, please change the line below
read_file_path = "./simulation-{}/[tcp_version]/sinr.txt".format(simulation)

# The output file contains the html version of the graph
if tcp_version == "all":
	output_file_path = "./simulation-{}/sinr.html".format(simulation)
else:
	output_file_path = "./simulation-{}/{}/sinr.html".format(simulation, tcp_version)

def get_sinr_arr(tcp_version_arg):
	file = open(read_file_path.replace("[tcp_version]", tcp_version_arg))
	arr = []
	while file:
		line = file.readline()
		if not line:
			break
		line = line.split(" ")
		arr.append((float)(line[3].replace("\n", "")))
	return arr

def get_sinr_time_arr(tcp_version_arg):
	file = open(read_file_path.replace("[tcp_version]", tcp_version_arg))
	arr = []
	while file:
		line = file.readline()
		if not line:
			break
		line = line.split(" ")
		arr.append(float(line[0].replace("\n", "")))
	file.close()
	return arr


##############
# PLOT GRAPH #
##############
output_file(output_file_path)
t = Title()

t.text = "Signal-to-Interference-plus-Noise Ratio (SINR)"

if tcp_version != "all":
	t.text += " - {}".format(tcp_version)

# Change the width and height as suitable and set x_range, y_range to see the data better
plot = figure(
    title=t, plot_width=700, plot_height=700, x_range=(0, 3))

# Typically, SINR is similar for all TCP versions - Look at throughput.py to make changes
# to generate all TCP versions if needed
source1 = ColumnDataSource(dict(x=get_sinr_time_arr("new-reno"), y=get_sinr_arr("new-reno")))
glyph1 = Step(x="x", y="y", line_color="#002953", line_width=2, mode="after")
plot.add_glyph(source1, glyph1)

# Set axis label
plot.xaxis.axis_label = 'Time (s)'
plot.yaxis.axis_label = 'SINR (dB)'
curdoc().add_root(plot)

show(plot)