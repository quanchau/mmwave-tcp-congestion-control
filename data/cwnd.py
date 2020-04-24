# Graph congestion window size from ns-3 file (step graph)
#
# Command line run: python cwnd.py [simulation-name] [tcp_version]
# Example: python cwnd.py 1b new-reno
# Example 2: python cwnd.py 1d all 
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
read_file_path = "./simulation-{}/[tcp_version]/cw.txt".format(simulation)

# The output file contains the html version of the graph
if tcp_version == "all":
	output_file_path = "./simulation-{}/cwnd.html".format(simulation)
else:
	output_file_path = "./simulation-{}/{}/cwnd.html".format(simulation, tcp_version)

def get_cw_arr(tcp_version_arg):
	file = open(read_file_path.replace("[tcp_version]", tcp_version_arg))
	prev = 0
	count = 0
	num = 0.0
	arr = []
	while file:
		line = file.readline()
		if not line:
			break
		line = line.split("\t")
		value = float(line[2])
		if tcp_version_arg == "yeah" and value > (429496651):
			value = 0
		arr.append(value)
	file.close()
	return arr

def get_cw_time_arr(tcp_version_arg):
	file = open(read_file_path.replace("[tcp_version]", tcp_version_arg))
	prev = 0
	count = 0
	num = 0.0
	arr = []
	while file:
		line = file.readline()
		if not line:
			break
		line = line.split("\t")
		arr.append(line[0])
	file.close()
	return arr


##############
# PLOT GRAPH #
##############
output_file(output_file_path)
t = Title()

t.text = "Congestion Window Size (cwnd)"

if tcp_version != "all":
	t.text += " - {}".format(tcp_version)

# Change the width and height as suitable and set x_range, y_range to see the data better
plot = figure(
    title=t, plot_width=700, plot_height=200, x_range=(0, 20))

if tcp_version == "all":
	source1 = ColumnDataSource(dict(x=get_cw_time_arr("new-reno"), y=get_cw_arr("new-reno")))
	glyph1 = Step(x="x", y="y", line_color="#002953", mode="after", line_width=2)
	plot.add_glyph(source1, glyph1)

	source2 = ColumnDataSource(dict(x=get_cw_time_arr("yeah"), y=get_cw_arr("yeah")))
	glyph2 = Step(x="x", y="y", line_color="#c7f464", mode="after", line_width=2)
	plot.add_glyph(source2, glyph2)

	source3 = ColumnDataSource(dict(x=get_cw_time_arr("hybla"), y=get_cw_arr("hybla")))
	glyph3 = Step(x="x", y="y", line_color="#ff6b6b", mode="after", line_width=2)
	plot.add_glyph(source3, glyph3)

	source4 = ColumnDataSource(dict(x=get_cw_time_arr("westwood"), y=get_cw_arr("westwood")))
	glyph4 = Step(x="x", y="y", line_color="#95d4f3", mode="after", line_width=2)
	plot.add_glyph(source4, glyph4)

	source5 = ColumnDataSource(dict(x=get_cw_time_arr("vegas"), y=get_cw_arr("vegas")))
	glyph5 = Step(x="x", y="y", line_color="#396AB1", mode="after", line_width=2)
	plot.add_glyph(source5, glyph5)
else:
	source1 = ColumnDataSource(dict(x=get_cw_time_arr(tcp_version), y=get_cw_arr(tcp_version)))
	glyph1 = Step(x="x", y="y", line_color="#002953", mode="after", line_width=2)
	plot.add_glyph(source1, glyph1)

# Set axis label
plot.xaxis.axis_label = 'Time (s)'
plot.yaxis.axis_label = 'cwnd (bytes)'
curdoc().add_root(plot)

show(plot)