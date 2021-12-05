proc annotationSlot { view plot id } {
  puts "annotationSlot: $view, $plot, $id"
}

set plot [create_charts_plot -type empty -xmin -100 -ymin -100 -xmax 100 -ymax 100]

set rect1 [create_charts_rectangle_annotation -plot $plot -id rect1 -tip "Rectangle 1" \
  -start {-50 -50} -end {50 50} -filled 1 -fill_color red -fill_alpha 0.5]

set_charts_property -plot $plot -name dataBox.fill.visible -value 0

#set_charts_property -plot $plot -name plotBox.fill.visible -value 1
#set_charts_property -plot $plot -name plotBox.fill.pattern -value "IMAGE,name=image:/home/colinw/dev/progs/charts/data/CQCharts/data/aerial_view.jpg"

set_charts_property -annotation $rect1 -name fill.alpha -value 0.2

connect_charts_signal -plot $plot -from annotationIdPressed -to annotationSlot
