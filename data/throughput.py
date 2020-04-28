# Graph Throughtput from ns-3 file using line graph
#
# Command line run: python throughput.py [simulation-name] [tcp_version]
# Example: python throughput.py 1b new-reno
# Example 2: python throughput.py 1d all 
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
read_file_path = "./simulation-{}/[tcp_version]/throughput.txt".format(simulation)

# The output file contains the html version of the graph
if tcp_version == "all":
	output_file_path = "./simulation-{}/throughput.html".format(simulation)
else:
	output_file_path = "./simulation-{}/{}/throughput.html".format(simulation, tcp_version)

def test_float(inp):
	try :  
	    float(inp) 
	    res = True
	except : 
	    res = False
	return res


def get_throughput_arr(tcp_version_arg):
	file = open(read_file_path.replace("[tcp_version]", tcp_version_arg))
	arr = []
	while file:
		line = file.readline()
		if not line:
			break
		if line and not "*" in line and line[0] >= '0' and line[0] <= '9':
			num = line.find(":")
			line = line[num + 1:]
			line = line.replace(" ", "")
			line = line.replace("Mbit/s", "")
			line = line.replace("\n", "")
			line = line.replace("\t", "")
			if test_float(line):
				arr.append(float(line))
			
	return arr

def get_throughput_time_arr(tcp_version_arg):
	file = open(read_file_path.replace("[tcp_version]", tcp_version_arg))
	arr = []
	while file:
		line = file.readline()
		if not line:
			break
		if line and not "*" in line and line[0] >= '0' and line[0] <= '9':
			num = line.find(":")
			line = line[0:num]
			line = line.replace("s", "")
			line = line.replace(":", "")
			
			if test_float(line):
				arr.append(float(line))
			if not line:
				break
	return arr

##############
# PLOT GRAPH #
##############
output_file(output_file_path)
t = Title()

t.text = "Throughput"

if tcp_version != "all":
	t.text += " - {}".format(tcp_version)

# Change the width and height as suitable and set x_range, y_range to see the data better
plot = figure(
    title=t, plot_width=700, plot_height=400, x_range=(0, 16), y_range=(0, 1500))

if tcp_version == "all":
	source1 = ColumnDataSource(dict(x=get_throughput_time_arr("new-reno"), y=get_throughput_arr("new-reno")))
	glyph1 = Line(x="x", y="y", line_color="#002953", line_width=2)
	plot.add_glyph(source1, glyph1)

	source2 = ColumnDataSource(dict(x=get_throughput_time_arr("yeah"), y=get_throughput_arr("yeah")))
	glyph2 = Line(x="x", y="y", line_color="#c7f464",  line_width=2)
	plot.add_glyph(source2, glyph2)

	source3 = ColumnDataSource(dict(x=get_throughput_time_arr("hybla"), y=get_throughput_arr("hybla")))
	glyph3 = Line(x="x", y="y", line_color="#ff6b6b", line_width=2)
	plot.add_glyph(source3, glyph3)

	source4 = ColumnDataSource(dict(x=get_throughput_time_arr("westwood"), y=get_throughput_arr("westwood")))
	glyph4 = Line(x="x", y="y", line_color="#95d4f3", line_width=2)
	plot.add_glyph(source4, glyph4)

	source5 = ColumnDataSource(dict(x=get_throughput_time_arr("vegas"), y=get_throughput_arr("vegas")))
	glyph5 = Line(x="x", y="y", line_color="#396AB1", line_width=2)
	plot.add_glyph(source5, glyph5)
else:
	source1 = ColumnDataSource(dict(x=get_throughput_time_arr(tcp_version), y=get_throughput_arr(tcp_version)))
	glyph1 = Line(x="x", y="y", line_color="#002953", line_width=2)
	plot.add_glyph(source1, glyph1)

# Set axis label
plot.xaxis.axis_label = 'Time (s)'
plot.yaxis.axis_label = 'Throughput (Mbps)'
curdoc().add_root(plot)

show(plot)