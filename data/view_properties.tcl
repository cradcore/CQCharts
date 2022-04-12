proc columnSelect { c } {
  qt_set_property -object $::globalProperties -property column -value $c
}

proc annotationSlot { viewId plotId id } {
  puts "annotationSlot: $viewId, $plotId, $id"

  set pos [get_charts_property -annotation $id -name position]
  puts "pos: $pos"

  set widget [get_charts_property -annotation $id -name widget]
  puts "widget: $widget"
}

#---

# load model
set model [load_charts_model -csv data/chord-cities.csv -comment_header]

# create empty plot
set plot [create_charts_plot -model $model -type empty]

#---

set modelView [qt_create_widget -type CQChartsModelViewHolder -name modelView]

qt_connect_widget -name $modelView -signal "columnClicked(int)" -proc columnSelect

set modelViewAnn [create_charts_widget_annotation -plot $plot -id modelView \
  -rectangle [list 1 1 99 49 V] -widget $modelView]

set modelView [get_charts_data -annotation $modelViewAnn -name widget_path]

#---

#set viewProperties [qt_create_widget -type CQChartsViewPropertiesWidget -name viewProperties]
set viewProperties [qt_create_widget -type CQChartsViewPropertiesControl -name viewProperties]

set viewPropertiesAnn [create_charts_widget_annotation -plot $plot -id viewProperties \
  -rectangle [list 1 50 99 99 V] -widget $viewProperties]

set viewProperties [get_charts_data -annotation $viewProperties -name widget_path]

#---

connect_charts_signal -plot $plot -from annotationIdPressed -to annotationSlot
