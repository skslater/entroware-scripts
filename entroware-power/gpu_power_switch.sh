#!/bin/bash

LOG_FILE=/dev/kmsg


remove_devices(){
  for_removal=()
  for gpu_vendor in "10de"; do
    for gpu_class in "300" "302"; do
      for gpu_device in `lspci -D -d "${gpu_vendor}::${gpu_class}" | awk '{print $1}' | cut -d. -f1`; do
        for pci_dev in `lspci -D -s ${gpu_device} | awk '{print $1}'`; do
          for_removal+=(`echo ${pci_dev} | sed 's/:/\\:/g'`)
        done
      done
    done
  done

  for i in ${for_removal[*]}; do
    echo "1" > /sys/bus/pci/devices/${i}/remove
    echo "entroware-power: Removed device: `echo ${i} | sed 's/\\\//g'`" >> ${LOG_FILE}
  done
}

gpu_on(){
  echo "\_SB.PCI0.PEG0.PEGP._ON" > /proc/acpi/call
  echo "entroware-power: dGPU Power: ON" >> ${LOG_FILE}
}

gpu_off(){
  echo "\_SB.PCI0.PEG0.PEGP._OFF" > /proc/acpi/call
  echo "entroware-power: dGPU Power: OFF" >> ${LOG_FILE}
}


if [ `prime-select query` == "intel" ]; then
  if [ "${1}" == "on" ]; then
    gpu_on
  elif [ "${1}" == "off" ]; then
    gpu_off
  elif [ "${1}" == "remove" ]; then
    remove_devices
    gpu_off
  fi
fi
