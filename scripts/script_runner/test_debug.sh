#!/bin/bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/scripts/script_runner
export SCRIPT_RUNNER_LOG_LEVEL=DEBUG
export SCRIPT_RUNNER_LOG_FILE=debug.log
./build/bin/script_runner
