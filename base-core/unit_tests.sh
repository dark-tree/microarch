#!/bin/bash


for arg in "$@"; do
  MODULE_NAME=$(echo $arg | sed 's:.*/::' | sed 's:\..*::')
  TEST_FILE="testbench/unit_tests/tests/sc_${MODULE_NAME}.cpp"
  if [ -f $TEST_FILE ]; then
    echo "Running tests for module ${MODULE_NAME}..."
    verilator -sc -exe --trace-fst $@ --top $MODULE_NAME $TEST_FILE > /dev/null
    make -j -C obj_dir -f "V${MODULE_NAME}.mk" > /dev/null
    "obj_dir/V${MODULE_NAME}" +trace
  fi
done
