#! /bin/bash

volume=$(wpctl get-volume @DEFAULT_AUDIO_SINK@)
echo "${volume#* }"
