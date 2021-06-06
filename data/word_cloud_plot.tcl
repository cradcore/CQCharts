# sankey link model
set model [load_charts_model -csv data/word_cloud.csv -first_line_header]

set plot [create_charts_plot -model $model -type wordCloud -columns {{value 0}}]
