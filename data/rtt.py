# Graph Round Trip Time (RTT) from ns-3 file using scratter plot
#
# Command line run: python rtt.py [simulation-name] [tcp_version]
# Example: python rtt.py 1b new-reno
# Example 2: python rtt.py 1d all 
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
read_file_path = "./simulation-{}/[tcp_version]/rtt.txt".format(simulation)

# The output file contains the html version of the graph
if tcp_version == "all":
	output_file_path = "./simulation-{}/rtt.html".format(simulation)
else:
	output_file_path = "./simulation-{}/{}/rtt.html".format(simulation, tcp_version)

def get_rtt_arr(tcp_version_arg):
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
		if prev == 0:
			prev = float(line[0])
			num = float(line[2])
			count = 1
			continue

		cur = float(line[0])
		if cur == prev:
			count += 1
			num += float(line[2])
		else:
			arr.append(str(round(num/count, 5)))
			prev = cur
			count = 1
			num = float(line[2])
	arr.append(round(num/count, 5))
	file.close()
	return arr

def get_rtt_time_arr(tcp_version_arg):
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
		if prev == 0:
			prev = float(line[0])
			num = float(line[2])
			count = 1
			continue

		cur = float(line[0])
		if cur == prev:
			count += 1
			num += float(line[2])
		else:
			arr.append(prev)

			prev = cur
			count = 1
			num = float(line[2])
	arr.append(prev)
	file.close()
	return arr

##############
# PLOT GRAPH #
##############
output_file(output_file_path)
t = Title()

t.text = "Round Trip Time (RTT)"

if tcp_version != "all":
	t.text += " - {}".format(tcp_version)

# Change the width and height as suitable and set x_range, y_range to see the data better
plot = figure(
    title=t, plot_width=700, plot_height=400,  x_range=(0, 16), y_range=(0, 0.4), output_backend="webgl")

if tcp_version == "all":
	source1 = ColumnDataSource(dict(x=get_rtt_time_arr("new-reno"), y=get_rtt_arr("new-reno")))
	plot.circle(x='x', y='y', source=source1, color="#002953")

	source2 = ColumnDataSource(dict(x=get_rtt_time_arr("yeah"), y=get_rtt_arr("yeah")))
	plot.circle(x='x', y='y', source=source2, color="#c7f464")

	source3 = ColumnDataSource(dict(x=get_rtt_time_arr("hybla"), y=get_rtt_arr("hybla")))
	plot.circle(x='x', y='y', source=source3, color="#ff6b6b")

	source4 = ColumnDataSource(dict(x=get_rtt_time_arr("westwood"), y=get_rtt_arr("westwood")))
	plot.circle(x='x', y='y', source=source4, color="#95d4f3")

	source5 = ColumnDataSource(dict(x=get_rtt_time_arr("vegas"), y=get_rtt_arr("vegas")))
	plot.circle(x='x', y='y', source=source5, color="#396AB1")
else:
	source1 = ColumnDataSource(dict(x=get_rtt_time_arr(tcp_version), y=get_rtt_arr(tcp_version)))
	plot.circle(x='x', y='y', source=source1, color="#002953")

# Set axis label
plot.xaxis.axis_label = 'Time (s)'
plot.yaxis.axis_label = 'RTT (s)'
curdoc().add_root(plot)

show(plot)