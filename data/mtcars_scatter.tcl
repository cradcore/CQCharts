set model [load_model -csv data/mtcars.csv -first_line_header]

set plot [create_plot -model $model -type scatter -columns {x=wt,y=mpg,color=("blue")} \
           -properties "color.map.enable=0" \
           -properties "symbol.fill.alpha=0.5"]
