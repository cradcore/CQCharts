# Sankey From/To
set model [load_charts_model -csv data/dot_fsm.csv -first_line_header]
#set model [load_charts_model -csv data/dot_siblings.csv -first_line_header]

set plot [create_charts_plot -model $model -type sankey \
  -columns {{from 0} {to 1} {attributes 2}} -title "sankey from/to"]

set_charts_property -plot $plot -name node.scaled -value 0
set_charts_property -plot $plot -name edge.scaled -value 0
