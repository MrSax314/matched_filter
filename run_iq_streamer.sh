#!/usr/bin/env bash
# Runs the LFM IQ streamer. Optional arg: path to a config .ini
# (defaults to the radar_sim.ini copied next to the executable at build time).
./build/libraries/lfm_iq_streamer/lfm_iq_streamer "$@"
