#!/bin/bash

current_path=`pwd`
current_time=`date +"%Y-%m-%d_%T"`
compressed_dashboards_name="dashboards.tar.gz"
compressed_datasources_name="datasources.tar.gz"

echo $current_time

dashboard_backup_folder="/media/usb/grafana/dashboards/$current_time"
datasource_backup_folder="/media/usb/grafana/datasources/$current_time"

if [ ! -d "$dashboard_backup_folder" ]; then 
  mkdir -p "$dashboard_backup_folder" 
fi

if [ ! -d "$datasource_backup_folder" ]; then 
  mkdir -p "$datasource_backup_folder" 
fi

python "${current_path}/saveDashboards.py" $dashboard_backup_folder
python "${current_path}/saveDatasources.py" $datasource_backup_folder

tar -zcvf "/media/usb/grafana/$compressed_dashboards_name" $dashboard_backup_folder
tar -zcvf "/media/usb/grafana/$compressed_datasources_name" $datasource_backup_folder
