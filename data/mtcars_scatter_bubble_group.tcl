set model [load_model -csv data/mtcars.csv -first_line_header]

#set plot [create_plot -model $model -type scatter -columns "x=wt,y=mpg,group=cyl,symbolSize=qsec"]

set plot [create_plot -model $model -type scatter -columns "x=wt,y=mpg,group=cyl,color=mpg"]
